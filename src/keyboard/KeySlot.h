#pragma once
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <array>

// KeySlot: data model for a single key in the 40-key keyboard launcher.
// Each key can hold media (image/video/camera), an independent effects chain,
// transparency settings, and playback behavior (latch, random).
struct KeySlot
{
    // === Identity ===
    int row = 0;            // 0-3
    int col = 0;            // 0-9
    char keyChar = ' ';     // Display character (Q, W, E, etc.)
    int keyCode = 0;        // JUCE key code for triggering

    // === Media ===
    enum class MediaType { None, Image, VideoFile, Camera };
    MediaType mediaType = MediaType::None;
    juce::File mediaFile;           // For Image or VideoFile
    int cameraDeviceIndex = -1;     // For Camera

    // === Per-key Effects ===
    struct EffectSlot
    {
        std::string effectName;         // Registry name e.g. "ripple"
        std::vector<float> params;      // Parameter values [0,1]
        bool enabled = true;
    };
    std::vector<EffectSlot> effects;

    // === Audio Mappings (per key) ===
    // Stored separately — will be wired in Phase 2

    // === Transparency ===
    enum class TransparencyMode { Alpha, LumaKey, ChromaKey, Light };
    TransparencyMode transparencyMode = TransparencyMode::Alpha;
    float opacity = 1.0f;               // Master opacity [0,1]
    float lumaKeyThreshold = 0.1f;      // Luma key: below this = transparent
    float lumaKeySoftness = 0.05f;      // Luma key: transition range
    float chromaKeyR = 0.0f;            // Chroma key target color
    float chromaKeyG = 1.0f;
    float chromaKeyB = 0.0f;
    float chromaKeyTolerance = 0.2f;
    float chromaKeySoftness = 0.1f;

    // === Playback Behavior ===
    bool latched = false;               // true = toggle, false = momentary
    int latchBeatDuration = 0;          // 0 = infinite, N = auto-release after N beats
    bool ignoreRandom = false;          // Skip in random mode
    int randomBeatDuration = 4;         // When random triggers this key, hold for N beats

    // === Runtime State (not serialized) ===
    bool active = false;                // Currently playing
    int activationOrder = 0;            // For stacking (lower = bottom, higher = top)
    int beatsSinceActivation = 0;       // For auto-release countdown
    bool activatedByRandom = false;     // Was this key activated by random mode?

    // === Helpers ===
    bool hasMedia() const { return mediaType != MediaType::None; }
    bool hasEffects() const { return !effects.empty(); }
    bool isEmpty() const { return !hasMedia() && !hasEffects(); }

    void clear()
    {
        mediaType = MediaType::None;
        mediaFile = juce::File();
        cameraDeviceIndex = -1;
        effects.clear();
        transparencyMode = TransparencyMode::Alpha;
        opacity = 1.0f;
        latched = false;
        latchBeatDuration = 0;
        ignoreRandom = false;
        randomBeatDuration = 4;
        deactivate();
    }

    void activate(int order)
    {
        active = true;
        activationOrder = order;
        beatsSinceActivation = 0;
        activatedByRandom = false;
    }

    void deactivate()
    {
        active = false;
        activationOrder = 0;
        beatsSinceActivation = 0;
        activatedByRandom = false;
    }
};

// The full keyboard: 4 rows × 10 columns = 40 keys
struct KeyboardLayout
{
    static constexpr int kNumRows = 4;
    static constexpr int kNumCols = 10;
    static constexpr int kNumKeys = kNumRows * kNumCols;

    std::array<KeySlot, kNumKeys> keys;
    int nextActivationOrder = 1;    // Monotonically increasing for stacking

    // Key character layout matching physical keyboard
    static constexpr char kKeyChars[kNumRows][kNumCols] = {
        { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
        { 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P' },
        { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';' },
        { 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/' }
    };

    KeyboardLayout()
    {
        for (int r = 0; r < kNumRows; ++r)
        {
            for (int c = 0; c < kNumCols; ++c)
            {
                auto& key = keys[static_cast<size_t>(r * kNumCols + c)];
                key.row = r;
                key.col = c;
                key.keyChar = kKeyChars[r][c];
            }
        }
    }

    KeySlot& at(int row, int col) { return keys[static_cast<size_t>(row * kNumCols + col)]; }
    const KeySlot& at(int row, int col) const { return keys[static_cast<size_t>(row * kNumCols + col)]; }

    KeySlot* findByChar(char c)
    {
        for (auto& key : keys)
            if (key.keyChar == c)
                return &key;
        return nullptr;
    }

    // Get active keys sorted by activation order (bottom to top)
    std::vector<KeySlot*> getActiveKeysSorted()
    {
        std::vector<KeySlot*> active;
        for (auto& key : keys)
            if (key.active)
                active.push_back(&key);
        std::sort(active.begin(), active.end(),
            [](const KeySlot* a, const KeySlot* b) {
                return a->activationOrder < b->activationOrder;
            });
        return active;
    }

    void activateKey(KeySlot& key)
    {
        if (!key.active)
        {
            key.activate(nextActivationOrder++);
        }
    }

    void deactivateKey(KeySlot& key)
    {
        key.deactivate();
    }

    void toggleKey(KeySlot& key)
    {
        if (key.active)
            deactivateKey(key);
        else
            activateKey(key);
    }
};
