#include "PluginEditor.h"
#include <cmath>

SakuraLookAndFeel::SakuraLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff9efff));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x33120d27));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId, juce::Colour(0xfff9efff));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xaa201536));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff1b122e));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xfff9efff));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff9659a8));
}

void SakuraLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float position, float start, float end, juce::Slider&)
{
    auto area = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(8.0f);
    const float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.36f;
    const auto centre = area.getCentre();
    g.setColour(juce::Colour(0x551b0d31));
    g.fillEllipse(centre.x - radius - 5.0f, centre.y - radius - 5.0f, radius * 2.0f + 10.0f, radius * 2.0f + 10.0f);
    g.setColour(juce::Colour(0xff251640));
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour(juce::Colour(0xffef83cf));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle(-2.0f, -radius + 7.0f, 4.0f, radius * 0.55f, 2.0f);
    g.setColour(juce::Colour(0xff9be7ff));
    g.fillPath(pointer, juce::AffineTransform::rotation(start + position * (end - start)).translated(centre.x, centre.y));
}

void SakuraLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&, bool hover, bool down)
{
    auto area = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto base = down ? juce::Colour(0xff79427f) : hover ? juce::Colour(0xffd66db7) : juce::Colour(0xffa858a4);
    g.setColour(base);
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(juce::Colour(0x77ffffff));
    g.drawRoundedRectangle(area, 10.0f, 1.0f);
}

void SakuraLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    auto area = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(1.0f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(area, 8.0f);
    g.setColour(juce::Colour(0x66ef83cf));
    g.drawRoundedRectangle(area, 8.0f, 1.0f);
    juce::Path arrow;
    arrow.addTriangle((float)width - 23.0f, (float)height * 0.42f,
                      (float)width - 11.0f, (float)height * 0.42f,
                      (float)width - 17.0f, (float)height * 0.62f);
    g.setColour(juce::Colour(0xff9be7ff));
    g.fillPath(arrow);
}

SakuraFontAudioProcessorEditor::SakuraFontAudioProcessorEditor(SakuraFontAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      keyboard(processor.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&look);
    setResizable(true, true);
    setResizeLimits(820, 590, 1400, 920);
    setSize(1000, 680);

    title.setText("SAKURAFONT", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(32.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffffddf5));
    subtitle.setText("dreamy SoundFont instrument", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffa9e3f6));
    fileLabel.setText(processor.getLoadedFileName(), juce::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff6efff));
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    dropHint.setText("DRAG & DROP .SF2", juce::dontSendNotification);
    dropHint.setColour(juce::Label::textColourId, juce::Colour(0x99ffd7ef));
    dropHint.setJustificationType(juce::Justification::centred);

    configureKnob(gain, gainLabel, "VOLUME");
    configureKnob(attack, attackLabel, "ATTACK");
    configureKnob(decay, decayLabel, "DECAY");
    configureKnob(sustain, sustainLabel, "SUSTAIN");
    configureKnob(release, releaseLabel, "RELEASE");
    configureKnob(reverb, reverbLabel, "REVERB");
    configureKnob(chorus, chorusLabel, "CHORUS");

    bankLabel.setText("BANK", juce::dontSendNotification);
    programLabel.setText("PRESET", juce::dontSendNotification);
    for (auto* label : { &bankLabel, &programLabel })
    {
        label->setColour(juce::Label::textColourId, juce::Colour(0xfff3cce9));
        label->setJustificationType(juce::Justification::centredLeft);
    }

    for (int i = 0; i <= 128; ++i)
        bankBox.addItem("Bank " + juce::String(i), i + 1);
    for (int i = 0; i < 128; ++i)
        programBox.addItem(juce::String(i + 1).paddedLeft('0', 3) + "  Program", i + 1);

    keyboard.setAvailableRange(24, 108);
    keyboard.setLowestVisibleKey(36);
    keyboard.setKeyWidth(22.0f);
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xfff6efff));
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff241436));
    keyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour(0x5584569b));
    keyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour(0x55ef83cf));
    keyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0xaa9be7ff));

    for (auto* component : { static_cast<juce::Component*>(&title), &subtitle, &fileLabel, &dropHint,
                             &loadButton, &panicButton, &gain, &attack, &decay, &sustain, &release,
                             &reverb, &chorus, &gainLabel, &attackLabel, &decayLabel, &sustainLabel,
                             &releaseLabel, &reverbLabel, &chorusLabel, &bankBox, &programBox,
                             &bankLabel, &programLabel, &keyboard })
        addAndMakeVisible(component);

    gainA = std::make_unique<SliderAttachment>(processor.state, "gain", gain);
    attackA = std::make_unique<SliderAttachment>(processor.state, "attack", attack);
    decayA = std::make_unique<SliderAttachment>(processor.state, "decay", decay);
    sustainA = std::make_unique<SliderAttachment>(processor.state, "sustain", sustain);
    releaseA = std::make_unique<SliderAttachment>(processor.state, "release", release);
    reverbA = std::make_unique<SliderAttachment>(processor.state, "reverb", reverb);
    chorusA = std::make_unique<SliderAttachment>(processor.state, "chorus", chorus);
    bankA = std::make_unique<ComboAttachment>(processor.state, "bank", bankBox);
    programA = std::make_unique<ComboAttachment>(processor.state, "program", programBox);

    loadButton.onClick = [this] { loadClicked(); };
    panicButton.onClick = [this] { processor.panic(); };
    bankBox.onChange = [this] { updatePreset(); };
    programBox.onChange = [this] { updatePreset(); };

    juce::Random random;
    for (auto& petal : petals)
    {
        petal.x = random.nextFloat() * (float)getWidth();
        petal.y = random.nextFloat() * (float)getHeight();
        petal.speed = 0.35f + random.nextFloat() * 1.2f;
        petal.phase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        petal.size = 3.0f + random.nextFloat() * 7.0f;
    }
    startTimerHz(30);
}

