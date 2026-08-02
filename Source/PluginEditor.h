#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class SakuraLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SakuraLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
};

class SakuraFontAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer,
                                              public juce::FileDragAndDropTarget
{
public:
    explicit SakuraFontAudioProcessorEditor(SakuraFontAudioProcessor&);
    ~SakuraFontAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    struct Petal
    {
        float x = 0.0f;
        float y = 0.0f;
        float speed = 1.0f;
        float phase = 0.0f;
        float size = 5.0f;
    };

    void timerCallback() override;
    void loadClicked();
    void loadFile(const juce::File&);
    void updatePreset();
    void configureKnob(juce::Slider&, juce::Label&, const juce::String&);

    SakuraFontAudioProcessor& processor;
    SakuraLookAndFeel look;

    juce::TextButton loadButton { "LOAD .SF2" };
    juce::TextButton panicButton { "PANIC" };
    juce::Label title, subtitle, fileLabel, dropHint;
    juce::Slider gain, attack, decay, sustain, release, reverb, chorus;
    juce::Label gainLabel, attackLabel, decayLabel, sustainLabel, releaseLabel, reverbLabel, chorusLabel;
    juce::ComboBox bankBox, programBox;
    juce::Label bankLabel, programLabel;
    juce::MidiKeyboardComponent keyboard;
    std::unique_ptr<juce::FileChooser> chooser;
    std::array<Petal, 34> petals;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> gainA, attackA, decayA, sustainA, releaseA, reverbA, chorusA;
    std::unique_ptr<ComboAttachment> bankA, programA;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SakuraFontAudioProcessorEditor)
};
