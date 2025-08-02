#include "../../PluginProcessor.h"
#include "Knob.h"

Knob::Knob(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& paramID,
    const juce::String& labelText, const KnobParams& params, KnobStyle style)
    : apvts(apvts), processor(processor)
{
    initialize(apvts, processor, paramID, labelText, params, style);
}

Knob::~Knob()
{
    slider.setLookAndFeel(nullptr);
    cleanup();
}

void Knob::cleanup()
{
    stopTimer();
    attachment.reset();
    processor.getModulationRouter().unregisterTarget(this);
}

void Knob::initialize(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& paramID,
    const juce::String& labelText, const KnobParams& params, KnobStyle style)
{
    this->paramID = paramID;
    this->params = params;
    this->style = style;

    // Set the slider style based on the provided knob style
    switch (style)
    {
    case KnobStyle::Rotary:
        slider.setLookAndFeel(nullptr);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        break;

    case KnobStyle::LinearVertical:
        slider.setLookAndFeel(nullptr);
        customLookAndFeel = std::make_unique<LinearVerticalLookAndFeel>();
        slider.setLookAndFeel(customLookAndFeel.get());
        slider.setSliderStyle(juce::Slider::LinearVertical);
        break;
    }

    updateTheme();

    // Set the width of the text box based on the parameters
    int actualTextBoxWidth = (params.textBoxWidth > 0) ? params.textBoxWidth : textBoxWidth;
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, actualTextBoxWidth, textBoxHeight);

    // Set the range and default value for the slider
    slider.setRange(params.minValue, params.maxValue, params.stepSize);
    slider.setDoubleClickReturnValue(true, params.defaultValue);
    addAndMakeVisible(slider);

    // Set the custom value-to-text formatter
    if (params.valueToText)
    {
        valueToTextFormatter = params.valueToText;
        slider.textFromValueFunction = [this](double value)
            {
                return valueToTextFormatter(static_cast<float>(value));
            };
    }

    // Set up the label for the knob
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(UI::Fonts::defaultFontSize));
    label.setJustificationType(juce::Justification::centredBottom);
    addAndMakeVisible(label);

    // Enable mouse interactions for the slider
    setInterceptsMouseClicks(true, true);
    slider.addMouseListener(this, true);

    // Hook into slider value change to forward via onValueChange callback
    slider.onValueChange = [this]()
        {
            if (onValueChange)
                onValueChange();
        };

    // Ensure text is updated and formatted immediately after initialization
    slider.updateText();
    slider.repaint();

    // Restore modulation state from APVTS
    const auto ids = KnobModulationEngine::getParameterIDsFor(paramID);

    if (auto* modeParam = apvts.getRawParameterValue(ids[0]))
        modEngine.setMode(static_cast<ModulationMode>(static_cast<int>(modeParam->load())));

    if (auto* indexParam = apvts.getRawParameterValue(ids[1]))
        modEngine.setSourceIndex(static_cast<int>(indexParam->load()));

    if (auto* minParam = apvts.getRawParameterValue(ids[2]))
        if (auto* maxParam = apvts.getRawParameterValue(ids[3]))
            modEngine.setRange(minParam->load(), maxParam->load());

    if (modEngine.getMode() == ModulationMode::Envelope)
    {
        processor.getModulationRouter().connectIfAlive(
            { ModulationSourceType::Envelope, modEngine.getSourceIndex() }, this);
    }
    else if (modEngine.getMode() == ModulationMode::LFO)
    {
        processor.getModulationRouter().connectIfAlive(
            { ModulationSourceType::LFO, modEngine.getSourceIndex() }, this);
    }
}

void Knob::bindToParameter()
{
    if (apvts.getParameter(paramID) == nullptr)
    {
        DBG("Knob::bindToParameter() - ERROR: Parameter not found: " + paramID);
        jassertfalse;
        return;
    }

    // Only bind once
    if (!attachment)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    }
}

bool Knob::isLearning() const
{
    return isMidiLearnActive;
}

int Knob::getAssignedMidiCC() const
{
    return midiCC;
}