SakuraFontAudioProcessorEditor::~SakuraFontAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SakuraFontAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 21);
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour(0xfff3cce9));
    label.setJustificationType(juce::Justification::centred);
}

void SakuraFontAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient background(juce::Colour(0xff100a22), 0, 0,
                                    juce::Colour(0xff39163e), (float)getWidth(), (float)getHeight(), false);
    g.setGradientFill(background);
    g.fillAll();

    g.setColour(juce::Colour(0x18bfeaff));
    for (int i = 0; i < 18; ++i)
        g.drawLine(0.0f, (float)(i * 44), (float)getWidth(), (float)(i * 44 + 130), 1.0f);

    g.setColour(juce::Colour(0x18ffe4f5));
    g.fillEllipse((float)getWidth() - 180.0f, 20.0f, 135.0f, 135.0f);

    g.setColour(juce::Colour(0x44f5a9da));
    juce::Path originalCharacterSilhouette;
    originalCharacterSilhouette.startNewSubPath((float)getWidth() - 118.0f, 62.0f);
    originalCharacterSilhouette.cubicTo((float)getWidth() - 152.0f, 75.0f, (float)getWidth() - 156.0f, 130.0f, (float)getWidth() - 126.0f, 154.0f);
    originalCharacterSilhouette.cubicTo((float)getWidth() - 94.0f, 178.0f, (float)getWidth() - 58.0f, 139.0f, (float)getWidth() - 67.0f, 101.0f);
    originalCharacterSilhouette.cubicTo((float)getWidth() - 73.0f, 73.0f, (float)getWidth() - 96.0f, 54.0f, (float)getWidth() - 118.0f, 62.0f);
    g.fillPath(originalCharacterSilhouette);

    for (const auto& petal : petals)
    {
        g.setColour(juce::Colour(0x99f7a8d2));
        juce::Path shape;
        shape.addEllipse(-petal.size * 0.5f, -petal.size, petal.size, petal.size * 1.7f);
        g.fillPath(shape, juce::AffineTransform::rotation(petal.phase).translated(petal.x, petal.y));
    }

    auto panel = getLocalBounds().toFloat().reduced(24.0f);
    panel.removeFromTop(90.0f);
    panel.removeFromBottom(145.0f);
    g.setColour(juce::Colour(0x5c17102d));
    g.fillRoundedRectangle(panel, 22.0f);
    g.setColour(juce::Colour(0x66f39bd2));
    g.drawRoundedRectangle(panel, 22.0f, 1.5f);

    auto dropArea = juce::Rectangle<float>(30.0f, 105.0f, (float)getWidth() - 60.0f, 54.0f);
    g.setColour(juce::Colour(0x252cd8ff));
    g.fillRoundedRectangle(dropArea, 12.0f);
    g.setColour(juce::Colour(0x669be7ff));
    g.drawRoundedRectangle(dropArea, 12.0f, 1.0f);
}

