#pragma once
// ShowFile (lane one-save S1, ruling-one-save A-2 / A-3): the show file's VERSION and the copy kept before an older
// file is written over.
//  - A show written by this build starts with "version": 2. A version counts ONLY as an integer of 2 or more; anything
//    else (no key, 0, 1, a negative number, a string, a bool, a real number) is "no version", and then the one
//    grandfathered presence test names the shape: no top-level "layers" array = 0 (the old shape, converted in memory
//    by ShowMigration), else 1 (the bf9b shape). From 2 on, no reader tells a shape by a key's presence.
//  - Two blocks ride in the file beside what Composition::toVar writes -- "keys" and "layout" (ShowExtras). They are
//    NOT model fields and toVar never carries them (toVar also runs on the HTTP thread, GET /api/composition).
//  - Before ANY existing file that is not a readable version-2 show is written over, its bytes are copied to
//    <its folder>/backups/<name>.<tag>.json (tag v0, v1, v3.., or "other") and the copy is verified; when the copy
//    cannot be made NOTHING is written (saveWithBackup). The rule is about the FILE on disk, whatever is in memory.
// Message thread only. juce_core plus ShowMigration::isLegacyShow; unit rows tests/test_show_file.cpp (SF-1..SF-10)
// and tests/test_show_backup.cpp (SB-1..SB-7).
#include "core/SafeFileWrite.h"
#include "model/ShowMigration.h"
#include <juce_core/juce_core.h>
#include <functional>

namespace showfile
{
inline constexpr int kShowVersion = 2;

// The version a parsed show file states (see the header: an integer >= 2), else 0 (old shape) or 1 (bf9b shape).
inline int versionOf(const juce::var& root)
{
    if (auto* obj = root.getDynamicObject())
    {
        const juce::var v = obj->getProperty("version");
        if ((v.isInt() || v.isInt64()) && static_cast<juce::int64>(v) >= 2)
            return static_cast<int>(static_cast<juce::int64>(v));
    }
    return ShowMigration::isLegacyShow(root) ? 0 : 1;
}

// The two blocks the file carries beside the composition: "keys" (BindingManager's object) and "layout" (the window
// dividers). Raw vars: void = absent. Whoever applies a block validates it whole; a bad block never stops a load.
struct ShowExtras
{
    juce::var keys;
    juce::var layout;
};

enum class Backup { NotNeeded, Made, AlreadyThere, Failed };

inline const char* backupName(Backup b)
{
    switch (b)
    {
        case Backup::NotNeeded:    return "not_needed";
        case Backup::Made:         return "made";
        case Backup::AlreadyThere: return "already_there";
        case Backup::Failed:       return "failed";
    }
    return "failed";
}

// The tag of the copy a file with this parsed content needs before it is written over; "" = none (a version-2 show).
inline juce::String backupTagFor(const juce::var& parsed)
{
    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr || obj->getProperty("decks").getArray() == nullptr)
        return "other";                                   // does not parse, not an object, or no "decks": not a show
    const int version = versionOf(parsed);
    return version == kShowVersion ? juce::String() : "v" + juce::String(version);
}

// <target's folder>/backups/<base name>.<tag>.json; n >= 2 gives "<base name>.<tag> (n).json".
inline juce::File backupFileFor(const juce::File& target, const juce::String& tag, int n = 1)
{
    juce::String name = target.getFileNameWithoutExtension() + "." + tag;
    if (n > 1)
        name += " (" + juce::String(n) + ")";
    return target.getParentDirectory().getChildFile("backups").getChildFile(name + ".json");
}

// Reads `target` as it is on disk NOW and, when it is anything but a version-2 show, keeps a verified copy of its
// bytes under backups/. An existing copy with the same bytes is kept (AlreadyThere); one with other bytes is never
// overwritten (the copy takes the next free name). Failed = the folder or the copy could not be made or verified:
// the caller must then write NOTHING. `write` is the unit rows' seam (safewrite::WriteFn).
inline Backup backupBeforeOverwrite(const juce::File& target, const safewrite::WriteFn& write)
{
    if (!target.existsAsFile() || target.getSize() == 0)
        return Backup::NotNeeded;                         // missing or empty: nothing to keep
    juce::MemoryBlock bytes;
    if (!target.loadFileAsData(bytes))
        return Backup::Failed;                            // it is there and cannot be read: never written over blind
    const juce::String tag = backupTagFor(juce::JSON::parse(bytes.toString()));
    if (tag.isEmpty())
        return Backup::NotNeeded;

    const juce::File dir = target.getParentDirectory().getChildFile("backups");
    if (!dir.isDirectory())
        dir.createDirectory();
    if (!dir.isDirectory())
        return Backup::Failed;                            // e.g. a plain FILE named "backups" sits there
    for (int n = 1; n <= 999; ++n)
    {
        const juce::File copy = backupFileFor(target, tag, n);
        if (!copy.exists())
            return safewrite::copyVerified(target, copy, write) ? Backup::Made : Backup::Failed;
        if (safewrite::holdsExactly(copy, bytes.getData(), bytes.getSize()))
            return Backup::AlreadyThere;
    }
    return Backup::Failed;
}

inline Backup backupBeforeOverwrite(const juce::File& target)
{
    return backupBeforeOverwrite(target, safewrite::writeBytes);
}

struct SaveOutcome
{
    bool saved = false;
    Backup backup = Backup::NotNeeded;
};

// The ONE order of a show write: the copy rule first, fail-closed, then `writeShow(target)` (the caller's verified
// write). MainComponent::writeShow is its only src caller; the unit rows call it with a real Composition.
inline SaveOutcome saveWithBackup(const juce::File& target, const std::function<bool(const juce::File&)>& writeShow,
                                  const safewrite::WriteFn& copyWriter = safewrite::writeBytes)
{
    SaveOutcome out;
    out.backup = backupBeforeOverwrite(target, copyWriter);
    if (out.backup == Backup::Failed)
        return out;                                       // nothing is written
    out.saved = writeShow(target);
    return out;
}
} // namespace showfile
