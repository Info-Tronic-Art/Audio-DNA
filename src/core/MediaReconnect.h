#pragma once
#include <juce_core/juce_core.h>

// Decide whether a clip's renderer-side video player must be (re)opened.
//
// Video players are keyed by clip id and never closed, so a plain
// reconnect-if-missing guard misses the id-stable content-swap case: undo/redo
// of a video->video replaceContent keeps the same clip id but changes the media
// file, leaving videoPlayers_[id] decoding the WRONG file. Reopen iff no player
// exists OR the loaded file differs from the file the clip now wants. A matching
// file returns false so the initial-drop double-apply stays a no-op (idempotent).
inline bool needsVideoReopen(const juce::File& loadedFile,
                             const juce::File& clipFile,
                             bool playerExists)
{
    return !playerExists || loadedFile != clipFile;
}