float Knob::getSliderValue() const
{
    return static_cast<float>(slider.getValue());
}

void Knob::setSliderValue(float value, juce::NotificationType notify)
{
    if (params.isDiscrete)
    {
        int numSteps = static_cast<int>((params.maxValue - params.minValue) / params.stepSize);
        float scaledValue = params.minValue + params.stepSize * static_cast<int>((value * numSteps) + 0.5f);
        scaledValue = juce::jlimit(params.minValue, params.maxValue, scaledValue);
        slider.setValue(scaledValue, notify);
    }
    else
    {
        slider.setValue(value, notify);
    }
}

void Knob::assignMidiCC(int cc)
{
    if (midiCC == cc)
        return;

    midiCC = cc;
    isMidiLearnActive = false;
    isMidiAssigned = true;
    stopTimer();
    repaint();
}

void Knob::handleMidiCC(int ccNumber, float ccValue)
{
    if (midiCC == ccNumber)
    {
        setSliderValue(ccValue);
    }
}

void Knob::forgetMidiCC()
{
    midiCC = -1;
    isMidiAssigned = false;
    isMidiLearnActive = false;
    glowAlpha = 0.4f;
    stopTimer();
    repaint();
}

void Knob::resized()
{
    auto bounds = getLocalBounds();
    slider.setBounds(bounds.removeFromTop(bounds.getHeight() - labelHeight));
    label.setBounds(bounds);
}

void Knob::paint(juce::Graphics& g)
{
    getLookAndFeel().setColour(juce::Slider::thumbColourId, UI::Colors::KnobThumb);

    auto layout = slider.getLookAndFeel().getSliderLayout(slider);
    auto knobBounds = layout.sliderBounds.toFloat();

    const float safeGlow = juce::jlimit(0.0f, 1.0f, glowAlpha);

    switch (style)
    {
    case KnobStyle::Rotary:
    {
        float diameter = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) / 1.5f;
        auto ringArea = juce::Rectangle<float>(diameter, diameter).withCentre(knobBounds.getCentre());

        juce::Path ringPath;
        ringPath.addEllipse(ringArea);

        if (isMidiAssigned)
        {
            g.setColour(UI::Colors::MidiConnected);
            g.strokePath(ringPath, juce::PathStrokeType(strokeThickness));
        }
        else if (isMidiLearnActive)
        {
            g.setColour(UI::Colors::MidiLearning.withAlpha(safeGlow));
            g.strokePath(ringPath, juce::PathStrokeType(strokeThickness));
        }

        break;
    }

    case KnobStyle::LinearVertical:
    {
        const float trackWidth = 4.0f;
        auto trackX = knobBounds.getCentreX() - trackWidth / 2.0f;
        auto trackRect = juce::Rectangle<float>(trackX, knobBounds.getY(), trackWidth, knobBounds.getHeight());

        juce::Path trackPath;
        trackPath.addRoundedRectangle(trackRect, trackWidth / 2.0f);

        if (isMidiAssigned)
        {
            g.setColour(UI::Colors::MidiConnected);
            g.strokePath(trackPath, juce::PathStrokeType(strokeThickness));
        }
        else if (isMidiLearnActive)
        {
            g.setColour(UI::Colors::MidiLearning.withAlpha(safeGlow));
            g.strokePath(trackPath, juce::PathStrokeType(strokeThickness));
        }

        break;
    }
    }

    // Modulation Range Overlay
    const auto mode = modEngine.getMode();
    if (mode == ModulationMode::Envelope || mode == ModulationMode::LFO)
        drawModulationOverlay(g);
}

