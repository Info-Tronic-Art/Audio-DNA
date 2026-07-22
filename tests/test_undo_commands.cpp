#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "core/UndoManager.h"
#include "core/Command.h"
#include "core/CompositeCommand.h"
#include "core/ClipCommands.h"
#include "core/DeckCommands.h"
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

// ===========================================================================
// Step 4 composites: column ops, layer/deck clear, multi-cell drop growth.
// These mirror the exact command shapes built by the MainComponent handlers.
// ===========================================================================

// ---------------------------------------------------------------------------
// SetColumnCountCmd (#25 menu Add Column + drop-growth restore)
// ---------------------------------------------------------------------------

TEST_CASE("SetColumnCountCmd: add column grows count, undo/redo round-trip", "[undo][column]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const int before = deck.numColumns;   // 12
    deck.addColumn();                      // live mutation (mutate-then-push): 13
    const int after = deck.numColumns;     // 13

    mgr.perform(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc),
                0, before, after, "Add Column"));

    REQUIRE(deck.numColumns == after);
    for (auto& L : deck.layers)
        REQUIRE(static_cast<int>(L.clips.size()) >= after);   // every layer grown
    REQUIRE(mgr.undoDescription() == "Add Column");

    mgr.undo();
    REQUIRE(deck.numColumns == before);    // count restored (cells stay, invisible)
    mgr.redo();
    REQUIRE(deck.numColumns == after);
}

// ---------------------------------------------------------------------------
// RemoveColumnCmd (#26): removing the last column restores its cells on undo
// ---------------------------------------------------------------------------

TEST_CASE("RemoveColumnCmd: remove-column-with-clips restores cells + count", "[undo][column]")
{
    Composition comp = makeComp();         // 12 columns, 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const int col = deck.numColumns - 1;   // 11 (the last column)
    const int before = deck.numColumns;    // 12
    Clip c0 = richClip(1, "l0c11");
    Clip c2 = richClip(2, "l2c11");
    deck.setClip(0, col, c0);              // layer 0 occupied at last column
    deck.setClip(2, col, c2);              // layer 2 occupied; layer 1 empty

    // Mirror kColumnRemove: snapshot the removed column, then remove.
    std::vector<std::optional<Clip>> removed;
    for (auto& L : deck.layers)
        removed.push_back(L.getClipAt(col) ? std::optional<Clip>(*L.getClipAt(col))
                                           : std::nullopt);
    deck.removeColumn(col);

    mgr.perform(std::make_unique<RemoveColumnCmd>(deckResolverFor(svc), noopMedia(),
                0, col, before, std::move(removed), "Remove Column"));

    REQUIRE(deck.numColumns == before - 1);          // 11
    REQUIRE(deck.getClip(0, col) == nullptr);        // column no longer addressable
    REQUIRE(mgr.undoDescription() == "Remove Column");

    mgr.undo();
    REQUIRE(deck.numColumns == before);              // 12 restored
    REQUIRE(*deck.getClip(0, col) == c0);            // occupied cells restored
    REQUIRE(deck.getClip(1, col) == nullptr);        // empty layer stays empty
    REQUIRE(*deck.getClip(2, col) == c2);

    mgr.redo();
    REQUIRE(deck.numColumns == before - 1);
    REQUIRE(deck.getClip(0, col) == nullptr);
}

// ---------------------------------------------------------------------------
// ClearLayerClipsCmd (#19): clear one layer's clips row + runtime
// ---------------------------------------------------------------------------

