#include "OscillatorComponent.h"
#include "BinaryData.h"

OscillatorComponent::OscillatorComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int i)
    : apvtsRef(apvts), processorRef(processor), index(i), waveformSelector(ComboBox::Mode::Image), octaveSelector(ComboBox::Mode::Text),
    volumeKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    panKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    voicesKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    detuneKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary)
{
    const auto waveformSpec = Oscillator::getComboBoxParamSpecs(Oscillator::ParamID::Waveform, index);
    const auto octaveSpec = Oscillator::getComboBoxParamSpecs(Oscillator::ParamID::Octave, index);

    // Title Label
    titleLabel.setText(name, juce::dontSendNotification);
    titleLabel.setFont(juce::Font(UI::Fonts::headerFontSize));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Bypass toggle
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::textColourId, UI::Colors::OscillatorText);
    bypassButton.setColour(juce::ToggleButton::tickColourId, UI::Colors::OscillatorText);
    bypassButton.setColour(juce::ToggleButton::tickDisabledColourId, UI::Colors::OscillatorText.withAlpha(0.4f));
    addAndMakeVisible(bypassButton);

    const auto bypassSpec = Oscillator::getToggleParamSpecs(Oscillator::ParamID::Bypass, index);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvtsRef, bypassSpec.first, bypassButton);

    // Waveform selector
    createWaveformSelector(); // images and menu items
    addAndMakeVisible(waveformSelector);

    waveformLabel.setText("Waveform:", juce::dontSendNotification);
    waveformLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    waveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waveformLabel);

    // Octave selector
    for (int j = 0; j < octaveSpec.choices.size(); ++j)
        octaveSelector.addTextItem(j + 1, octaveSpec.choices[j]);

    octaveSelector.setSelectedId(octaveSpec.defaultIndex + 1);
    addAndMakeVisible(octaveSelector);

    octaveLabel.setText("Octave:", juce::dontSendNotification);
    octaveLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    octaveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(octaveLabel);

    // Knobs via helper
    setupKnob(volumeKnob, Oscillator::ParamID::Volume);
    setupKnob(panKnob, Oscillator::ParamID::Pan);
    setupKnob(voicesKnob, Oscillator::ParamID::Voices);
    setupKnob(detuneKnob, Oscillator::ParamID::Detune);

    // Bind knob parameters
    volumeKnob.bindToParameter();
    panKnob.bindToParameter();
    voicesKnob.bindToParameter();
    detuneKnob.bindToParameter();

    // Attachments
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef, waveformSpec.paramID, waveformSelector);

    octaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef, octaveSpec.paramID, octaveSelector);

    updateTheme();
}

OscillatorComponent::~OscillatorComponent()
{
    // Reset APVTS attachments
    bypassAttachment.reset();
    waveformAttachment.reset();
    octaveAttachment.reset();
}

void OscillatorComponent::registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    for (Oscillator::ParamID id : {
        Oscillator::ParamID::Volume,
            Oscillator::ParamID::Pan,
            Oscillator::ParamID::Detune,
            Oscillator::ParamID::Voices
    })
    {
        auto spec = Oscillator::getKnobParamSpecs(id, index);
        KnobModulationEngine::registerParameters(layout, spec.id);
    }
}

int OscillatorComponent::getTotalHeight()
{
    return titleHeight + selectorHeight + knobRowHeight + 10;
}

int OscillatorComponent::getTotalWidth()
{
    return totalWidth;
}

void OscillatorComponent::paint(juce::Graphics& g)
{
    g.fillAll(UI::Colors::OscillatorBackground);
    g.setColour(UI::Colors::OscillatorText);
    g.setFont(juce::Font(UI::Fonts::defaultFontSize));
}

void OscillatorComponent::resized()
{
    auto area = getLocalBounds().reduced(rowPadding);

    // First row: Bypass + Title centered visually
    auto titleArea = area.removeFromTop(titleHeight);

    // Reserve space for bypass button
    auto bypassArea = titleArea.removeFromLeft(bypassWidth);
    bypassButton.setBounds(bypassArea.reduced(5));

    // Calculate remaining title area, then center the label *within the full row*
    const int titleX = (getWidth() - titleWidth) / 2;
    titleLabel.setBounds(titleX, bypassArea.getY(), titleWidth, titleArea.getHeight());

    // Second row: Waveform and Octave selectors
    auto selectorRow = area.removeFromTop(selectorHeight);
    auto waveformArea = selectorRow.removeFromLeft(selectorRow.getWidth() / 2);
    auto octaveArea = selectorRow;

    waveformLabel.setBounds(waveformArea.removeFromLeft(selectorWidth).reduced(rowPadding));
    waveformSelector.setBounds(waveformArea.reduced(rowPadding));

    octaveLabel.setBounds(octaveArea.removeFromLeft(selectorWidth).reduced(rowPadding));
    octaveSelector.setBounds(octaveArea.reduced(rowPadding));

    // Third row: Knobs
    auto knobRow = area.removeFromTop(knobRowHeight);
    auto knobWidth = knobRow.getWidth() / numKnobs;

    volumeKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    panKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    voicesKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    detuneKnob.setBounds(knobRow.reduced(knobSpacing));
}

