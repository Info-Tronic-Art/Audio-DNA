// S166-L8: headless model-layer coverage for the composition-tier oracle
// (src/test/TestServer.cpp's handleAddGlobalEffect / handleRemoveGlobalEffect /
// handleSetGlobalEffectBypass / handleSetCompositionParams / handleSetClipOpacity).
//
// SCOPE, read before extending: this exercises the REAL Composition,
// Clip::EffectSlot, and EffectLibrary types those handlers mutate — the same
// vector, the same struct fields, the same registry lookups — proving the
// model-layer contract the endpoints depend on: an unknown effect name has
// no EffectDef, a known one round-trips its declared defaults, add/remove
// obey vector semantics and reject an out-of-range index without mutating
// anything, a bypass write touches only its own slot, and the plain scalar
// fields (masterOpacity/masterSpeed/compOpacity/clipOpacity) read back
// whatever was last written.
//
// It does NOT execute TestServer.cpp's own handler bodies (JSON parsing/
// dispatch, or the GL-thread fence in handleAddGlobalEffect/
// handleRemoveGlobalEffect via executeOnGLThread) — this rig's ctest cannot
// link Renderer.cpp/CompositorEngine.cpp headlessly (no display/GL context),
// the same documented constraint test_renderer_source_confinement.cpp and
// test_clip_replace_media_retire.cpp work around by mirroring their target
// mechanism instead of including it. TestServer.cpp is one translation unit
// with those handlers, so pulling in any symbol from it pulls in the whole
// Renderer/CompositorEngine link graph. The live HTTP surface (JSON body
// shapes, the GL-thread confinement actually holding under a running
// render loop, the render-frame diff) is verified by the behavioral gate
// against the running app, not here.

#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "effects/EffectLibrary.h"

namespace {

EffectLibrary& testLibrary()
{
    static EffectLibrary lib = [] {
        EffectLibrary l;
        l.registerDefaults();
        return l;
    }();
    return lib;
}

} // namespace

TEST_CASE("composition-tier oracle: unknown effect name has no EffectDef", "[composition-tier-oracle]")
{
    // Mirrors handleAddGlobalEffect's validation gate: a null EffectDef is
    // what turns into TestServer.cpp's 404 "Effect not found" response
    // rather than a silently-added, garbage-named slot.
    REQUIRE(testLibrary().getEffectDef("definitely_not_a_real_effect_xyz") == nullptr);
}

TEST_CASE("composition-tier oracle: adding a known global effect uses registry defaults", "[composition-tier-oracle]")
{
    auto& lib = testLibrary();
    REQUIRE(lib.getNumEffects() > 0);
    juce::String name = lib.getEffectNames()[0];
    const auto* def = lib.getEffectDef(name);
    REQUIRE(def != nullptr);

    Composition comp;
    comp.initDefault();
    REQUIRE(comp.globalEffects.empty());

    // The same construction handleAddGlobalEffect performs: name + enabled
    // = true + bypassed = false + paramValues seeded from
    // def->params[].defaultValue (EffectStackView::itemDropped's own
    // append-to-chain construction, EffectStackView.cpp:512-519).
    Clip::EffectSlot slot;
    slot.effectName = name.toStdString();
    slot.enabled = true;
    slot.bypassed = false;
    for (const auto& p : def->params)
        slot.paramValues.push_back(p.defaultValue);
    comp.globalEffects.push_back(slot);

    REQUIRE(comp.globalEffects.size() == 1);
    CHECK(comp.globalEffects[0].effectName == name.toStdString());
    CHECK(comp.globalEffects[0].enabled);
    CHECK_FALSE(comp.globalEffects[0].bypassed);
    REQUIRE(comp.globalEffects[0].paramValues.size() == def->params.size());
    for (size_t i = 0; i < def->params.size(); ++i)
        CHECK(comp.globalEffects[0].paramValues[i] == def->params[i].defaultValue);
}

TEST_CASE("composition-tier oracle: bypass write flips only the targeted slot", "[composition-tier-oracle]")
{
    Composition comp;
    comp.initDefault();
    Clip::EffectSlot a; a.effectName = "a";
    Clip::EffectSlot b; b.effectName = "b";
    comp.globalEffects.push_back(a);
    comp.globalEffects.push_back(b);

    REQUIRE_FALSE(comp.globalEffects[0].bypassed);
    REQUIRE_FALSE(comp.globalEffects[1].bypassed);

    // handleSetGlobalEffectBypass's actual write: a single indexed field
    // assignment, index 1 only.
    comp.globalEffects[1].bypassed = true;

    CHECK_FALSE(comp.globalEffects[0].bypassed);
    CHECK(comp.globalEffects[1].bypassed);
}

TEST_CASE("composition-tier oracle: remove-by-index rejects out-of-range and leaves the vector untouched", "[composition-tier-oracle]")
{
    Composition comp;
    comp.initDefault();
    Clip::EffectSlot a; a.effectName = "a";
    Clip::EffectSlot b; b.effectName = "b";
    comp.globalEffects.push_back(a);
    comp.globalEffects.push_back(b);

    // handleRemoveGlobalEffect's actual bounds check + erase.
    auto tryRemove = [&comp](int index) {
        if (index >= 0 && index < static_cast<int>(comp.globalEffects.size()))
        {
            comp.globalEffects.erase(comp.globalEffects.begin() + index);
            return true;
        }
        return false;
    };

    CHECK_FALSE(tryRemove(-1));
    CHECK_FALSE(tryRemove(2));
    REQUIRE(comp.globalEffects.size() == 2); // untouched by either rejected call

    CHECK(tryRemove(0));
    REQUIRE(comp.globalEffects.size() == 1);
    CHECK(comp.globalEffects[0].effectName == "b");
}

TEST_CASE("composition-tier oracle: the four render-dead scalars round-trip", "[composition-tier-oracle]")
{
    // masterOpacity/masterSpeed/compOpacity/clipOpacity: no renderer
    // consumer exists yet (S166-L8 packet), so this proves only that the
    // model-layer fields hold whatever was last written — exactly what
    // handleSetCompositionParams/handleGetCompositionParams and
    // handleSetClipOpacity read back over HTTP.
    Composition comp;
    comp.initDefault();

    comp.masterOpacity = 0.42f;
    comp.masterSpeed = 2.5f;
    comp.compOpacity = 0.13f;
    CHECK(comp.masterOpacity == 0.42f);
    CHECK(comp.masterSpeed == 2.5f);
    CHECK(comp.compOpacity == 0.13f);

    Clip clip;
    clip.clipOpacity = 0.77f;
    CHECK(clip.clipOpacity == 0.77f);
}
