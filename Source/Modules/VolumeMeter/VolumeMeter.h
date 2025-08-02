#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

class DigitalSynthesizerAudioProcessor;
/**
 * @class VolumeMeter
 * @brief A stereo volume meter component displaying left and right channel levels in dB.
 */
class VolumeMeter : public juce::Component, private juce::Timer
{
public:
    static constexpr float minDisplayDb = -50.0f;          ///< Minimum visible dB level for display purposes.
    static constexpr float maxDisplayDb = 5.0f;            ///< Maximum visible dB level for display purposes.
    static constexpr float initialVolumeDb = minDisplayDb; ///< Initial dB level representing silence or no signal.

    /** 
     * @brief Constructor 
     */
    VolumeMeter();

    /** 
     * @brief Destructor
     */
    ~VolumeMeter() override;

    /**
     * @brief Manually stops the timer and nullifies the processor reference.
     */
    void cleanup();

    /**
    * @brief Sets the processor reference and channel to monitor.
    * @param processor A reference to the audio processor.
    */
    void setAudioProcessorReference(DigitalSynthesizerAudioProcessor& processor);

    /**
     * @brief Updates the visual appearance of the component to match the current theme.
     */
    void updateTheme();

    /**
     * @brief Sets the dB levels for both left and right channels.
     * @param leftDb  Level of the left channel in dB
     * @param rightDb Level of the right channel in dB
     */
    void setLevels(float leftDb, float rightDb);

    /**
     * @brief Repaints and renders the meter graphics.
     * @param g JUCE graphics context
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Handles resizing of the component.
     */
    void resized() override;

    /**
     * @brief Gets the total width of the volume meter in pixels.
     * @return total width in pixels.
    */
    int getTotalWidth() const noexcept;

private:
    DigitalSynthesizerAudioProcessor* processor = nullptr; ///< Pointer to the associated audio processor.

    float leftLevelDb{ initialVolumeDb };       ///< Current level of the left channel in dB.
    float rightLevelDb{ initialVolumeDb };      ///< Current level of the right channel in dB.
    float leftDisplayDb{ initialVolumeDb };     ///< Displayed level for left channel (smoothed).
    float rightDisplayDb{ initialVolumeDb };    ///< Displayed level for right channel (smoothed).

    static constexpr int totalMeterWidth = 120; ///< Preferred width of the volume meter in pixels.
    static constexpr int meterWidth = 20;       ///< Width of each volume meter (L and R).
    static constexpr int meterSpacing = 7;      ///< Spacing between the L and R meters.
    static constexpr int meterMargin = 10;      ///< Inner margin between the edge and meters.

    /**
     * @brief Timer callback to update the smoothed display levels.
     */
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolumeMeter)
};
