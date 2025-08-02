#include "KnobModulation.h"
#include "Knob.h"

void KnobModulationEngine::registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& paramID)
{
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        paramID + "_MOD_SOURCE",
        "Mod Source",
        juce::StringArray{ "None", "Manual", "Midi", "Envelope", "LFO" },
        static_cast<int>(ModulationMode::None)
    ));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        paramID + "_MOD_INDEX",
        "Mod Index",
        0, std::max(NUM_OF_ENVELOPES, NUM_OF_LFOS) - 1, 0
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        paramID + "_MOD_MIN",
        "Mod Min",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        paramID + "_MOD_MAX",
        "Mod Max",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        1.0f
    ));
}

void KnobModulationEngine::setValue(float normalized)
{
    value = juce::jlimit(0.0f, 1.0f, normalized);
}

void KnobModulationEngine::setMode(ModulationMode newMode)
{
    mode = newMode;

    if (mode == ModulationMode::Envelope || mode == ModulationMode::LFO)
    {
        min = 0.0f;
        max = 1.0f;
        delta = 1.0f;
    }
}

ModulationMode KnobModulationEngine::getMode() const
{
    return mode;
}

void KnobModulationEngine::setSourceIndex(int index)
{
    modSourceIndex = index;
}

int KnobModulationEngine::getSourceIndex() const
{
    return modSourceIndex;
}

void KnobModulationEngine::clear()
{
    mode = ModulationMode::Manual;
    value = 0.0f;
    min = 0.0f;
    max = 1.0f;
    delta = 1.0f;
    dragging = false;
}

void KnobModulationEngine::setRange(float minNormalized, float maxNormalized)
{
    min = juce::jlimit(0.0f, 1.0f, minNormalized);
    max = juce::jlimit(min, 1.0f, maxNormalized);
    delta = max - min;
}

std::pair<float, float> KnobModulationEngine::getRange() const
{
    return { min, max };
}

void KnobModulationEngine::beginRangeEdit(bool editingMin_, juce::Point<float> startPos)
{
    dragging = true;
    editingMin = editingMin_;
    dragStart = startPos;
}

void KnobModulationEngine::updateDrag(juce::Point<float> currentPos)
{
    if (!dragging)
        return;

    float deltaY = currentPos.y - dragStart.y;
    float sensitivity = 0.01f;

    if (editingMin)
    {
        float newMin = min - deltaY * sensitivity;
        newMin = juce::jlimit(0.0f, max, newMin);
        min = newMin;
    }
    else
    {
        float newMax = max - deltaY * sensitivity;
        newMax = juce::jlimit(min, 1.0f, newMax);
        max = newMax;
    }

    delta = max - min;
    dragStart = currentPos;
}

void KnobModulationEngine::endRangeEdit()
{
    dragging = false;
}

bool KnobModulationEngine::isEditing() const
{
    return dragging;
}

float KnobModulationEngine::getModulationValue() const
{
    return value;
}

void KnobModulationEngine::shiftRange(float deltaY)
{
    float sensitivity = 0.01f;
    float shiftAmount = -deltaY * sensitivity;

    // Attempt to shift the range by preserving delta
    float newMin = min + shiftAmount;
    float newMax = max + shiftAmount;

    // Clamp range within [0.0, 1.0] with edge shrinking
    if (newMin < 0.0f)
    {
        newMin = 0.0f;
        newMax = newMin + delta;
        if (newMax > 1.0f) newMax = 1.0f;
    }
    else if (newMax > 1.0f)
    {
        newMax = 1.0f;
        newMin = newMax - delta;
        if (newMin < 0.0f) newMin = 0.0f;
    }

    min = newMin;
    max = newMax;
    delta = max - min;
}

juce::StringArray KnobModulationEngine::getParameterIDsFor(const juce::String& paramID)
{
    return {
        paramID + "_MOD_SOURCE",
        paramID + "_MOD_INDEX",
        paramID + "_MOD_MIN",
        paramID + "_MOD_MAX"
    };
}

