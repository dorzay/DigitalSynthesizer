#include "EnvelopeComponent.h"

EnvelopeComponent::EnvelopeComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int index, const std::unordered_map<std::string, Linkable*>& targets)
    : envelopeIndex(index),
    processor(processor),
    attackKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::LinearVertical),
    decayKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::LinearVertical),
    sustainKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::LinearVertical),
    releaseKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::LinearVertical)
{
    // Envelope Title
    titleLabel.setText(name, juce::dontSendNotification);
    titleLabel.setFont(juce::Font(UI::Fonts::headerFontSize));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Mode ComboBox    
    modeLabel.setText("Mode:", juce::dontSendNotification);
    modeLabel.setJustificationType(juce::Justification::centredRight);
    modeLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(modeLabel);

    int modeId = 1;
    for (const auto& [mode, label] : Envelope::getModeList())
    {
        modeSelector.addItem(label, modeId++);
    }

    modeSelector.setSelectedId(1);
    modeSelector.updateTheme();
    addAndMakeVisible(modeSelector);

    const auto modeSpec = Envelope::getEnvelopeModeParamSpecs(index);
    modeSelectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, modeSpec.paramID, modeSelector);

    // Link ComboBox
    linkLabel.setText("Link:", juce::dontSendNotification);
    linkLabel.setJustificationType(juce::Justification::centredRight);
    linkLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(linkLabel);

    setLinkableTargets(targets);
    linkTargetSelector.updateTheme();
    addAndMakeVisible(linkTargetSelector);

    const auto linkSpec = Envelope::getEnvelopeLinkParamSpecs(index);
    linkSelectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, linkSpec.paramID, linkTargetSelector);

    // ADSR Knobs
    auto specs = Envelope::getParamSpecs(index);
    knobs = {
        { &attackKnob,  Envelope::ADSR::Attack,  FormattingUtils::FormatType::Time },
        { &decayKnob,   Envelope::ADSR::Decay,   FormattingUtils::FormatType::Time },
        { &sustainKnob, Envelope::ADSR::Sustain, FormattingUtils::FormatType::Percent },
        { &releaseKnob, Envelope::ADSR::Release, FormattingUtils::FormatType::Time }
    };
    for (const auto& [knobPtr, type, formatType] : knobs)
    {
        const auto& spec = specs[static_cast<int>(type)];
        setupKnob(apvts, *knobPtr, spec, formatType);
    }

    updateTheme();
}

EnvelopeComponent::~EnvelopeComponent()
{
    attackKnob.getSlider().removeListener(this);
    decayKnob.getSlider().removeListener(this);
    sustainKnob.getSlider().removeListener(this);
    releaseKnob.getSlider().removeListener(this);

    attackKnob.cleanup();
    decayKnob.cleanup();
    sustainKnob.cleanup();
    releaseKnob.cleanup();

    if (currentlyLinkedTarget != nullptr)
    {
        processor.unregisterEnvelopeLink(currentlyLinkedTarget, this);
        currentlyLinkedTarget = nullptr;
    }
}

void EnvelopeComponent::registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    const auto specs = Envelope::getParamSpecs(index);

    for (const auto& spec : specs)
    {
        KnobModulationEngine::registerParameters(layout, spec.id);
    }
}

