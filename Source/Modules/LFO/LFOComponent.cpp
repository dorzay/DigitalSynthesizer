#include "LFOComponent.h"

LFOComponent::LFOComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& lfoName, int lfoIndex)
    : apvtsRef(apvts), processorRef(processor), index(lfoIndex), name(lfoName),
    freqKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    shapeKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    stepsKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary)
{
    initializeUI();
    setupAttachments();
    updateDynamicVisibility();
    updateLFOGraph();
    updateTheme();
}

LFOComponent::~LFOComponent()
{
    freqKnob.cleanup();
    shapeKnob.cleanup();
    stepsKnob.cleanup();

    typeAttachment.reset();
    modeAttachment.reset();
    bypassAttachment.reset();

    const auto freqID = LFO::getKnobParamSpecs(LFO::ParamID::Freq, index).id;
    const auto shapeID = LFO::getKnobParamSpecs(LFO::ParamID::Shape, index).id;
    const auto stepsID = LFO::getKnobParamSpecs(LFO::ParamID::Steps, index).id;
    const auto typeID = LFO::getComboBoxParamSpecs(LFO::ParamID::Type, index).paramID;

    apvtsRef.removeParameterListener(freqID, this);
    apvtsRef.removeParameterListener(shapeID, this);
    apvtsRef.removeParameterListener(stepsID, this);
    apvtsRef.removeParameterListener(typeID, this);
}

void LFOComponent::resized()
{
    updateDynamicVisibility();

    auto bounds = getLocalBounds().reduced(rowPadding);

    auto titleRow = bounds.removeFromTop(selectorHeight);
    const int oneThirdWidth = titleRow.getWidth() / 3;

    auto bypassArea = titleRow.removeFromLeft(oneThirdWidth);
    bypassButton.setBounds(bypassArea.reduced(rowPadding));

    auto titleArea = titleRow.removeFromLeft(oneThirdWidth);
    titleLabel.setBounds(titleArea.reduced(rowPadding));
    titleLabel.setJustificationType(juce::Justification::centred);

    auto randomArea = titleRow.removeFromRight(oneThirdWidth);
    juce::Font font = UI::Fonts::defaultFont;
    int textWidth = font.getStringWidth("Random");
    int textHeight = static_cast<int>(font.getHeight());

    int buttonWidth = textWidth + randomButtonExtraW;
    int buttonHeight = textHeight + randomButtonExtraH;
    int buttonX = randomArea.getRight() - buttonWidth - rowPadding - randomButtonPadding;
    int buttonY = randomArea.getY() + (randomArea.getHeight() - buttonHeight) / 2;

    randomizeButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);

    constexpr int layoutYOffset = 10;
    bounds.translate(0, layoutYOffset);

    auto knobRow = bounds.removeFromBottom(knobHeight).reduced(rowPadding);
    const int totalKnobWidth = knobRow.getWidth();
    const int knobY = knobRow.getY();

    std::vector<juce::Component*> visibleKnobs;
    if (freqKnob.isVisible())
    {
        freqKnob.getSlider().setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidthFreq, textBoxHeightFreq);
        visibleKnobs.push_back(&freqKnob);
    }
    if (shapeKnob.isVisible()) visibleKnobs.push_back(&shapeKnob);
    if (stepsKnob.isVisible()) visibleKnobs.push_back(&stepsKnob);

    const int numVisible = static_cast<int>(visibleKnobs.size());
    if (numVisible > 0)
    {
        const int knobWidth = totalKnobWidth / numVisible;
        int x = knobRow.getX();

        for (auto* knob : visibleKnobs)
        {
            knob->setBounds(x, knobY, knobWidth, knobRow.getHeight());
            x += knobWidth;
        }
    }

    auto middleArea = bounds;
    auto leftHalf = middleArea.removeFromLeft(middleArea.getWidth() / 2).reduced(rowPadding);
    auto rightHalf = middleArea.reduced(rowPadding);

    const int selectorTotalHeight = leftHalf.getHeight();
    const int selectorHeightEach = (selectorTotalHeight - selectorSpacing) / 2;

    auto modeRow = leftHalf.removeFromTop(selectorHeightEach);
    modeRow.translate(0, -selectorYOffset);
    modeLabel.setBounds(modeRow.removeFromLeft(labelWidth));
    modeSelector.setBounds(modeRow.removeFromLeft(comboBoxWidth));

    leftHalf.removeFromTop(selectorSpacing);

    auto typeRow = leftHalf.removeFromTop(selectorHeightEach);
    typeRow.translate(0, -selectorYOffset);
    typeLabel.setBounds(typeRow.removeFromLeft(labelWidth));
    typeSelector.setBounds(typeRow.removeFromLeft(comboBoxWidth));

    const int graphWidth = static_cast<int>(rightHalf.getWidth() * graphWidthRatioPct / 100.0f);
    const int graphHeight = rightHalf.getHeight();
    const int graphX = rightHalf.getX() + (rightHalf.getWidth() - graphWidth) / 2 + graphTranslateX;
    const int graphY = rightHalf.getY() + graphTranslateY;

    graphBounds = { graphX, graphY, graphWidth, graphHeight };
    graph.setGraphBounds(graphBounds);

    updateLFOGraph();
}