void ModulationRouter::registerTarget(ModulatableParameter* target)
{
    // No-op for now. Could track all targets if needed.
}

void ModulationRouter::unregisterTarget(ModulatableParameter* target)
{
    disconnect(target);
}

void ModulationRouter::connect(const ModulationSourceID& source, ModulatableParameter* target)
{
    // First remove the target from any previous connection
    disconnect(target);

    // Store in both maps
    sourceToTargets[source].push_back(target);
    targetToSource[target] = source;

    // Notify the target of its new modulation mode
    switch (source.type)
    {
    case ModulationSourceType::Envelope:
        target->setModulationMode(ModulationMode::Envelope);
        break;

    case ModulationSourceType::LFO:
        target->setModulationMode(ModulationMode::LFO);
        break;

    default:
        break;
    }
}

void ModulationRouter::disconnect(ModulatableParameter* target)
{
    if (target == nullptr)
        return;

    auto it = targetToSource.find(target);
    if (it != targetToSource.end())
    {
        const auto& source = it->second;
        auto& targets = sourceToTargets[source];

        // Remove the target from source's list
        targets.erase(std::remove(targets.begin(), targets.end(), target), targets.end());

        // If no more targets, erase the source key entirely
        if (targets.empty())
            sourceToTargets.erase(source);

        // Clear reverse lookup
        targetToSource.erase(it);

        // Notify target it has no modulation
        target->setModulationMode(ModulationMode::Manual);
        target->clearModulation();
    }
}

void ModulationRouter::pushModulationValue(const ModulationSourceID& source, float normalizedValue)
{
    lastModValues[source] = normalizedValue;

    auto it = sourceToTargets.find(source);
    if (it == sourceToTargets.end())
        return;

    auto& targets = it->second;
    for (auto targetIt = targets.begin(); targetIt != targets.end();)
    {
        if (ModulatableParameter* target = *targetIt)
        {
            target->setModulationValue(normalizedValue);
            ++targetIt;
        }
        else
        {
            // Remove dangling null pointers
            targetIt = targets.erase(targetIt);
        }
    }
}

void ModulationRouter::disconnectAllTargetsUsing(const ModulationSourceID& source)
{
    auto it = sourceToTargets.find(source);
    if (it == sourceToTargets.end())
        return;

    for (auto* target : it->second)
    {
        if (target)
        {
            target->clearModulation();
            target->setModulationMode(ModulationMode::Manual);

            // Reset to default value
            if (auto* knob = dynamic_cast<Knob*>(target))
            {
                if (auto* param = knob->getAPVTS().getParameter(knob->getParamID()))
                    param->setValueNotifyingHost(param->getDefaultValue());
            }

            targetToSource.erase(target);
        }
    }

    sourceToTargets.erase(it);
}

void ModulationRouter::disconnectAll()
{
    for (auto& [target, source] : targetToSource)
    {
        if (target)
        {
            target->clearModulation();
            target->setModulationMode(ModulationMode::Manual);
        }
    }

    sourceToTargets.clear();
    targetToSource.clear();
    lastModValues.clear();
}

std::optional<ModulationSourceID> ModulationRouter::getSourceForTarget(ModulatableParameter* target) const
{
    auto it = targetToSource.find(target);
    if (it != targetToSource.end())
        return it->second;

    return std::nullopt;
}

void ModulationRouter::retriggerPush(const ModulationSourceID& source)
{
    auto valueIt = lastModValues.find(source);
    if (valueIt == lastModValues.end())
        return;

    float value = valueIt->second;

    auto it = sourceToTargets.find(source);
    if (it == sourceToTargets.end())
        return;

    for (auto* target : it->second)
    {
        if (target)
            target->setModulationValue(value);
    }
}

void ModulationRouter::connectIfAlive(const ModulationSourceID& source, ModulatableParameter* target)
{
    if (lastModValues.find(source) != lastModValues.end())
    {
        connect(source, target);
        retriggerPush(source);
    }
}