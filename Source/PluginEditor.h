#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SakuraLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SakuraLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
};

class SakuraFontAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit SakuraFontAudioProcessorEditor(SakuraFontAudioProcessor&);
    ~SakuraFontAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadClicked();
    void updatePreset();

    SakuraFontAudioProcessor& processor;
    SakuraLookAndFeel look;
    juce::TextButton loadButton { "LOAD .SF2" }, panicButton { "PANIC" };
    juce::Label title, subtitle, fileLabel, bankLabel, programLabel;
    juce::Slider gain, reverb, chorus;
    juce::Slider bank, program;
    juce::Label gainLabel, reverbLabel, chorusLabel;
    std::unique_ptr<juce::FileChooser> chooser;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> gainA, reverbA, chorusA, bankA, programA;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SakuraFontAudioProcessorEditor)
};