void EnvelopeComponent::setLinkableTargets(const std::unordered_map<std::string, Linkable*>& targets)
{
    linkableTargets = targets;
    linkTargetSelector.clear();

    int id = 1;
    linkTargetSelector.addItem("-", id++);

    for (const auto& [name, ptr] : linkableTargets)
    {
        linkTargetSelector.addItem(name, id++);
    }
    linkTargetSelector.setSelectedId(1, juce::dontSendNotification);
    currentlyLinkedTarget = nullptr;

    linkTargetSelector.onChange = [this]()
        {
            auto selectedName = linkTargetSelector.getText().toStdString();

            if (selectedName == "-")
            {
                if (currentlyLinkedTarget)
                {
                    currentlyLinkedTarget->setEnvelope(nullptr);
                    currentlyLinkedTarget = nullptr;
                }

                // Clean up all modulated knobs using this envelope
                processor.getModulationRouter().disconnectAllTargetsUsing({ ModulationSourceType::Envelope, envelopeIndex });

                return;
            }

            auto it = linkableTargets.find(selectedName);
            if (it != linkableTargets.end())
            {
                // Unlink the previous target
                if (currentlyLinkedTarget && currentlyLinkedTarget != it->second)
                {
                    currentlyLinkedTarget->setEnvelope(nullptr);
                }

                processor.registerEnvelopeLinkOwnership(it->second, this);
                it->second->setEnvelope(processor.getEnvelope(envelopeIndex));
                currentlyLinkedTarget = it->second;
            }

        };
}

void EnvelopeComponent::unlinkTarget(Linkable* target)
{
    if (currentlyLinkedTarget == target)
    {
        target->setEnvelope(nullptr);
        currentlyLinkedTarget = nullptr;
        linkTargetSelector.setSelectedId(1, juce::NotificationType::sendNotificationSync); // Reset UI to "-"
        linkTargetSelector.repaint();
    }
}

int EnvelopeComponent::getTotalWidth()
{
    return totalWidth;
}

void EnvelopeComponent::paint(juce::Graphics& g)
{
    g.fillAll(UI::Colors::EnvelopeBackground);
    g.setColour(UI::Colors::EnvelopeText);
    g.setFont(juce::Font(UI::Fonts::defaultFontSize));

    g.setColour(UI::Colors::EnvelopeGraphStroke);
    g.drawRect(envelopeGraphArea, 2);
    drawEnvelopeGraph(g);
}

void EnvelopeComponent::resized()
{
    auto bounds = getLocalBounds().reduced(rowPadding);

    // First Row: Mode ComboBox, Title, Link ComboBox
    auto titleRow = bounds.removeFromTop(selectorHeight);
    const int oneThirdWidth = titleRow.getWidth() / 3;

    auto modeArea = titleRow.removeFromLeft(oneThirdWidth);
    modeLabel.setBounds(
        modeArea.removeFromLeft(selectorLabelWidth).reduced(rowPadding).translated(labelSelectorOffsetX, 0));
    modeSelector.setBounds(
        modeArea.reduced(rowPadding).translated(labelSelectorOffsetX, 0));

    auto titleArea = titleRow.removeFromLeft(oneThirdWidth);
    titleLabel.setBounds(titleArea.reduced(rowPadding));

    auto linkArea = titleRow.removeFromLeft(oneThirdWidth);
    linkLabel.setBounds(
        linkArea.removeFromLeft(selectorLabelWidth).reduced(rowPadding).translated(-labelSelectorOffsetX, 0));
    linkTargetSelector.setBounds(
        linkArea.reduced(rowPadding).translated(-labelSelectorOffsetX, 0));

    // Layout Remaining Space: Graph Left, Knobs Right
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds;

    // Envelope Graph Area
    envelopeGraphArea = leftColumn;
    envelopeGraphArea.removeFromLeft(graphLeftMargin);
    envelopeGraphArea.reduce(graphReduceX, graphReduceY);
    envelopeGraphArea.translate(graphTranslateX, graphTranslateY);

    // ADSR Knobs Area
    auto knobRow = rightColumn;
    knobRow.translate(0, -knobOffsetY);

    const int knobWidth = knobRow.getWidth() / 4;
    attackKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    decayKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    sustainKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    releaseKnob.setBounds(knobRow.reduced(knobSpacing));
}

void EnvelopeComponent::updateTheme()
{
    modeLabel.setColour(juce::Label::textColourId, UI::Colors::EnvelopeText);
    modeSelector.updateTheme();

    titleLabel.setColour(juce::Label::textColourId, UI::Colors::EnvelopeText);
    linkLabel.setColour(juce::Label::textColourId, UI::Colors::EnvelopeText);
    linkTargetSelector.updateTheme();

    for (const auto& [knobPtr, type, formatType] : knobs)
    {
        knobPtr->updateTheme();
    }

    repaint();
}

