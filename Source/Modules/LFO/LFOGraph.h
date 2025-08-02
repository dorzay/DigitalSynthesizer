#pragma once

#include "LFO.h"
#include <JuceHeader.h>

/**
 * @class LFOGraph
 * @brief Renders a visual waveform representation of the LFO.
 */
class LFOGraph
{
public:
    /**
     * @brief Represents a labeled grid line on the graph.
     */
    struct GridLine
    {
        juce::Line<float> line;          ///< The graphical line to draw.
        juce::String label;              ///< Text label to show near the line.
        juce::Point<int> labelPosition;  ///< Pixel position for label.
        juce::Justification justification = juce::Justification::centredLeft; ///< Label justification.
    };

    /**
     * @brief Sets the LFO parameters to be rendered.
     * @param type The LFO waveform type (sine, triangle, square, steps).
     * @param shape The morphing shape parameter [0.0 – 1.0].
     * @param freqHz Frequency in Hz (used to determine x-axis scale).
     * @param steps Number of steps (used only in Steps mode).
     */
    void setParameters(LFO::Type type, float shape, float freqHz, int steps);

    /**
     * @brief Sets the drawable area of the graph in pixels.
     * @param bounds Rectangle area inside which to render the waveform and grid.
     */
    void setGraphBounds(juce::Rectangle<int> bounds);

    /**
     * @brief Links a live LFO instance for sample-based waveform generation.
     * @param lfoPtr Pointer to the LFO object.
     */
    void setLFOReference(const LFO* lfoPtr);

    /**
     * @brief Regenerates the waveform path and grid lines.
     * Should be called after setting new parameters, LFO reference, or bounds.
     */
    void generate();

    /**
     * @brief Returns the waveform path for rendering.
     * @return A const reference to the juce::Path representing the waveform.
     */
    const juce::Path& getLFOPath() const;

    /**
     * @brief Returns the horizontal (X-axis) grid lines and labels.
     * @return A const reference to a list of GridLine structures.
     */
    const std::vector<GridLine>& getXGridLines() const;

    /**
     * @brief Returns the vertical (Y-axis) grid lines and labels.
     * @return A const reference to a list of GridLine structures.
     */
    const std::vector<GridLine>& getYGridLines() const;

private:
    const LFO* lfo = nullptr;               ///< Pointer to live LFO instance for sample rendering.
    static constexpr int numSamples = 1000; ///< Number of samples per waveform cycle (resolution).

    LFO::Type currentType = LFO::Type::Sine; ///< Active waveform type.
    float currentShape = 0.5f;               ///< Current morphing shape parameter.
    float currentFreqHz = 1.0f;              ///< Current LFO frequency in Hz.
    int currentSteps = 4;                    ///< Number of steps used in Steps mode.

    juce::Rectangle<int> bounds;      ///< Current graph bounds (in pixels).
    juce::Path lfoPath;               ///< Current waveform path.
    std::vector<GridLine> xGridLines; ///< X-axis grid lines and their labels.
    std::vector<GridLine> yGridLines; ///< Y-axis grid lines and their labels.
    float yGraphPadding = 0.05f;     ///< Extra vertical padding added to the graph's Y-axis range.

    /**
     * @brief Computes and stores vertical grid lines.
     */
    void computeYGridLines();

    /**
     * @brief Computes and stores horizontal grid lines.
     * @param durationInSeconds X-axis duration for the grid.
     */
    void computeXGridLines(float durationInSeconds);
};
