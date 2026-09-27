#pragma once
// CanvasSizeCombo -- the Composition inspector's Output Settings "Resolution" dropdown
// (s-rta-0926b canvas fix round). The composition canvas can be any size (a composition
// file, the TestServer), so the dropdown must never show a size the canvas is not:
//   - a preset is selected only on an exact width AND height match;
//   - any other size shows an extra "Custom (W x H)" item holding the live size;
//   - portrait, square and 4:3 presets make those shapes reachable from the UI.
// Ids 1..4 are the four original 16:9 presets, unchanged. Header-only so the headless
// test (tests/test_canvas_size_combo.cpp) drives the same code the inspector runs.
#include <juce_gui_basics/juce_gui_basics.h>

namespace CanvasSizeCombo
{
struct Preset { int w; int h; const char* label; };

inline constexpr Preset kPresets[] = {
    { 1920, 1080, "1920x1080" },
    { 1280,  720, "1280x720" },
    { 2560, 1440, "2560x1440" },
    { 3840, 2160, "3840x2160" },
    { 1080, 1920, "1080x1920 (portrait)" },
    { 1080, 1080, "1080x1080 (square)" },
    { 1024,  768, "1024x768 (4:3)" },
};
inline constexpr int kNumPresets = static_cast<int>(sizeof(kPresets) / sizeof(kPresets[0]));
inline constexpr int kCustomId = 100;   // never a preset id (presets are 1..kNumPresets)

// The combo id for a canvas size: its preset's id on an exact match, else kCustomId.
inline int idFor(int w, int h)
{
    for (int i = 0; i < kNumPresets; ++i)
        if (kPresets[i].w == w && kPresets[i].h == h) return i + 1;
    return kCustomId;
}

inline juce::String customLabel(int w, int h)
{
    return "Custom (" + juce::String(w) + " x " + juce::String(h) + ")";
}

// Rebuild the items for the canvas size w x h and select it (no notification).
// The Custom item exists only while the size matches no preset.
inline void show(juce::ComboBox& combo, int w, int h)
{
    combo.clear(juce::dontSendNotification);
    for (int i = 0; i < kNumPresets; ++i)
        combo.addItem(kPresets[i].label, i + 1);
    const int id = idFor(w, h);
    if (id == kCustomId)
        combo.addItem(customLabel(w, h), kCustomId);
    combo.setSelectedId(id, juce::dontSendNotification);
}

// The size a user selection asks for. False for Custom (it IS the current size) or no selection.
inline bool sizeFor(int id, int& w, int& h)
{
    if (id < 1 || id > kNumPresets) return false;
    w = kPresets[id - 1].w;
    h = kPresets[id - 1].h;
    return true;
}
} // namespace CanvasSizeCombo