void LFOComponent::paint(juce::Graphics& g)
{
    g.fillAll(UI::Colors::FilterBackground);
    g.setColour(UI::Colors::EnvelopeGraphStroke);
    g.drawRect(graphBounds, 2);
    drawLFOGraph(g);
}

void LFOComponent::updateTheme()
{
    titleLabel.setColour(juce::Label::textColourId, UI::Colors::LFOText);
    modeLabel.setColour(juce::Label::textColourId, UI::Colors::LFOText);
    typeLabel.setColour(juce::Label::textColourId, UI::Colors::LFOText);

    bypassButton.setColour(juce::ToggleButton::textColourId, UI::Colors::LFOText);
    bypassButton.setColour(juce::ToggleButton::tickColourId, UI::Colors::LFOText);
    bypassButton.setColour(juce::ToggleButton::tickDisabledColourId, UI::Colors::LFOText.withAlpha(0.4f));

    modeSelector.updateTheme();
    typeSelector.updateTheme();

    freqKnob.updateTheme();
    shapeKnob.updateTheme();
    stepsKnob.updateTheme();

    randomizeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    randomizeButton.setColour(juce::TextButton::textColourOffId, UI::Colors::LFOText);
    randomizeButton.setColour(juce::TextButton::textColourOnId, UI::Colors::LFOText);

    repaint();
}

void LFOComponent::registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    for (LFO::ParamID id : { LFO::ParamID::Freq, LFO::ParamID::Shape, LFO::ParamID::Steps })
    {
        auto spec = LFO::getKnobParamSpecs(id, index);
        KnobModulationEngine::registerParameters(layout, spec.id);
    }
}

void LFOComponent::updateDynamicVisibility()
{
    const auto selectedType = static_cast<LFO::Type>(typeSelector.getSelectedId() - 1);
    const bool isSteps = (selectedType == LFO::Type::Steps);

    stepsKnob.setVisible(isSteps);
    randomizeButton.setVisible(isSteps);
}

void LFOComponent::initializeUI()
{
    titleLabel.setText(name, juce::dontSendNotification);
    titleLabel.setFont(juce::Font(UI::Fonts::headerFontSize));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(bypassButton);

    modeLabel.setText("Mode:", juce::dontSendNotification);
    modeLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    modeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(modeLabel);

    auto modeSpec = LFO::getComboBoxParamSpecs(LFO::ParamID::Mode, index);
    for (int i = 0; i < static_cast<int>(LFO::Mode::Count); ++i)
        modeSelector.addItem(modeSpec.choices[i], i + 1);
    modeSelector.setSelectedId(modeSpec.defaultIndex + 1);
    addAndMakeVisible(modeSelector);

    typeLabel.setText("Type:", juce::dontSendNotification);
    typeLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(typeLabel);

    auto typeSpec = LFO::getComboBoxParamSpecs(LFO::ParamID::Type, index);
    for (int i = 0; i < static_cast<int>(LFO::Type::Count); ++i)
        typeSelector.addItem(typeSpec.choices[i], i + 1);
    typeSelector.setSelectedId(typeSpec.defaultIndex + 1);
    addAndMakeVisible(typeSelector);
    typeSelector.onChange = [this]()
        {
            updateDynamicVisibility();
            resized();
            updateLFOGraph();
        };

    setupKnob(freqKnob, LFO::getKnobParamSpecs(LFO::ParamID::Freq, index));
    setupKnob(shapeKnob, LFO::getKnobParamSpecs(LFO::ParamID::Shape, index));
    setupKnob(stepsKnob, LFO::getKnobParamSpecs(LFO::ParamID::Steps, index));

    freqKnob.getSlider().onValueChange = [this]() { updateLFOGraph(); };
    shapeKnob.getSlider().onValueChange = [this]() { updateLFOGraph(); };
    stepsKnob.getSlider().onValueChange = [this]() { updateLFOGraph(); };

    randomizeButton.setButtonText("Random");
    randomizeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    randomizeButton.setColour(juce::TextButton::textColourOffId, UI::Colors::LFOText);
    randomizeButton.setColour(juce::TextButton::textColourOnId, UI::Colors::LFOText);
    randomizeButton.onClick = [this]()
        {
            if (auto* lfo = processorRef.getLFO(index))
            {
                lfo->randomizeSteps();
                updateLFOGraph();
            }
        };
    addAndMakeVisible(randomizeButton);

    if (auto* lfo = processorRef.getLFO(index))
        graph.setLFOReference(lfo);
}

