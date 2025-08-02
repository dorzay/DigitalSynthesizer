#pragma once

#include "../../Common.h"
#include <JuceHeader.h>

/**
 * @brief Enum representing the modulation mode applied to a knob.
 */
enum class ModulationMode
{
    None = 0,
    Manual,     ///< Controlled only by mouse
    Midi,       ///< Controlled by MIDI Learn + mouse
    Envelope,   ///< Controlled by Envelope (ignores mouse)
    LFO         ///< Controlled by LFO (future use)
};

/**
 * @brief Enum representing the type of modulation source.
 */
enum class ModulationSourceType
{
    Envelope,
    LFO
};

/**
 * @brief Handles modulation logic for a Knob, including min/max bounds, mode, and drag editing.
 */
class KnobModulationEngine
{
public:
    /**
     * @brief Applies a normalized modulation value.
     * @param normalized Value in range [0.0, 1.0].
     */
    void setValue(float normalized);

    /**
     * @brief Sets the modulation mode.
     * @param newMode One of: Manual, MIDI, Envelope, or LFO.
     */
    void setMode(ModulationMode newMode);

    /**
     * @brief Gets the current modulation mode.
     * @return The active modulation mode.
     */
    ModulationMode getMode() const;

    /**
     * @brief Sets the modulation source index.
     * @param index Source index (e.g., LFO 0, Envelope 1).
     */
    void setSourceIndex(int index);

    /**
     * @brief Gets the index of the modulation source.
     * @return The source index (LFO/Envelope).
     */
    int getSourceIndex() const;

    /**
     * @brief Resets modulation state and range.
     */
    void clear();

    /**
     * @brief Sets the modulation range.
     * @param minNormalized Minimum value (0.0–1.0).
     * @param maxNormalized Maximum value (0.0–1.0).
     */
    void setRange(float minNormalized, float maxNormalized);

    /**
     * @brief Returns the modulation range.
     * @return Pair of [min, max] values.
     */
    std::pair<float, float> getRange() const;

    /**
     * @brief Starts a modulation drag session.
     * @param editingMin_ True if editing min; otherwise max.
     * @param startPos Initial mouse position.
     */
    void beginRangeEdit(bool editingMin_, juce::Point<float> startPos);

    /**
     * @brief Updates the range during drag.
     * @param currentPos Current mouse position.
     */
    void updateDrag(juce::Point<float> currentPos);

    /**
     * @brief Ends the modulation drag edit.
     */
    void endRangeEdit();

    /**
     * @brief Returns true if currently editing.
     * @return True if in drag-edit mode.
     */
    bool isEditing() const;

    /**
     * @brief Returns the last modulation value.
     * @return Last applied modulation value.
     */
    float getModulationValue() const;

    /**
     * @brief Shifts the modulation range vertically.
     * @param deltaY Drag delta in Y-axis.
     */
    void shiftRange(float deltaY);

    /**
     * @brief Registers APVTS parameters for storing modulation state.
     * @param layout Reference to the parameter layout.
     * @param paramID Base parameter ID (e.g., \"LFO2_FREQ\").
     */
    static void registerParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& paramID);

    /**
     * @brief Returns all modulation-related parameter IDs for a base parameter.
     * @param paramID The base parameter ID (e.g., \"LFO2_FREQ\").
     * @return List of parameter IDs: _MOD_SOURCE, _MOD_INDEX, _MOD_MIN, _MOD_MAX
     */
    static juce::StringArray getParameterIDsFor(const juce::String& paramID);


private:
    ModulationMode mode = ModulationMode::Manual; ///< Current modulation mode (Manual, MIDI, Envelope, LFO).
    int modSourceIndex = 0;                       ///< Index of the selected modulation source.
    float value = 0.0f;                           ///< Last received normalized modulation value.
    float min = 0.0f;                             ///< Lower modulation range boundary (normalized, [0.0–1.0]).
    float max = 1.0f;                             ///< Upper modulation range boundary (normalized, [0.0–1.0]).
    float delta = 1.0f;                           ///< Cached delta = max - min. Used for range shifting.
    bool dragging = false;                        ///< True if the user is currently dragging to edit modulation range.
    bool editingMin = false;                      ///< True if editing min (Shift+Left), false if editing max (Shift+Right).
    juce::Point<float> dragStart;                 ///< Starting mouse position of the drag gesture.
};

