#pragma once

// AppSettings: the machine's settings.json -- ONE JSON object holding several independent keys
// (s-rta-0927 outputs-c3 = plan5 slice C3, .harmony/.reports/s-rta-0926b/plan5-final.md section 9).
// update() is READ-MODIFY-WRITE: it replaces one key and keeps every other key, known or not, so two writers never
// clobber each other. (Before C3 the MilkDrop writer replaced the whole file with an object holding its one key,
// which would have erased "outputs", and the outputs writer would have erased "milkDropPresetDir".)
// A missing file reads as empty; a corrupt file (or one whose root is not an object) reads as empty and the next
// update() rewrites it as a valid object. Message thread only (every writer runs there) -- no locking.
// Pure juce_core, unit-tested with a temp file (tests/test_app_settings.cpp).

#include <juce_core/juce_core.h>

class AppSettings
{
public:
    // The keys (one place, so the app and the ctest write the same ones).
    static constexpr const char* kMilkDropPresetDir = "milkDropPresetDir";   // string: Preferences > Video folder
    static constexpr const char* kOutputs = "outputs";                       // output::wantedToVar(): the wanted set

    explicit AppSettings(juce::File file) : file_(std::move(file)) {}

    // <userApplicationDataDirectory>/Audio-DNA/settings.json -- ~/Library/Audio-DNA/settings.json on macOS, where
    // JUCE's userApplicationDataDirectory is ~/Library.
    static juce::File defaultFile();

    // TEST MODE only (a test-server build running --test-mode, MainComponent's appSettingsFile()): `overridePath`
    // (AUDIODNA_SETTINGS_FILE) when it is an absolute path, else a scratch file in the temp directory -- the same one
    // for the whole run. NEVER defaultFile(): a test-mode launch without the variable cannot touch the real file.
    static juce::File testModeFile(const juce::String& overridePath);

    const juce::File& file() const noexcept { return file_; }

    // The value stored under `key`; a void var when the file, or the key, is absent (or the file is corrupt).
    juce::var read(const juce::String& key) const;

    // Sets `key` to `value` and rewrites the file, keeping every other key. False if the file could not be written.
    bool update(const juce::String& key, const juce::var& value) const;

private:
    juce::var readRoot() const;   // the file's root object, or a fresh empty object

    juce::File file_;
};
