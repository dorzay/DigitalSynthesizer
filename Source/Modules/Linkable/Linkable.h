#pragma once

#include <JuceHeader.h>

class Envelope;

/**
 * @brief Interface for modules that can be linked to an Envelope.
 */
class Linkable
{
public:
    /**
     * @brief Virtual destructor for safe inheritance.
     */
    virtual ~Linkable() = default;

    /**
     * @brief Assign an Envelope to this object.
     * @param envelope Pointer to the Envelope object to be linked.
     */
    virtual void setEnvelope(Envelope* envelope) = 0;

    /**
     * @brief Get the unique name of this linkable object.
     * @return Name of the object.
     */
    virtual juce::String getLinkableName() const = 0;
};