TEST_CASE("ClearLayerClipsCmd: clear one layer, undo restores clips + runtime", "[undo][clearclips]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    Clip a = richClip(1, "a"), b = richClip(2, "b");
    deck.setClip(1, 0, a);
    deck.setClip(1, 4, b);
    deck.getLayer(1)->activeClipColumn = 4;          // mark an active clip

    // Mirror kLayerClearClips (Wave 1-D: selected layer only).
    LayerClipsSnapshot before = captureLayerClips(*deck.getLayer(1));
    REQUIRE(layerClipsSnapshotHasContent(before));
    deck.getLayer(1)->clips.clear();
    deck.getLayer(1)->ensureColumns(deck.numColumns);
    deck.getLayer(1)->clearActiveClip();
    LayerClipsSnapshot after = captureLayerClips(*deck.getLayer(1));

    mgr.perform(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopMedia(),
                0, 1, before, after, "Clear Layer Clips"));

    REQUIRE(deck.getClip(1, 0) == nullptr);
    REQUIRE(deck.getClip(1, 4) == nullptr);
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);

    mgr.undo();
    REQUIRE(*deck.getClip(1, 0) == a);               // clips restored (deep-equal)
    REQUIRE(*deck.getClip(1, 4) == b);
    REQUIRE(deck.getLayer(1)->activeClipColumn == 4);// runtime restored

    mgr.redo();
    REQUIRE(deck.getClip(1, 0) == nullptr);
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);
}

// ---------------------------------------------------------------------------
// Deck clear-clips (#23): composite of ClearLayerClipsCmd, empties skipped
// ---------------------------------------------------------------------------

TEST_CASE("Deck clear-clips composite: clear all layers, one entry, undo restores all", "[undo][composite][clearclips]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    Clip a = richClip(1, "a"), b = richClip(2, "b");
    deck.setClip(0, 2, a);                 // layer 0 has content
    deck.setClip(2, 7, b);                 // layer 2 has content; layer 1 empty

    // Mirror kDeckClearClips: one ClearLayerClipsCmd per layer WITH content.
    auto composite = std::make_unique<CompositeCommand>("Clear Deck Clips");
    for (int l = 0; l < deck.getNumLayers(); ++l)
    {
        auto* layer = deck.getLayer(l);
        LayerClipsSnapshot cBefore = captureLayerClips(*layer);
        if (!layerClipsSnapshotHasContent(cBefore)) continue;   // skip empty layer 1
        layer->clips.clear();
        layer->ensureColumns(deck.numColumns);
        layer->clearActiveClip();
        LayerClipsSnapshot cAfter = captureLayerClips(*layer);
        composite->add(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopMedia(),
                       0, l, cBefore, cAfter, "Clear Layer Clips"));
    }
    REQUIRE(composite->size() == 2);       // only layers 0 and 2 contributed
    mgr.perform(std::move(composite));

    REQUIRE(deck.getClip(0, 2) == nullptr);
    REQUIRE(deck.getClip(2, 7) == nullptr);
    REQUIRE(mgr.historySize() == 1);       // one gesture

    mgr.undo();
    REQUIRE(*deck.getClip(0, 2) == a);     // all restored
    REQUIRE(*deck.getClip(2, 7) == b);

    mgr.redo();
    REQUIRE(deck.getClip(0, 2) == nullptr);
    REQUIRE(deck.getClip(2, 7) == nullptr);
}

// ---------------------------------------------------------------------------
// Multi-video drop composite (#7): N cells + column growth undone together
// ---------------------------------------------------------------------------

TEST_CASE("Multi-video drop composite: N cells + column growth, undo restores both", "[undo][composite][column]")
{
    Composition comp = makeComp();         // 12 columns
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const int colsBefore = deck.numColumns;    // 12
    const int startCol = 11;                    // drop starts inside the grid
    const int colsAfter = startCol + 3;         // 14 (grows past the grid)
    Clip v0 = richClip(1, "v0"), v1 = richClip(2, "v1"), v2 = richClip(3, "v2");

    // Mirror onMultiVideoDropped's live growth + placement (mutate-then-push).
    while (deck.numColumns < colsAfter)
    {
        deck.numColumns++;
        for (auto& L : deck.layers) L.clips.resize(static_cast<size_t>(deck.numColumns));
    }
    deck.setClip(0, startCol + 0, v0);
    deck.setClip(0, startCol + 1, v1);
    deck.setClip(0, startCol + 2, v2);

    // Same composite shape: SetColumnCountCmd FIRST (undoes LAST), then N SetClipCmds.
    auto composite = std::make_unique<CompositeCommand>("Drop 3 Videos");
    composite->add(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc),
                   0, colsBefore, colsAfter, "Resize Columns"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 0, startCol + 0, std::nullopt, std::optional<Clip>(v0), "Drop 3 Videos"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 0, startCol + 1, std::nullopt, std::optional<Clip>(v1), "Drop 3 Videos"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 0, startCol + 2, std::nullopt, std::optional<Clip>(v2), "Drop 3 Videos"));
    mgr.perform(std::move(composite));

    REQUIRE(mgr.historySize() == 1);
    REQUIRE(deck.numColumns == colsAfter);
    REQUIRE(*deck.getClip(0, startCol + 0) == v0);
    REQUIRE(*deck.getClip(0, startCol + 2) == v2);

    mgr.undo();
    REQUIRE(deck.numColumns == colsBefore);          // column growth undone
    REQUIRE(deck.getClip(0, startCol + 0) == nullptr);   // cells restored (empty)
    REQUIRE(deck.getClip(0, startCol + 2) == nullptr);

    mgr.redo();
    REQUIRE(deck.numColumns == colsAfter);
    REQUIRE(*deck.getClip(0, startCol + 1) == v1);
}

