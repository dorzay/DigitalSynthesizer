#pragma once

#include "../../Common.h"
#include "../../Modules/Filter/Filter.h"
#include "../Filter/TalkBoxFilter.h"
#include <JuceHeader.h>

/** @brief Describes a single axis grid line and its label. */
struct FilterGraphGridLine
{
    /** @brief Default constructor. */
    FilterGraphGridLine()
        : line(), labelPosition(), label(), justification(juce::Justification::centred) {}

    juce::Line<float>  line;           ///< Line segment
    juce::Point<float> labelPosition;  ///< Label draw position
    juce::String label;                ///< Label text
    juce::Justification justification; ///< Label alignment
};

/**
 * @class FilterGraph
 * @brief A real-time frequency response graph for visualizing filter behavior.
 */
class FilterGraph : public juce::Component
{
public:
    /**
     * @brief Constructs a FilterGraph component.
     */
    FilterGraph();

    /**
     * @brief Sets the sample rate for frequency response calculations.
     * @param newSampleRate The current audio sample rate in Hz.
     */
    void setSampleRate(double newSampleRate);

    /**
     * @brief Sets the filter type to visualize (LowPass, HighPass, BandPass).
     * @param newType The filter type enum from Filter::Type.
     */
    void setType(Filter::Type newType);

    /**
     * @brief Sets the filter slope to visualize (12 dB or 24 dB).
     * @param newSlope The filter slope enum from Filter::Slope.
     */
    void setSlope(Filter::Slope newSlope);

    /**
     * @brief Sets the filter's cutoff frequency.
     * @param hz Cutoff frequency in Hz, clamped to valid range.
     */
    void setCutoffFrequency(float hz);

    /**
     * @brief Sets the resonance (Q factor) of the filter.
     * @param res Resonance value in [0.0 – 1.0] range.
     */
    void setResonance(float res);

    /**
     * @brief Sets the drive amount.
     * @param drive Drive value in [0.0 – 1.0] range.
     */
    void setDrive(float drive);

    /**
     * @brief Sets the mix level.
     * @param mix Mix value in [0.0 – 1.0] range, controlling graph opacity.
     */
    void setMix(float mix);

    /**
     * @brief Overridden JUCE function. Renders the graph visuals.
     * @param g The JUCE graphics context to draw into.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Returns the list of X-axis (frequency) grid lines.
     * @return Const reference to vector of FilterGraphGridLine.
     */
    const std::vector<FilterGraphGridLine>& getXGridLines() const;

    /**
     * @brief Returns the list of Y-axis (dB) grid lines.
     * @return Const reference to vector of FilterGraphGridLine.
     */
    const std::vector<FilterGraphGridLine>& getYGridLines() const;

    /**
     * @brief Computes and stores X (frequency) and Y (dB) grid lines.
     * @param plotArea The bounds of the inner graph area used for plotting.
     */
    void generateAxisGridLines(juce::Rectangle<float> plotArea);

    /**
     * @brief Sets the Talkbox formant bands for rendering.
     * @param bands Array of formant band filters (freq, Q, gain).
     */
    void setTalkboxBands(const std::array<TalkboxFilter::FormantBand, TalkboxFilter::numFormants>& bands);

private:
    double sampleRate = 44100.0;                 ///< Current sample rate
    Filter::Type type = Filter::Type::LowPass;   ///< Selected filter type
    Filter::Slope slope = Filter::Slope::dB24;   ///< Selected slope
    float cutoffHz = 1000.0f;                    ///< Cutoff frequency in Hz
    float resonance = 0.0f;                      ///< Resonance amount [0.0 – 1.0]
    float drive = 1.0f;                          ///< Drive amount [0.0 – 1.0]
    float mix = 1.0f;                            ///< Mix level [0.0 – 1.0]

    juce::dsp::IIR::Coefficients<float>::Ptr coeff1; ///< First filter stage
    juce::dsp::IIR::Coefficients<float>::Ptr coeff2; ///< Second filter stage (24 dB only)

    static constexpr int numFrequencyBins = 1024;   ///< Number of frequency points
    static constexpr float minFrequency = 20.0f;    ///< Minimum frequency (Hz)
    static constexpr float maxFrequency = 20000.0f; ///< Maximum frequency (Hz)
    static constexpr float minDecibels = -55.0f;    ///< Minimum dB shown
    static constexpr float dBInterval = 5.0f;       ///< dB grid spacing
    float maxDecibels = 25.0f;                      ///< Maximum dB shown (dynamic)

    const std::vector<float> frequencyTicks{        ///< Frequency ticks to display
        20.0f, 100.0f, 1000.0f, 5000.0f, 20000.0f
    };

    std::array<TalkboxFilter::FormantBand, TalkboxFilter::numFormants> talkboxBands{};  ///< Formant band data

    std::vector<FilterGraphGridLine> xGridLines; ///< Frequency axis lines
    std::vector<FilterGraphGridLine> yGridLines; ///< Decibel axis lines
    juce::Rectangle<float> lastPlotArea;         ///< Last used plot bounds
 
    /**
     * @brief Updates the internal IIR coefficients.
     */
    void updateCoefficients();

    /**
     * @brief Computes normalized X position for a frequency.
     * @param freq Frequency in Hz.
     * @return Normalized [0.0 – 1.0].
     */
    float getLogFrequencyPosition(float freq) const;

    /**
     * @brief Converts a dB value to Y pixel coordinate.
     * @param dB dB value.
     * @param top Top Y boundary.
     * @param bottom Bottom Y boundary.
     * @return Y pixel position.
     */
    float getDecibelY(float dB, float top, float bottom) const;

    /**
     * @brief Draws the background grid lines.
     * @param g Graphics context.
     * @param plotArea Plot area bounds.
     */
    void drawGrid(juce::Graphics& g, juce::Rectangle<float> plotArea);

    /**
     * @brief Draws the frequency response curve.
     * @param g Graphics context.
     * @param plotArea Plot area bounds.
     */
    void drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> plotArea);

    /**
     * @brief Computes the magnitude response at each bin.
     * @param magnitudes Vector to fill with magnitudes.
     */
    void generateFrequencyResponse(std::vector<float>& magnitudes);

    /**
     * @brief Computes Q factor from resonance value.
     * @return Q factor for filter design.
     */
    float computeQFromResonance() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterGraph)
};