#include "FilterComponent.h"

FilterComponent::FilterComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int filterIndex, const std::unordered_map<std::string, Linkable*>& targets)
    : filterIndex(filterIndex),
     processor(processor),
    cutoffKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    resonanceKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    driveKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    mixKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    morphKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary),
    factorKnob(apvts, processor, "", "", Knob::KnobParams(), Knob::KnobStyle::Rotary)
{
    // Title
    titleLabel.setText(name, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(UI::Fonts::headerFontSize));
    addAndMakeVisible(titleLabel);

    // Filter Type ComboBox
    typeLabel.setText("Type:", juce::dontSendNotification);
    typeLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(typeLabel);
    addAndMakeVisible(typeSelector);

    auto typeSpec = Filter::getComboBoxParamSpecs(Filter::ParamID::Type, filterIndex);
    for (int i = 0; i < static_cast<int>(Filter::Type::Count); ++i)
        typeSelector.addItem(typeSpec.choices[i], i + 1);
    typeSelector.setSelectedId(typeSpec.defaultIndex + 1);

    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, typeSpec.paramID, typeSelector);

    // Bypass Toggle
    bypassToggle.setButtonText("Bypass");
    auto [paramID, label] = Filter::getToggleParamSpecs(Filter::ParamID::Bypass, filterIndex);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramID, bypassToggle);
    addAndMakeVisible(bypassToggle);

    // Link ComboBox
    linkLabel.setText("Link:", juce::dontSendNotification);
    linkLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(linkLabel);
    setLinkableTargets(targets);
    linkSelector.updateTheme();
    addAndMakeVisible(linkSelector);
    auto linkSpec = Filter::getComboBoxParamSpecs(Filter::ParamID::Link, filterIndex);
    linkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, linkSpec.paramID, linkSelector);

    // Slope ComboBox
    slopeLabel.setText("Slope:", juce::dontSendNotification);
    slopeLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(slopeLabel);
    addAndMakeVisible(slopeSelector);
    auto slopeSpec = Filter::getComboBoxParamSpecs(Filter::ParamID::Slope, filterIndex);
    for (int i = 0; i < slopeSpec.choices.size(); ++i)
        slopeSelector.addItem(slopeSpec.choices[i], i + 1);
    slopeSelector.setSelectedId(slopeSpec.defaultIndex + 1);
    slopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, slopeSpec.paramID, slopeSelector);

    // Vowel ComboBox (Talkbox Only)
    vowelLabel.setText("Vowel:", juce::dontSendNotification);
    vowelLabel.setFont(juce::Font(UI::Fonts::defaultFontSize));
    addAndMakeVisible(vowelLabel);
    addAndMakeVisible(vowelSelector);
    auto vowelSpec = TalkboxFilter::getComboBoxParamSpecs(TalkboxFilter::ParamID::Vowel, filterIndex);
    for (int i = 0; i < vowelSpec.choices.size(); ++i)
        vowelSelector.addItem(vowelSpec.choices[i], i + 1);
    vowelSelector.setSelectedId(vowelSpec.defaultIndex + 1);
    vowelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, vowelSpec.paramID, vowelSelector);

    // Knobs
    setupKnob(apvts, cutoffKnob, Filter::getKnobParamSpecs(Filter::ParamID::Cutoff, filterIndex));
    setupKnob(apvts, resonanceKnob, Filter::getKnobParamSpecs(Filter::ParamID::Resonance, filterIndex));
    setupKnob(apvts, driveKnob, Filter::getKnobParamSpecs(Filter::ParamID::Drive, filterIndex));
    setupKnob(apvts, mixKnob, Filter::getKnobParamSpecs(Filter::ParamID::Mix, filterIndex));

    // Talkbox knobs
    setupKnob(apvts, morphKnob, TalkboxFilter::getKnobParamSpecs(TalkboxFilter::ParamID::Morph, filterIndex));
    setupKnob(apvts, factorKnob, TalkboxFilter::getKnobParamSpecs(TalkboxFilter::ParamID::Factor, filterIndex));

    // Graph
    filterGraph.setSampleRate(processor.getSampleRate());
    addAndMakeVisible(filterGraph);

    updateViewForFilterType();

    // Listeners
    typeSelector.onChange = [this]()
        {
            updateGraphFromKnobs();
            updateCutoffKnobFormat();
            updateViewForFilterType();
        };
    slopeSelector.onChange = [this]() { updateGraphFromKnobs(); };
    cutoffKnob.getSlider().onValueChange = [this]() { updateGraphFromKnobs(); };
    resonanceKnob.getSlider().onValueChange = [this]() { updateGraphFromKnobs(); };
    driveKnob.getSlider().onValueChange = [this]() { updateGraphFromKnobs(); };
    mixKnob.getSlider().onValueChange = [this]() { updateGraphFromKnobs(); };

    updateTheme();
}

