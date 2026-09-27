#include "model/AppSettings.h"

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
    auto root = readRoot();
    root.getDynamicObject()->setProperty(juce::Identifier(key), value);
    file_.getParentDirectory().createDirectory();
    return file_.replaceWithText(juce::JSON::toString(root));
}
