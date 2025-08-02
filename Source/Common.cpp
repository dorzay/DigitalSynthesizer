#include "Common.h"
#include "BinaryData.h"

juce::File getProjectPath()
{
    auto currentSourceFile = juce::File(__FILE__);
    return currentSourceFile.getParentDirectory().getParentDirectory();
}

namespace UI
{
    namespace Colors
    {
        static const std::map<juce::String, juce::Colour*> colorsMap = {
            { "MainText",                    &MainText                    },
            { "MainBackground",              &MainBackground              },
            { "MenuBarText",                 &MenuBarText                 },
            { "MenuBarHeaderText",           &MenuBarHeaderText           },
            { "MenuBarHighlightedText",      &MenuBarHighlightedText      },
            { "MenuBarBackground",           &MenuBarBackground           },
            { "MenuBarHighlightedBackground",&MenuBarHighlightedBackground},
            { "OscillatorText",              &OscillatorText              },
            { "OscillatorBackground",        &OscillatorBackground        },
            { "EnvelopeText",                &EnvelopeText                },
            { "EnvelopeBackground",          &EnvelopeBackground          },
            { "EnvelopeGraphCurve",          &EnvelopeGraphCurve          },
            { "EnvelopeGraphGridLines",      &EnvelopeGraphGridLines      },
            { "EnvelopeGraphGridText",       &EnvelopeGraphGridText       },
            { "EnvelopeGraphStroke",         &EnvelopeGraphStroke         },
            { "FilterText",                  &FilterText                  },
            { "FilterBackground",            &FilterBackground            },
            { "FilterGraphCurve",            &FilterGraphCurve            },
            { "FilterGraphGridLines",        &FilterGraphGridLines        },
            { "FilterGraphGridText",         &FilterGraphGridText         },
            { "FilterGraphStroke",           &FilterGraphStroke           },
            { "FilterGraphFlood",            &FilterGraphFlood            },
            { "LFOText",                     &LFOText                     },
            { "LFOBackground",               &LFOBackground               },
            { "LFOGraphCurve",               &LFOGraphCurve               },
            { "LFOGraphGridLines",           &LFOGraphGridLines           },
            { "LFOGraphGridText",            &LFOGraphGridText            },
            { "LFOGraphStroke",              &LFOGraphStroke              },
            { "ComboBoxData",                &ComboBoxData                },
            { "ComboBoxArrow",               &ComboBoxArrow               },
            { "ComboBoxOutline",             &ComboBoxOutline             },
            { "ComboBoxBackground",          &ComboBoxBackground          },
            { "ComboBoxHighlightBackground", &ComboBoxHighlightBackground },
            { "KnobThumb",                   &KnobThumb                   },
            { "KnobTextBoxText",             &KnobTextBoxText             },
            { "KnobTextBoxOutline",          &KnobTextBoxOutline          },
            { "KnobSliderFill",              &KnobSliderFill              },
            { "KnobSliderOutline",           &KnobSliderOutline           },
            { "VolumeMeterText",             &VolumeMeterText             },
            { "VolumeMeterBackground",       &VolumeMeterBackground       },
            { "VolumeMeterBarBackground",    &VolumeMeterBarBackground    },
            { "MidiLearning",                & MidiLearning               },
            { "MidiConnected",               & MidiConnected              },
            { "ModulationRing",              & ModulationRing             }
        };