FilterComponent::~FilterComponent()
{
    cutoffKnob.cleanup();
    resonanceKnob.cleanup();
    driveKnob.cleanup();
    mixKnob.cleanup();
    morphKnob.cleanup();
    factorKnob.cleanup();

    typeAttachment.reset();
    linkAttachment.reset();
    bypassAttachment.reset();
    slopeAttachment.reset();
    vowelAttachment.reset();

    if (currentlyLinkedTarget != nullptr)
    {
        processor.unregisterFilterLink(currentlyLinkedTarget, this);
        currentlyLinkedTarget = nullptr;
    }
}

void FilterComponent::registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    for (Filter::ParamID id : { Filter::ParamID::Cutoff, Filter::ParamID::Resonance, Filter::ParamID::Drive, Filter::ParamID::Mix })
    {
        const auto spec = Filter::getKnobParamSpecs(id, index);
        KnobModulationEngine::registerParameters(layout, spec.id);
    }

    // Talkbox-specific knobs
    {
        const auto morphSpec = TalkboxFilter::getKnobParamSpecs(TalkboxFilter::ParamID::Morph, index);
        const auto factorSpec = TalkboxFilter::getKnobParamSpecs(TalkboxFilter::ParamID::Factor, index);

        KnobModulationEngine::registerParameters(layout, morphSpec.id);
        KnobModulationEngine::registerParameters(layout, factorSpec.id);
    }
}

int FilterComponent::getTotalWidth()
{
    return totalWidth;
}

void FilterComponent::paint(juce::Graphics& g)
{
    g.fillAll(UI::Colors::FilterBackground);
    g.setColour(UI::Colors::FilterText);
    g.setFont(juce::Font(UI::Fonts::defaultFontSize));

    // Draw bounding box for graph area
    g.setColour(UI::Colors::FilterGraphStroke);
    g.drawRect(graphArea);

    g.setFont(12.0f);

    // Axis lines
    g.setColour(UI::Colors::FilterGraphGridLines);
    for (const auto& grid : filterGraph.getYGridLines()) g.drawLine(grid.line);
    for (const auto& grid : filterGraph.getXGridLines()) g.drawLine(grid.line);

    // Axis labels
    g.setColour(UI::Colors::FilterGraphGridText);
    for (const auto& grid : filterGraph.getYGridLines()) {
        g.drawText(grid.label,
            static_cast<int>(grid.labelPosition.getX()), static_cast<int>(grid.labelPosition.getY()),
            40, 16, grid.justification);
    }
    for (const auto& grid : filterGraph.getXGridLines()) {
        g.drawText(grid.label,
            static_cast<int>(grid.labelPosition.getX()), static_cast<int>(grid.labelPosition.getY()),
            40, 16, grid.justification);
    }

    // Draw FilterGraph content
    g.reduceClipRegion(graphArea.toNearestInt());
    filterGraph.paint(g);

    // Forward current knob values to update graph
    updateGraphFromKnobs();
}