void EnvelopeComponent::setupKnob(juce::AudioProcessorValueTreeState& apvts, Knob& knob, const Envelope::KnobParamSpecs& spec, FormattingUtils::FormatType formatType)
{
    knob.initialize(apvts, processor, spec.id, spec.name,
        Knob::KnobParams(spec.defaultValue, spec.minValue, spec.maxValue, spec.stepSize, false,
            nullptr, // No specific formatting function for knob setup
            overideTextBoxWidth),
        Knob::KnobStyle::LinearVertical);

    knob.bindToParameter();
    processor.registerKnob(&knob);
    knob.getSlider().addListener(this);
    addAndMakeVisible(knob);

    // Set the knob's text formatting function based on the format type
    knob.getSlider().textFromValueFunction = [formatType](double value) {
        switch (formatType)
        {
        case FormattingUtils::FormatType::Time:
            return FormattingUtils::formatValue(static_cast<float>(value), formatType, Envelope::MIN_ADSR_TIME_MS, Envelope::MAX_ADSR_TIME_MS);

        case FormattingUtils::FormatType::Percent:
            return FormattingUtils::formatValue(static_cast<float>(value), formatType);

        default:
            return FormattingUtils::formatValue(static_cast<float>(value), formatType);
        }
        };

    knob.getSlider().updateText();
}

void EnvelopeComponent::drawEnvelopeGraph(juce::Graphics& g)
{
    float attackNorm = attackKnob.getSliderValue();
    float decayNorm = decayKnob.getSliderValue();
    float sustain = sustainKnob.getSliderValue();
    float releaseNorm = releaseKnob.getSliderValue();

    float attackMs = FormattingUtils::normalizedToValue(attackNorm, FormattingUtils::FormatType::Time, Envelope::MIN_ADSR_TIME_MS, Envelope::MAX_ADSR_TIME_MS);
    float decayMs = FormattingUtils::normalizedToValue(decayNorm, FormattingUtils::FormatType::Time, Envelope::MIN_ADSR_TIME_MS, Envelope::MAX_ADSR_TIME_MS);
    float releaseMs = FormattingUtils::normalizedToValue(releaseNorm, FormattingUtils::FormatType::Time, Envelope::MIN_ADSR_TIME_MS, Envelope::MAX_ADSR_TIME_MS);

    EnvelopeGraph graph;
    graph.setParameters(attackMs, decayMs, sustain, releaseMs);
    graph.setGraphBounds(envelopeGraphArea);
    graph.generate();

    g.setFont(12.0f);

    // Draw Y grid lines
    g.setColour(UI::Colors::EnvelopeGraphGridLines);
    for (const auto& grid : graph.getYGridLines())
    {
        g.drawLine(grid.line);
    }

    g.setColour(UI::Colors::EnvelopeGraphGridText);
    for (const auto& grid : graph.getYGridLines())
    {
        g.drawText(grid.label,
            static_cast<int>(grid.labelPosition.getX()), static_cast<int>(grid.labelPosition.getY()),
           40, 16, grid.justification);
    }

    // Draw X grid lines
    g.setColour(UI::Colors::EnvelopeGraphGridLines);
    for (const auto& grid : graph.getXGridLines())
    {
        g.drawLine(grid.line);
    }

    g.setColour(UI::Colors::EnvelopeGraphGridText);
    for (const auto& grid : graph.getXGridLines())
    {
        g.drawText(grid.label,
            static_cast<int>(grid.labelPosition.getX()), static_cast<int>(grid.labelPosition.getY()),
            40, 16, grid.justification);
    }

    // Draw envelope path
    g.setColour(UI::Colors::EnvelopeGraphCurve);
    g.strokePath(graph.getEnvelopePath(), juce::PathStrokeType(2.0f));
}

void EnvelopeComponent::sliderValueChanged(juce::Slider*)
{
    repaint();
}