        const std::map<juce::String, std::map<juce::String, juce::Colour>> Presets::themes = {
            { "Dark", {
                { "MainText", juce::Colours::white },
                { "MainBackground", juce::Colour::fromRGB(20, 20, 20) },
                { "MenuBarText", juce::Colours::white },
                { "MenuBarHeaderText", juce::Colours::lightgrey },
                { "MenuBarHighlightedText", juce::Colours::black },
                { "MenuBarBackground", juce::Colour::fromRGB(30, 30, 30) },
                { "MenuBarHighlightedBackground", juce::Colour::fromRGB(255, 140, 0) },
                { "OscillatorText", juce::Colours::white },
                { "OscillatorBackground", juce::Colour::fromRGB(50, 50, 70) },
                { "EnvelopeText", juce::Colours::white },
                { "EnvelopeBackground", juce::Colour::fromRGB(50, 50, 70) },
                { "EnvelopeGraphCurve", juce::Colours::limegreen },
                { "EnvelopeGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "EnvelopeGraphGridText", juce::Colours::white },
                { "EnvelopeGraphStroke", juce::Colours::white },
                { "FilterText", juce::Colours::white },
                { "FilterBackground", juce::Colour::fromRGB(50, 50, 70) },
                { "FilterGraphCurve", juce::Colours::limegreen },
                { "FilterGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "FilterGraphGridText", juce::Colours::white },
                { "FilterGraphStroke", juce::Colours::white },
                { "FilterGraphFlood", juce::Colours::lightblue },
                { "LFOText", juce::Colours::white },
                { "LFOBackground", juce::Colour::fromRGB(50, 50, 70) },
                { "LFOGraphCurve", juce::Colours::limegreen },
                { "LFOGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "LFOGraphGridText", juce::Colours::white },
                { "LFOGraphStroke", juce::Colours::white },
                { "ComboBoxData", juce::Colours::white },
                { "ComboBoxArrow", juce::Colours::white },
                { "ComboBoxOutline", juce::Colour::fromRGB(90, 40, 10) },
                { "ComboBoxBackground", juce::Colour::fromRGB(40, 40, 40) },
                { "ComboBoxHighlightBackground", juce::Colour::fromRGB(90, 40, 10) },
                { "KnobThumb", juce::Colour::fromRGB(255, 220, 0) },
                { "KnobTextBoxText", juce::Colours::white },
                { "KnobTextBoxOutline", juce::Colours::white },
                { "KnobSliderFill", juce::Colour::fromRGB(200, 100, 0) },
                { "KnobSliderOutline", juce::Colours::white },
                { "VolumeMeterText", juce::Colours::white },
                { "VolumeMeterBackground", juce::Colour::fromRGB(50, 50, 70) },
                { "VolumeMeterBarBackground", juce::Colour::fromRGB(26, 30, 32) },
            }},
            { "Light", {
                { "MainText", juce::Colours::black },
                { "MainBackground", juce::Colour::fromRGB(250, 250, 250) },
                { "MenuBarText", juce::Colours::black },
                { "MenuBarHeaderText", juce::Colours::darkgrey },
                { "MenuBarHighlightedText", juce::Colours::white },
                { "MenuBarBackground", juce::Colour::fromRGB(240, 240, 240) },
                { "MenuBarHighlightedBackground", juce::Colour::fromRGB(0, 122, 204) },
                { "OscillatorText", juce::Colours::black },
                { "OscillatorBackground", juce::Colour::fromRGB(225, 225, 240) },
                { "EnvelopeText", juce::Colours::black },
                { "EnvelopeBackground", juce::Colour::fromRGB(225, 225, 240) },
                { "EnvelopeGraphCurve", juce::Colours::limegreen },
                { "EnvelopeGraphGridLines", juce::Colours::darkgrey.withAlpha(0.5f) },
                { "EnvelopeGraphGridText", juce::Colours::black },
                { "EnvelopeGraphStroke", juce::Colours::black },
                { "FilterText", juce::Colours::black },
                { "FilterBackground", juce::Colour::fromRGB(225, 225, 240) },
                { "FilterGraphCurve", juce::Colours::limegreen },
                { "FilterGraphGridLines", juce::Colours::darkgrey.withAlpha(0.5f) },
                { "FilterGraphGridText", juce::Colours::black },
                { "FilterGraphStroke", juce::Colours::black },
                { "FilterGraphFlood", juce::Colours::lightblue },
                { "LFOText", juce::Colours::black },
                { "LFOBackground", juce::Colour::fromRGB(225, 225, 240) },
                { "LFOGraphCurve", juce::Colours::limegreen },
                { "LFOGraphGridLines", juce::Colours::darkgrey.withAlpha(0.5f) },
                { "LFOGraphGridText", juce::Colours::black },
                { "LFOGraphStroke", juce::Colours::black },
                { "ComboBoxData", juce::Colours::black },
                { "ComboBoxArrow", juce::Colours::black },
                { "ComboBoxOutline", juce::Colour::fromRGB(180, 210, 250) },
                { "ComboBoxBackground", juce::Colour::fromRGB(250, 250, 250) },
                { "ComboBoxHighlightBackground", juce::Colour::fromRGB(180, 210, 250) },
                { "KnobThumb", juce::Colour::fromRGB(0, 100, 255) },
                { "KnobTextBoxText", juce::Colours::black },
                { "KnobTextBoxOutline", juce::Colours::black },
                { "KnobSliderFill", juce::Colour::fromRGB(0, 100, 200) },
                { "KnobSliderOutline", juce::Colours::black },
                { "VolumeMeterText", juce::Colours::black },
                { "VolumeMeterBackground", juce::Colour::fromRGB(225, 225, 240) },
                { "VolumeMeterBarBackground", juce::Colour::fromRGB(163, 167, 170) },
            }},
            { "Retro", {
                { "MainText", juce::Colour::fromRGB(50, 19, 19) },
                { "MainBackground", juce::Colour::fromRGB(249, 245, 240) },
                { "MenuBarText", juce::Colour::fromRGB(50, 19, 19) },
                { "MenuBarHeaderText", juce::Colour::fromRGB(244, 153, 26) },
                { "MenuBarHighlightedText", juce::Colours::white },
                { "MenuBarBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "MenuBarHighlightedBackground", juce::Colour::fromRGB(244, 153, 26) },
                { "OscillatorText", juce::Colour::fromRGB(50, 19, 19) },
                { "OscillatorBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "EnvelopeText", juce::Colour::fromRGB(50, 19, 19) },
                { "EnvelopeBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "EnvelopeGraphCurve", juce::Colours::limegreen },
                { "EnvelopeGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "EnvelopeGraphGridText", juce::Colour::fromRGB(50, 19, 19) },
                { "EnvelopeGraphStroke", juce::Colour::fromRGB(50, 19, 19) },
                { "FilterText", juce::Colour::fromRGB(50, 19, 19) },
                { "FilterBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "FilterGraphCurve", juce::Colours::limegreen },
                { "FilterGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "FilterGraphGridText", juce::Colour::fromRGB(50, 19, 19) },
                { "FilterGraphStroke", juce::Colour::fromRGB(50, 19, 19) },
                { "FilterGraphFlood", juce::Colours::lightblue },
                { "LFOText", juce::Colour::fromRGB(50, 19, 19) },
                { "LFOBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "LFOGraphCurve", juce::Colours::limegreen },
                { "LFOGraphGridLines", juce::Colours::grey.withAlpha(0.4f) },
                { "LFOGraphGridText", juce::Colour::fromRGB(50, 19, 19) },
                { "LFOGraphStroke", juce::Colour::fromRGB(50, 19, 19) },
                { "ComboBoxData", juce::Colour::fromRGB(50, 19, 19) },
                { "ComboBoxArrow", juce::Colour::fromRGB(50, 19, 19) },
                { "ComboBoxOutline", juce::Colour::fromRGB(244, 153, 26) },
                { "ComboBoxBackground", juce::Colour::fromRGB(249, 245, 240) },
                { "ComboBoxHighlightBackground", juce::Colour::fromRGB(244, 153, 26) },
                { "KnobThumb", juce::Colour::fromRGB(244, 153, 26) },
                { "KnobTextBoxText", juce::Colour::fromRGB(50, 19, 19) },
                { "KnobTextBoxOutline", juce::Colour::fromRGB(50, 19, 19) },
                { "KnobSliderFill", juce::Colour::fromRGB(244, 153, 26) },
                { "KnobSliderOutline", juce::Colour::fromRGB(50, 19, 19) },
                { "VolumeMeterText", juce::Colour::fromRGB(50, 19, 19) },
                { "VolumeMeterBackground", juce::Colour::fromRGB(242, 234, 211) },
                { "VolumeMeterBarBackground", juce::Colour::fromRGB(163, 167, 170) },
            }}
        };

        static const std::vector<std::pair<int, juce::String>> availableThemes = {
            { 1, "Dark" },
            { 2, "Light" },
            { 3, "Retro" }
        };

        void applyTheme(const juce::String& name)
        {
            auto it = Presets::themes.find(name);
            if (it == Presets::themes.end())
                return;

            for (auto& [key, newColour] : it->second)
                if (auto cv = colorsMap.find(key); cv != colorsMap.end())
                    *(cv->second) = newColour;
        }

        void applyThemeByID(int id)
        {
            for (auto& [themeID, name] : availableThemes)
                if (themeID == id)
                    return applyTheme(name);
        }

        const std::vector<std::pair<int, juce::String>>& getAvailableThemeNames()
        {
            return availableThemes;
        }
    }

    namespace Fonts
    {
        static juce::Typeface::Ptr customFont = juce::Typeface::createSystemTypefaceFor(
            BinaryData::NexaExtraLight_ttf, BinaryData::NexaExtraLight_ttfSize
        );

        juce::Font defaultFont(customFont ? customFont : juce::Font(defaultFontSize));
    }
}

namespace FormattingUtils
{
    float normalizedToValue(float normalizedValue, FormatType formatType, float minValue, float maxValue, int enumCount)
    {
        float value = 0.0f;

        switch (formatType)
        {
        case FormatType::Normal:
            value = normalizedValue;
            break;

        case FormatType::Pan:
            value = juce::jmap(normalizedValue, 0.0f, 1.0f, minValue, maxValue);
            break;

        case FormatType::Time:
        {
            float curved = std::pow(std::clamp(normalizedValue, 0.0f, 1.0f), envelopeTimeExponent);
            value = juce::jmap(curved, 0.0f, 1.0f, minValue, maxValue);
            break;
        }

        case FormatType::Percent:
            value = juce::jmap(normalizedValue, 0.0f, 1.0f, minValue, maxValue) * 100.0f;
            break;

        case FormatType::FrequencyLowPass:
        {
            float warped = std::sqrt(std::clamp(normalizedValue, 0.0f, 1.0f));
            float logFreq = juce::jmap(warped, 0.0f, 1.0f, logFreqMin, logFreqMax);
            value = std::pow(10.0f, logFreq);
            break;
        }

        case FormatType::FrequencyHighPass:
        {
            float warped = std::pow(std::clamp(normalizedValue, 0.0f, 1.0f), 2.0f);
            float logFreq = juce::jmap(warped, 0.0f, 1.0f, logFreqMin, logFreqMax);
            value = std::pow(10.0f, logFreq);
            break;
        }

        case FormatType::Slope:
        {
            float index = juce::jmap(std::clamp(normalizedValue, 0.0f, 1.0f), 0.0f, 1.0f, 0.0f, static_cast<float>(enumCount - 1));
            value = (static_cast<float>(static_cast<int>(index)) + 1) * 12.0f;
            break;
        }

        case FormatType::Resonance:
            value = juce::jmap(normalizedValue, 0.0f, 1.0f, resonanceMin, resonanceMax);
            break;

        case FormatType::VowelCenterFrequency:
        {
            float logFreq = juce::jmap(normalizedValue, 0.0f, 1.0f, logVowelMorphMin, logVowelMorphMax);
            value = std::exp(logFreq);
            break;
        }

        case FormatType::LFOFrequency:
        {
            float warped = std::sqrt(std::clamp(normalizedValue, 0.0f, 1.0f));
            float logFreq = juce::jmap(warped, 0.0f, 1.0f,std::log10(lfoFreqMinHz), std::log10(lfoFreqMaxHz));
            value = std::pow(10.0f, logFreq);
            break;
        }

        default:
            value = juce::jmap(normalizedValue, 0.0f, 1.0f, minValue, maxValue);
            break;
        }

        return value;
    }

    float valueToNormalized(float value, FormatType formatType, float minValue, float maxValue, int enumCount)
    {
        value = std::clamp(value, minValue, maxValue);
        float norm = 0.0f;

        switch (formatType)
        {
        case FormatType::Normal:
            norm = value;
            break;

        case FormatType::Pan:
            norm = juce::jmap(value, minValue, maxValue, 0.0f, 1.0f);
            break;

        case FormatType::Time:
        {
            float lin = juce::jmap(value, minValue, maxValue, 0.0f, 1.0f);
            norm = std::pow(lin, 1.0f / envelopeTimeExponent);
            break;
        }

        case FormatType::Percent:
            value /= 100.0f;
            norm = juce::jmap(value, minValue, maxValue, 0.0f, 1.0f);
            break;

        case FormatType::FrequencyLowPass:
        {
            float logValue = std::log10(std::clamp(value, freqMinHz, freqMaxHz));
            float warped = juce::jmap(logValue, logFreqMin, logFreqMax, 0.0f, 1.0f);
            norm = std::pow(warped, 2.0f);
            break;
        }

        case FormatType::FrequencyHighPass:
        {
            float logValue = std::log10(std::clamp(value, freqMinHz, freqMaxHz));
            float warped = juce::jmap(logValue, logFreqMin, logFreqMax, 0.0f, 1.0f);
            norm = std::sqrt(warped);
            break;
        }

        case FormatType::Slope:
        {
            float maxDb = static_cast<float>(enumCount) * 12.0f;
            if (maxDb > 0.0f)
            {
                float clamped = std::clamp(value, 0.0f, maxDb);
                norm = clamped / maxDb;
            }
            else
            {
                norm = 0.0f;
            }
            break;
        }

        case FormatType::Resonance:
        {
            float clamped = juce::jlimit(resonanceMin, resonanceMax, value);
            norm = juce::jmap(clamped, resonanceMin, resonanceMax, 0.0f, 1.0f);
            break;
        }

        case FormatType::VowelCenterFrequency:
        {
            float logValue = std::log(std::clamp(value, vowelMorphMinHz, vowelMorphMaxHz));
            norm = juce::jmap(logValue, logVowelMorphMin, logVowelMorphMax, 0.0f, 1.0f);
            break;
        }

        case FormatType::LFOFrequency:
        {
            float logValue = std::log10(std::clamp(value, lfoFreqMinHz, lfoFreqMaxHz));
            float warped = juce::jmap(logValue,std::log10(lfoFreqMinHz), std::log10(lfoFreqMaxHz), 0.0f, 1.0f);
            norm = std::pow(warped, 2.0f);
            break;
        }

        default:
            norm = juce::jmap(value, minValue, maxValue, 0.0f, 1.0f);
            break;
        }

        return norm;
    }

    juce::String formatValue(float normalized, FormatType formatType, float minValue, float maxValue, int enumCount)
    {
        float realValue = normalizedToValue(normalized, formatType, minValue, maxValue, enumCount);

        switch (formatType)
        {
        case FormatType::Normal:
            return juce::String(normalized, 2); 
        
        case FormatType::Discrete:
            return juce::String(static_cast<int>(realValue));

        case FormatType::Pan:
        {
            const int panValue = static_cast<int>(std::round((normalized - 0.5f) * 200.0f));
            if (panValue == 0)
                return "0";
            return (panValue > 0 ? "+" : "") + juce::String(panValue);
        }

        case FormatType::Time:
            if (realValue < 1000.0f)
                return juce::String(realValue, 1) + " ms";
            else
                return juce::String(realValue / 1000.0f, 1) + " s";

        case FormatType::Percent:
            return juce::String(realValue, 0) + "%";

        case FormatType::FrequencyLowPass:
        case FormatType::FrequencyHighPass:
        case FormatType::VowelCenterFrequency:
        {
            if (formatType == FormatType::FrequencyLowPass && std::abs(realValue - 1000.0f) < 3.0f)
                realValue = 1000.0f;

            if (realValue >= 1000.0f)
                return juce::String(realValue / 1000.0f, 2) + " kHz";
            else
                return juce::String(static_cast<int>(realValue)) + " Hz";
        }

        case FormatType::Slope:
            return juce::String(static_cast<int>(realValue)) + " dB";

        case FormatType::Resonance:
            return juce::String(realValue, 2) + " Q";

        case FormatType::LFOFrequency:
            return juce::String(realValue, 2) + " Hz";

        default:
            return "";
        }
    }
}

namespace MidiController
{
    /// CC <-> Knob index mapping
    const std::map<int, int> ccToKnobIndex = {
        {74, 1}, {71, 2}, {76, 3}, {77, 4},
        {93, 5}, {73, 6}, {75, 7}, {18, 8},
        {19, 9}, {16, 10}, {17, 11}, {91, 12},
        {79, 13}, {72, 14}
    };

    /// Set of all assigned CCs — derived from the map keys
    const std::set<int> assignedKnobs = []
        {
            std::set<int> s;
            for (const auto& [cc, _] : ccToKnobIndex)
                s.insert(cc);
            return s;
        }();
}