void Knob::drawModulationOverlay(juce::Graphics& g)
{
    const auto mode = modEngine.getMode();
    const auto [min, max] = modEngine.getRange();

    if (mode == ModulationMode::Manual || min >= max)
        return;

    g.setColour(UI::Colors::ModulationRing);

    const auto layout = slider.getLookAndFeel().getSliderLayout(slider);
    const auto knobBounds = layout.sliderBounds.toFloat();

    switch (style)
    {
    case KnobStyle::Rotary:
    {
        const float diameter = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) / 1.5f;
        const auto ringArea = juce::Rectangle<float>(diameter, diameter).withCentre(knobBounds.getCentre());

        const auto params = slider.getRotaryParameters();
        const float angleStart = juce::jmap(min, 0.0f, 1.0f, params.startAngleRadians, params.endAngleRadians);
        const float angleEnd = juce::jmap(max, 0.0f, 1.0f, params.startAngleRadians, params.endAngleRadians);

        juce::Path modPath;
        modPath.addCentredArc(
            ringArea.getCentreX(), ringArea.getCentreY(),
            ringArea.getWidth() / 2.0f,
            ringArea.getHeight() / 2.0f,
            0.0f, angleStart, angleEnd, true
        );

        g.strokePath(modPath, juce::PathStrokeType(strokeThickness));
        break;
    }

    case KnobStyle::LinearVertical:
    {
        const float trackWidth = 4.0f;
        const float trackX = knobBounds.getCentreX() - trackWidth / 2.0f;
        const auto trackRect = juce::Rectangle<float>(
            trackX, knobBounds.getY(), trackWidth, knobBounds.getHeight()
        );

        const float yStart = trackRect.getY() + (1.0f - max) * trackRect.getHeight();
        const float yEnd = trackRect.getY() + (1.0f - min) * trackRect.getHeight();
        const float height = yEnd - yStart;

        juce::Rectangle<float> modRect(trackX, yStart, trackWidth, height);
        juce::Path modPath;
        modPath.addRoundedRectangle(modRect, trackWidth / 2.0f);

        g.strokePath(modPath, juce::PathStrokeType(strokeThickness));
        break;
    }

    default:
        break;
    }
}

