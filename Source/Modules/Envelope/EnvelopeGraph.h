#pragma once

#include "../../Common.h"
#include <vector>
#include <cmath>
#include <limits>
#include <juce_graphics/juce_graphics.h>
#include <JuceHeader.h>

/** @brief A single ADSR envelope point: time (ms) and amplitude [0.0–1.0]. */
struct EnvelopePoint
{
    float timeMs;      ///< Time in milliseconds
    float amplitude;   ///< Amplitude value (0.0 to 1.0)
};

/** @brief A grid line with label, position, and justification. */
struct GridLine
{
    juce::Line<float> line;                                           ///< Line segment
    juce::String label;                                               ///< Label text
    juce::Point<float> labelPosition;                                 ///< Label draw position
    juce::Justification justification = juce::Justification::centred; ///< Label alignment
};

/**
 * @class EnvelopeGraph
 * @brief Compute ADSR points, path, and grid for drawing.
 */
class EnvelopeGraph
{
public:
    /**
     * @brief Compute ADSR points (time vs. amplitude).
     * @param attackMs Attack time in ms.
     * @param decayMs Decay time in ms.
     * @param sustainLevel Sustain level [0.0–1.0].
     * @param releaseMs Release time in ms.
     * @param minDisplayMs Min display duration in ms.
     * @return List of EnvelopePoint pairs.
     */
    static std::vector<EnvelopePoint> getEnvelopePoints(float attackMs, float decayMs, float sustainLevel, float releaseMs, float minDisplayMs = 100.0f);

    /**
     * @brief Get a X-axis limit.
     * @param totalMs Total time in ms.
     * @param step Round step in ms (default: 500).
     * @param minLimit Min allowed limit (default: 500).
     * @param maxLimit Max allowed limit (default: 15000).
     * @return Clamped axis limit in ms.
     */
    static float getRelaxedXLimit(float totalMs, float step = 500.0f, float minLimit = 500.0f, float maxLimit = 15000.0f);

    /**
     * @brief Get total ADSR duration (attack + decay + release).
     * @param attackMs Attack time in ms.
     * @param decayMs Decay time in ms.
     * @param releaseMs Release time in ms.
     * @return Sum of times in ms.
     */
    static float getTotalDuration(float attackMs, float decayMs, float releaseMs);

    /**
     * @brief Find first time where amplitude >= threshold.
     * @param points List of EnvelopePoint.
     * @param amplitudeThreshold Threshold value.
     * @return Time in ms, or -1 if not found.
     */
    static float getTimeToAmplitude(const std::vector<EnvelopePoint>& points, float amplitudeThreshold);

    /**
     * @brief Set ADSR parameters for graph generation.
     * @param attackMs  Attack time in ms.
     * @param decayMs   Decay time in ms.
     * @param sustainLevel Sustain level [0.0–1.0].
     * @param releaseMs Release time in ms.
     */
    void setParameters(float attackMs, float decayMs, float sustainLevel, float releaseMs);

    /**
     * @brief Set pixel bounds for drawing the graph.
     * @param bounds Rectangle area.
     */
    void setGraphBounds(juce::Rectangle<int> bounds);

    /** @brief Generate path and grid lines. */
    void generate();

    /**
     * @brief Get the envelope curve path.
     * @return JUCE Path of the envelope.
     */
    const juce::Path& getEnvelopePath() const;

    /**
     * @brief Get horizontal (Y-axis) grid lines.
     * @return Vector of GridLine.
     */
    const std::vector<GridLine>& getYGridLines() const;

    /**
     * @brief Get vertical (X-axis) grid lines.
     * @return Vector of GridLine.
     */
    const std::vector<GridLine>& getXGridLines() const;

private:
    float attackMs = 0.0f;            ///< Attack time in ms
    float decayMs = 0.0f;             ///< Decay time in ms
    float sustainLevel = 0.0f;        ///< Sustain level [0.0–1.0]
    float releaseMs = 0.0f;           ///< Release time in ms

    juce::Rectangle<int> graphBounds; ///< Graph area
    juce::Path envelopePath;          ///< Curve path
    std::vector<GridLine> xGridLines; ///< X-axis lines
    std::vector<GridLine> yGridLines; ///< Y-axis lines
    float xLimit = 1000.0f;           ///< Relaxed max ms
                                      
    const float yAxisExtra = 0.05f;   ///< Extra Y-axis headroom

    /**
     * @brief Compute the envelope curve path based on ADSR parameters.
     */
    void computeEnvelopePath();

    /**
     * @brief Compute Y-axis grid lines (amplitude grid).
     */
    void computeYGridLines();

    /**
     * @brief Compute X-axis grid lines (time grid).
     */
    void computeXGridLines();
};