void FilterComponent::resized()
{
    auto bounds = getLocalBounds().reduced(rowPadding);

    // Title Row: Title Centered | Link on Right | Bypass on Left
    auto titleRow = bounds.removeFromTop(selectorHeight);
    const int oneThirdWidth = titleRow.getWidth() / 3;

    // Bypass Toggle on Left
    auto bypassArea = titleRow.removeFromLeft(oneThirdWidth);
    bypassToggle.setBounds(bypassArea.reduced(rowPadding));

    // Title in Center
    auto titleArea = titleRow.removeFromLeft(oneThirdWidth);
    titleLabel.setBounds(titleArea.reduced(rowPadding));
    titleLabel.setJustificationType(juce::Justification::centred);

    // Link on Right
    auto linkArea = titleRow;
    linkLabel.setBounds(linkArea.removeFromLeft(selectorLabelWidth).reduced(rowPadding));
    linkSelector.setBounds(linkArea.reduced(rowPadding).translated(selectorOffsetX, 0));

    // Split remaining space: left for controls, right for graph
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() * 7 / 12);
    auto rightColumn = bounds;

    // Left Column

    // Row 1: Type ComboBox + Slope/Vowel ComboBox
    auto comboRow = leftColumn.removeFromTop(comboRowHeight);

    auto leftHalf = comboRow.removeFromLeft(comboRow.getWidth() / 2);
    auto rightHalf = comboRow;

    const int pairWidth = selectorLabelWidth + comboBoxWidth;

    // Left Half: Type
    {
        int offset = (leftHalf.getWidth() - pairWidth) / 2;
        auto pairArea = leftHalf.withTrimmedLeft(offset).withWidth(pairWidth);

        typeLabel.setBounds(pairArea.removeFromLeft(selectorLabelWidth));
        typeSelector.setBounds(pairArea.removeFromLeft(comboBoxWidth));
    }

    // Right Half: Slope or Vowel
    {
        int offset = (rightHalf.getWidth() - pairWidth) / 2;
        auto pairArea = rightHalf.withTrimmedLeft(offset).withWidth(pairWidth);

        if (vowelSelector.isVisible())
        {
            vowelLabel.setBounds(pairArea.removeFromLeft(selectorLabelWidth));
            vowelSelector.setBounds(pairArea.removeFromLeft(comboBoxWidth));
        }
        else
        {
            slopeLabel.setBounds(pairArea.removeFromLeft(selectorLabelWidth));
            slopeSelector.setBounds(pairArea.removeFromLeft(comboBoxWidth));
        }
    }

    // Row 2: Knobs
    auto knobRow = leftColumn;
    knobRow.setHeight(knobRowHeight);

    const int knobWidth = knobRow.getWidth() / 4;

    if (morphKnob.isVisible())
    {
        morphKnob.getSlider().setTextBoxStyle(juce::Slider::TextBoxBelow, true, wideTextBoxWidth, textBoxHeight);
        morphKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));

        factorKnob.getSlider().setTextBoxStyle(juce::Slider::TextBoxBelow, true, narrowTextBoxWidth, textBoxHeight);
        factorKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    }
    else
    {
        cutoffKnob.getSlider().setTextBoxStyle(juce::Slider::TextBoxBelow, true, wideTextBoxWidth, textBoxHeight);
        cutoffKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));

        resonanceKnob.getSlider().setTextBoxStyle(juce::Slider::TextBoxBelow, true, narrowTextBoxWidth, textBoxHeight);
        resonanceKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    }

    driveKnob.setBounds(knobRow.removeFromLeft(knobWidth).reduced(knobSpacing));
    mixKnob.setBounds(knobRow.reduced(knobSpacing));

    // Right Column: Graph Area
    graphArea = rightColumn;
    graphArea.removeFromLeft(graphLeftMargin);
    graphArea.reduce(graphReduceX, graphReduceY);
    graphArea.translate(graphTranslateX, graphTranslateY);

    lastGraphArea = graphArea.toFloat();
    filterGraph.setBounds(graphArea);
    filterGraph.generateAxisGridLines(graphArea.toFloat());
}

void FilterComponent::updateTheme()
{
    titleLabel.setColour(juce::Label::textColourId, UI::Colors::FilterText);
    typeLabel.setColour(juce::Label::textColourId, UI::Colors::FilterText);
    linkLabel.setColour(juce::Label::textColourId, UI::Colors::FilterText);
    slopeLabel.setColour(juce::Label::textColourId, UI::Colors::FilterText);
    vowelLabel.setColour(juce::Label::textColourId, UI::Colors::FilterText);

    bypassToggle.setColour(juce::ToggleButton::textColourId, UI::Colors::FilterText);
    bypassToggle.setColour(juce::ToggleButton::tickColourId, UI::Colors::FilterText);
    bypassToggle.setColour(juce::ToggleButton::tickDisabledColourId, UI::Colors::FilterText.withAlpha(0.4f));

    typeSelector.updateTheme();
    slopeSelector.updateTheme();
    linkSelector.updateTheme();
    vowelSelector.updateTheme();

    cutoffKnob.updateTheme();
    resonanceKnob.updateTheme();
    driveKnob.updateTheme();
    mixKnob.updateTheme();
    morphKnob.updateTheme();
    factorKnob.updateTheme();

    repaint();
}

void FilterComponent::updateCutoffKnobFormat()
{
    auto selectedType = static_cast<Filter::Type>(typeSelector.getSelectedId() - 1);

    FormattingUtils::FormatType cutoffFormat = FormattingUtils::FormatType::FrequencyLowPass;

    switch (selectedType)
    {
    case Filter::Type::LowPass:
    case Filter::Type::BandPass:
        cutoffFormat = FormattingUtils::FormatType::FrequencyLowPass;
        break;

    case Filter::Type::HighPass:
        cutoffFormat = FormattingUtils::FormatType::FrequencyHighPass;
        break;

    default:
        cutoffFormat = FormattingUtils::FormatType::FrequencyLowPass;
        break;
    }

    auto spec = Filter::getKnobParamSpecs(Filter::ParamID::Cutoff, filterIndex);

    cutoffKnob.getSlider().textFromValueFunction = [spec, cutoffFormat](double value)
        {
            return FormattingUtils::formatValue(
                static_cast<float>(value),
                cutoffFormat,
                spec.minValue,
                spec.maxValue
            );
        };

    cutoffKnob.getSlider().updateText();
}