void Knob::mouseDown(const juce::MouseEvent& event)
{
    lastDragPosition = event.position;

    // === SHIFT + CLICK: Begin modulation edit ===
    if (event.mods.isShiftDown())
    {
        modEngine.beginRangeEdit(event.mods.isLeftButtonDown(), event.position);
        return;
    }

    // === RIGHT CLICK: Open modulation menu ===
    if (!event.mods.isRightButtonDown())
        return;

    juce::PopupMenu menu;

    // --- MIDI Controller ---
    menu.addItem(ModMenuID::MidiLearn, "MIDI Controller", true, isMidiAssigned);

    // --- Envelope submenu ---
    juce::PopupMenu envelopeMenu;
    const auto envelopeSources = processor.getAvailableModulationSources(ModulationSourceType::Envelope);
    const auto currentSource = processor.getModulationRouter().getSourceForTarget(this);
    const bool isEnvelopeMod = currentSource.has_value() && currentSource->type == ModulationSourceType::Envelope;

    for (const auto& [id, label] : envelopeSources)
    {
        int itemID = ModMenuID::EnvelopeBase + id.index;
        bool isLinked = processor.isEnvelopeLinkedToOscillator(id.index);
        bool isChecked = isEnvelopeMod && currentSource->index == id.index;
        envelopeMenu.addItem(itemID, label, isLinked, isChecked);
    }
    menu.addSubMenu("Envelope", envelopeMenu);

    // --- LFO submenu ---
    juce::PopupMenu lfoMenu;
    const auto lfoSources = processor.getAvailableModulationSources(ModulationSourceType::LFO);
    const bool isLfoMod = currentSource.has_value() && currentSource->type == ModulationSourceType::LFO;

    for (const auto& [id, label] : lfoSources)
    {
        int itemID = ModMenuID::LfoBase + id.index;
        bool isChecked = isLfoMod && currentSource->index == id.index;
        auto* lfo = processor.getLFO(id.index);
        bool isEnabled = lfo && !lfo->isBypassed();
        lfoMenu.addItem(itemID, label, isEnabled, isChecked);
    }
    menu.addSubMenu("LFO", lfoMenu);

    // --- Clean option ---
    menu.addItem(ModMenuID::Clean, "Clean");

    // === Show menu ===
    menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result)
        {
            if (result == 0)
                return; // Menu dismissed

            // 1) Disconnect any live modulation and clear MIDI CC
            processor.getModulationRouter().disconnect(this);
            forgetMidiCC();

            // 2) Reset all APVTS parameters (main + sub-parameters) to their defaults
            const auto subIDs = KnobModulationEngine::getParameterIDsFor(paramID);

            switch (result)
            {
            case ModMenuID::MidiLearn:
                isMidiLearnActive = true;
                glowAlpha = 0.4f;
                startTimerHz(midiLearnBlinkRateHz);
                break;

            case ModMenuID::Clean:
            {
                // Gather the main parameter ID plus the four _MOD_* children
                juce::StringArray allParamIDs;
                allParamIDs.add(paramID);
                allParamIDs.addArray(subIDs);

                // Loop and reset each parameter in the APVTS
                for (const auto& idToClear : allParamIDs)
                    if (auto* p = apvts.getParameter(idToClear))
                        p->setValueNotifyingHost(p->getDefaultValue());

                break;
            }

            default:
                // --- Envelope selection ---
                if (result >= ModMenuID::EnvelopeBase && result < ModMenuID::EnvelopeBase + NUM_OF_ENVELOPES)
                {
                    int index = result - ModMenuID::EnvelopeBase;
                    processor.getModulationRouter().connect({ ModulationSourceType::Envelope, index }, this);

                    if (auto* modeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(subIDs[0])))
                        modeParam->setValueNotifyingHost(modeParam->convertTo0to1(static_cast<int>(ModulationMode::Envelope)));
                    if (auto* indexParam = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter(subIDs[1])))
                        indexParam->setValueNotifyingHost(indexParam->convertTo0to1(static_cast<float>(index)));
                }
                // --- LFO selection ---
                else if (result >= ModMenuID::LfoBase && result < ModMenuID::LfoBase + NUM_OF_LFOS)
                {
                    int index = result - ModMenuID::LfoBase;
                    processor.getModulationRouter().connect({ ModulationSourceType::LFO, index }, this);

                    if (auto* modeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(subIDs[0])))
                        modeParam->setValueNotifyingHost(modeParam->convertTo0to1(static_cast<int>(ModulationMode::LFO)));
                    if (auto* indexParam = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter(subIDs[1])))
                        indexParam->setValueNotifyingHost(indexParam->convertTo0to1(static_cast<float>(index)));
                }
                break;
            }
        });
}

void Knob::mouseDrag(const juce::MouseEvent& event)
{
    if (modEngine.isEditing())
    {
        modEngine.updateDrag(event.position);
        repaint();
        return;
    }

    const auto mode = modEngine.getMode();
    if (mode == ModulationMode::Envelope || mode == ModulationMode::LFO)
    {
        // Perform modulation offset shift
        float deltaY = event.position.y - lastDragPosition.y;
        modEngine.shiftRange(deltaY);
        lastDragPosition = event.position;

        // === Commit the live-shifted range immediately ===
        const auto ids = KnobModulationEngine::getParameterIDsFor(paramID);
        if (auto* minParam = apvts.getParameter(ids[2]))
            minParam->setValueNotifyingHost(modEngine.getRange().first);
        if (auto* maxParam = apvts.getParameter(ids[3]))
            maxParam->setValueNotifyingHost(modEngine.getRange().second);

        repaint();
        return;
    }

    // Fallback to default slider behavior in Manual/MIDI mode
    juce::Component::mouseDrag(event);
}

