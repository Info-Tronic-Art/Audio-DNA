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

    // Get directories for different save types
    static juce::File getPresetsDirectory();   // FX presets
    static juce::File getFxSaveDirectory();    // FX quick saves
    static juce::File getDeckDirectory();      // Deck saves

    // Enumerate available preset files.
    static juce::Array<juce::File> getAvailablePresets();

    // Deck save/load — saves everything: audio path, image path, FX, mappings, slot assignments
    struct DeckState
    {
        juce::File audioFile;
        juce::File imageFile;
        juce::File imageFolderPath;
        int slideshowBeatsPerImage = 8;
        int beatRandomCount = 4;
        bool beatRandomEnabled = false;
        int audioSourceMode = 1;    // Combo box ID: 1=Mic, 2=File
        int viewportResolution = 0; // Combo box ID
        int outputDisplay = 1;      // Combo box ID: 1=Off, 2+=display
        float inputGain = 1.0f;
        float masterVideoLevel = 1.0f;
        bool showAudioPanel = true;
        bool showFxPanel = true;
        bool showWavePanel = true;
        bool showKeysPanel = true;
        bool showPresetsPanel = true;
        // Slot assignments
        juce::StringArray slotFiles;
        // Keyboard layout
        juce::Array<juce::var> keyboardKeys;
    };

    static bool saveDeck(const juce::File& file,
                         const DeckState& deck,
                         const EffectChain& chain,
                         const MappingEngine& engine);

    static bool loadDeck(const juce::File& file,
                         DeckState& deck,
                         EffectChain& chain,
                         MappingEngine& engine);

    // String conversion helpers for enums
    static juce::String sourceToString(MappingSource source);
    static MappingSource stringToSource(const juce::String& str);
    static juce::String curveToString(MappingCurve curve);
    static MappingCurve stringToCurve(const juce::String& str);
};