void SakuraFontAudioProcessorEditor::resized()
{
    const int width = getWidth();
    title.setBounds(30, 16, 350, 42);
    subtitle.setBounds(33, 54, 330, 24);
    loadButton.setBounds(40, 113, 125, 38);
    panicButton.setBounds(174, 113, 82, 38);
    fileLabel.setBounds(272, 113, width - 470, 38);
    dropHint.setBounds(width - 195, 113, 150, 38);

    const int panelTop = 178;
    const int knobWidth = juce::jmax(90, (width - 90) / 7);
    juce::Slider* sliders[] = { &gain, &attack, &decay, &sustain, &release, &reverb, &chorus };
    juce::Label* labels[] = { &gainLabel, &attackLabel, &decayLabel, &sustainLabel, &releaseLabel, &reverbLabel, &chorusLabel };
    for (int i = 0; i < 7; ++i)
    {
        const int x = 35 + i * knobWidth;
        sliders[i]->setBounds(x, panelTop, knobWidth - 5, 145);
        labels[i]->setBounds(x, panelTop + 132, knobWidth - 5, 22);
    }

    bankLabel.setBounds(45, 345, 80, 22);
    bankBox.setBounds(45, 369, 180, 36);
    programLabel.setBounds(245, 345, 100, 22);
    programBox.setBounds(245, 369, juce::jmin(320, width - 310), 36);

    keyboard.setBounds(24, getHeight() - 132, getWidth() - 48, 106);
}

bool SakuraFontAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    return files.size() == 1 && juce::File(files[0]).getFileExtension().equalsIgnoreCase(".sf2");
}

void SakuraFontAudioProcessorEditor::filesDropped(const juce::StringArray& files, int, int)
{
    if (files.size() == 1)
        loadFile(juce::File(files[0]));
}

void SakuraFontAudioProcessorEditor::timerCallback()
{
    for (auto& petal : petals)
    {
        petal.y += petal.speed;
        petal.x += std::sin(petal.phase) * 0.45f;
        petal.phase += 0.018f;
        if (petal.y > (float)getHeight() + 12.0f)
        {
            petal.y = -12.0f;
            petal.x = std::fmod(petal.x + 173.0f, (float)juce::jmax(1, getWidth()));
        }
    }
    fileLabel.setText(processor.getLoadedFileName(), juce::dontSendNotification);
    repaint();
}

void SakuraFontAudioProcessorEditor::loadClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Choose a SoundFont", juce::File{}, "*.sf2");
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                         [this](const juce::FileChooser& fc) { loadFile(fc.getResult()); });
}

void SakuraFontAudioProcessorEditor::loadFile(const juce::File& file)
{
    if (file.existsAsFile() && !processor.loadSoundFont(file))
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                               "SoundFont could not be loaded",
                                               "Choose a valid .sf2 SoundFont file.");
}

void SakuraFontAudioProcessorEditor::updatePreset()
{
    processor.selectPreset(bankBox.getSelectedItemIndex(), programBox.getSelectedItemIndex());
}
