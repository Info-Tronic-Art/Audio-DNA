#include "model/AppSettings.h"
#include "core/SafeFileWrite.h"

juce::File AppSettings::defaultFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Audio-DNA")
        .getChildFile("settings.json");
}

juce::File AppSettings::testModeFile(const juce::String& overridePath)
{
    if (juce::File::isAbsolutePath(overridePath))
        return juce::File(overridePath);
    static const juce::File scratch = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                          .getNonexistentChildFile("test-mode-settings", ".json", false);
    return scratch;
}

juce::var AppSettings::readRoot() const
{
    if (file_.existsAsFile())
    {
        auto parsed = juce::JSON::parse(file_.loadFileAsString());
        if (parsed.getDynamicObject() != nullptr)
            return parsed;
    }
    return juce::var(new juce::DynamicObject());
}

juce::var AppSettings::read(const juce::String& key) const
{
    return readRoot().getDynamicObject()->getProperty(juce::Identifier(key));
}

bool AppSettings::update(const juce::String& key, const juce::var& value) const
{
    // Lane one-save S1 (ruling-one-save A-15): a file that is there, is not empty and does not parse to an object
    // would be rewritten below WITHOUT its other keys. It is first copied beside itself as "settings.json.unreadable"
    // (replacing an older copy; the copy is verified); when that copy cannot be made the file is left as it is.
    if (file_.existsAsFile() && file_.getSize() > 0
        && juce::JSON::parse(file_.loadFileAsString()).getDynamicObject() == nullptr
        && !safewrite::copyVerified(file_, file_.getSiblingFile(file_.getFileName() + ".unreadable")))
        return false;

    auto root = readRoot();
    root.getDynamicObject()->setProperty(juce::Identifier(key), value);
    file_.getParentDirectory().createDirectory();
    // The verified writer (A-1): the bytes are read back before the swap -- a write cut short leaves the old file.
    return safewrite::writeTextVerified(file_, juce::JSON::toString(root));
}
