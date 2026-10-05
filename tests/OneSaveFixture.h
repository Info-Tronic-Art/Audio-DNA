#pragma once
// OneSaveFixture (lane one-save, ruling-one-save section 5 "FIXTURES"): the show FILES the lane's unit rows read,
// built in code. None is a file of the user's; every test writes them into its own temp folder only.
//   F-OLD      old shape, as files written before decks became boxes of clips: no "version", no top-level "layers";
//              decks "Deck 1" (id 0) and "Deck 2" (id 100) of 3 rows each; every row carries its layer settings; one
//              row says "persistent"; the old keys "globalTransitionSpeed" and "outputDisplay" are present. Deck 2's
//              row 3 settings DIFFER from Deck 1's (opacity 0.4, Multiply against 1.0, Additive).
//   F-V1       the bf9b shape with no "version" key.
//   F-V3       "version": 3 plus an unknown key "futureThing".
//   F-GARBAGE  the 10 bytes `not json!!`.
#include "model/Composition.h"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

namespace OneSaveFixture
{
inline Clip imageClip(uint32_t id, const std::string& name)
{
    Clip c;
    c.id = id;
    c.name = name;
    c.mediaType = Clip::MediaType::Image;
    return c;
}

// A show of the CURRENT shape, in memory: 3 shared layers (layer 1 at opacity 0.6), decks "Deck 1" and "Deck 2" with
// a few clips, a canvas of 1280 x 720.
inline Composition currentShow()
{
    Composition c;
    c.initDefault();
    c.name = "one-save fixture";
    c.outputWidth = 1280;
    c.outputHeight = 720;
    c.layers[1].opacity = 0.6f;
    c.addDeck("Deck 2");
    c.decks[0].setClip(0, 0, imageClip(11, "a"));
    c.decks[0].setClip(1, 2, imageClip(12, "b"));
    c.decks[1].setClip(2, 1, imageClip(21, "c"));
    return c;
}

// A row as an old file wrote it: Layer::toVar (it always wrote "type") + the row's "clips" (+ "persistent").
inline juce::var oldRow(const Layer& settings, const std::vector<Clip>& clips, int columns, bool persistent = false)
{
    ClipRow row;
    row.ensureColumns(columns);
    for (size_t i = 0; i < clips.size() && i < static_cast<size_t>(columns); ++i)
        row.clips[i] = clips[i];
    juce::var v = settings.toVar();
    auto* obj = v.getDynamicObject();
    obj->setProperty("clips", row.toVar().getDynamicObject()->getProperty("clips"));
    if (persistent)
        obj->setProperty("persistent", true);
    return v;
}

inline Layer rowSettings(const std::string& name, uint32_t id, Layer::Type type, float opacity,
                         Layer::MixMode blend = Layer::MixMode::Additive)
{
    Layer l;
    l.name = name;
    l.id = id;
    l.type = type;
    l.opacity = opacity;
    l.blendMode = blend;
    return l;
}

inline juce::var oldDeck(const std::string& name, int id, int columns, const std::vector<juce::var>& rows)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("name", juce::String(name));
    obj->setProperty("id", id);
    obj->setProperty("numColumns", columns);
    juce::Array<juce::var> arr;
    for (const auto& r : rows)
        arr.add(r);
    obj->setProperty("layers", arr);
    return juce::var(obj);
}

// F-OLD as a parsed var.
inline juce::var oldShowVar()
{
    Composition base;
    base.initDefault();
    base.name = "old show";
    juce::var v = base.toVar();
    auto* obj = v.getDynamicObject();
    obj->removeProperty("version");   // an old file never had one
    obj->removeProperty("layers");    // ... nor a shared layer stack
    obj->setProperty("outputDisplay", -1);
    obj->setProperty("globalTransitionSpeed", 0.5);

    const Layer l1 = rowSettings("Layer 1", 0, Layer::Type::Opaque, 1.0f);
    const Layer l2 = rowSettings("Layer 2", 1, Layer::Type::Transparent, 0.8f, Layer::MixMode::Screen);
    const Layer l3 = rowSettings("Layer 3", 2, Layer::Type::Transparent, 1.0f);
    const Layer l3b = rowSettings("Layer 3", 2, Layer::Type::Transparent, 0.4f, Layer::MixMode::Multiply);
    juce::Array<juce::var> decks;
    decks.add(oldDeck("Deck 1", 0, 3, { oldRow(l1, { imageClip(1, "d1r1") }, 3),
                                        oldRow(l2, { imageClip(2, "d1r2") }, 3, true),
                                        oldRow(l3, { imageClip(3, "d1r3") }, 3) }));
    decks.add(oldDeck("Deck 2", 100, 3, { oldRow(l1, {}, 3),
                                          oldRow(l2, { imageClip(4, "d2r2a"), imageClip(5, "d2r2b") }, 3),
                                          oldRow(l3b, {}, 3) }));
    obj->setProperty("decks", decks);
    return v;
}

// F-V1 as a parsed var: what the build before this lane wrote.
inline juce::var v1ShowVar()
{
    juce::var v = currentShow().toVar();
    v.getDynamicObject()->removeProperty("version");
    return v;
}

// F-V3 as a parsed var: a show from a later build.
inline juce::var v3ShowVar()
{
    juce::var v = currentShow().toVar();
    auto* obj = v.getDynamicObject();
    obj->setProperty("version", 3);
    obj->setProperty("futureThing", "a key this build does not know");
    return v;
}

inline const char* const kGarbage = "not json!!";   // F-GARBAGE (10 bytes)

// A fresh temp folder per test, removed at the end. A name of its own per PROCESS: ctest runs the cases of one binary
// as parallel processes, and two of them asking for "the next free name" at once would get the same folder.
struct TempDir
{
    juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("audiodna-test-one-save-" + juce::Uuid().toString());
    TempDir() { REQUIRE(dir.createDirectory()); }
    ~TempDir() { dir.deleteRecursively(); }

    // Writes `text` (its UTF-8 bytes) to <dir>/<name> -- not through the code under test -- and returns the file.
    juce::File write(const juce::String& name, const juce::String& text) const
    {
        const juce::File f = dir.getChildFile(name);
        REQUIRE(f.replaceWithData(text.toRawUTF8(), text.getNumBytesAsUTF8()));
        return f;
    }
    juce::File writeVar(const juce::String& name, const juce::var& v) const { return write(name, juce::JSON::toString(v)); }
};

inline juce::MemoryBlock bytesOf(const juce::File& f)
{
    juce::MemoryBlock b;
    REQUIRE(f.loadFileAsData(b));
    return b;
}

// The names of a parsed object's keys, in file order.
inline std::vector<std::string> keysOf(const juce::var& v)
{
    std::vector<std::string> out;
    if (auto* obj = v.getDynamicObject())
        for (const auto& p : obj->getProperties())
            out.push_back(p.name.toString().toStdString());
    return out;
}
} // namespace OneSaveFixture
