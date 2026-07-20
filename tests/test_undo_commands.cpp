#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "core/UndoManager.h"
#include "core/Command.h"
#include "core/CompositeCommand.h"
#include "core/ClipCommands.h"
#include "core/UndoService.h"
#include "core/MediaReconnect.h"
#include <optional>

// ============================================================================
// Hand-written deep-equality for Clip (spec §7: deliberately NOT toVar-based).
//
// FIELD COVERAGE: every SERIALIZED / structural field of the CURRENT Clip
// struct (post Wave 1-C) is compared. Deliberately EXCLUDED, with reasons:
//   - thumbnail (juce::Image)          — pixel compare is not meaningful/cheap.
//   - Runtime/mutable render state: playing, playheadPosition, beatsPlayed,
//     hasBeenTriggered, presetPlaylistIndex, presetBeatsPlayed — updated by the
//     render thread, not part of a clip's structural identity. Value-copy undo
//     restores them too, but equality here targets structural state.
// Every other field is included, so a future dropped field surfaces as a test
// failure rather than a silent weakening. Floats use exact == because value-copy
// undo makes bit-identical copies (no arithmetic, so no epsilon needed).
// ============================================================================

static bool operator==(const Clip::SourceParam& a, const Clip::SourceParam& b)
{
    return a.name == b.name && a.uniformName == b.uniformName
        && a.value == b.value && a.defaultValue == b.defaultValue;
}

static bool operator==(const Clip::EffectSlot& a, const Clip::EffectSlot& b)
{
    return a.effectName == b.effectName && a.paramValues == b.paramValues
        && a.dryWet == b.dryWet && a.enabled == b.enabled && a.bypassed == b.bypassed;
}

static bool operator==(const Clip::PresetEntry& a, const Clip::PresetEntry& b)
{
    return a.presetPath == b.presetPath && a.presetName == b.presetName
        && a.mood == b.mood && a.energy == b.energy;
}

template <typename T>
static bool vecEq(const std::vector<T>& a, const std::vector<T>& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (!(a[i] == b[i])) return false;
    return true;
}

static bool operator==(const Clip& a, const Clip& b)
{
    for (int i = 0; i < Clip::kMaxCuepoints; ++i)
        if (a.cuepoints[i] != b.cuepoints[i]) return false;

    return a.name == b.name
        && a.id == b.id
        // Media
        && a.mediaType == b.mediaType && a.mediaFile == b.mediaFile
        && a.cameraDeviceIndex == b.cameraDeviceIndex && a.sourceType == b.sourceType
        && a.hasAlpha == b.hasAlpha
        && vecEq(a.sequenceFiles, b.sequenceFiles) && a.sequenceFps == b.sequenceFps
        && a.beatDivision == b.beatDivision && a.videoBeats == b.videoBeats
        && vecEq(a.sourceParams, b.sourceParams)
        && vecEq(a.effects, b.effects)
        // Transport
        && a.transportMode == b.transportMode && a.loopMode == b.loopMode
        && a.speed == b.speed && a.reverse == b.reverse && a.startOffset == b.startOffset
        && a.inPoint == b.inPoint && a.outPoint == b.outPoint
        && a.beatSnapMode == b.beatSnapMode && a.beatSnap == b.beatSnap
        && a.numCuepoints == b.numCuepoints
        // Autopilot
        && a.autopilotAction == b.autopilotAction && a.autopilotSpecificCol == b.autopilotSpecificCol
        && a.autopilotDuration == b.autopilotDuration && a.autopilotCustomBeats == b.autopilotCustomBeats
        // Video properties
        && a.clipOpacity == b.clipOpacity && a.clipWidth == b.clipWidth && a.clipHeight == b.clipHeight
        && a.blendOverride == b.blendOverride && a.alphaType == b.alphaType
        && a.channelR == b.channelR && a.channelG == b.channelG
        && a.channelB == b.channelB && a.channelA == b.channelA
        // Transform
        && a.positionX == b.positionX && a.positionY == b.positionY
        && a.scale == b.scale && a.rotation == b.rotation
        && a.anchorX == b.anchorX && a.anchorY == b.anchorY
        // MilkDrop playlist (structural config; runtime index excluded)
        && vecEq(a.presetPlaylist, b.presetPlaylist)
        && a.playlistCycleMode == b.playlistCycleMode && a.playlistTrigger == b.playlistTrigger
        && a.playlistTriggerBeats == b.playlistTriggerBeats
        && a.playlistBlendSeconds == b.playlistBlendSeconds
        && a.playlistEnabled == b.playlistEnabled
        // Content lock
        && a.contentLocked == b.contentLocked;
}

