#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
float secondsToTimecents(float seconds)
{
    return juce::jlimit(-12000.0f, 8000.0f, 1200.0f * std::log2(juce::jmax(0.001f, seconds)));
}

float sustainToCentibels(float level)
{
    return juce::jlimit(0.0f, 1440.0f, -200.0f * std::log10(juce::jmax(0.0001f, level)));
}
}

SakuraFontAudioProcessor::SakuraFontAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "STATE", createParameters())
{
}

SakuraFontAudioProcessor::~SakuraFontAudioProcessor()
{
    destroySynth();
}

juce::AudioProcessorValueTreeState::ParameterLayout SakuraFontAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Volume", 0.0f, 1.5f, 0.8f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.001f, 8.0f, 0.001f, 0.35f), 0.01f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.01f, 8.0f, 0.001f, 0.35f), 0.35f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", 0.0f, 1.0f, 0.85f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.01f, 12.0f, 0.001f, 0.35f), 0.8f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("reverb", "Reverb", 0.0f, 1.0f, 0.25f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("chorus", "Chorus", 0.0f, 1.0f, 0.15f));
    p.push_back(std::make_unique<juce::AudioParameterInt>("bank", "Bank", 0, 128, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("program", "Program", 0, 127, 0));
    return { p.begin(), p.end() };
}

void SakuraFontAudioProcessor::createSynth(double sampleRate)
{
    destroySynth();
    std::scoped_lock lock(synthMutex);
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
    if (synth != nullptr)
    {
        delete_fluid_synth(synth);
        synth = nullptr;
    }
    if (settings != nullptr)
    {
        delete_fluid_settings(settings);
        settings = nullptr;
    }
    soundFontId = -1;
}

void SakuraFontAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    createSynth(sampleRate);
    if (loadedFile.existsAsFile())
        loadSoundFont(loadedFile);
}

void SakuraFontAudioProcessor::releaseResources() {}

bool SakuraFontAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SakuraFontAudioProcessor::applyParameters()
{
    if (synth == nullptr)
        return;

    const auto gain = state.getRawParameterValue("gain")->load();
    const auto attack = state.getRawParameterValue("attack")->load();
    const auto decay = state.getRawParameterValue("decay")->load();
    const auto sustain = state.getRawParameterValue("sustain")->load();
    const auto release = state.getRawParameterValue("release")->load();
    const auto reverb = state.getRawParameterValue("reverb")->load();
    const auto chorus = state.getRawParameterValue("chorus")->load();

    fluid_synth_set_gain(synth, gain);
    fluid_synth_set_reverb_group_roomsize(synth, -1, 0.15 + 0.75 * reverb);
    fluid_synth_set_reverb_group_level(synth, -1, reverb);
    fluid_synth_set_chorus_group_level(synth, -1, chorus * 3.0);

    fluid_synth_set_gen(synth, 0, GEN_VOLENVATTACK, secondsToTimecents(attack));
    fluid_synth_set_gen(synth, 0, GEN_VOLENVDECAY, secondsToTimecents(decay));
    fluid_synth_set_gen(synth, 0, GEN_VOLENVSUSTAIN, sustainToCentibels(sustain));
    fluid_synth_set_gen(synth, 0, GEN_VOLENVRELEASE, secondsToTimecents(release));
}

void SakuraFontAudioProcessor::renderRange(juce::AudioBuffer<float>& buffer, int start, int count)
{
    if (synth == nullptr || count <= 0)
        return;

    fluid_synth_write_float(synth, count,
                            buffer.getWritePointer(0, start), 0, 1,
                            buffer.getWritePointer(1, start), 0, 1);
}

void SakuraFontAudioProcessor::handleMidi(const juce::MidiMessage& m)
{
    if (synth == nullptr)
        return;

    const int channel = juce::jlimit(0, 15, m.getChannel() - 1);
    if (m.isNoteOn())
        fluid_synth_noteon(synth, channel, m.getNoteNumber(), juce::roundToInt(m.getFloatVelocity() * 127.0f));
    else if (m.isNoteOff())
        fluid_synth_noteoff(synth, channel, m.getNoteNumber());
    else if (m.isController())
        fluid_synth_cc(synth, channel, m.getControllerNumber(), m.getControllerValue());
    else if (m.isPitchWheel())
        fluid_synth_pitch_bend(synth, channel, m.getPitchWheelValue());
    else if (m.isProgramChange())
        fluid_synth_program_change(synth, channel, m.getProgramChangeNumber());
    else if (m.isAftertouch())
        fluid_synth_key_pressure(synth, channel, m.getNoteNumber(), m.getAfterTouchValue());
    else if (m.isChannelPressure())
        fluid_synth_channel_pressure(synth, channel, m.getChannelPressureValue());
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        fluid_synth_all_notes_off(synth, channel);
}

void SakuraFontAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    keyboardState.processNextMidiBuffer(midi, 0, buffer.getNumSamples(), true);

    std::scoped_lock lock(synthMutex);
    if (synth == nullptr)
        return;

    applyParameters();
    int cursor = 0;
    for (const auto metadata : midi)
    {
        const int position = juce::jlimit(0, buffer.getNumSamples(), metadata.samplePosition);
        renderRange(buffer, cursor, position - cursor);
        handleMidi(metadata.getMessage());
        cursor = position;
    }
    renderRange(buffer, cursor, buffer.getNumSamples() - cursor);
}

bool SakuraFontAudioProcessor::loadSoundFont(const juce::File& file)
{
    if (!file.existsAsFile() || file.getFileExtension().toLowerCase() != ".sf2")
        return false;

    if (synth == nullptr)
        createSynth(currentSampleRate);

    std::scoped_lock lock(synthMutex);
    if (synth == nullptr)
        return false;

    if (soundFontId >= 0)
        fluid_synth_sfunload(synth, soundFontId, 1);

    const int newId = fluid_synth_sfload(synth, file.getFullPathName().toRawUTF8(), 1);
    if (newId < 0)
        return false;

    soundFontId = newId;
    loadedFile = file;
    fluid_synth_program_select(synth, 0, soundFontId,
                               static_cast<int>(state.getRawParameterValue("bank")->load()),
                               static_cast<int>(state.getRawParameterValue("program")->load()));
    return true;
}

void SakuraFontAudioProcessor::selectPreset(int bank, int program)
{
    std::scoped_lock lock(synthMutex);
    if (synth != nullptr && soundFontId >= 0)
        fluid_synth_program_select(synth, 0, soundFontId, bank, program);
}

void SakuraFontAudioProcessor::panic()
{
    std::scoped_lock lock(synthMutex);
    if (synth != nullptr)
        fluid_synth_system_reset(synth);
    keyboardState.reset();
}

juce::String SakuraFontAudioProcessor::getLoadedFileName() const
{
    return loadedFile.existsAsFile() ? loadedFile.getFileName() : "Drop an .sf2 here or click LOAD";
}

void SakuraFontAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto tree = state.copyState();
    tree.setProperty("sf2Path", loadedFile.getFullPathName(), nullptr);
    if (auto xml = tree.createXml())
        copyXmlToBinary(*xml, destination);
}

void SakuraFontAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
    {
        auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid())
        {
            state.replaceState(tree);
            const auto path = tree.getProperty("sf2Path").toString();
            if (path.isNotEmpty())
                loadSoundFont(juce::File(path));
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
