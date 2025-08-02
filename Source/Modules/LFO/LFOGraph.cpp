#include "LFOGraph.h"

void LFOGraph::setParameters(LFO::Type type, float shape, float freqHz, int steps)
{
    currentType = type;
    currentShape = shape;
    currentFreqHz = freqHz;
    currentSteps = steps;
}

void LFOGraph::setGraphBounds(juce::Rectangle<int> newBounds)
{
    bounds = newBounds;
}

void LFOGraph::setLFOReference(const LFO* lfoPtr)
{
    lfo = lfoPtr;
}

void LFOGraph::generate()
{
    lfoPath.clear();
    xGridLines.clear();
    yGridLines.clear();

    if (bounds.isEmpty() || lfo == nullptr)
        return;

    // Determine graph time duration from frequency
    const float durationSeconds = 1.0f / currentFreqHz;

    // Precompute Y scaling with padding
    const float yMin = -yGraphPadding;
    const float yMax = 1.0f + yGraphPadding;
    const float yRange = yMax - yMin;
    const float yScale = static_cast<float>(bounds.getHeight()) / yRange;

    // Start path at first point
    float firstValue = lfo->getValueAtPhase(0.0f);
    float x0 = static_cast<float>(bounds.getX());
    float y0 = bounds.getBottom() - ((firstValue - yMin) * yScale);
    lfoPath.startNewSubPath(x0, y0);

    // Plot waveform over 1 cycle
    for (int i = 1; i < numSamples; ++i)
    {
        float phase = static_cast<float>(i) / static_cast<float>(numSamples - 1);
        float value = lfo->getValueAtPhase(phase);

        float x = bounds.getX() + (phase * bounds.getWidth());
        float y = bounds.getBottom() - ((value - yMin) * yScale);

        lfoPath.lineTo(x, y);
    }

    // Generate fixed grid lines
    computeYGridLines();
    computeXGridLines(durationSeconds);
}

const juce::Path& LFOGraph::getLFOPath() const
{
    return lfoPath;
}

const std::vector<LFOGraph::GridLine>& LFOGraph::getXGridLines() const
{
    return xGridLines;
}

const std::vector<LFOGraph::GridLine>& LFOGraph::getYGridLines() const
{
    return yGridLines;
}

void LFOGraph::computeYGridLines()
{
    static constexpr std::array<float, 3> yTicks = { 0.0f, 0.5f, 1.0f };

    for (float yVal : yTicks)
    {
        float yNorm = (yVal + yGraphPadding) / (1.0f + 2 * yGraphPadding); // Normalize from [-0.05, 1.05]
        float y = bounds.getBottom() - (yNorm * bounds.getHeight());

        GridLine line;
        line.line = juce::Line<float>(
            static_cast<float>(bounds.getX()), y,
            static_cast<float>(bounds.getRight()), y);

        line.label = juce::String(yVal, 2);
        line.labelPosition = { bounds.getX() - 30, static_cast<int>(y) - 8 };
        line.justification = juce::Justification::centredLeft;

        yGridLines.push_back(std::move(line));
    }
}

void LFOGraph::computeXGridLines(float durationInSeconds)
{
    static constexpr std::array<float, 3> xTicks = { 0.0f, 0.5f, 1.0f };

    for (float phase : xTicks)
    {
        float t = phase * durationInSeconds;
        float x = bounds.getX() + (phase * bounds.getWidth());

        GridLine line;
        line.line = juce::Line<float>(
            x, static_cast<float>(bounds.getY()),
            x, static_cast<float>(bounds.getBottom()));

        line.label = juce::String(t, 2) + "s";
        line.labelPosition = { static_cast<int>(x) - 20, bounds.getBottom() + 4 };
        line.justification = juce::Justification::centred;

        xGridLines.push_back(std::move(line));
    }
}