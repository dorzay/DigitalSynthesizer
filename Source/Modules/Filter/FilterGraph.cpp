#include "FilterGraph.h"

FilterGraph::FilterGraph()
{
    setInterceptsMouseClicks(false, false);
    updateCoefficients();
}

void FilterGraph::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    updateCoefficients();
}

void FilterGraph::setType(Filter::Type newType)
{
    type = newType;
    updateCoefficients();
}

void FilterGraph::setSlope(Filter::Slope newSlope)
{
    slope = newSlope;
    maxDecibels = (slope == Filter::Slope::dB24) ? 45.0f : 25.0f;

    updateCoefficients();

    if (!lastPlotArea.isEmpty())
        generateAxisGridLines(lastPlotArea);

    if (auto* parent = getParentComponent())
        parent->repaint();
}

void FilterGraph::setCutoffFrequency(float cutoff)
{
    cutoffHz = FormattingUtils::normalizedToValue(
        cutoff,
        FormattingUtils::FormatType::FrequencyLowPass,
        FormattingUtils::freqMinHz,
        FormattingUtils::freqMaxHz);

    updateCoefficients();
}

void FilterGraph::setResonance(float res)
{
    resonance = res;
    updateCoefficients();
}

void FilterGraph::setDrive(float newDrive)
{
    drive = juce::jmax(0.0f, newDrive);
    repaint();
}

void FilterGraph::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
    repaint();
}

void FilterGraph::paint(juce::Graphics& g)
{
    auto plotArea = getLocalBounds().toFloat();

    g.fillAll(UI::Colors::FilterBackground.withAlpha(0.9f));

    drawGrid(g, plotArea);
    drawResponseCurve(g, plotArea);
}

const std::vector<FilterGraphGridLine>& FilterGraph::getXGridLines() const
{
    return xGridLines;
}

const std::vector<FilterGraphGridLine>& FilterGraph::getYGridLines() const
{
    return yGridLines;
}

void FilterGraph::generateAxisGridLines(juce::Rectangle<float> plotArea)
{
    xGridLines.clear();
    yGridLines.clear();

    const float left = plotArea.getX();
    const float right = plotArea.getRight();
    const float top = plotArea.getY();
    const float bottom = plotArea.getBottom();

    const float width = plotArea.getWidth();
    const float height = plotArea.getHeight();

    // Y Axis (Decibels)
    for (float dB = minDecibels; dB <= maxDecibels; dB += dBInterval)
    {
        float y = getDecibelY(dB, top, bottom);

        FilterGraphGridLine grid;
        grid.line = juce::Line<float>(left, y, right, y);

        bool showLabel = (static_cast<int>(dB) % 10 == 0);
        if (showLabel)
        {
            grid.label = juce::String((int)dB) + " dB";
            grid.labelPosition = {
                left - 47.0f,
                y - 8.0f
            };
            grid.justification = juce::Justification::centredRight;
        }

        yGridLines.push_back(grid);
    }

    // X Axis (Frequency)
    for (float freq : frequencyTicks)
    {
        float normX = getLogFrequencyPosition(freq);
        float x = left + normX * width;

        FilterGraphGridLine grid;
        grid.line = juce::Line<float>(x, top, x, bottom);

        juce::String label = (freq >= 1000.0f)
            ? juce::String(freq / 1000.0f, 1) + "k"
            : juce::String((int)freq);

        grid.label = label + "Hz";
        grid.labelPosition = {
            x - 20.0f,
            bottom + 4.0f
        };
        grid.justification = juce::Justification::centredTop;

        xGridLines.push_back(grid);
    }
}

void FilterGraph::setTalkboxBands(const std::array<TalkboxFilter::FormantBand, TalkboxFilter::numFormants>& bands)
{
    talkboxBands = bands;
    repaint();
}

void FilterGraph::updateCoefficients()
{
    if (type == Filter::Type::Talkbox)
        return;

    coeff1.reset();
    coeff2.reset();

    float q = computeQFromResonance();

    switch (type)
    {
    case Filter::Type::LowPass:
        coeff1 = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz, q);
        break;
    case Filter::Type::HighPass:
        coeff1 = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoffHz, q);
        break;
    case Filter::Type::BandPass:
        coeff1 = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, cutoffHz, q);
        break;
    default:
        coeff1 = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz, q);
        break;
    }

    if (slope == Filter::Slope::dB24)
    {
        switch (type)
        {
        case Filter::Type::LowPass:
            coeff2 = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz, q);
            break;
        case Filter::Type::HighPass:
            coeff2 = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoffHz, q);
            break;
        case Filter::Type::BandPass:
            coeff2 = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, cutoffHz, q);
            break;
        default:
            coeff2 = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffHz, q);
            break;
        }
    }

    repaint();
}

float FilterGraph::getLogFrequencyPosition(float freq) const
{
    float minLog = std::log10(minFrequency);
    float maxLog = std::log10(maxFrequency);
    float fLog = std::log10(freq);

    return juce::jlimit(0.0f, 1.0f, (fLog - minLog) / (maxLog - minLog));
}

float FilterGraph::getDecibelY(float dB, float top, float bottom) const
{
    return juce::jmap(juce::jlimit(minDecibels, maxDecibels, dB), minDecibels, maxDecibels, bottom, top);
}