void Knob::mouseUp(const juce::MouseEvent& event)
{
    if (modEngine.isEditing())
    {
        modEngine.endRangeEdit();
        const auto ids = KnobModulationEngine::getParameterIDsFor(paramID);

        if (auto* minParam = apvts.getParameter(ids[2]))
            minParam->setValueNotifyingHost(modEngine.getRange().first);

        if (auto* maxParam = apvts.getParameter(ids[3]))
            maxParam->setValueNotifyingHost(modEngine.getRange().second);

        repaint();
    }
    // Otherwise, if in Envelope or LFO modulation mode, commit the
    // “shiftRange” delta that was applied during mouseDrag
    else if (modEngine.getMode() == ModulationMode::Envelope
        || modEngine.getMode() == ModulationMode::LFO)
    {
        const auto ids = KnobModulationEngine::getParameterIDsFor(paramID);

        if (auto* minParam = apvts.getParameter(ids[2]))
            minParam->setValueNotifyingHost(modEngine.getRange().first);

        if (auto* maxParam = apvts.getParameter(ids[3]))
            maxParam->setValueNotifyingHost(modEngine.getRange().second);

        repaint();
    }
    else
    {
        // Fallback to the default Component behavior
        juce::Component::mouseUp(event);
    }
}

void Knob::timerCallback()
{
    if (!isShowing())
    {
        stopTimer();
        return;
    }

    // otherwise, continue animating…
    glowAlpha = increasingGlow ? glowAlpha + glowIncrement
        : glowAlpha - glowIncrement;
    if (glowAlpha >= glowMax) increasingGlow = false;
    if (glowAlpha <= glowMin) increasingGlow = true;
    repaint();
}

void Knob::updateTheme()
{
    label.setColour(juce::Label::textColourId, UI::Colors::KnobTextBoxText);

    slider.setColour(juce::Slider::thumbColourId, UI::Colors::KnobThumb);
    slider.setColour(juce::Slider::textBoxTextColourId, UI::Colors::KnobTextBoxText);
    slider.setColour(juce::Slider::textBoxOutlineColourId, UI::Colors::KnobTextBoxOutline);

    switch (style)
    {
    case KnobStyle::Rotary:
        slider.setColour(juce::Slider::rotarySliderFillColourId, UI::Colors::KnobSliderFill);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, UI::Colors::KnobSliderOutline);
        break;

    case KnobStyle::LinearVertical:
        slider.setColour(juce::Slider::trackColourId, UI::Colors::KnobSliderFill);
        slider.setColour(juce::Slider::backgroundColourId, UI::Colors::KnobSliderOutline);
        break;
    }

    repaint();
}

void Knob::clearModulation()
{
    modEngine.clear();
}

void Knob::setModulationMode(ModulationMode mode)
{
    modEngine.setMode(mode);

    // Always disable text entry in all modes
    slider.setTextBoxIsEditable(false);

    switch (mode)
    {
    case ModulationMode::Manual:
    case ModulationMode::Midi:
        slider.setInterceptsMouseClicks(true, true);  // allow drag + wheel
        break;

    case ModulationMode::Envelope:
    case ModulationMode::LFO:
        slider.setInterceptsMouseClicks(false, false);

        // Restore modulation range from APVTS if available
        if (auto* minParam = apvts.getParameter(paramID + "_MOD_MIN"))
        {
            if (auto* maxParam = apvts.getParameter(paramID + "_MOD_MAX"))
            {
                float minValue = minParam->getValue();
                float maxValue = maxParam->getValue();
                modEngine.setRange(minValue, maxValue);
            }
        }
        break;
    }
}

ModulationMode Knob::getModulationMode() const
{
    return modEngine.getMode();
}

bool Knob::isModulated() const
{
    return modEngine.getMode() == ModulationMode::Envelope || modEngine.getMode() == ModulationMode::LFO;
}

void Knob::setModulationValue(float normalizedValue)
{
    modEngine.setValue(normalizedValue);

    if (modEngine.getMode() == ModulationMode::Envelope || modEngine.getMode() == ModulationMode::LFO)
    {
        const auto [min, max] = modEngine.getRange();
        float modScaled = min + (max - min) * normalizedValue;
        float final = juce::jlimit(0.0f, 1.0f, modScaled);

        if (auto* param = apvts.getParameter(paramID))
            param->setValueNotifyingHost(final);
    }
}

void Knob::setModulationRange(float minNormalized, float maxNormalized)
{
    modEngine.setRange(minNormalized, maxNormalized);
}

std::pair<float, float> Knob::getModulationRange() const
{
    return modEngine.getRange();
}