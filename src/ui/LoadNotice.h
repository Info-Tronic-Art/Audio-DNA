#pragma once
#include <string>

// LoadNotice -- lane bf9b S3.4 (s-rta-1002b, ruling-bf9b amendment 9(d)). What the yellow notice in the header row
// (beside the audio-device notice) says after a composition load: an old show converted to the shared layer stack
// (Composition::migrationNote, the one note ShowMigration writes) and / or routine pads left empty by the load
// (Composition::routineLoadNote). The label shows `text`; its tooltip shows `details`. MainComponent keeps it visible
// until the next save, the next load or a click on it. Pure (no JUCE): tests/test_layer_strip_source_deck.cpp.
namespace LoadNotice
{
inline constexpr const char* kConvertedText =
    "Old show converted -- layer looks now come from the first deck (hover for details)";

struct Notice
{
    std::string text;
    std::string details;
    bool shown() const { return !text.empty(); }
};

inline Notice forLoad(const std::string& migrationNote, const std::string& routineLoadNote)
{
    Notice n;
    if (!migrationNote.empty())
    {
        n.text = kConvertedText;
        n.details = migrationNote;
        if (!routineLoadNote.empty())
            n.details += "\n" + routineLoadNote;
    }
    else if (!routineLoadNote.empty())
    {
        n.text = routineLoadNote;
        n.details = routineLoadNote;
    }
    return n;
}
} // namespace LoadNotice