// ---------------------------------------------------------------------------
// Test fixtures
// ---------------------------------------------------------------------------

namespace
{
    Composition makeComp()
    {
        Composition comp;
        comp.initDefault();   // 1 deck, 3 layers, 12 columns of empty cells
        return comp;
    }

    // Resolver bound through UndoService (also exercises resolveLayer).
    ClipLayerResolver resolverFor(UndoService& svc)
    {
        return [&svc](int d, int l) { return svc.resolveLayer(d, l); };
    }

    // Deck resolver bound through UndoService (SwapClipsCmd re-resolves the Deck
    // for numColumns + both affected layers).
    ClipDeckResolver deckResolverFor(UndoService& svc)
    {
        return [&svc](int d) { return svc.resolveDeck(d); };
    }

    ClipMediaHook noopMedia()
    {
        return [](const Clip&) {};
    }

    // A clip with a distinctive non-default value in many structural fields, so
    // deep-equal exercises operator== coverage (value-copy fidelity).
    Clip richClip(uint32_t id, const std::string& name)
    {
        Clip c;
        c.name = name;
        c.id = id;
        c.mediaType = Clip::MediaType::Video;
        c.mediaFile = juce::File("/tmp/" + juce::String(name) + ".mp4");
        c.hasAlpha = true;
        c.beatDivision = 8.0f;
        c.videoBeats = 16.0f;
        c.speed = 1.5f;
        c.reverse = true;
        c.inPoint = 0.1f;
        c.outPoint = 0.9f;
        c.clipOpacity = 0.42f;
        c.clipWidth = 1280;
        c.clipHeight = 720;
        c.blendOverride = Clip::BlendOverride::Override;
        c.alphaType = Clip::AlphaType::Straight;
        c.channelR = false; c.channelG = true; c.channelB = false; c.channelA = false;
        c.positionX = 12.5f; c.positionY = -8.0f; c.scale = 2.5f; c.rotation = 45.0f;
        c.anchorX = 3.0f; c.anchorY = -4.0f;
        c.cuepoints[0] = 0.25f; c.cuepoints[1] = 0.5f; c.numCuepoints = 2;

        Clip::EffectSlot fx;
        fx.effectName = "ripple"; fx.paramValues = { 0.1f, 0.2f }; fx.dryWet = 0.33f;
        c.effects.push_back(fx);

        Clip::PresetEntry pe;
        pe.presetPath = "/a.milk"; pe.presetName = "A"; pe.mood = "dark"; pe.energy = 0.2f;
        c.presetPlaylist.push_back(pe);
        c.playlistEnabled = true;
        c.playlistCycleMode = Clip::PlaylistCycleMode::PingPong;
        c.playlistTriggerBeats = 32;
        return c;
    }
}

// ---------------------------------------------------------------------------
// SetClipCmd
// ---------------------------------------------------------------------------