/**
 * @brief Struct identifying a specific modulation source by type and index.
 */
struct ModulationSourceID
{
    ModulationSourceType type;
    int index;

    bool operator==(const ModulationSourceID& other) const noexcept
    {
        return type == other.type && index == other.index;
    }
};

// Required to use ModulationSourceID as a key in unordered_map
namespace std
{
    template <>
    struct hash<ModulationSourceID>
    {
        std::size_t operator()(const ModulationSourceID& id) const noexcept
        {
            return std::hash<int>()(static_cast<int>(id.type)) ^ std::hash<int>()(id.index);
        }
    };
}

/**
 * @brief Interface for any parameter that can be modulated.
 */
class ModulatableParameter
{
public:
    /**
     * @brief Apply a normalized modulation value (0.0 - 1.0) to the parameter.
     */
    virtual void setModulationValue(float normalizedValue) = 0;

    /**
     * @brief Set the normalized modulation bounds [min, max], both in 0.0 - 1.0 range.
     */
    virtual void setModulationRange(float minNormalized, float maxNormalized) = 0;

    /**
     * @brief Retrieve the current modulation bounds.
     */
    virtual std::pair<float, float> getModulationRange() const = 0;

    /**
     * @brief Set the active modulation mode.
     */
    virtual void setModulationMode(ModulationMode mode) = 0;

    /**
     * @brief Retrieve the current modulation mode.
     */
    virtual ModulationMode getModulationMode() const = 0;

    /**
     * @brief Clear any modulation source or override applied to the parameter.
     */
    virtual void clearModulation() = 0;

    virtual ~ModulatableParameter() = default;
};

/**
 * @brief Central class that manages connections between modulation sources and targets.
 *
 * Each source (Envelope, LFO, etc.) is identified by a ModulationSourceID.
 * Each target is a pointer to a ModulatableParameter.
 */
class ModulationRouter
{
public:
    /**
     * @brief Register a target to be available for modulation.
     */
    void registerTarget(ModulatableParameter* target);

    /**
     * @brief Unregister a previously registered target.
     */
    void unregisterTarget(ModulatableParameter* target);

    /**
     * @brief Connect a target to a modulation source, replacing any existing link.
     */
    void connect(const ModulationSourceID& source, ModulatableParameter* target);

    /**
     * @brief Disconnect a target from its current modulation source.
     */
    void disconnect(ModulatableParameter* target);

    /**
     * @brief Push a modulation value from a source to all linked targets.
     * @param source The ID of the modulator (e.g., Envelope 0)
     * @param normalizedValue The value in range [0.0, 1.0]
     */
    void pushModulationValue(const ModulationSourceID& source, float normalizedValue);

    /**
     * @brief Disconnects all modulation targets that are currently linked to a given source.
     * @param source The modulation source to disconnect all targets from.
     */
    void disconnectAllTargetsUsing(const ModulationSourceID& source);

    /**
     * @brief Disconnects all modulation sources and targets.
     * restoring the modulation router to an empty state.
     */
    void disconnectAll();

    /**
     * @brief Returns the modulation source (if any) assigned to a given target.
     * @param target The knob or other modulatable parameter.
     * @return The source ID if connected, or std::nullopt.
     */
    std::optional<ModulationSourceID> getSourceForTarget(ModulatableParameter* target) const;

    /**
     * @brief Re-applies the last modulation value to all targets of a given source.
     */
    void retriggerPush(const ModulationSourceID& source);

    /**
     * @brief Connects a target to a modulation source only if the source has pushed a value before.
     * @param source The modulation source (LFO or Envelope ID).
     * @param target The target parameter (e.g., a Knob) to modulate.
     */
    void connectIfAlive(const ModulationSourceID& source, ModulatableParameter* target);

private:
    std::unordered_map<ModulationSourceID, std::vector<ModulatableParameter*>> sourceToTargets;
    std::unordered_map<ModulatableParameter*, ModulationSourceID> targetToSource;
    std::unordered_map<ModulationSourceID, float> lastModValues;
};
