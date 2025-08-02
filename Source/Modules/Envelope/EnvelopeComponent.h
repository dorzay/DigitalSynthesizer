#pragma once

#include "../../Common.h"
#include "../../PluginProcessor.h"
#include "../Knob/Knob.h"
#include "../ComboBox/ComboBox.h"
#include "Envelope.h"
#include "EnvelopeGraph.h"
#include <JuceHeader.h>

/**
 * @class EnvelopeComponent
 * @brief GUI for controlling a single ADSR envelope.
 */
class EnvelopeComponent : public juce::Component, private juce::Slider::Listener
{
public:
    /**
     * @brief Constructs an EnvelopeComponent.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param processor Reference to the owning processor.
     * @param name Display name for the envelope.
     * @param index Zero-based index of this envelope instance.
     * @param targets Map of display names to Linkable targets for linking.
     */
    EnvelopeComponent(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& name, int index, const std::unordered_map<std::string, Linkable*>& targets);

    /** @brief Destructor. Releases APVTS attachments to prevent UI crashes. */
    ~EnvelopeComponent() override;

    /**
     * @brief Registers modulation-related parameters for all knobs.
     * @param index Zero-based index of the envelope instance.
     * @param layout Reference to the APVTS parameter layout.
     */
    static void registerModulationParameters(int index, juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    /**
     * @brief Sets the list of linkable targets for this envelope.
     * @param targets Map of display names to Linkable pointers.
     */
    void setLinkableTargets(const std::unordered_map<std::string, Linkable*>& targets);

    /**
     * @brief Unlinks this envelope from a target, resetting the link to null.
     * @param target The Linkable instance to unlink.
     */
    void unlinkTarget(Linkable* target);

    /**
     * @brief Returns the total width of the component in pixels.
     * @return Width of this EnvelopeComponent.
     */
    static int getTotalWidth();

    /**
     * @brief Renders the envelope component visuals.
     * @param g Graphics context used for drawing.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Resizes and lays out all child components.
     */
    void resized() override;

    /**
     * @brief Applies the current color theme to all subcomponents.
     */
    void updateTheme();

private:
    DigitalSynthesizerAudioProcessor& processor;                ///< Reference to owning processor
    std::unordered_map<std::string, Linkable*> linkableTargets; ///< Available linkable targets
    int envelopeIndex;                                          ///< Zero-based index of this envelope

    /**
     * @brief Helper to initialize and configure a knob.
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param knob Reference to the Knob to set up.
     * @param spec The parameter specification for the knob.
     * @param formatType Formatting type for the knob display.
     */
    void setupKnob(juce::AudioProcessorValueTreeState& apvts, Knob& knob, const Envelope::KnobParamSpecs& spec, FormattingUtils::FormatType formatType);

    /**
     * @brief Draws the ADSR envelope curve within the designated area.
     * @param g Graphics context to draw onto.
     */
    void drawEnvelopeGraph(juce::Graphics& g);

    /**
     * @brief Called when a slider value changes to update the envelope graph.
     * @param slider The slider that triggered the event.
     */
    void sliderValueChanged(juce::Slider* slider) override;

    static constexpr int titleHeight = 40;                        ///< Height of the title label row
    static constexpr int selectorHeight = 50;                     ///< Height of selector rows
    static constexpr int knobRowHeight = 120;                     ///< Height of the knob row
    static constexpr int totalWidth = 600;                        ///< Total fixed width

    static constexpr int rowPadding = 5;                          ///< Padding between rows
    static constexpr int knobSpacing = 10;                        ///< Spacing between knobs

    static constexpr int selectorLabelWidth = 60;                 ///< Width of selector labels
    static constexpr int selectorComboWidth = 110;                ///< Width of selector ComboBoxes
    static constexpr int overideTextBoxWidth = 120;               ///< Width of override text box

    static constexpr int labelSelectorOffsetX = 10;               ///< X offset for selector labels

    static constexpr int graphLeftMargin = 33;                    ///< Left margin for the graph
    static constexpr int graphReduceX = 10;                       ///< X-axis reduction for graph bounds
    static constexpr int graphReduceY = 20;                       ///< Y-axis reduction for graph bounds
    static constexpr int graphTranslateX = -10;                   ///< X translation for graph drawing
    static constexpr int graphTranslateY = -12;                   ///< Y translation for graph drawing
    static constexpr int knobOffsetY = 10;                        ///< Y offset for knobs

    juce::Label titleLabel;                                       ///< Title label (e.g., "Envelope 1")
    Knob attackKnob;                                              ///< Attack time control
    Knob decayKnob;                                               ///< Decay time control
    Knob sustainKnob;                                             ///< Sustain level control
    Knob releaseKnob;                                             ///< Release time control
    juce::Label modeLabel;                                        ///< Label for mode selector
    ComboBox modeSelector;                                        ///< ComboBox for envelope mode
    juce::Label linkLabel;                                        ///< Label for link target selector
    ComboBox linkTargetSelector;                                  ///< ComboBox for link target selection
    juce::Rectangle<int> envelopeGraphArea;                       ///< Bounds for drawing envelope graph
    Linkable* currentlyLinkedTarget = nullptr;                    ///< Currently linked target

    std::vector<std::tuple<Knob*, Envelope::ADSR, FormattingUtils::FormatType>> knobs; ///< Knob list for loop-based init

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeSelectorAttachment; ///< Attachment for mode selector
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> linkSelectorAttachment; ///< Attachment for link target selector

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeComponent)
};