TEST_CASE("SetClipCmd: drop onto an empty cell", "[undo][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(1001, "loop");
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                0, 0, 0, std::nullopt, std::optional<Clip>(clipA), "Drop 'loop'"));

    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr);
    REQUIRE(*comp.decks[0].getClip(0, 0) == clipA);          // execute → post-state
    REQUIRE(mgr.undoDescription() == "Drop 'loop'");

    mgr.undo();
    REQUIRE(comp.decks[0].getClip(0, 0) == nullptr);         // undo → initial (empty)

    mgr.redo();
    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr);
    REQUIRE(*comp.decks[0].getClip(0, 0) == clipA);          // redo → post-state
}

TEST_CASE("SetClipCmd: replace an existing cell (deep-equal both ways)", "[undo][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(1, "before");
    Clip clipB = richClip(2, "after");
    clipB.clipOpacity = 0.77f;   // make them clearly distinct
    comp.decks[0].setClip(1, 3, clipA);

    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                0, 1, 3, std::optional<Clip>(clipA), std::optional<Clip>(clipB), "Replace"));

    REQUIRE(*comp.decks[0].getClip(1, 3) == clipB);
    mgr.undo();
    REQUIRE(*comp.decks[0].getClip(1, 3) == clipA);          // execute→undo == initial
    mgr.redo();
    REQUIRE(*comp.decks[0].getClip(1, 3) == clipB);          // execute→undo→redo == post
}

TEST_CASE("SetClipCmd: clear a cell (after = nullopt)", "[undo][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(7, "victim");
    comp.decks[0].setClip(2, 5, clipA);

    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                0, 2, 5, std::optional<Clip>(clipA), std::nullopt, "Clear Clip"));

    REQUIRE(comp.decks[0].getClip(2, 5) == nullptr);
    mgr.undo();
    REQUIRE(comp.decks[0].getClip(2, 5) != nullptr);
    REQUIRE(*comp.decks[0].getClip(2, 5) == clipA);
    mgr.redo();
    REQUIRE(comp.decks[0].getClip(2, 5) == nullptr);
}

TEST_CASE("SetClipCmd: media hook fires for playable clips only", "[undo][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    int hookCalls = 0;
    ClipMediaHook counting = [&hookCalls](const Clip&) { ++hookCalls; };

    Clip video = richClip(1, "vid");         // MediaType::Video → playable
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), counting,
                0, 0, 0, std::nullopt, std::optional<Clip>(video), "Drop 'vid'"));
    REQUIRE(hookCalls == 1);                  // reconnect guard invoked on apply

    mgr.undo();                               // clears cell → no media reconnect
    REQUIRE(hookCalls == 1);
    mgr.redo();                               // re-applies clip → reconnect again
    REQUIRE(hookCalls == 2);
}

// ---------------------------------------------------------------------------
// ToggleClipLockCmd
// ---------------------------------------------------------------------------

TEST_CASE("ToggleClipLockCmd: flips contentLocked without touching other state", "[undo][lock]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(9, "locktest");
    clipA.contentLocked = false;
    clipA.playheadPosition = 0.5;   // runtime field the toggle must NOT disturb
    comp.decks[0].setClip(0, 2, clipA);

    mgr.perform(std::make_unique<ToggleClipLockCmd>(resolverFor(svc),
                0, 0, 2, false, true, "Lock Content"));
    REQUIRE(comp.decks[0].getClip(0, 2)->contentLocked == true);
    REQUIRE(comp.decks[0].getClip(0, 2)->playheadPosition == 0.5);  // untouched

    mgr.undo();
    REQUIRE(comp.decks[0].getClip(0, 2)->contentLocked == false);
    mgr.redo();
    REQUIRE(comp.decks[0].getClip(0, 2)->contentLocked == true);
}

// ---------------------------------------------------------------------------
// CompositeCommand of SetClipCmds (multi-cell clear — step-4 shape, exercised now)
// ---------------------------------------------------------------------------

