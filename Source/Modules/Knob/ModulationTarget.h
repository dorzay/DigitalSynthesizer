#pragma once

#include "KnobModulation.h"
#include <JuceHeader.h>

/**
 * @brief Proxy target for routing modulation values into an APVTS parameter.
 */
class ModulationTarget : public ModulatableParameter,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    /**
     * @brief Returns the list of all base parameter IDs that support modulation.
     * @return const std::vector<juce::String>& Vector of base parameter IDs.
     */
    static const std::vector<juce::String>& getAllBaseParameterIDs();

    /**
     * @brief Constructs a modulation proxy for a given base parameter.
     * @param apvts Reference to the plugin’s AudioProcessorValueTreeState.
     * @param router Reference to the processor’s ModulationRouter, for connect/disconnect.
     * @param baseParamID The ID of the parameter to modulate (without "_MOD_*" suffix).
     */
    ModulationTarget(juce::AudioProcessorValueTreeState& apvts,
        ModulationRouter& router,
        const juce::String& baseParamID);

    /**
     * @brief Destructor, uregisters this target from the ModulationRouter.
     */
    ~ModulationTarget() override;

    /**
    * @brief Sets the applied modulation value.
    * @param normalizedValue Modulation amount in normalized range.
    */
    void setModulationValue(float normalizedValue) override;

    /**
     * @brief Sets the modulation range.
     * @param minNormalized Minimum modulation value.
     * @param maxNormalized Maximum modulation value.
     */
    void setModulationRange(float minNormalized, float maxNormalized) override;

    /**
     * @brief Returns the current modulation range.
     * @return A pair [min, max] of normalized values.
     */
    std::pair<float, float> getModulationRange() const override;

    /**
     * @brief Sets the modulation mode.
     * @param newMode The desired modulation mode.
     */
    void setModulationMode(ModulationMode newMode) override;

    /**
     * @brief Returns the current modulation mode.
     * @return The active modulation mode.
     */
    ModulationMode getModulationMode() const override;

    /**
     * @brief Clears all modulation state and range.
     */
    void clearModulation() override;

    /**
     * @brief Called by the APVTS when any listened parameter changes.
     * @param parameterID The full ID of the parameter that changed.
     * @param newValue The new normalized value of that parameter.
     */
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    juce::AudioProcessorValueTreeState& apvts; ///< Reference to the processor’s parameter state.
    ModulationRouter& modulationRouter;        ///< Reference to the central modulation router.

    juce::RangedAudioParameter* baseParam = nullptr; ///< Pointer to the base (unmodulated) parameter.
    juce::AudioParameterFloat* minParam = nullptr;   ///< Pointer to the minimum range parameter (_MOD_MIN).
    juce::AudioParameterFloat* maxParam = nullptr;   ///< Pointer to the maximum range parameter (_MOD_MAX).

    juce::String sourceParamID; ///< Full parameter ID for choosing the modulation source (_MOD_SOURCE).
    juce::String indexParamID;  ///< Full parameter ID for choosing the modulation index (_MOD_INDEX).
    juce::String minParamID;    ///< Full parameter ID for the normalized minimum modulation bound.
    juce::String maxParamID;    ///< Full parameter ID for the normalized maximum modulation bound.

    int currentSourceIndex = 0; ///< Last seen modulation source index.
    ModulationMode currentMode = ModulationMode::Manual; ///< Last seen modulation source mode.
    std::pair<float, float> currentRange{ 0.0f, 1.0f };  ///< Currently cached normalized modulation range [min, max].
};
