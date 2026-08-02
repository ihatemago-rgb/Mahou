#include "PluginProcessor.h"
#include "PluginEditor.h"

SakuraFontAudioProcessor::SakuraFontAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "STATE", createParameters())
{
}

SakuraFontAudioProcessor::~SakuraFontAudioProcessor() { destroySynth(); }

juce::AudioProcessorValueTreeState::ParameterLayout SakuraFontAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.5f, 0.8f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverb", "Reverb", 0.0f, 1.0f, 0.25f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("chorus", "Chorus", 0.0f, 1.0f, 0.15f));
    p.push_back(std::make_unique<juce::AudioParameterInt>("bank", "Bank", 0, 16383, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("program", "Program", 0, 127, 0));
    return { p.begin(), p.end() };
}

void SakuraFontAudioProcessor::createSynth(double sampleRate)
{
    destroySynth();
    settings = new_fluid_settings();
    fluid_settings_setnum(settings, "synth.sample-rate", sampleRate);
    fluid_settings_setint(settings, "synth.threadsafe-api", 0);
    fluid_settings_setint(settings, "synth.reverb.active", 1);
    fluid_settings_setint(settings, "synth.chorus.active", 1);
    synth = new_fluid_synth(settings);
}

void SakuraFontAudioProcessor::destroySynth()
{
    std::scoped_lock lock(synthMutex);
    if (synth != nullptr) { delete_fluid_synth(synth); synth = nullptr; }
    if (settings != nullptr) { delete_fluid_settings(settings); settings = nullptr; }
    soundFontId = -1;
}

void SakuraFontAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    createSynth(sampleRate);
    if (loadedFile.existsAsFile()) loadSoundFont(loadedFile);
}

void SakuraFontAudioProcessor::releaseResources() {}

bool SakuraFontAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SakuraFontAudioProcessor::applyParameters()
{
    if (synth == nullptr) return;
    const auto gain = state.getRawParameterValue("gain")->load();
    const auto reverb = state.getRawParameterValue("reverb")->load();
    const auto chorus = state.getRawParameterValue("chorus")->load();
    fluid_synth_set_gain(synth, gain);
    fluid_synth_set_reverb_group_roomsize(synth, -1, 0.15 + 0.75 * reverb);
    fluid_synth_set_reverb_group_level(synth, -1, reverb);
    fluid_synth_set_chorus_group_level(synth, -1, chorus * 3.0);
}

void SakuraFontAudioProcessor::renderRange(juce::AudioBuffer<float>& buffer, int start, int count)
{
    if (synth == nullptr || count <= 0) return;
    fluid_synth_write_float(synth, count,
                            buffer.getWritePointer(0, start), 0, 1,
                            buffer.getWritePointer(1, start), 0, 1);
}

void SakuraFontAudioProcessor::handleMidi(const juce::MidiMessage& m)
{
    if (synth == nullptr) return;
    const int ch = juce::jlimit(0, 15, m.getChannel() - 1);
    if (m.isNoteOn()) fluid_synth_noteon(synth, ch, m.getNoteNumber(), (int)m.getVelocity());
    else if (m.isNoteOff()) fluid_synth_noteoff(synth, ch, m.getNoteNumber());
    else if (m.isController()) fluid_synth_cc(synth, ch, m.getControllerNumber(), m.getControllerValue());
    else if (m.isPitchWheel()) fluid_synth_pitch_bend(synth, ch, m.getPitchWheelValue());
    else if (m.isProgramChange()) fluid_synth_program_change(synth, ch, m.getProgramChangeNumber());
    else if (m.isAftertouch()) fluid_synth_key_pressure(synth, ch, m.getNoteNumber(), m.getAfterTouchValue());
    else if (m.isChannelPressure()) fluid_synth_channel_pressure(synth, ch, m.getChannelPressureValue());
    else if (m.isAllNotesOff() || m.isAllSoundOff()) fluid_synth_all_notes_off(synth, ch);
}

void SakuraFontAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    std::scoped_lock lock(synthMutex);
    if (synth == nullptr) return;
    applyParameters();

    int cursor = 0;
    for (const auto metadata : midi)
    {
        const int pos = juce::jlimit(0, buffer.getNumSamples(), metadata.samplePosition);
        renderRange(buffer, cursor, pos - cursor);
        handleMidi(metadata.getMessage());
        cursor = pos;
    }
    renderRange(buffer, cursor, buffer.getNumSamples() - cursor);
}

bool SakuraFontAudioProcessor::loadSoundFont(const juce::File& file)
{
    if (!file.existsAsFile()) return false;
    if (synth == nullptr) createSynth(currentSampleRate);

    std::scoped_lock lock(synthMutex);
    if (synth == nullptr) return false;
    if (soundFontId >= 0) fluid_synth_sfunload(synth, soundFontId, 1);
    soundFontId = fluid_synth_sfload(synth, file.getFullPathName().toRawUTF8(), 1);
    if (soundFontId < 0) return false;
    loadedFile = file;
    fluid_synth_program_select(synth, 0, soundFontId,
                               (int)state.getRawParameterValue("bank")->load(),
                               (int)state.getRawParameterValue("program")->load());
    return true;
}

void SakuraFontAudioProcessor::selectPreset(int bank, int program)
{
    std::scoped_lock lock(synthMutex);
    if (synth == nullptr || soundFontId < 0) return;
    fluid_synth_program_select(synth, 0, soundFontId, bank, program);
}

void SakuraFontAudioProcessor::panic()
{
    std::scoped_lock lock(synthMutex);
    if (synth != nullptr) fluid_synth_system_reset(synth);
}

juce::String SakuraFontAudioProcessor::getLoadedFileName() const
{
    return loadedFile.existsAsFile() ? loadedFile.getFileName() : "No SoundFont loaded";
}

void SakuraFontAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto tree = state.copyState();
    tree.setProperty("sf2Path", loadedFile.getFullPathName(), nullptr);
    if (auto xml = tree.createXml()) copyXmlToBinary(*xml, dest);
}

void SakuraFontAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid())
        {
            state.replaceState(tree);
            auto path = tree.getProperty("sf2Path").toString();
            if (path.isNotEmpty()) loadSoundFont(juce::File(path));
        }
    }
}

juce::AudioProcessorEditor* SakuraFontAudioProcessor::createEditor()
{
    return new SakuraFontAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SakuraFontAudioProcessor();
}