TEST_CASE("CompositeCommand clears multiple cells as one undo unit", "[undo][composite][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip a = richClip(1, "a");
    Clip b = richClip(2, "b");
    comp.decks[0].setClip(0, 0, a);
    comp.decks[0].setClip(1, 1, b);

    auto composite = std::make_unique<CompositeCommand>("Clear 2 Clips");
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 0, 0, std::optional<Clip>(a), std::nullopt, "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 1, 1, std::optional<Clip>(b), std::nullopt, "Clear Clip"));
    REQUIRE_FALSE(composite->isEmpty());
    mgr.perform(std::move(composite));

    REQUIRE(comp.decks[0].getClip(0, 0) == nullptr);
    REQUIRE(comp.decks[0].getClip(1, 1) == nullptr);
    REQUIRE(mgr.historySize() == 1);          // one slot for the whole gesture

    mgr.undo();
    REQUIRE(*comp.decks[0].getClip(0, 0) == a);   // both restored (deep-equal)
    REQUIRE(*comp.decks[0].getClip(1, 1) == b);

    mgr.redo();
    REQUIRE(comp.decks[0].getClip(0, 0) == nullptr);
    REQUIRE(comp.decks[0].getClip(1, 1) == nullptr);
}

// ---------------------------------------------------------------------------
// SwapClipsCmd (spec §2 #3): drag-name-bar move/swap, one undo unit
// ---------------------------------------------------------------------------

TEST_CASE("SwapClipsCmd: swap two occupied cells, deep-equal both directions", "[undo][swap]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip a = richClip(1, "a");
    Clip b = richClip(2, "b");
    b.clipOpacity = 0.55f;                    // make the two clearly distinct
    comp.decks[0].setClip(0, 2, a);           // src cell
    comp.decks[0].setClip(1, 5, b);           // dst cell (occupied → a real swap)
    const int cols = comp.decks[0].numColumns;

    int hookCalls = 0;
    ClipMediaHook counting = [&hookCalls](const Clip&) { ++hookCalls; };

    // After the swap: src holds b, dst holds a; numColumns unchanged.
    mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), counting,
        0, /*src*/ 0, 2, /*dst*/ 1, 5,
        std::optional<Clip>(a), std::optional<Clip>(b),   // src before/after
        std::optional<Clip>(b), std::optional<Clip>(a),   // dst before/after
        cols, cols, "Swap Clips"));

    REQUIRE(*comp.decks[0].getClip(0, 2) == b);
    REQUIRE(*comp.decks[0].getClip(1, 5) == a);
    REQUIRE(comp.decks[0].numColumns == cols);
    REQUIRE(mgr.undoDescription() == "Swap Clips");
    REQUIRE(hookCalls == 2);                   // media hook fired for BOTH cells

    mgr.undo();                                // execute→undo == initial
    REQUIRE(*comp.decks[0].getClip(0, 2) == a);
    REQUIRE(*comp.decks[0].getClip(1, 5) == b);
    REQUIRE(comp.decks[0].numColumns == cols);

    mgr.redo();                                // execute→undo→redo == post
    REQUIRE(*comp.decks[0].getClip(0, 2) == b);
    REQUIRE(*comp.decks[0].getClip(1, 5) == a);
    REQUIRE(comp.decks[0].numColumns == cols);
}