void LFOComponent::setupKnob(Knob& knob, const LFO::KnobParamSpecs& spec)
{
    // Init knob
    knob.initialize(apvtsRef, processorRef, spec.id, spec.name,
        Knob::KnobParams(spec.defaultValue, spec.minValue, spec.maxValue, spec.stepSize, spec.isDiscrete),
        Knob::KnobStyle::Rotary);

    knob.bindToParameter();

    // Set formatted display text based on format type
    knob.getSlider().textFromValueFunction = [spec](double value)
        {
            float realValue = static_cast<float>(value);
            switch (spec.formatType)
            {
            case FormattingUtils::FormatType::Percent:
                return FormattingUtils::formatValue(realValue, spec.formatType);

            case FormattingUtils::FormatType::Time:
                return FormattingUtils::formatValue(realValue, spec.formatType,
                    Envelope::MIN_ADSR_TIME_MS, Envelope::MAX_ADSR_TIME_MS);

            case FormattingUtils::FormatType::LFOFrequency:
                return FormattingUtils::formatValue(
                    realValue,
                    spec.formatType, FormattingUtils::lfoFreqMinHz, FormattingUtils::lfoFreqMaxHz);

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

void LFOComponent::setupAttachments()
{
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvtsRef,
        LFO::getToggleParamSpecs(LFO::ParamID::Bypass, index).first,
        bypassButton
    );

    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef,
        LFO::getComboBoxParamSpecs(LFO::ParamID::Mode, index).paramID,
        modeSelector
    );

    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvtsRef,
        LFO::getComboBoxParamSpecs(LFO::ParamID::Type, index).paramID,
        typeSelector
    );

    const auto freqID = LFO::getKnobParamSpecs(LFO::ParamID::Freq, index).id;
    const auto shapeID = LFO::getKnobParamSpecs(LFO::ParamID::Shape, index).id;
    const auto stepsID = LFO::getKnobParamSpecs(LFO::ParamID::Steps, index).id;
    const auto typeID = LFO::getComboBoxParamSpecs(LFO::ParamID::Type, index).paramID;

    apvtsRef.addParameterListener(freqID, this);
    apvtsRef.addParameterListener(shapeID, this);
    apvtsRef.addParameterListener(stepsID, this);
    apvtsRef.addParameterListener(typeID, this);
}

void LFOComponent::updateLFOGraph()
{
    if (auto* lfo = processorRef.getLFO(index))
        lfo->updateFromAPVTS(apvtsRef);

    const auto freqID = LFO::getKnobParamSpecs(LFO::ParamID::Freq, index).id;
    const auto shapeID = LFO::getKnobParamSpecs(LFO::ParamID::Shape, index).id;
    const auto stepsID = LFO::getKnobParamSpecs(LFO::ParamID::Steps, index).id;
    const auto typeID = LFO::getComboBoxParamSpecs(LFO::ParamID::Type, index).paramID;

    const float freqNorm = apvtsRef.getRawParameterValue(freqID)->load();
    const float shape = apvtsRef.getRawParameterValue(shapeID)->load();
    const int   steps = static_cast<int>(apvtsRef.getRawParameterValue(stepsID)->load());
    const auto  type = static_cast<LFO::Type>(static_cast<int>(apvtsRef.getRawParameterValue(typeID)->load()));

    const float freqHz = FormattingUtils::normalizedToValue(
        freqNorm,
        FormattingUtils::FormatType::LFOFrequency,
        FormattingUtils::lfoFreqMinHz,
        FormattingUtils::lfoFreqMaxHz
    );

    graph.setParameters(type, shape, freqHz, steps);
    graph.generate();
    repaint();
}

void LFOComponent::drawLFOGraph(juce::Graphics& g)
{
    g.setFont(12.0f);

    g.setColour(UI::Colors::EnvelopeGraphGridLines);
    for (const auto& grid : graph.getYGridLines())
        g.drawLine(grid.line);

    g.setColour(UI::Colors::EnvelopeGraphGridText);
    for (const auto& grid : graph.getYGridLines())
        g.drawText(grid.label,
            grid.labelPosition.getX(), grid.labelPosition.getY(),
            40, 16, grid.justification);

    g.setColour(UI::Colors::EnvelopeGraphGridLines);
    for (const auto& grid : graph.getXGridLines())
        g.drawLine(grid.line);

    g.setColour(UI::Colors::EnvelopeGraphGridText);
    for (const auto& grid : graph.getXGridLines())
        g.drawText(grid.label,
            grid.labelPosition.getX(), grid.labelPosition.getY(),
            40, 16, grid.justification);

    g.setColour(UI::Colors::EnvelopeGraphCurve);
    g.strokePath(graph.getLFOPath(), juce::PathStrokeType(2.0f));
}

void LFOComponent::parameterChanged(const juce::String& /*paramID*/, float /*newValue*/)
{
    juce::MessageManager::callAsync([this]
        {
            updateLFOGraph();
        });
}