// ---------------------------------------------------------------------------
// Multi-select clear composite (#4): matches kClipClear's blank-Clip{} after
// ---------------------------------------------------------------------------

TEST_CASE("Multi-select clear composite (Clear N Clips): one entry, undo restores all", "[undo][composite][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    Clip a = richClip(1, "a"), b = richClip(2, "b"), c = richClip(3, "c");
    deck.setClip(0, 0, a);
    deck.setClip(1, 3, b);
    deck.setClip(2, 6, c);

    // kClipClear sets each selected cell to a blank Clip{} (HEAD behavior),
    // then composites the N SetClipCmds into one "Clear 3 Clips" entry.
    auto composite = std::make_unique<CompositeCommand>("Clear 3 Clips");
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 0, 0, std::optional<Clip>(a), std::optional<Clip>(Clip{}), "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 1, 3, std::optional<Clip>(b), std::optional<Clip>(Clip{}), "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopMedia(),
                   0, 2, 6, std::optional<Clip>(c), std::optional<Clip>(Clip{}), "Clear Clip"));
    mgr.perform(std::move(composite));

    REQUIRE(mgr.historySize() == 1);                 // one gesture, not three
    REQUIRE(deck.getClip(0, 0) != nullptr);          // clear = blank clip, not empty
    REQUIRE(*deck.getClip(0, 0) == Clip{});

    mgr.undo();
    REQUIRE(*deck.getClip(0, 0) == a);               // all three restored
    REQUIRE(*deck.getClip(1, 3) == b);
    REQUIRE(*deck.getClip(2, 6) == c);

    mgr.redo();
    REQUIRE(*deck.getClip(0, 0) == Clip{});
    REQUIRE(*deck.getClip(2, 6) == Clip{});
}

// ---------------------------------------------------------------------------
// Empty-composite guard + stale-coordinate safety
// ---------------------------------------------------------------------------

TEST_CASE("Empty composite is never performed (pushCommands guard contract)", "[undo][composite]")
{
    Composition comp = makeComp();
    UndoManager mgr;

    // An all-empty deck clear yields zero children; pushCommands must not push it.
    auto composite = std::make_unique<CompositeCommand>("Clear Deck Clips");
    REQUIRE(composite->isEmpty());
    if (!composite->isEmpty())                       // the guard in pushCommands
        mgr.perform(std::move(composite));
    REQUIRE(mgr.historySize() == 0);                 // nothing pushed
}

TEST_CASE("Deck commands no-op on stale coordinates (never crash)", "[undo][resolve][column]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr, nullptr);
    UndoManager mgr;
    comp.decks[0].setClip(0, 0, richClip(1, "keep"));

    // Stale DECK index → SetColumnCountCmd apply is a safe no-op.
    mgr.perform(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc),
                9, 12, 16, "Resize Columns"));
    REQUIRE(comp.decks[0].numColumns == 12);         // untouched (bad deck index)

    // Stale LAYER index → ClearLayerClipsCmd apply is a safe no-op.
    LayerClipsSnapshot emptySnap;
    mgr.perform(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopMedia(),
                0, 9, emptySnap, emptySnap, "Clear Layer Clips"));
    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr); // layer 0 untouched
}
