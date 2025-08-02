#include "EnvelopeGraph.h"

std::vector<EnvelopePoint> EnvelopeGraph::getEnvelopePoints(float attackMs, float decayMs, float sustainLevel, float releaseMs, float minDisplayMs)
{
    std::vector<EnvelopePoint> points;

    // Define the key time points
    float t0 = 0.0f;
    float t1 = t0 + attackMs;
    float t2 = t1 + decayMs;
    float t3 = t2;  // Sustain duration is zero — flat point
    float t4 = t3 + releaseMs;

    // Build the ADSR shape
    points.push_back({ t0,  0.0f });           // Start at 0
    points.push_back({ t1,  1.0f });           // Peak after attack
    points.push_back({ t2,  sustainLevel });   // Decay down to sustain
    points.push_back({ t3,  sustainLevel });   // Hold sustain
    points.push_back({ t4,  0.0f });           // Release to 0

    if (t4 < minDisplayMs)
    {
        points.push_back({ minDisplayMs, 0.0f });
    }

    return points;
}

float EnvelopeGraph::getRelaxedXLimit(float totalMs, float step, float minLimit, float maxLimit)
{
    float relaxed = std::ceil(totalMs / step) * step;
    return std::max(minLimit, std::min(relaxed, maxLimit));
}

float EnvelopeGraph::getTotalDuration(float attackMs, float decayMs, float releaseMs)
{
    return attackMs + decayMs + releaseMs;
}

float EnvelopeGraph::getTimeToAmplitude(const std::vector<EnvelopePoint>& points, float amplitudeThreshold)
{
    for (const auto& pt : points)
    {
        if (pt.amplitude >= amplitudeThreshold)
            return pt.timeMs;
    }

    // Not found
    return -1.0f;
}

void EnvelopeGraph::setParameters(float attack, float decay, float sustain, float release)
{
    attackMs = attack;
    decayMs = decay;
    sustainLevel = sustain;
    releaseMs = release;
}

void EnvelopeGraph::setGraphBounds(juce::Rectangle<int> bounds)
{
    graphBounds = bounds;
}

void EnvelopeGraph::generate()
{
    // Clear outputs
    envelopePath.clear();
    xGridLines.clear();
    yGridLines.clear();

    // Compute relaxed X limit
    float totalDuration = getTotalDuration(attackMs, decayMs, releaseMs);
    xLimit = getRelaxedXLimit(totalDuration);

    computeEnvelopePath();
    computeYGridLines();
    computeXGridLines();
}

const juce::Path& EnvelopeGraph::getEnvelopePath() const
{
    return envelopePath;
}

void EnvelopeGraph::computeEnvelopePath()
{
    auto points = getEnvelopePoints(attackMs, decayMs, sustainLevel, releaseMs);

    bool isFirst = true;

    for (const auto& pt : points)
    {
        float x = juce::jmap(pt.timeMs,
            0.0f, xLimit,
            static_cast<float>(graphBounds.getX()),
            static_cast<float>(graphBounds.getRight()));

        float y = juce::jmap(pt.amplitude,
            1.00f + yAxisExtra, 0.0f,
            static_cast<float>(graphBounds.getY()),
            static_cast<float>(graphBounds.getBottom()));

        if (isFirst)
        {
            envelopePath.startNewSubPath(x, y);
            isFirst = false;
        }
        else
        {
            envelopePath.lineTo(x, y);
        }
    }
}

const std::vector<GridLine>& EnvelopeGraph::getYGridLines() const
{
    return yGridLines;
}

const std::vector<GridLine>& EnvelopeGraph::getXGridLines() const
{
    return xGridLines;
}

void EnvelopeGraph::computeYGridLines()
{
    const int numSteps = 5;
    for (int i = 0; i <= numSteps; ++i)
    {
        float amp = i * 0.2f;

        float y = juce::jmap(amp, 1.0f + yAxisExtra, 0.0f,
            static_cast<float>(graphBounds.getY()),
            static_cast<float>(graphBounds.getBottom()));

        GridLine grid;
        grid.line = juce::Line<float>(
            static_cast<float>(graphBounds.getX()), y,
            static_cast<float>(graphBounds.getRight()), y
        );

        grid.label = juce::String(amp, 2);
        grid.labelPosition = {
            static_cast<float>(graphBounds.getX()) - 47.0f,
            y - 8.0f
        };
        grid.justification = juce::Justification::centredRight;

        yGridLines.push_back(grid);
    }
}

void EnvelopeGraph::computeXGridLines()
{
    float step = 0.0f;

    if (xLimit <= 1000.0f)
        step = 250.0f;
    else if (xLimit <= 3000.0f)
        step = 500.0f;
    else if (xLimit <= 8000.0f)
        step = 1000.0f;
    else
        step = 2000.0f;

    for (float t = 0.0f; t <= xLimit; t += step)
    {
        float x = juce::jmap(t, 0.0f, xLimit,
            static_cast<float>(graphBounds.getX()),
            static_cast<float>(graphBounds.getRight()));

        GridLine grid;
        grid.line = juce::Line<float>(x, static_cast<float>(graphBounds.getY()), x, static_cast<float>(graphBounds.getBottom())
        );

        if (t < 1000.0f)
            grid.label = juce::String::formatted("%.0f ms", t);
        else
            grid.label = juce::String::formatted("%.1f s", t / 1000.0f);

        grid.labelPosition = {
            x - 20.0f,
            static_cast<float>(graphBounds.getBottom()) + 4.0f
        };

        grid.justification = juce::Justification::centred;
        xGridLines.push_back(grid);
    }
}