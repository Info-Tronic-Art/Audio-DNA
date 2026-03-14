#pragma once
#include <juce_core/juce_core.h>
#include "effects/EffectChain.h"
#include "mapping/MappingEngine.h"
#include "mapping/MappingTypes.h"

// PresetManager: save/load effect chain state + mappings as JSON files.
//
// Serialized format:
// {
//   "name": "preset name",
//   "effects": [
//     { "name": "Ripple", "enabled": true, "order": 0,
//       "params": [ { "name": "intensity", "value": 0.5 }, ... ] }
//   ],
//   "mappings": [
//     { "source": "RMS", "targetEffect": 0, "targetParam": 0,
//       "curve": "Exponential", "inputMin": 0.0, "inputMax": 1.0,
//       "outputMin": 0.0, "outputMax": 1.0, "smoothing": 0.15, "enabled": true }
//   ]
// }
class PresetManager
{
public:
    PresetManager() = default;

    // Save current state to a JSON file. Returns true on success.
    static bool savePreset(const juce::File& file,
                           const juce::String& presetName,
                           const EffectChain& chain,
                           const MappingEngine& engine);

    // Load a preset from JSON. Applies effect enable/params and rebuilds mappings.
    // Returns true on success.
    static bool loadPreset(const juce::File& file,
                           EffectChain& chain,
                           MappingEngine& engine);

    // Get the default presets directory (next to the executable or in app data).
    static juce::File getPresetsDirectory();

    // Enumerate available preset files.
    static juce::Array<juce::File> getAvailablePresets();

    // String conversion helpers for enums
    static juce::String sourceToString(MappingSource source);
    static MappingSource stringToSource(const juce::String& str);
    static juce::String curveToString(MappingCurve curve);
    static MappingCurve stringToCurve(const juce::String& str);
};
