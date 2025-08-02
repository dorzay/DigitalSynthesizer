#include "VolumeMeter.h"
#include "../../PluginProcessor.h"

VolumeMeter::VolumeMeter()
{
    startTimerHz(30);
}

VolumeMeter::~VolumeMeter()
{
    cleanup();
}

void VolumeMeter::cleanup()
{
    stopTimer();
    processor = nullptr;
}

void VolumeMeter::setAudioProcessorReference(DigitalSynthesizerAudioProcessor& p)
{
    processor = &p;
    startTimerHz(30);
}

void VolumeMeter::updateTheme()
{
    repaint();
}

void VolumeMeter::setLevels(float leftDb, float rightDb)
{
    leftLevelDb = leftDb;
    rightLevelDb = rightDb;
    repaint();
}

void VolumeMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(UI::Colors::VolumeMeterBackground);
    g.setColour(UI::Colors::VolumeMeterText);
    g.setFont(juce::Font(UI::Fonts::defaultFontSize));

    g.drawFittedText("Master", bounds.removeFromTop(40), juce::Justification::centred, 1);

    const int meterHeight = bounds.getHeight() - 45;

    auto drawMeter = [&](int x, float levelDb, const juce::String& label)
        {
            juce::Rectangle<float> meterBounds(
                static_cast<float>(x),
                static_cast<float>(bounds.getY()),
                static_cast<float>(meterWidth),
                static_cast<float>(meterHeight)
            );

            float normalized = juce::jmap(juce::jlimit(minDisplayDb, maxDisplayDb, levelDb), minDisplayDb, maxDisplayDb, 0.0f, 1.0f);
            float filledHeight = normalized * static_cast<float>(meterHeight);

            // Background
            g.setColour(UI::Colors::VolumeMeterBarBackground);
            g.fillRect(meterBounds);

            // Gradient fill
            juce::ColourGradient gradient(
                juce::Colours::green, meterBounds.getBottomLeft(),
                juce::Colours::red, meterBounds.getTopLeft(), false);
            gradient.addColour(0.7, juce::Colours::yellow);

            g.setGradientFill(gradient);
            g.fillRect(meterBounds.removeFromBottom(filledHeight));

            // Graduation ticks
            g.setFont(10.0f);
            g.setColour(UI::Colors::VolumeMeterText);
            float interval = 5.0f;
            for (float db = std::floor(maxDisplayDb / interval) * interval; db >= minDisplayDb; db -= interval)
            {
                float norm = juce::jmap(db, minDisplayDb, maxDisplayDb, 0.0f, 1.0f);
                float y = static_cast<float>(bounds.getY() + meterHeight) - norm * meterHeight;
                juce::String labelText = (db >= 0.0f ? "+" : "") + juce::String((int)db);

                if (label == "L")
                {
                    g.drawLine(x - 6.0f, y, x - 2.0f, y, 1.0f);
                    g.drawText(labelText, static_cast<int>(x - 30.0f), static_cast<int>(y - 6.0f), 24, 12, juce::Justification::right);
                }
                else
                {
                    g.drawLine(x + meterWidth + 2.0f, y, x + meterWidth + 6.0f, y, 1.0f);
                    g.drawText(labelText, static_cast<int>(x + meterWidth + 8.0f), static_cast<int>(y - 6.0f), 24, 12, juce::Justification::left);
                }
            }

            // dB level text
            g.setFont(9.0f);
            juce::String dbText = juce::String(juce::jlimit(minDisplayDb, maxDisplayDb, levelDb), 1) + " dB";
            g.drawText(dbText, x - 10, bounds.getY() + meterHeight + 4, meterWidth + 20, 16, juce::Justification::centred);

            // L/R label
            g.drawText(label, x, bounds.getY() + meterHeight + 20, meterWidth, 16, juce::Justification::centred);
        };

    int centerX = bounds.getCentreX();
    drawMeter(centerX - meterSpacing - meterWidth, leftDisplayDb, "L");
    drawMeter(centerX + meterSpacing, rightDisplayDb, "R");
}

void VolumeMeter::resized()
{
}

int VolumeMeter::getTotalWidth() const noexcept
{
    return totalMeterWidth;
}

void VolumeMeter::timerCallback()
{
    if (processor == nullptr || !isVisible())
    {
        stopTimer();
        return;
    }

    const float leftDb = processor->getMasterVolumeL();
    const float rightDb = processor->getMasterVolumeR();

    constexpr float smoothing = 0.2f;

    leftDisplayDb = (leftDb > leftDisplayDb)
        ? leftDb
        : (1.0f - smoothing) * leftDisplayDb + smoothing * leftDb;

    rightDisplayDb = (rightDb > rightDisplayDb)
        ? rightDb
        : (1.0f - smoothing) * rightDisplayDb + smoothing * rightDb;

    repaint();
}