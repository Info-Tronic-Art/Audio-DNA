#pragma once
// SafeFileWrite (lane one-save S1, ruling-one-save A-1): the VERIFIED writer. juce::File::replaceWithText writes a
// hidden temporary file, DROPS the result of that write and swaps whatever was stored (juce_File.cpp
// replaceWithText / juce_TemporaryFile.cpp): a write cut short -- a full disk -- puts a cut-off file over the target
// and reports success. Here the temporary file is READ BACK and its bytes must equal the bytes written BEFORE the
// swap; no status flag is trusted alone. False at any step: the temporary file is deleted, the target is untouched.
// Users: Composition::saveToFile (the show), showfile::backupBeforeOverwrite (the copy kept before an old file is
// written over), AppSettings::update (settings.json). Message thread only -- nothing here may run on the audio
// callback, the analysis thread or the render thread. Pure juce_core; unit rows tests/test_safe_write.cpp (SW-1..4).
#include <juce_core/juce_core.h>
#include <cstring>
#include <functional>

namespace safewrite
{
// Stores `size` bytes of `data` in `dest` (a file that does not exist yet) and answers whether it believes it did.
// The seam of the unit rows: a test passes a writer that stores half the bytes and reports success.
using WriteFn = std::function<bool(const juce::File& dest, const void* data, size_t size)>;

// The production writer. `dest` is deleted first: a juce::FileOutputStream opens an existing file at its END.
inline bool writeBytes(const juce::File& dest, const void* data, size_t size)
{
    if (dest.exists() && !dest.deleteFile())
        return false;
    juce::FileOutputStream out(dest);
    if (!out.openedOk())
        return false;
    if (size > 0 && !out.write(data, size))
        return false;
    out.flush();
    return out.getStatus().wasOk();
}

// True when `file` holds exactly `size` bytes equal to `data`.
inline bool holdsExactly(const juce::File& file, const void* data, size_t size)
{
    juce::MemoryBlock back;
    if (!file.existsAsFile() || !file.loadFileAsData(back))
        return false;
    return back.getSize() == size && (size == 0 || std::memcmp(back.getData(), data, size) == 0);
}

// Writes `text` (its UTF-8 bytes, as they are) over `target`: temporary file beside the target -> read back, bytes
// equal -> swap. False = nothing changed: the target holds what it held, no temporary file is left behind.
inline bool writeTextVerified(const juce::File& target, const juce::String& text, const WriteFn& write)
{
    const char* const data = text.toRawUTF8();
    const size_t size = text.getNumBytesAsUTF8();
    juce::TemporaryFile temp(target, juce::TemporaryFile::useHiddenFile);
    const juce::File tempFile = temp.getFile();
    const bool stored = write(tempFile, data, size);   // the writer (its stream) is gone before the read-back
    if (!stored || !holdsExactly(tempFile, data, size))
    {
        tempFile.deleteFile();
        return false;
    }
    return temp.overwriteTargetFileWithTemporary();
}

inline bool writeTextVerified(const juce::File& target, const juce::String& text)
{
    return writeTextVerified(target, text, writeBytes);
}

// Copies `from` to `to` (replacing an older `to`), then `to`'s bytes must equal `from`'s; else `to` is deleted and
// the answer is false.
inline bool copyVerified(const juce::File& from, const juce::File& to, const WriteFn& write)
{
    juce::MemoryBlock bytes;
    if (!from.existsAsFile() || !from.loadFileAsData(bytes))
        return false;
    if (write(to, bytes.getData(), bytes.getSize()) && holdsExactly(to, bytes.getData(), bytes.getSize()))
        return true;
    to.deleteFile();
    return false;
}

inline bool copyVerified(const juce::File& from, const juce::File& to)
{
    return copyVerified(from, to, writeBytes);
}
} // namespace safewrite