TEST_CASE("SwapClipsCmd: move to a far empty column grows then undo shrinks numColumns", "[undo][swap]")
{
    Composition comp = makeComp();            // numColumns == 12
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;

    Clip x = richClip(42, "mover");
    comp.decks[0].setClip(0, 3, x);           // src at (0,3)
    const int colsBefore = comp.decks[0].numColumns;   // 12
    const int dstCol = 15;                    // beyond current column count
    const int colsAfter = dstCol + 1;         // 16

    // Move x from (0,3) onto empty (1,15): src empties, dst gets x, cols 12→16.
    mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), noopMedia(),
        0, /*src*/ 0, 3, /*dst*/ 1, dstCol,
        std::optional<Clip>(x), std::nullopt,             // src before/after
        std::nullopt,          std::optional<Clip>(x),    // dst before/after
        colsBefore, colsAfter, "Move Clip"));

    REQUIRE(comp.decks[0].numColumns == colsAfter);
    REQUIRE(comp.decks[0].getClip(0, 3) == nullptr);            // src emptied
    REQUIRE(comp.decks[0].getClip(1, dstCol) != nullptr);
    REQUIRE(*comp.decks[0].getClip(1, dstCol) == x);           // moved (deep-equal)

    mgr.undo();
    REQUIRE(comp.decks[0].numColumns == colsBefore);           // count shrunk back
    REQUIRE(*comp.decks[0].getClip(0, 3) == x);                // src restored
    REQUIRE(comp.decks[0].getClip(1, dstCol) == nullptr);      // far cell cleared

    mgr.redo();
    REQUIRE(comp.decks[0].numColumns == colsAfter);
    REQUIRE(comp.decks[0].getClip(0, 3) == nullptr);
    REQUIRE(*comp.decks[0].getClip(1, dstCol) == x);
}

// ---------------------------------------------------------------------------
// UndoService coordinate resolution — stale coordinates return null, never crash
// ---------------------------------------------------------------------------

TEST_CASE("UndoService::resolveDeck handles valid, out-of-range, and null", "[undo][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);

    REQUIRE(svc.resolveDeck(0) == &comp.decks[0]);
    REQUIRE(svc.resolveDeck(-1) == nullptr);
    REQUIRE(svc.resolveDeck(5) == nullptr);          // out of range

    UndoService empty;                                // no composition wired
    REQUIRE(empty.resolveDeck(0) == nullptr);
}

TEST_CASE("UndoService::resolveLayer returns null for stale layer coordinates", "[undo][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);

    REQUIRE(svc.resolveLayer(0, 0) != nullptr);
    REQUIRE(svc.resolveLayer(0, 2) != nullptr);      // 3 layers: 0,1,2
    REQUIRE(svc.resolveLayer(0, 3) == nullptr);      // beyond count
    REQUIRE(svc.resolveLayer(0, -1) == nullptr);
    REQUIRE(svc.resolveLayer(9, 0) == nullptr);      // stale deck

    // Remove a layer; the previously-valid trailing index is now stale.
    REQUIRE(comp.decks[0].removeLayer(2));
    REQUIRE(svc.resolveLayer(0, 2) == nullptr);
}

// ---------------------------------------------------------------------------
// needsVideoReopen — the pure decision behind the media-hook file-compare fix
// ---------------------------------------------------------------------------

TEST_CASE("needsVideoReopen covers missing/match/mismatch/no-player", "[undo][media]")
{
    juce::File a("/tmp/a.mp4");
    juce::File b("/tmp/b.mp4");

    // No player yet -> must open (regardless of the 'loaded' arg).
    REQUIRE(needsVideoReopen(juce::File(), a, /*playerExists*/false) == true);
    // Player exists with the SAME file -> no reopen (keeps double-apply idempotent).
    REQUIRE(needsVideoReopen(a, a, true) == false);
    // Player exists with a DIFFERENT file -> reopen (the replace-undo bug case).
    REQUIRE(needsVideoReopen(b, a, true) == true);
    // Player exists but its file is empty/unknown vs a real clip file -> reopen.
    REQUIRE(needsVideoReopen(juce::File(), a, true) == true);
}

TEST_CASE("UndoService::resolveClip returns null for empty cells and stale coords", "[undo][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);

    REQUIRE(svc.resolveClip(0, 0, 0) == nullptr);    // valid coord, empty cell
    comp.decks[0].setClip(0, 0, richClip(1, "x"));
    REQUIRE(svc.resolveClip(0, 0, 0) != nullptr);    // now occupied
    REQUIRE(svc.resolveClip(0, 3, 0) == nullptr);    // stale layer
    REQUIRE(svc.resolveClip(0, 0, 999) == nullptr);  // column past the grid
}