void FilterComponent::updateViewForFilterType()
{
    const auto selectedId = typeSelector.getSelectedId() - 1;
    const bool isTalkbox = (selectedId >= 0 && selectedId < static_cast<int>(Filter::Type::Count))
        && static_cast<Filter::Type>(selectedId) == Filter::Type::Talkbox;

    // Default visibility
    slopeLabel.setVisible(true);
    slopeSelector.setVisible(true);
    cutoffKnob.setVisible(true);
    resonanceKnob.setVisible(true);

    vowelLabel.setVisible(false);
    vowelSelector.setVisible(false);
    morphKnob.setVisible(false);
    factorKnob.setVisible(false);

    // Talkbox override
    if (isTalkbox)
    {
        slopeLabel.setVisible(false);
        slopeSelector.setVisible(false);
        cutoffKnob.setVisible(false);
        resonanceKnob.setVisible(false);

        vowelLabel.setVisible(true);
        vowelSelector.setVisible(true);
        morphKnob.setVisible(true);
        factorKnob.setVisible(true);
    }

    resized();
}

void FilterComponent::setLinkableTargets(const std::unordered_map<std::string, Linkable*>& targets)
{
    linkableTargets = targets;
    linkSelector.clear();

    int id = 1;
    linkSelector.addItem("-", id++); // Unlinked default

    for (const auto& [name, ptr] : linkableTargets)
        linkSelector.addItem(name, id++);

    linkSelector.setSelectedId(1, juce::dontSendNotification);
    currentlyLinkedTarget = nullptr;

    linkSelector.onChange = [this]()
        {
            const auto selectedName = linkSelector.getText().toStdString();

            // User selected "-"
            if (selectedName == "-")
            {
                if (currentlyLinkedTarget)
                {
                    if (auto* osc = dynamic_cast<Oscillator*>(currentlyLinkedTarget))
                        osc->setFilter(nullptr);

                    currentlyLinkedTarget = nullptr;
                }
                return;
            }

            // Link to new target
            auto it = linkableTargets.find(selectedName);
            if (it != linkableTargets.end())
            {
                if (currentlyLinkedTarget && currentlyLinkedTarget != it->second)
                {
                    if (auto* prevOsc = dynamic_cast<Oscillator*>(currentlyLinkedTarget))
                        prevOsc->setFilter(nullptr);
                }

                if (auto* osc = dynamic_cast<Oscillator*>(it->second))
                    osc->setFilter(processor.getFilter(this->filterIndex));

                processor.registerFilterLinkOwnership(it->second, this);
                currentlyLinkedTarget = it->second;
            }
        };
}

void FilterComponent::unlinkTarget(Linkable* target)
{
    if (currentlyLinkedTarget == target)
    {
        if (auto* osc = dynamic_cast<Oscillator*>(target))
            osc->setFilter(nullptr);

        currentlyLinkedTarget = nullptr;
        linkSelector.setSelectedId(1, juce::dontSendNotification); // reset to "-"
    }
}

void FilterComponent::setupKnob(juce::AudioProcessorValueTreeState& apvts, Knob& knob, const Filter::KnobParamSpecs& spec)
{
    knob.initialize(apvts, processor, spec.id, spec.name,
        Knob::KnobParams(spec.defaultValue, spec.minValue, spec.maxValue, spec.stepSize, false,
            nullptr, 0),
        Knob::KnobStyle::Rotary);

    knob.bindToParameter();
    processor.registerKnob(&knob);
    addAndMakeVisible(knob);

    knob.getSlider().textFromValueFunction = [spec](double value)
        {
            return FormattingUtils::formatValue(static_cast<float>(value), spec.formatType, spec.minValue, spec.maxValue);
        };

    knob.getSlider().updateText();
}

void FilterComponent::updateGraphFromKnobs()
{
    auto type = static_cast<Filter::Type>(typeSelector.getSelectedId() - 1);

    filterGraph.setType(type);
    filterGraph.setSlope(static_cast<Filter::Slope>(slopeSelector.getSelectedId() - 1));
    filterGraph.setCutoffFrequency(cutoffKnob.getSliderValue());
    filterGraph.setResonance(resonanceKnob.getSliderValue());
    filterGraph.setDrive(driveKnob.getSliderValue());
    filterGraph.setMix(mixKnob.getSliderValue());

    if (type == Filter::Type::Talkbox)
    {
        const auto& talkboxFilter = processor.getFilter(filterIndex)->getTalkboxFilter();
        filterGraph.setTalkboxBands(talkboxFilter.getFormantBandsForGraph());
    }

    filterGraph.generateAxisGridLines(lastGraphArea.toFloat());
    filterGraph.repaint();
    repaint(graphArea);
}