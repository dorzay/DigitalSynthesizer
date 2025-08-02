#pragma once

#include "../Oscillator/Oscillator.h"
#include <JuceHeader.h>

/**
 * @brief Returns the default list of linkable target names (Oscillators, etc).
 */
inline juce::StringArray getDefaultLinkableTargetNames()
{
    juce::StringArray names;

    for (int i = 0; i < NUM_OF_OSCILLATORS; ++i)
        names.add(Oscillator::getDefaultLinkableName(i));

    return names;
}
