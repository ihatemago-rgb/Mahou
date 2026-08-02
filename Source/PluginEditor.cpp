#include "PluginEditor.h"

SakuraLookAndFeel::SakuraLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff7efff));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x33181232));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void SakuraLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float pos, float start, float end, juce::Slider&)
{
    auto area = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(8.0f);
    auto r = juce::jmin(area.getWidth(), area.getHeight()) * 0.40f;
    auto c = area.getCentre();
    g.setColour(juce::Colour(0xff25173f)); g.fillEllipse(c.x-r, c.y-r, r*2, r*2);
    g.setColour(juce::Colour(0xfff08ad4)); g.drawEllipse(c.x-r, c.y-r, r*2, r*2, 2.5f);
    juce::Path p; p.addRoundedRectangle(-2.0f, -r+8.0f, 4.0f, r*0.62f, 2.0f);
    g.setColour(juce::Colour(0xff9de8ff));
    g.fillPath(p, juce::AffineTransform::rotation(start + pos*(end-start)).translated(c.x, c.y));
}

void SakuraLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                              const juce::Colour&, bool hover, bool down)
{
    auto a = b.getLocalBounds().toFloat().reduced(1.0f);
    auto base = down ? juce::Colour(0xff8e4b91) : hover ? juce::Colour(0xffd56eb9) : juce::Colour(0xffad5aa7);
    g.setColour(base); g.fillRoundedRectangle(a, 10.0f);
    g.setColour(juce::Colour(0x66ffffff)); g.drawRoundedRectangle(a, 10.0f, 1.0f);
}

SakuraFontAudioProcessorEditor::SakuraFontAudioProcessorEditor(SakuraFontAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&look);
    setSize(720, 440);

    title.setText("SAKURAFONT", juce::dontSendNotification);
    title.setFont(juce::Font(30.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffffdcf5));
    subtitle.setText("dreamy SoundFont instrument", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffa9dff4));

    fileLabel.setText(processor.getLoadedFileName(), juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setColour(juce::Label::textColourId, juce::Colour(0xffeee7ff));

    for (auto* c : { &title, &subtitle, &fileLabel, &loadButton, &panicButton,
                     &gain, &reverb, &chorus, &bank, &program,
                     &gainLabel, &reverbLabel, &chorusLabel, &bankLabel, &programLabel }) addAndMakeVisible(c);

    auto setupKnob = [](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
    };
    setupKnob(gain); setupKnob(reverb); setupKnob(chorus);
    bank.setSliderStyle(juce::Slider::LinearHorizontal); bank.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);
    program.setSliderStyle(juce::Slider::LinearHorizontal); program.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);

    gainLabel.setText("GAIN", juce::dontSendNotification);
    reverbLabel.setText("REVERB", juce::dontSendNotification);
    chorusLabel.setText("CHORUS", juce::dontSendNotification);
    bankLabel.setText("BANK", juce::dontSendNotification);
    programLabel.setText("PROGRAM", juce::dontSendNotification);
    for (auto* l : { &gainLabel, &reverbLabel, &chorusLabel, &bankLabel, &programLabel }) {
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colour(0xfff3cce9));
    }

    gainA = std::make_unique<SliderAttachment>(processor.state, "gain", gain);
    reverbA = std::make_unique<SliderAttachment>(processor.state, "reverb", reverb);
    chorusA = std::make_unique<SliderAttachment>(processor.state, "chorus", chorus);
    bankA = std::make_unique<SliderAttachment>(processor.state, "bank", bank);
    programA = std::make_unique<SliderAttachment>(processor.state, "program", program);

    loadButton.onClick = [this] { loadClicked(); };
    panicButton.onClick = [this] { processor.panic(); };
    bank.onValueChange = [this] { updatePreset(); };
    program.onValueChange = [this] { updatePreset(); };
    startTimerHz(4);
}

SakuraFontAudioProcessorEditor::~SakuraFontAudioProcessorEditor() { setLookAndFeel(nullptr); }

void SakuraFontAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bg(juce::Colour(0xff140d26), 0, 0, juce::Colour(0xff3a183f), (float)getWidth(), (float)getHeight(), false);
    g.setGradientFill(bg); g.fillAll();

    g.setColour(juce::Colour(0x22ffffff));
    for (int i = 0; i < 36; ++i) {
        const float x = (float)((i * 83) % getWidth());
        const float y = (float)((i * 47) % getHeight());
        g.fillEllipse(x, y, 3.0f + (i % 4), 3.0f + (i % 4));
    }

    g.setColour(juce::Colour(0x301e1636));
    g.fillRoundedRectangle(28.0f, 100.0f, (float)getWidth()-56.0f, 302.0f, 22.0f);
    g.setColour(juce::Colour(0x55f7a5dc));
    g.drawRoundedRectangle(28.0f, 100.0f, (float)getWidth()-56.0f, 302.0f, 22.0f, 1.5f);

    // Original anime-inspired mascot silhouette: moon, hair ribbon, and headphones.
    g.setColour(juce::Colour(0x22ffe3f5)); g.fillEllipse(560.0f, 12.0f, 118.0f, 118.0f);
    g.setColour(juce::Colour(0x66f4b6dc));
    juce::Path ribbon; ribbon.addTriangle(635, 49, 673, 32, 664, 69); ribbon.addTriangle(635, 49, 608, 24, 606, 67); g.fillPath(ribbon);
    g.setColour(juce::Colour(0x339de8ff)); g.drawEllipse(585, 39, 65, 65, 7.0f);
}

void SakuraFontAudioProcessorEditor::resized()
{
    title.setBounds(32, 18, 330, 38); subtitle.setBounds(34, 54, 300, 24);
    loadButton.setBounds(38, 116, 130, 38); panicButton.setBounds(178, 116, 90, 38);
    fileLabel.setBounds(286, 116, 390, 38);

    const int y = 176, knobW = 150;
    gain.setBounds(52, y, knobW, 150); reverb.setBounds(214, y, knobW, 150); chorus.setBounds(376, y, knobW, 150);
    gainLabel.setBounds(52, 318, knobW, 24); reverbLabel.setBounds(214, 318, knobW, 24); chorusLabel.setBounds(376, 318, knobW, 24);
    bankLabel.setBounds(535, 184, 120, 24); bank.setBounds(532, 210, 135, 34);
    programLabel.setBounds(535, 266, 120, 24); program.setBounds(532, 292, 135, 34);
}

void SakuraFontAudioProcessorEditor::timerCallback() { fileLabel.setText(processor.getLoadedFileName(), juce::dontSendNotification); }

void SakuraFontAudioProcessorEditor::loadClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Choose a SoundFont", juce::File{}, "*.sf2");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            auto f = fc.getResult();
            if (f.existsAsFile() && !processor.loadSoundFont(f))
                juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Load failed", "FluidSynth could not load this SoundFont.");
        });
}

void SakuraFontAudioProcessorEditor::updatePreset()
{
    processor.selectPreset((int)bank.getValue(), (int)program.getValue());
}