void FilterGraph::drawGrid(juce::Graphics& g, juce::Rectangle<float> plotArea)
{
    g.setColour(UI::Colors::FilterGraphGridLines);
    g.setFont(10.0f);

    const float left = plotArea.getX();
    const float right = plotArea.getRight();
    const float top = plotArea.getY();
    const float bottom = plotArea.getBottom();

    // Horizontal lines (dB axis)
    for (float dB = minDecibels; dB <= maxDecibels; dB += dBInterval)
    {
        float y = getDecibelY(dB, top, bottom);
        g.drawHorizontalLine((int)y, left, right);
    }

    // Vertical lines (Frequency axis)
    for (float freq : frequencyTicks)
    {
        float normX = getLogFrequencyPosition(freq);
        float x = left + normX * plotArea.getWidth();
        g.drawVerticalLine((int)x, top, bottom);
    }

    g.setColour(UI::Colors::FilterGraphStroke);
    g.drawRect(plotArea, 2);
}

void FilterGraph::drawResponseCurve(juce::Graphics& g, juce::Rectangle<float> plotArea)
{
    std::vector<float> magnitudes(numFrequencyBins, 0.0f);
    generateFrequencyResponse(magnitudes);

    juce::Path responsePath;
    const float left = plotArea.getX();
    const float right = plotArea.getRight();
    const float top = plotArea.getY();
    const float bottom = plotArea.getBottom();
    const float width = plotArea.getWidth();

    for (int i = 0; i < numFrequencyBins; ++i)
    {
        float freq = juce::jmap((float)i, 0.0f, (float)(numFrequencyBins - 1), minFrequency, maxFrequency);
        float normX = getLogFrequencyPosition(freq);
        float x = left + normX * width;

        float dB = juce::Decibels::gainToDecibels(magnitudes[i], minDecibels);
        float y = getDecibelY(dB, top, bottom);

        if (i == 0)
            responsePath.startNewSubPath(x, y);
        else
            responsePath.lineTo(x, y);
    }

    // --- Drive flood overlay ---
    if (drive > 0.0f && mix > 0.0f)
    {
        float zeroY = getDecibelY(0.0f, top, bottom);
        float peakY = getDecibelY(maxDecibels, top, bottom);
        float floodSize = (drive * 0.5f) * plotArea.getHeight();
        float floodTop = juce::jmax(peakY, zeroY - floodSize);
        juce::Rectangle<float> overlay = plotArea.withTop(floodTop);

        // Smooth alpha fade between 0.0 → 0.5 drive
        float floodStrength = juce::jlimit(0.0f, 1.0f, drive / 0.5f);
        float floodAlpha = floodStrength * mix * 0.15f;

        g.setColour(UI::Colors::FilterGraphFlood.withAlpha(floodAlpha));
        g.fillRect(overlay);
    }

    // --- Flat 0 dB line (if mix < 1.0) ---
    if (mix < 0.99f)
    {
        g.setColour(juce::Colours::grey.withAlpha(1.0f - mix));
        float zeroY = getDecibelY(0.0f, top, bottom);
        g.drawLine(left, zeroY, right, zeroY, 2.0f);
    }

    // --- Draw response curve ---
    g.setColour(UI::Colors::FilterGraphCurve.withAlpha(mix));
    g.strokePath(responsePath, juce::PathStrokeType(2.0f));
}

void FilterGraph::generateFrequencyResponse(std::vector<float>& magnitudes)
{
    magnitudes.assign(numFrequencyBins, 0.0f);

    if (type == Filter::Type::Talkbox)
    {
        // --- Drive scaling for Talkbox ---
        float shaped = std::pow(drive, 1.5f);
        float gainBoost = 1.0f + shaped * 3.0f;

        for (int i = 0; i < numFrequencyBins; ++i)
        {
            float freq = juce::jmap((float)i, 0.0f, (float)(numFrequencyBins - 1), minFrequency, maxFrequency);
            float totalMag = 0.0f;

            for (const auto& band : talkboxBands)
            {
                auto coeff = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, band.frequency, band.q);
                float mag = coeff->getMagnitudeForFrequency(freq, sampleRate);
                totalMag += band.gain * mag;
            }

            magnitudes[i] = gainBoost * totalMag;
        }
    }
    else
    {
        // Default: Ladder filter response
        float shaped = std::pow(drive, 1.5f);
        float gainBoost = 1.0f + shaped * 3.0f;

        for (int i = 0; i < numFrequencyBins; ++i)
        {
            float freq = juce::jmap((float)i, 0.0f, (float)(numFrequencyBins - 1), minFrequency, maxFrequency);

            float mag1 = coeff1 ? coeff1->getMagnitudeForFrequency(freq, sampleRate) : 1.0f;
            float mag2 = coeff2 ? coeff2->getMagnitudeForFrequency(freq, sampleRate) : 1.0f;

            magnitudes[i] = gainBoost * mag1 * mag2;
        }
    }
}

float FilterGraph::computeQFromResonance() const
{
    return FormattingUtils::normalizedToValue(
        resonance,
        FormattingUtils::FormatType::Resonance,
        FormattingUtils::resonanceMin,
        FormattingUtils::resonanceMax
    );
}