void OscillatorComponent::updateTheme()
{
    titleLabel.setColour(juce::Label::textColourId, UI::Colors::OscillatorText);
    waveformLabel.setColour(juce::Label::textColourId, UI::Colors::OscillatorText);
    octaveLabel.setColour(juce::Label::textColourId, UI::Colors::OscillatorText);

    bypassButton.setColour(juce::ToggleButton::textColourId, UI::Colors::OscillatorText);
    bypassButton.setColour(juce::ToggleButton::tickColourId, UI::Colors::OscillatorText);
    bypassButton.setColour(juce::ToggleButton::tickDisabledColourId, UI::Colors::OscillatorText.withAlpha(0.4f));

    volumeKnob.updateTheme();
    panKnob.updateTheme();
    voicesKnob.updateTheme();
    detuneKnob.updateTheme();

    octaveSelector.updateTheme();

    repaint();
}

void OscillatorComponent::createWaveformSelector()
{
    // Load waveform Drawables
    waveformImages.emplace_back(juce::Drawable::createFromImageData(BinaryData::Sine_png, BinaryData::Sine_pngSize));
    waveformImages.emplace_back(juce::Drawable::createFromImageData(BinaryData::Square_png, BinaryData::Square_pngSize));
    waveformImages.emplace_back(juce::Drawable::createFromImageData(BinaryData::Triangle_png, BinaryData::Triangle_pngSize));
    waveformImages.emplace_back(juce::Drawable::createFromImageData(BinaryData::Sawtooth_png, BinaryData::Sawtooth_pngSize));
    waveformImages.emplace_back(juce::Drawable::createFromImageData(BinaryData::WhiteNoise_png, BinaryData::WhiteNoise_pngSize));

    waveformSelector.setImageList(waveformImages);
    waveformSelector.clear(juce::dontSendNotification);

    // Get spec for display consistency
    const auto waveformSpec = Oscillator::getComboBoxParamSpecs(Oscillator::ParamID::Waveform, index);

    // Create scaled images for popup menu rendering
    for (int j = 0; j < waveformImages.size(); ++j)
    {
        juce::Image img(juce::Image::ARGB,
            static_cast<int>(ComboBox::imageWidth * ComboBox::popupImageScaleFactor),
            static_cast<int>(ComboBox::imageHeight * ComboBox::popupImageScaleFactor),
            true);

        juce::Graphics g(img);
        g.addTransform(juce::AffineTransform::scale(ComboBox::popupImageScaleFactor));
        waveformImages[j]->drawWithin(g,
            juce::Rectangle<float>(0, 0, ComboBox::imageWidth, ComboBox::imageHeight),
            juce::RectanglePlacement::centred, 1.0f);

        waveformSelector.getRootMenu()->addItem(j + 1, "", true, false, img);
    }

    waveformSelector.setSelectedId(waveformSpec.defaultIndex + 1);
}

void OscillatorComponent::setupKnob(Knob& knob, Oscillator::ParamID id)
{
    const auto spec = Oscillator::getKnobParamSpecs(id, index);

    knob.initialize(apvtsRef, processorRef, spec.id, spec.name,
        Knob::KnobParams(spec.defaultValue, spec.minValue, spec.maxValue, spec.stepSize, spec.isDiscrete),
        Knob::KnobStyle::Rotary);

    knob.getSlider().textFromValueFunction = [spec](double value)
        {
            float realValue = static_cast<float>(value);

            switch (spec.formatType)
            {
            case FormattingUtils::FormatType::Percent:
            case FormattingUtils::FormatType::Pan:
            case FormattingUtils::FormatType::Normal:
                return FormattingUtils::formatValue(realValue, spec.formatType, spec.minValue, spec.maxValue);

            case FormattingUtils::FormatType::Discrete:
                return FormattingUtils::formatValue(
                    FormattingUtils::valueToNormalized(realValue, spec.formatType, spec.minValue, spec.maxValue),
                    spec.formatType, spec.minValue, spec.maxValue);

            default:
                return FormattingUtils::formatValue(
                    FormattingUtils::valueToNormalized(realValue, spec.formatType, spec.minValue, spec.maxValue),
                    spec.formatType, spec.minValue, spec.maxValue);
            }
        };

    knob.getSlider().updateText();
    addAndMakeVisible(knob);
    processorRef.registerKnob(&knob);
}
