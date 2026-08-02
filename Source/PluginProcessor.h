#pragma once

#include <JuceHeader.h>
#include <fluidsynth.h>
#include <mutex>

class SakuraFontAudioProcessor final : public juce::AudioProcessor
{
public:
    SakuraFontAudioProcessor();
    ~SakuraFontAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    bool loadSoundFont(const juce::File& file);
    void selectPreset(int bank, int program);
    void panic();
    juce::String getLoadedFileName() const;
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    juce::AudioProcessorValueTreeState state;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void createSynth(double sampleRate);
    void destroySynth();
    void applyParameters();
    void renderRange(juce::AudioBuffer<float>& buffer, int start, int count);
    void handleMidi(const juce::MidiMessage& message);

    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;
    int soundFontId = -1;
    juce::File loadedFile;
    double currentSampleRate = 44100.0;
    juce::MidiKeyboardState keyboardState;
    mutable std::mutex synthMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SakuraFontAudioProcessor)
};
