#pragma once

#include "../../Common.h"
#include "KnobModulation.h"
#include <JuceHeader.h>

class DigitalSynthesizerAudioProcessor;

/**
 * @class Knob
 * @brief A rotary knob component with MIDI learn functionality.
 */
class Knob : public juce::Component, public ModulatableParameter, private juce::Timer
{
public:
    /**
     * @enum KnobStyle
     * @brief Defines visual styles for the Knob slider.
     */
    enum class KnobStyle
    {
        Rotary,         ///< Circular knob
        LinearVertical  ///< Vertical slider
    };

    /**
     * @struct KnobParams
     * @brief Defines settings for a knob, including min/max values and step size.
     */
    struct KnobParams
    {
        float defaultValue;
        float minValue;
        float maxValue;
        float stepSize;
        bool isDiscrete;
        int textBoxWidth = 0;

        std::function<juce::String(float)> valueToText;

        KnobParams() noexcept
            : defaultValue(0.0f),
            minValue(0.0f),
            maxValue(1.0f),
            stepSize(0.01f),
            isDiscrete(false),
            valueToText(nullptr) {}

        KnobParams(float defaultValue, float minValue, float maxValue, float stepSize, bool isDiscrete)
            : defaultValue(defaultValue),
            minValue(minValue),
            maxValue(maxValue),
            stepSize(stepSize),
            isDiscrete(isDiscrete),
            valueToText(nullptr),
            textBoxWidth(0) {}

        KnobParams(float defaultValue, float minValue, float maxValue, float stepSize, bool isDiscrete,
            std::function<juce::String(float)> valueToText, int textBoxWidth)
            : defaultValue(defaultValue),
            minValue(minValue),
            maxValue(maxValue),
            stepSize(stepSize),
            isDiscrete(isDiscrete),
            valueToText(std::move(valueToText)),
            textBoxWidth(textBoxWidth) {}
    };

    /**
     * @brief Constructs a Knob object.
     * @param apvts Reference to the AudioProcessorValueTreeState for parameter binding.
     * @param Reference to the pluginProcessor.
     * @param paramID The ID of the parameter associated with this knob.
     * @param labelText The text label displayed under the knob.
     * @param params Knob behavior parameters including range and stepping.
     * @param style The visual style of the knob (rotary or linear vertical).
     */
    Knob(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& paramID,
        const juce::String& labelText, const KnobParams& params, KnobStyle style = KnobStyle::Rotary);

    /**
     * @brief Destructor to safely release LookAndFeel.
     */
    ~Knob();

    /**
     * @brief Releases attachment to avoid accessing destroyed APVTS or slider.
     */
    void cleanup();

    /**
     * @brief Initializes the knob after default construction.
     * @param apvts Reference to the AudioProcessorValueTreeState for parameter binding.
     * @param Reference to the pluginProcessor.
     * @param paramID The ID of the parameter associated with this knob.
     * @param labelText The text label displayed under the knob.
     * @param params Knob behavior parameters including range and stepping.
     * @param style The visual style of the knob (rotary or linear vertical).
     */
    void initialize(juce::AudioProcessorValueTreeState& apvts, DigitalSynthesizerAudioProcessor& processor, const juce::String& paramID,
        const juce::String& labelText, const KnobParams& params, KnobStyle style = KnobStyle::Rotary);

    /**
     * @brief Binds this knob to the APVTS parameter using SliderAttachment.
     */
    void bindToParameter();

    /** 
     * @brief Exposes the APVTS reference (for reset logic).
     */
    juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }

    /** 
     * @brief Returns the associated parameter ID.
     */
    const juce::String& getParamID() const { return paramID; }

    /**
     * @brief Checks if the knob is in MIDI Learn mode.
     * @return `true` if the knob is in MIDI Learn mode, `false` otherwise.
     */
    bool isLearning() const;

    /**
     * @brief Gets the assigned MIDI CC number.
     * @return The assigned MIDI CC number, or -1 if unassigned.
     */
    int getAssignedMidiCC() const;

    /**
     * @brief Returns a reference to the internal JUCE slider.
     * Used to attach listeners or perform custom slider operations.
     */
    juce::Slider& getSlider() { return slider; }

    /**
     * @brief Retrieves the current slider value.
     * @return The current value of the slider.
     */
    float getSliderValue() const;

    /**
     * @brief Sets the knob's slider value.
     * @param value New value to set on the slider.
     * @param notify Determines whether to notify the host of the change.
     */
    void setSliderValue(float value, juce::NotificationType notify = juce::sendNotificationSync);

    /**
     * @brief Assigns a MIDI CC number to this knob.
     * @param cc The MIDI CC number to assign.
     */
    void assignMidiCC(int cc);

    /**
     * @brief Handles incoming MIDI CC messages and updates the knob value.
     * @param ccNumber The incoming MIDI CC number.
     * @param ccValue The normalized MIDI CC value (0.0 to 1.0).
     */
    void handleMidiCC(int ccNumber, float ccValue);

    /**
     * @brief Unassigns any MIDI CC mapping from this knob.
     */
    void forgetMidiCC();

    /**
     * @brief Handles component resizing and layout adjustments.
     */
    void resized() override;

    /** 
     * @brief Custom painting for visual feedback like MIDI learn indication.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Draws a visual representation of the modulation range on top of the knob.
     * @param g The graphics context to draw with.
     */
    void drawModulationOverlay(juce::Graphics& g);

    /**
     * @brief Handles right-click interactions for the MIDI learn context menu.
     * @param event Mouse event containing click details.
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse dragging to edit modulation min/max when Shift is held.
     * @param event Mouse drag event.
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse release to finalize modulation edit.
     * @param event Mouse release event.
     */
    void mouseUp(const juce::MouseEvent& event) override;


    /** 
     * @brief Timer callback for handling glow animation in MIDI Learn mode.
     */
    void timerCallback() override;

    /**
    * @brief Updates the visual appearance of the knob to match the current theme.
    */
    void updateTheme();

    /**
     * @brief Callback triggered when the slider value changes. 
     */
    std::function<void()> onValueChange;

    /**
     * @brief Clears modulation values and resets bounds to full range.
     */
    void clearModulation() override;

    /**
     * @brief Sets the modulation mode for this knob.
     */
    void setModulationMode(ModulationMode mode) override;

    /**
     * @brief Returns the currently active modulation mode.
     */
    ModulationMode getModulationMode() const override;

private:
    DigitalSynthesizerAudioProcessor& processor; ///< Reference to the processor.
    juce::AudioProcessorValueTreeState& apvts;   ///< Reference to the APVTS.

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment; ///< Parameter attachment.
    juce::String paramID; ///< ID of the linked parameter.
    KnobParams params;    ///< Parameter configuration.

    KnobStyle style;     ///< Determines visual style of the knob.
    juce::Slider slider; ///< The rotary slider component.
    juce::Label label;   ///< Label displayed below the knob.

    bool isMidiLearnActive = false;                 ///< True if MIDI Learn is active.
    bool isMidiAssigned  = false;                   ///< True if a MIDI CC is assigned.
    float glowAlpha = 0.4f;                         ///< Glow effect alpha for MIDI Learn.
    bool increasingGlow = true;                     ///< Glow animation direction.
    int midiCC = -1;                                ///< Assigned MIDI CC (-1 if none).
    static constexpr int midiLearnBlinkRateHz = 30; ///< MIDI Learn blink rate (Hz).
    static constexpr float glowIncrement = 0.05f;   ///< Glow intensity increment.
    static constexpr float glowMax = 1.0f;          ///< Maximum glow intensity.
    static constexpr float glowMin = 0.2f;          ///< Minimum glow intensity.
    std::function<juce::String(float)> valueToTextFormatter; ///< Optional value formatter.

    static constexpr int textBoxWidth = 40;   ///< Width of value textbox.
    static constexpr int textBoxHeight = 20;  ///< Height of value textbox.
    static constexpr int strokeThickness = 6; ///< Knob stroke thickness.
    static constexpr int labelHeight = 20;    ///< Height of the label.

    /**
     * @brief Menu item IDs for modulation source routing.
     */
    enum ModMenuID
    {
        MidiLearn = 1,     ///< MIDI Learn command.
        EnvelopeBase = 10, ///< Envelope 0 = 10, etc.
        LfoBase = 20,      ///< LFO 0 = 20, etc.
        Clean = 99         ///< Clear modulation.
    };

    KnobModulationEngine modEngine;      ///< Modulation logic engine.
    juce::Point<float> lastDragPosition; ///< Cached drag position.

    /**
     * @brief Checks whether this knob currently has an active modulation connection.
     * @return true if this knob is connected to an Envelope or LFO, false otherwise.
     */
    bool isModulated() const;

    /**
     * @brief Applies a normalized modulation value (0.0–1.0) to the knob.
     * Only active in Envelope or LFO mode.
     */
    void setModulationValue(float normalizedValue) override;

    /**
     * @brief Sets the modulation range boundaries in normalized [0.0–1.0] space.
     */
    void setModulationRange(float minNormalized, float maxNormalized) override;

    /**
     * @brief Retrieves the current modulation boundaries.
     * @return A pair {min, max}, both in normalized [0.0, 1.0] space.
     */
    std::pair<float, float> getModulationRange() const override;

    /**
     * @brief Custom LookAndFeel for vertical sliders inside knobs.
     * Draws a filled track with a rounded thumb using consistent styling.
     */
    class LinearVerticalLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPos, float minSliderPos, float maxSliderPos,
            const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            if (style == juce::Slider::LinearVertical)
            {
                const float trackWidth = 4.0f;
                auto trackX = x + width / 2.0f - trackWidth / 2.0f;

                // Background track
                g.setColour(slider.findColour(juce::Slider::backgroundColourId));
                g.fillRect(juce::Rectangle<float>(trackX, static_cast<float>(y), trackWidth, static_cast<float>(height)));

                // Active fill
                g.setColour(slider.findColour(juce::Slider::trackColourId));
                g.fillRect(juce::Rectangle<float>(trackX, sliderPos, trackWidth, (float)(y + height - sliderPos)));

                // Thumb (circle)
                const float thumbSize = 10.0f;
                g.setColour(slider.findColour(juce::Slider::thumbColourId));

                float centerX = x + width * 0.5f - thumbSize * 0.5f;
                g.fillEllipse(centerX, sliderPos - thumbSize * 0.5f, thumbSize, thumbSize);

            }
            else
            {
                LookAndFeel_V4::drawLinearSlider(g, x, y, width, height,
                    sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }
    };

    std::unique_ptr<LinearVerticalLookAndFeel> customLookAndFeel;
};
