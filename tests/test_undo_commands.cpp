#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "core/UndoManager.h"
#include "core/Command.h"
#include "core/CompositeCommand.h"
#include "core/ClipCommands.h"
#include "core/DeckCommands.h"
#include "core/EffectCommands.h"
#include "core/TriggerCommands.h"
#include "core/UndoService.h"
#include "core/MediaReconnect.h"
#include <optional>
#include <random>

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
// Hand-written deep-equality for Layer (step-5 RemoveLayerCmd full-Layer restore).
//
// FIELD COVERAGE: every field of the CURRENT Layer struct is compared —
// identity, controls, blend/keying, 3D, video props, transition, transform,
// feedback, per-layer effect chain, autopilot defaults, the clips row, and the
// runtime trigger fields. RemoveLayerCmd stores + restores a full Layer VALUE,
// so nothing is deliberately excluded: a future added/dropped field surfaces as
// a test failure rather than a silent weakening. Floats use exact == because the
// restore is a bit-identical value copy (no arithmetic).
// ---------------------------------------------------------------------------

static bool operator==(const FeedbackConfig& a, const FeedbackConfig& b)
{
    return a.enabled == b.enabled && a.amount == b.amount
        && a.scaleX == b.scaleX && a.scaleY == b.scaleY && a.rotation == b.rotation
        && a.offsetX == b.offsetX && a.offsetY == b.offsetY && a.lumaKey == b.lumaKey
        && a.presetName == b.presetName;
}

static bool clipsEq(const std::vector<std::optional<Clip>>& a,
                    const std::vector<std::optional<Clip>>& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (a[i].has_value() != b[i].has_value()) return false;
        if (a[i].has_value() && !(*a[i] == *b[i])) return false;
    }
    return true;
}

static bool operator==(const Layer& a, const Layer& b)
{
    return a.name == b.name && a.id == b.id && a.type == b.type
        // Controls
        && a.opacity == b.opacity && a.visible == b.visible && a.bypassed == b.bypassed
        && a.solo == b.solo && a.muted == b.muted && a.autopilotEnabled == b.autopilotEnabled
        && a.ignoreColumnTrigger == b.ignoreColumnTrigger && a.persistent == b.persistent
        && a.folded == b.folded
        // Blend / keying
        && a.blendMode == b.blendMode
        && a.keyingMode == b.keyingMode && a.keyThreshold == b.keyThreshold
        && a.keySoftness == b.keySoftness && a.chromaKeyR == b.chromaKeyR
        && a.chromaKeyG == b.chromaKeyG && a.chromaKeyB == b.chromaKeyB
        && a.chromaKeyTolerance == b.chromaKeyTolerance
        // FX-only / 3D
        && a.dryWetMix == b.dryWetMix
        && a.rotationX == b.rotationX && a.rotationY == b.rotationY && a.rotationZ == b.rotationZ
        && a.rotationSpeed == b.rotationSpeed && a.scale3D == b.scale3D
        // Video props
        && a.layerWidth == b.layerWidth && a.layerHeight == b.layerHeight && a.autoSize == b.autoSize
        // Transition
        && a.transitionMode == b.transitionMode && a.transitionBlendMode == b.transitionBlendMode
        && a.transitionSpeed == b.transitionSpeed
        // Transform
        && a.positionX == b.positionX && a.positionY == b.positionY
        && a.layerScale == b.layerScale && a.layerRotation == b.layerRotation
        && a.layerAnchorX == b.layerAnchorX && a.layerAnchorY == b.layerAnchorY
        // Feedback + per-layer effects
        && a.feedback == b.feedback
        && vecEq(a.layerEffects, b.layerEffects)
        // Autopilot defaults
        && a.defaultAutopilotAction == b.defaultAutopilotAction
        && a.defaultAutopilotDuration == b.defaultAutopilotDuration
        && a.defaultAutopilotCustomBeats == b.defaultAutopilotCustomBeats
        && a.autopilotLoops == b.autopilotLoops && a.autopilotEndOfVideo == b.autopilotEndOfVideo
        // Clips row + runtime
        && clipsEq(a.clips, b.clips)
        && a.activeClipColumn == b.activeClipColumn && a.previousClipColumn == b.previousClipColumn
        && a.crossfadeProgress == b.crossfadeProgress && a.pendingTriggerColumn == b.pendingTriggerColumn
        && a.pendingTriggerSnapOverride == b.pendingTriggerSnapOverride;
}

// ---------------------------------------------------------------------------
// Hand-written deep-equality for Deck (step-6 RemoveDeckCmd full-Deck restore).
//
// FIELD COVERAGE against Deck.h: every PUBLIC instance field — name, id,
// numColumns, and the layers vector (element-wise via Layer operator==). The
// only Deck member NOT compared is the private `nextLayerId_` id counter: it is
// inaccessible to a free operator== AND is an internal allocation counter, not
// structural identity — the same class of exclusion as Clip/Layer runtime
// fields. A full-Deck VALUE copy (RemoveDeckCmd) still restores nextLayerId_
// bit-identically via the implicit copy ctor; it is simply not asserted here.
// A future added PUBLIC Deck field surfaces as a compile-visible gap in this
// list rather than a silent weakening.
// ---------------------------------------------------------------------------

static bool operator==(const Deck& a, const Deck& b)
{
    return a.name == b.name && a.id == b.id
        && a.numColumns == b.numColumns
        && vecEq(a.layers, b.layers);
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

    ClipMediaDisposeHook noopDispose()
    {
        return [](const Clip&) {};
    }

    // Headless GL-fence hook: the renderer isn't linked in this target, so the
    // fence is a pass-through that runs the mutation directly (mirrors
    // UndoService::withDeckDetached's renderer==nullptr branch).
    DeckFenceHook noopFence()
    {
        return [](const std::function<void()>& m) { if (m) m(); };
    }

    // Composition resolver for deck-vector commands (add/remove/switch) — the app
    // binds the same shape to &composition_.
    CompositionResolver compResolverFor(Composition& comp)
    {
        return [&comp]() { return &comp; };
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(1001, "loop");
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(1, "before");
    Clip clipB = richClip(2, "after");
    clipB.clipOpacity = 0.77f;   // make them clearly distinct
    comp.decks[0].setClip(1, 3, clipA);

    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(7, "victim");
    comp.decks[0].setClip(2, 5, clipA);

    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                0, 2, 5, std::optional<Clip>(clipA), std::nullopt, "Clear Clip"));

    REQUIRE(comp.decks[0].getClip(2, 5) == nullptr);
    mgr.undo();
    REQUIRE(comp.decks[0].getClip(2, 5) != nullptr);
    REQUIRE(*comp.decks[0].getClip(2, 5) == clipA);
    mgr.redo();
    REQUIRE(comp.decks[0].getClip(2, 5) == nullptr);
}

// DOUBLE-APPLY TRAP (media-leak fix, L1): dispose must key off the command's
// OWN before_/after_ snapshot, not off live cell state — a pre-mutating
// handler (kClipClear) empties the cell BEFORE the command is even
// constructed, so live state is already empty by execute() time. This test
// exercises exactly the clear -> undo (reconnect, no dispose) -> redo
// (dispose again) cycle the packet's fail-first oracle checks end to end.
TEST_CASE("SetClipCmd: dispose hook fires on clear/redo, NOT on undo (trap a)", "[undo][setclip][media]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip clipA = richClip(7, "victim");         // MediaType::Video -> playable
    comp.decks[0].setClip(2, 5, clipA);

    int disposeCalls = 0, reconnectCalls = 0;
    ClipMediaHook reconnect = [&reconnectCalls](const Clip&) { ++reconnectCalls; };
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip& c) {
        ++disposeCalls; REQUIRE(c.id == 7);       // disposed clip is the one that left
    };

    // Mirrors kClipClear: the live handler already cleared the cell before
    // the command is built (double-apply shape) — apply()'s dispose call
    // must still fire, keyed off before_, not off (already-empty) live state.
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), reconnect, dispose,
                0, 2, 5, std::optional<Clip>(clipA), std::nullopt, "Clear Clip"));
    REQUIRE(disposeCalls == 1);
    REQUIRE(reconnectCalls == 0);

    mgr.undo();                                   // restores the clip -> reconnect, no dispose
    REQUIRE(disposeCalls == 1);
    REQUIRE(reconnectCalls == 1);

    mgr.redo();                                   // clears again -> dispose again (idempotent
    REQUIRE(disposeCalls == 2);                   // on the real Renderer: id already closed)
    REQUIRE(reconnectCalls == 1);
}

TEST_CASE("SetClipCmd: media hook fires for playable clips only", "[undo][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    int hookCalls = 0;
    ClipMediaHook counting = [&hookCalls](const Clip&) { ++hookCalls; };

    Clip video = richClip(1, "vid");         // MediaType::Video → playable
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), counting, noopDispose(),
                0, 0, 0, std::nullopt, std::optional<Clip>(video), "Drop 'vid'"));
    REQUIRE(hookCalls == 1);                  // reconnect guard invoked on apply

    mgr.undo();                               // clears cell → no media reconnect
    REQUIRE(hookCalls == 1);
    mgr.redo();                               // re-applies clip → reconnect again
    REQUIRE(hookCalls == 2);
}

// Fence invocation count (family-fence fix round 3, 2026-07-28): SetClipCmd
// routes every execute/undo/redo through the fence exactly once, same shape
// as SetColumnCountCmd's / EffectStackCmd's fence tests. Closes the round-1/
// round-2 GL-FENCE EXEMPTION this command carried.
TEST_CASE("SetClipCmd: fence fires once per execute/undo/redo", "[undo][setclip][fence]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    Clip clipA = richClip(1001, "loop");
    mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), countingFence, noopMedia(), noopDispose(),
                0, 0, 0, std::nullopt, std::optional<Clip>(clipA), "Drop 'loop'"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(*comp.decks[0].getClip(0, 0) == clipA);
}

// ---------------------------------------------------------------------------
// ToggleClipLockCmd
// ---------------------------------------------------------------------------

TEST_CASE("ToggleClipLockCmd: flips contentLocked without touching other state", "[undo][lock]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip a = richClip(1, "a");
    Clip b = richClip(2, "b");
    comp.decks[0].setClip(0, 0, a);
    comp.decks[0].setClip(1, 1, b);

    auto composite = std::make_unique<CompositeCommand>("Clear 2 Clips");
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                   0, 0, 0, std::optional<Clip>(a), std::nullopt, "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip a = richClip(1, "a");
    Clip b = richClip(2, "b");
    b.clipOpacity = 0.55f;                    // make the two clearly distinct
    comp.decks[0].setClip(0, 2, a);           // src cell
    comp.decks[0].setClip(1, 5, b);           // dst cell (occupied → a real swap)
    const int cols = comp.decks[0].numColumns;

    int hookCalls = 0, disposeCalls = 0;
    ClipMediaHook counting = [&hookCalls](const Clip&) { ++hookCalls; };
    ClipMediaDisposeHook countingDispose = [&disposeCalls](const Clip&) { ++disposeCalls; };

    // After the swap: src holds b, dst holds a; numColumns unchanged.
    mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), noopFence(), counting, countingDispose,
        0, /*src*/ 0, 2, /*dst*/ 1, 5,
        std::optional<Clip>(a), std::optional<Clip>(b),   // src before/after
        std::optional<Clip>(b), std::optional<Clip>(a),   // dst before/after
        cols, cols, "Swap Clips"));

    REQUIRE(*comp.decks[0].getClip(0, 2) == b);
    REQUIRE(*comp.decks[0].getClip(1, 5) == a);
    REQUIRE(comp.decks[0].numColumns == cols);
    REQUIRE(mgr.undoDescription() == "Swap Clips");
    REQUIRE(hookCalls == 2);                   // media hook fired for BOTH cells

    // TRAP: a swap RELOCATES both ids (a and b both stay live, just move
    // cells) — the dispose hook must NEVER fire across a swap/undo/redo.
    REQUIRE(disposeCalls == 0);

    mgr.undo();                                // execute→undo == initial
    REQUIRE(*comp.decks[0].getClip(0, 2) == a);
    REQUIRE(*comp.decks[0].getClip(1, 5) == b);
    REQUIRE(comp.decks[0].numColumns == cols);
    REQUIRE(disposeCalls == 0);

    mgr.redo();                                // execute→undo→redo == post
    REQUIRE(*comp.decks[0].getClip(0, 2) == b);
    REQUIRE(*comp.decks[0].getClip(1, 5) == a);
    REQUIRE(comp.decks[0].numColumns == cols);
    REQUIRE(disposeCalls == 0);
}

TEST_CASE("SwapClipsCmd: move to a far empty column grows then undo shrinks numColumns", "[undo][swap]")
{
    Composition comp = makeComp();            // numColumns == 12
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip x = richClip(42, "mover");
    comp.decks[0].setClip(0, 3, x);           // src at (0,3)
    const int colsBefore = comp.decks[0].numColumns;   // 12
    const int dstCol = 15;                    // beyond current column count
    const int colsAfter = dstCol + 1;         // 16

    // Move x from (0,3) onto empty (1,15): src empties, dst gets x, cols 12→16.
    mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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

// Fence invocation count (family-fence fix round 3, 2026-07-28): SwapClipsCmd
// routes every execute/undo/redo through the fence exactly once (one fence
// covers both applyCell() calls plus the numColumns write). Closes the
// round-1/round-2 GL-FENCE EXEMPTION this command carried.
TEST_CASE("SwapClipsCmd: fence fires once per execute/undo/redo", "[undo][swap][fence]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Clip a = richClip(1, "a");
    Clip b = richClip(2, "b");
    comp.decks[0].setClip(0, 2, a);           // src cell
    comp.decks[0].setClip(1, 5, b);           // dst cell (occupied → a real swap)
    const int cols = comp.decks[0].numColumns;

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), countingFence, noopMedia(), noopDispose(),
        0, /*src*/ 0, 2, /*dst*/ 1, 5,
        std::optional<Clip>(a), std::optional<Clip>(b),   // src before/after
        std::optional<Clip>(b), std::optional<Clip>(a),   // dst before/after
        cols, cols, "Swap Clips"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(*comp.decks[0].getClip(0, 2) == b);
    REQUIRE(*comp.decks[0].getClip(1, 5) == a);
    REQUIRE(comp.decks[0].numColumns == cols);
}

// ---------------------------------------------------------------------------
// UndoService coordinate resolution — stale coordinates return null, never crash
// ---------------------------------------------------------------------------

TEST_CASE("UndoService::resolveDeck handles valid, out-of-range, and null", "[undo][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);

    REQUIRE(svc.resolveDeck(0) == &comp.decks[0]);
    REQUIRE(svc.resolveDeck(-1) == nullptr);
    REQUIRE(svc.resolveDeck(5) == nullptr);          // out of range

    UndoService empty;                                // no composition wired
    REQUIRE(empty.resolveDeck(0) == nullptr);
}

TEST_CASE("UndoService::resolveLayer returns null for stale layer coordinates", "[undo][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);

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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);

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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const int before = deck.numColumns;   // 12
    deck.addColumn();                      // live mutation (mutate-then-push): 13
    const int after = deck.numColumns;     // 13

    mgr.perform(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc), noopFence(),
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

// Fence invocation count (family-fence fix, 2026-07-28): SetColumnCountCmd
// routes every execute/undo/redo through the fence exactly once, mirroring
// the app's withDeckDetached GL fence — same shape as AddDeckCmd's test.
TEST_CASE("SetColumnCountCmd: fence fires once per execute/undo/redo", "[undo][column][fence]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    const int before = deck.numColumns;
    mgr.perform(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc), countingFence,
                0, before, before + 1, "Add Column"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(deck.numColumns == before + 1);
}

// ---------------------------------------------------------------------------
// RemoveColumnCmd (#26): removing the last column restores its cells on undo
// ---------------------------------------------------------------------------

TEST_CASE("RemoveColumnCmd: remove-column-with-clips restores cells + count", "[undo][column]")
{
    Composition comp = makeComp();         // 12 columns, 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
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

    // Family coverage (trap c): RemoveColumnCmd must dispose BOTH occupied
    // cells it removes, exactly once per execute/redo, none on undo.
    int disposeCalls = 0, reconnectCalls = 0;
    ClipMediaHook reconnect = [&reconnectCalls](const Clip&) { ++reconnectCalls; };
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };

    mgr.perform(std::make_unique<RemoveColumnCmd>(deckResolverFor(svc), noopFence(), reconnect, dispose,
                0, col, before, std::move(removed), "Remove Column"));

    REQUIRE(deck.numColumns == before - 1);          // 11
    REQUIRE(deck.getClip(0, col) == nullptr);        // column no longer addressable
    REQUIRE(mgr.undoDescription() == "Remove Column");
    REQUIRE(disposeCalls == 2);                      // c0 AND c2 disposed (layer 1 was empty)
    REQUIRE(reconnectCalls == 0);

    mgr.undo();
    REQUIRE(deck.numColumns == before);              // 12 restored
    REQUIRE(*deck.getClip(0, col) == c0);            // occupied cells restored
    REQUIRE(deck.getClip(1, col) == nullptr);        // empty layer stays empty
    REQUIRE(*deck.getClip(2, col) == c2);
    REQUIRE(disposeCalls == 2);                      // undo reconnects, does not dispose
    REQUIRE(reconnectCalls == 2);

    mgr.redo();
    REQUIRE(disposeCalls == 4);                      // redo disposes both again
    REQUIRE(deck.numColumns == before - 1);
    REQUIRE(deck.getClip(0, col) == nullptr);
}

// Fence invocation count (family-fence fix, 2026-07-28): RemoveColumnCmd
// routes every execute/undo/redo through the fence exactly once.
TEST_CASE("RemoveColumnCmd: fence fires once per execute/undo/redo", "[undo][column][fence]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const int col = deck.numColumns - 1;
    const int before = deck.numColumns;
    std::vector<std::optional<Clip>> removed;
    for (auto& L : deck.layers)
        removed.push_back(L.getClipAt(col) ? std::optional<Clip>(*L.getClipAt(col))
                                           : std::nullopt);
    deck.removeColumn(col);

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    mgr.perform(std::make_unique<RemoveColumnCmd>(deckResolverFor(svc), countingFence, noopMedia(), noopDispose(),
                0, col, before, std::move(removed), "Remove Column"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(deck.numColumns == before - 1);
}

// ---------------------------------------------------------------------------
// ClearLayerClipsCmd (#19): clear one layer's clips row + runtime
// ---------------------------------------------------------------------------

TEST_CASE("ClearLayerClipsCmd: clear one layer, undo restores clips + runtime", "[undo][clearclips]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
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

    // Family coverage (trap c): both cleared clips dispose exactly once per
    // execute/redo, none on undo (reconnect instead).
    int disposeCalls = 0, reconnectCalls = 0;
    ClipMediaHook reconnect = [&reconnectCalls](const Clip&) { ++reconnectCalls; };
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };

    mgr.perform(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopFence(), reconnect, dispose,
                0, 1, before, after, "Clear Layer Clips"));

    REQUIRE(deck.getClip(1, 0) == nullptr);
    REQUIRE(deck.getClip(1, 4) == nullptr);
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);
    REQUIRE(disposeCalls == 2);                      // a AND b disposed
    REQUIRE(reconnectCalls == 0);

    mgr.undo();
    REQUIRE(*deck.getClip(1, 0) == a);               // clips restored (deep-equal)
    REQUIRE(*deck.getClip(1, 4) == b);
    REQUIRE(deck.getLayer(1)->activeClipColumn == 4);// runtime restored
    REQUIRE(disposeCalls == 2);                      // undo reconnects, does not dispose
    REQUIRE(reconnectCalls == 2);

    mgr.redo();
    REQUIRE(deck.getClip(1, 0) == nullptr);
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);
    REQUIRE(disposeCalls == 4);                      // redo disposes both again
}

// Fence invocation count (family-fence fix, 2026-07-28): ClearLayerClipsCmd
// routes every execute/undo/redo through the fence exactly once.
TEST_CASE("ClearLayerClipsCmd: fence fires once per execute/undo/redo", "[undo][clearclips][fence]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    Clip a = richClip(1, "a");
    deck.setClip(1, 0, a);
    LayerClipsSnapshot before = captureLayerClips(*deck.getLayer(1));
    deck.getLayer(1)->clips.clear();
    deck.getLayer(1)->ensureColumns(deck.numColumns);
    deck.getLayer(1)->clearActiveClip();
    LayerClipsSnapshot after = captureLayerClips(*deck.getLayer(1));

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    mgr.perform(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), countingFence, noopMedia(), noopDispose(),
                0, 1, before, after, "Clear Layer Clips"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(deck.getClip(1, 0) == nullptr);
}

// ---------------------------------------------------------------------------
// Deck clear-clips (#23): composite of ClearLayerClipsCmd, empties skipped
// ---------------------------------------------------------------------------

TEST_CASE("Deck clear-clips composite: clear all layers, one entry, undo restores all", "[undo][composite][clearclips]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
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
        composite->add(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
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
    composite->add(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc), noopFence(),
                   0, colsBefore, colsAfter, "Resize Columns"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                   0, 0, startCol + 0, std::nullopt, std::optional<Clip>(v0), "Drop 3 Videos"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                   0, 0, startCol + 1, std::nullopt, std::optional<Clip>(v1), "Drop 3 Videos"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
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
// Multi-select clear composite (#4): matches kClipClear's after A2 fix
// (2026-07-30) — cleared cells are GENUINELY empty (nullopt), not a
// blank-but-occupied Clip{}. (HEAD behavior wrote Clip{}, which still
// has_value() — autopilot's occupancy scans wrongly accepted it; see
// ClipCommands.h SetClipCmd doc and .harmony/scout-sitting-triage.md Q4.)
// ---------------------------------------------------------------------------

TEST_CASE("Multi-select clear composite (Clear N Clips): one entry, undo restores all", "[undo][composite][setclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    Clip a = richClip(1, "a"), b = richClip(2, "b"), c = richClip(3, "c");
    deck.setClip(0, 0, a);
    deck.setClip(1, 3, b);
    deck.setClip(2, 6, c);

    // kClipClear calls Deck::clearCell() (nullopt) on each selected cell, then
    // composites the N SetClipCmds into one "Clear 3 Clips" entry.
    int disposeCalls = 0;
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };
    auto composite = std::make_unique<CompositeCommand>("Clear 3 Clips");
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), dispose,
                   0, 0, 0, std::optional<Clip>(a), std::nullopt, "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), dispose,
                   0, 1, 3, std::optional<Clip>(b), std::nullopt, "Clear Clip"));
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), dispose,
                   0, 2, 6, std::optional<Clip>(c), std::nullopt, "Clear Clip"));
    mgr.perform(std::move(composite));

    REQUIRE(mgr.historySize() == 1);                 // one gesture, not three
    REQUIRE(deck.getClip(0, 0) == nullptr);          // truly empty, not a blank clip
    REQUIRE(disposeCalls == 3);                      // all three selected clips disposed

    mgr.undo();
    REQUIRE(*deck.getClip(0, 0) == a);               // all three restored
    REQUIRE(*deck.getClip(1, 3) == b);
    REQUIRE(*deck.getClip(2, 6) == c);

    mgr.redo();
    REQUIRE(deck.getClip(0, 0) == nullptr);
    REQUIRE(deck.getClip(2, 6) == nullptr);
}

// A2 fix (2026-07-30): kClipClear's actual shape when the cleared cell is the
// layer's ACTIVE clip — a SetClipCmd (cell -> nullopt) PLUS a ClearActiveClipCmd
// (activeClipColumn -> -1) bundled as one composite, so activeClipColumn never
// dangles on a cleared cell and undo restores BOTH the clip and the layer's
// active-cell pointer in one gesture.
TEST_CASE("kClipClear composite: clearing the active cell empties it AND resets activeClipColumn", "[undo][composite][setclip][clearclip]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = deck.layers[0];

    Clip a = richClip(1, "active");
    deck.setClip(0, 3, a);
    L.activeClipColumn = 3;
    L.previousClipColumn = 1;
    L.crossfadeProgress = 0.5f;

    LayerRuntimeSnapshot rtBefore = captureLayerRuntime(L);
    L.clearActiveClip();                     // live: active -> -1, previous -> 3
    LayerRuntimeSnapshot rtAfter = captureLayerRuntime(L);
    REQUIRE_FALSE(rtBefore == rtAfter);

    auto composite = std::make_unique<CompositeCommand>("Clear Clip");
    composite->add(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                   0, 0, 3, std::optional<Clip>(a), std::nullopt, "Clear Clip"));
    composite->add(std::make_unique<ClearActiveClipCmd>(resolverFor(svc), 0, 0,
                   rtBefore, rtAfter, "Clear Clip"));
    mgr.perform(std::move(composite));

    REQUIRE(deck.getClip(0, 3) == nullptr);          // truly empty, not blank-Clip{}
    REQUIRE(L.activeClipColumn == -1);               // no longer dangling on the cleared cell

    mgr.undo();
    REQUIRE(*deck.getClip(0, 3) == a);               // exact clip restored
    REQUIRE(L.activeClipColumn == 3);                // activeClipColumn restored too
    REQUIRE(L.previousClipColumn == 1);
    REQUIRE(L.crossfadeProgress == 0.5f);

    mgr.redo();
    REQUIRE(deck.getClip(0, 3) == nullptr);          // re-empties
    REQUIRE(L.activeClipColumn == -1);
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
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    comp.decks[0].setClip(0, 0, richClip(1, "keep"));

    // Stale DECK index → SetColumnCountCmd apply is a safe no-op.
    mgr.perform(std::make_unique<SetColumnCountCmd>(deckResolverFor(svc), noopFence(),
                9, 12, 16, "Resize Columns"));
    REQUIRE(comp.decks[0].numColumns == 12);         // untouched (bad deck index)

    // Stale LAYER index → ClearLayerClipsCmd apply is a safe no-op.
    LayerClipsSnapshot emptySnap;
    mgr.perform(std::make_unique<ClearLayerClipsCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                0, 9, emptySnap, emptySnap, "Clear Layer Clips"));
    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr); // layer 0 untouched
}

// ===========================================================================
// Step 5 — layer ops (#13-18, #20). Fence injected as a headless pass-through.
// ===========================================================================

// ---------------------------------------------------------------------------
// AddLayerCmd (#16): append a layer; undo erases; redo re-adds the EXACT layer.
// ---------------------------------------------------------------------------

TEST_CASE("AddLayerCmd: add appends, undo removes, redo restores same layer", "[undo][layer]")
{
    Composition comp = makeComp();          // 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    const int before = deck.getNumLayers(); // 3

    mgr.perform(std::make_unique<AddLayerCmd>(deckResolverFor(svc), noopFence(), 0, "Add Layer"));
    REQUIRE(deck.getNumLayers() == before + 1);
    const uint32_t addedId = deck.layers.back().id;   // capture for determinism
    REQUIRE(mgr.undoDescription() == "Add Layer");

    mgr.undo();
    REQUIRE(deck.getNumLayers() == before);
    mgr.redo();
    REQUIRE(deck.getNumLayers() == before + 1);
    REQUIRE(deck.layers.back().id == addedId);        // redo re-inserts the SAME layer
}

// ---------------------------------------------------------------------------
// RemoveLayerCmd (#17): full-Layer restore on undo (deep-equal, field-complete).
// ---------------------------------------------------------------------------

TEST_CASE("RemoveLayerCmd: remove restores the full layer on undo (deep-equal)", "[undo][layer]")
{
    Composition comp = makeComp();          // 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    // Give the last layer distinctive state + a clip so the full-Layer restore bites.
    const int idx = deck.getNumLayers() - 1;   // 2
    Layer& L = deck.layers[static_cast<size_t>(idx)];
    L.name = "victim"; L.bypassed = true; L.solo = true; L.opacity = 0.33f;
    L.blendMode = Layer::MixMode::Multiply; L.layerScale = 2.0f; L.folded = true;
    L.feedback.enabled = true; L.feedback.amount = 0.7f;
    L.activeClipColumn = 4;
    deck.setClip(idx, 4, richClip(77, "onlayer"));
    const Layer expected = L;                  // full value snapshot for compare

    Layer removedCopy = *deck.getLayer(idx);

    // Family coverage (media-leak fix, L1 round 2): the removed layer carries
    // clip 77 (richClip -> MediaType::Video, playable) — real counting hooks
    // prove dispose/reconnect actually fire, not just that the layer erases.
    int disposeCalls = 0, reconnectCalls = 0;
    ClipMediaHook reconnect = [&reconnectCalls](const Clip&) { ++reconnectCalls; };
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };

    mgr.perform(std::make_unique<RemoveLayerCmd>(deckResolverFor(svc), noopFence(), reconnect, dispose,
                0, idx, removedCopy, "Remove Layer"));

    REQUIRE(deck.getNumLayers() == 2);         // removed
    REQUIRE(mgr.undoDescription() == "Remove Layer");
    REQUIRE(disposeCalls == 1);                // clip 77 disposed
    REQUIRE(reconnectCalls == 0);

    mgr.undo();
    REQUIRE(deck.getNumLayers() == 3);
    REQUIRE(deck.layers[static_cast<size_t>(idx)] == expected);   // full-Layer deep-equal
    REQUIRE(disposeCalls == 1);                // undo reconnects, does not dispose
    REQUIRE(reconnectCalls == 1);

    mgr.redo();
    REQUIRE(deck.getNumLayers() == 2);
    REQUIRE(disposeCalls == 2);                // redo disposes again
}

// Guard-refusal test (media-leak fix, L1 round 2 self-check): execute()'s
// erase guard refuses when only 1 layer remains, so the layer (and its clip)
// is still LIVE — disposeHook_ must NOT fire. Mirrors the equivalent
// RemoveDeckCmd guard-refusal test; dispose must be gated on the SAME
// condition as the erase, not fired unconditionally after it.
TEST_CASE("RemoveLayerCmd: refuses when only 1 layer remains, does not dispose", "[undo][layer]")
{
    Composition comp = makeComp();          // 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    while (deck.getNumLayers() > 1)
        deck.removeLayer(deck.getNumLayers() - 1);
    REQUIRE(deck.getNumLayers() == 1);
    deck.setClip(0, 0, richClip(1, "onlylayer"));

    Layer removedCopy = deck.layers[0];
    int disposeCalls = 0;
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };
    mgr.perform(std::make_unique<RemoveLayerCmd>(deckResolverFor(svc), noopFence(), noopMedia(), dispose,
                0, 0, removedCopy, "Remove Layer"));

    REQUIRE(deck.getNumLayers() == 1);         // guard held — layer kept
    REQUIRE(disposeCalls == 0);                // still-live clip must NOT be disposed
}

// ---------------------------------------------------------------------------
// MoveLayerCmd (#18): move + undo restores order (moveLayer(to,from) is inverse).
// ---------------------------------------------------------------------------

TEST_CASE("MoveLayerCmd: move up then undo restores original order", "[undo][layer]")
{
    Composition comp = makeComp();          // layers 0,1,2
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    deck.layers[0].name = "A"; deck.layers[1].name = "B"; deck.layers[2].name = "C";

    // Move layer 2 (C) up to index 1.
    mgr.perform(std::make_unique<MoveLayerCmd>(deckResolverFor(svc), noopFence(),
                0, 2, 1, "Move Layer Up"));
    REQUIRE(deck.layers[0].name == "A");
    REQUIRE(deck.layers[1].name == "C");
    REQUIRE(deck.layers[2].name == "B");

    mgr.undo();
    REQUIRE(deck.layers[0].name == "A");
    REQUIRE(deck.layers[1].name == "B");    // original order restored
    REQUIRE(deck.layers[2].name == "C");

    mgr.redo();
    REQUIRE(deck.layers[1].name == "C");
    REQUIRE(deck.layers[2].name == "B");
}

// ---------------------------------------------------------------------------
// ToggleLayerFlagCmd (#14 bypass / #15 solo / #20 fold): each flag round-trips.
// ---------------------------------------------------------------------------

TEST_CASE("ToggleLayerFlagCmd: bypass/solo/fold each round-trip independently", "[undo][layer][toggle]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = deck.layers[1];
    L.bypassed = false; L.solo = false; L.folded = false;

    // Bypass — LayerStrip toggles live, command wraps (before=false, after=true).
    L.bypassed = true;
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 1,
                ToggleLayerFlagCmd::Flag::Bypassed, false, true, "Bypass Layer"));
    REQUIRE(L.bypassed == true);
    mgr.undo();  REQUIRE(L.bypassed == false);
    mgr.redo();  REQUIRE(L.bypassed == true);

    // Solo — independent field; the bypass command must not have touched it.
    L.solo = true;
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 1,
                ToggleLayerFlagCmd::Flag::Solo, false, true, "Solo Layer"));
    REQUIRE(L.solo == true);
    REQUIRE(L.bypassed == true);            // bypass unaffected by the solo toggle
    mgr.undo();  REQUIRE(L.solo == false);

    // Fold.
    L.folded = true;
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 1,
                ToggleLayerFlagCmd::Flag::Folded, false, true, "Fold Layer"));
    REQUIRE(L.folded == true);
    mgr.undo();  REQUIRE(L.folded == false);
    mgr.redo();  REQUIRE(L.folded == true);
}

// ---------------------------------------------------------------------------
// ClearActiveClipCmd (#13): X-button clear restores layer runtime on undo.
// ---------------------------------------------------------------------------

TEST_CASE("ClearActiveClipCmd: X-button clear restores layer runtime on undo", "[undo][layer]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = deck.layers[0];
    deck.setClip(0, 3, richClip(1, "active"));
    L.activeClipColumn = 3;
    L.previousClipColumn = 1;
    L.crossfadeProgress = 0.5f;

    LayerRuntimeSnapshot before = captureLayerRuntime(L);
    L.clearActiveClip();                    // live: active -> -1, previous -> 3, crossfade -> 1
    LayerRuntimeSnapshot after = captureLayerRuntime(L);
    REQUIRE_FALSE(before == after);

    mgr.perform(std::make_unique<ClearActiveClipCmd>(resolverFor(svc), 0, 0,
                before, after, "Clear Layer Clip"));
    REQUIRE(L.activeClipColumn == -1);      // cleared (execute idempotent with live)

    mgr.undo();
    REQUIRE(L.activeClipColumn == 3);       // runtime restored
    REQUIRE(L.previousClipColumn == 1);
    REQUIRE(L.crossfadeProgress == 0.5f);

    mgr.redo();
    REQUIRE(L.activeClipColumn == -1);
}

// ---------------------------------------------------------------------------
// Coordinate consistency (spec §7): remove+undo keeps later-layer commands
// resolvable and correctly undoable under linear history.
// ---------------------------------------------------------------------------

TEST_CASE("Layer index consistency: remove+undo keeps later-layer commands resolvable", "[undo][layer][resolve]")
{
    Composition comp = makeComp();          // layers 0,1,2
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    // Toggle bypass on the last layer, then remove it, then undo the remove.
    deck.layers[2].bypassed = true;
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 2,
                ToggleLayerFlagCmd::Flag::Bypassed, false, true, "Bypass Layer"));

    Layer removed = deck.layers[2];
    // This test is about coordinate resolution across a remove/undo, not
    // media — layer 2 carries no clip here, so noopDispose() is deliberate,
    // not a placeholder (real dispose coverage lives in the RemoveLayerCmd
    // deep-equal test above).
    mgr.perform(std::make_unique<RemoveLayerCmd>(deckResolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                0, 2, removed, "Remove Layer"));
    REQUIRE(deck.getNumLayers() == 2);

    mgr.undo();                             // undo remove → layer 2 back (bypassed==true)
    REQUIRE(deck.getNumLayers() == 3);
    REQUIRE(svc.resolveLayer(0, 2) != nullptr);      // later index resolves again
    REQUIRE(deck.layers[2].bypassed == true);

    mgr.undo();                             // undo the earlier bypass → layer 2 resolves, false
    REQUIRE(deck.layers[2].bypassed == false);
}

// ---------------------------------------------------------------------------
// Stale-coordinate no-op safety for the new layer commands.
// ---------------------------------------------------------------------------

TEST_CASE("Layer commands no-op on stale coordinates (never crash)", "[undo][layer][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    // Stale DECK index → AddLayerCmd / MoveLayerCmd apply are safe no-ops.
    mgr.perform(std::make_unique<AddLayerCmd>(deckResolverFor(svc), noopFence(), 9, "Add Layer"));
    REQUIRE(comp.decks[0].getNumLayers() == 3);      // untouched
    mgr.perform(std::make_unique<MoveLayerCmd>(deckResolverFor(svc), noopFence(),
                9, 0, 1, "Move Layer Up"));
    REQUIRE(comp.decks[0].layers[0].id == 0);        // order untouched

    // Stale LAYER index → ToggleLayerFlagCmd / ClearActiveClipCmd are safe no-ops.
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 9,
                ToggleLayerFlagCmd::Flag::Bypassed, false, true, "Bypass Layer"));
    LayerRuntimeSnapshot rt;
    mgr.perform(std::make_unique<ClearActiveClipCmd>(resolverFor(svc), 0, 9,
                rt, rt, "Clear Layer Clip"));
    REQUIRE(comp.decks[0].getLayer(0) != nullptr);   // survived; deck intact
}

// ===========================================================================
// Step 6 — deck ops (#21 new, #22 remove, #24 switch). Fence + renderer re-point
// injected as headless pass-throughs. (#23 landed in step 4.)
// ===========================================================================

namespace
{
    // A deck with a name + distinctive layer/clip state, so full-Deck deep-equal
    // (RemoveDeckCmd) actually bites. initDefault gives 3 layers × 12 columns.
    Deck richDeck(const std::string& name, uint32_t id)
    {
        Deck d;
        d.name = name;
        d.id = id;
        d.initDefault();
        d.layers[1].bypassed = true;
        d.layers[1].opacity = 0.4f;
        d.layers[2].name = "top";
        d.layers[0].clips[3] = richClip(id * 10 + 3, name + "c3");
        d.layers[0].activeClipColumn = 3;
        return d;
    }
}

// ---------------------------------------------------------------------------
// AddDeckCmd (#21): append a deck, make it active; undo removes + restores the
// prior active index; redo re-inserts the EXACT same deck.
// ---------------------------------------------------------------------------

TEST_CASE("AddDeckCmd: add appends + activates, undo removes, redo restores same deck", "[undo][deck]")
{
    Composition comp = makeComp();          // 1 deck, active 0
    UndoManager mgr;
    const int before = static_cast<int>(comp.decks.size());   // 1

    mgr.perform(std::make_unique<AddDeckCmd>(compResolverFor(comp), noopFence(), "Add Deck"));

    REQUIRE(static_cast<int>(comp.decks.size()) == before + 1);
    REQUIRE(comp.activeDeckIndex == before);            // new deck is active
    REQUIRE(comp.decks[1].name == "Deck 2");            // faithful to kDeckNew naming
    REQUIRE(comp.decks[1].getNumLayers() == 0);         // kDeckNew: no initDefault → no layers
    REQUIRE(mgr.undoDescription() == "Add Deck");
    const Deck expected = comp.decks[1];               // capture for redo compare

    mgr.undo();
    REQUIRE(static_cast<int>(comp.decks.size()) == before);
    REQUIRE(comp.activeDeckIndex == 0);                 // prior active restored

    mgr.redo();
    REQUIRE(static_cast<int>(comp.decks.size()) == before + 1);
    REQUIRE(comp.activeDeckIndex == before);
    REQUIRE(comp.decks[1] == expected);                // redo re-inserts the SAME deck
}

// ---------------------------------------------------------------------------
// RemoveDeckCmd (#22): remove the ACTIVE deck at the LAST index → active clamps;
// undo restores the full deck (deep-equal) AND the prior active index.
// ---------------------------------------------------------------------------

TEST_CASE("RemoveDeckCmd: remove active last deck clamps active, undo restores deck + index", "[undo][deck]")
{
    Composition comp = makeComp();
    comp.decks.push_back(richDeck("Deck 2", 2));
    comp.decks.push_back(richDeck("Deck 3", 3));
    comp.activeDeckIndex = 2;                           // active == last (the edge)
    UndoManager mgr;

    const int removeIdx = comp.activeDeckIndex;         // 2
    const Deck expected = comp.decks[static_cast<size_t>(removeIdx)];
    Deck removedCopy = comp.decks[static_cast<size_t>(removeIdx)];

    // Family coverage (media-leak fix, L1 round 2): "Deck 3" (richDeck)
    // carries one clip on layer 0 (id 33, MediaType::Video) — real counting
    // hooks prove RemoveDeckCmd's execute()/undo() actually dispose/reconnect
    // it, not just that the deck erases/restores structurally.
    int disposeCalls = 0, reconnectCalls = 0;
    ClipMediaHook reconnect = [&reconnectCalls](const Clip&) { ++reconnectCalls; };
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };

    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), reconnect, dispose,
                removeIdx, std::move(removedCopy), removeIdx, "Remove Deck"));

    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.activeDeckIndex == 1);                 // clamped down (was 2, now off-end)
    REQUIRE(mgr.undoDescription() == "Remove Deck");
    REQUIRE(disposeCalls == 1);                         // the one clip on layer 0 disposed
    REQUIRE(reconnectCalls == 0);

    mgr.undo();
    REQUIRE(comp.decks.size() == 3);
    REQUIRE(comp.decks[2] == expected);                // full-Deck deep-equal restore
    REQUIRE(comp.activeDeckIndex == 2);                // prior active index restored
    REQUIRE(disposeCalls == 1);                         // undo reconnects, does not dispose
    REQUIRE(reconnectCalls == 1);

    mgr.redo();
    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.activeDeckIndex == 1);
    REQUIRE(disposeCalls == 2);                         // redo disposes again
}

// ---------------------------------------------------------------------------
// RemoveDeckCmd (#22): remove the active deck at a NON-last index → no clamp,
// remaining indices stay consistent; undo re-inserts at the same index.
// ---------------------------------------------------------------------------

TEST_CASE("RemoveDeckCmd: remove active non-last deck keeps indices consistent", "[undo][deck]")
{
    Composition comp = makeComp();
    comp.decks.push_back(richDeck("Deck 2", 2));
    comp.decks.push_back(richDeck("Deck 3", 3));
    comp.activeDeckIndex = 1;                           // active is the MIDDLE deck
    UndoManager mgr;

    const int removeIdx = comp.activeDeckIndex;         // 1
    const Deck expected = comp.decks[static_cast<size_t>(removeIdx)];
    const std::string survivorName = comp.decks[2].name;  // "Deck 3" — shifts to index 1
    Deck removedCopy = comp.decks[static_cast<size_t>(removeIdx)];

    // This test is about index-shift correctness on a non-last removal, not
    // media (real dispose/reconnect coverage lives in the deep-equal test
    // above) — noop hooks are deliberate here.
    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), noopMedia(), noopDispose(),
                removeIdx, std::move(removedCopy), removeIdx, "Remove Deck"));

    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.activeDeckIndex == 1);                // NOT clamped (still in range)
    REQUIRE(comp.decks[1].name == survivorName);       // old deck 2 shifted into slot 1

    mgr.undo();
    REQUIRE(comp.decks.size() == 3);
    REQUIRE(comp.decks[1] == expected);                // re-inserted at the same index
    REQUIRE(comp.decks[2].name == survivorName);       // survivor shifted back to 2
    REQUIRE(comp.activeDeckIndex == 1);
}

// ---------------------------------------------------------------------------
// SwitchDeckCmd (#24): switch active index, undo restores; double-switch redo
// chain; renderer re-point hook fires exactly on each apply.
// ---------------------------------------------------------------------------

TEST_CASE("SwitchDeckCmd: switch + undo restores active index, redo re-applies", "[undo][deck]")
{
    Composition comp = makeComp();
    comp.decks.push_back(richDeck("Deck 2", 2));
    comp.decks.push_back(richDeck("Deck 3", 3));
    comp.activeDeckIndex = 0;
    UndoManager mgr;

    int activateCalls = 0;
    DeckActivateHook countingActivate = [&activateCalls]() { ++activateCalls; };

    // User switches 0 -> 2 (mutate-then-push: the live switch already set index=2).
    comp.activeDeckIndex = 2;
    mgr.perform(std::make_unique<SwitchDeckCmd>(compResolverFor(comp), countingActivate,
                0, 2, "Switch Deck"));
    REQUIRE(comp.activeDeckIndex == 2);                // execute re-applies `after` (idempotent)
    REQUIRE(activateCalls == 1);                       // renderer re-point on execute
    REQUIRE(mgr.undoDescription() == "Switch Deck");

    mgr.undo();
    REQUIRE(comp.activeDeckIndex == 0);                // back to `before`
    REQUIRE(activateCalls == 2);                       // re-point on undo too

    mgr.redo();
    REQUIRE(comp.activeDeckIndex == 2);
    REQUIRE(activateCalls == 3);
}

TEST_CASE("SwitchDeckCmd: double-switch redo chain restores each active index", "[undo][deck]")
{
    Composition comp = makeComp();
    comp.decks.push_back(richDeck("Deck 2", 2));
    comp.decks.push_back(richDeck("Deck 3", 3));
    comp.activeDeckIndex = 0;
    UndoManager mgr;

    // Switch 0 -> 1, then 1 -> 2 (two user gestures).
    comp.activeDeckIndex = 1;
    mgr.perform(std::make_unique<SwitchDeckCmd>(compResolverFor(comp), nullptr, 0, 1, "Switch Deck"));
    comp.activeDeckIndex = 2;
    mgr.perform(std::make_unique<SwitchDeckCmd>(compResolverFor(comp), nullptr, 1, 2, "Switch Deck"));
    REQUIRE(comp.activeDeckIndex == 2);

    mgr.undo();  REQUIRE(comp.activeDeckIndex == 1);   // undo 2nd switch
    mgr.undo();  REQUIRE(comp.activeDeckIndex == 0);   // undo 1st switch
    mgr.redo();  REQUIRE(comp.activeDeckIndex == 1);   // redo 1st
    mgr.redo();  REQUIRE(comp.activeDeckIndex == 2);   // redo 2nd
}

// ---------------------------------------------------------------------------
// Fence invocation count: AddDeckCmd routes every execute/undo/redo through the
// fence exactly once (mirrors the app's withDeckDetached GL fence).
// ---------------------------------------------------------------------------

TEST_CASE("AddDeckCmd: fence fires once per execute/undo/redo", "[undo][deck][fence]")
{
    Composition comp = makeComp();
    UndoManager mgr;

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    mgr.perform(std::make_unique<AddDeckCmd>(compResolverFor(comp), countingFence, "Add Deck"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(comp.decks.size() == 2);
}

// ---------------------------------------------------------------------------
// Guard: removing the last deck is refused (composition must keep >=1 deck).
// The handler also guards (only builds the command when >1 deck); this proves
// the command's own defensive guard so a future call site can't empty the comp.
// ---------------------------------------------------------------------------

TEST_CASE("RemoveDeckCmd: refuses to remove the last remaining deck", "[undo][deck]")
{
    Composition comp = makeComp();                     // exactly 1 deck
    comp.decks[0].setClip(0, 0, richClip(1, "onlydeck"));   // give it a media clip
    UndoManager mgr;
    Deck removedCopy = comp.decks[0];

    // Guard-refusal test: execute()'s erase guard refuses (only 1 deck), so
    // the deck (and its clip) is still LIVE — disposeHook_ must NOT fire.
    // This is the exact scenario a mis-gated dispose call would get wrong
    // (dispose gated on the SAME guard as the erase, not fired unconditionally
    // after it — see DeckCommands.h's RemoveDeckCmd::execute() comment).
    int disposeCalls = 0;
    ClipMediaDisposeHook dispose = [&disposeCalls](const Clip&) { ++disposeCalls; };
    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), noopMedia(), dispose,
                0, std::move(removedCopy), 0, "Remove Deck"));

    REQUIRE(comp.decks.size() == 1);                   // guard held — deck kept
    REQUIRE(disposeCalls == 0);                        // still-live clip must NOT be disposed
    REQUIRE(comp.activeDeckIndex == 0);
}

// ---------------------------------------------------------------------------
// Stale-coordinate no-op safety for all three deck commands (never crash).
// ---------------------------------------------------------------------------

TEST_CASE("Deck commands no-op on stale coordinates (never crash)", "[undo][deck][resolve]")
{
    Composition comp = makeComp();
    comp.decks.push_back(richDeck("Deck 2", 2));       // 2 decks, active 0
    UndoManager mgr;

    // Null composition resolver → AddDeckCmd apply is a safe no-op.
    CompositionResolver nullComp = []() -> Composition* { return nullptr; };
    mgr.perform(std::make_unique<AddDeckCmd>(nullComp, noopFence(), "Add Deck"));
    REQUIRE(comp.decks.size() == 2);                   // untouched

    // RemoveDeckCmd with an out-of-range deck index → no erase. priorActiveIndex
    // matches deckIndex (9==9) so the ctor's active-deck invariant still holds —
    // the staleness is the OUT-OF-RANGE index, caught by execute()'s own guard.
    // Stale-index no-op — the default deck carries no media; noop deliberate.
    Deck dummy = comp.decks[0];
    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), noopMedia(), noopDispose(),
                9, std::move(dummy), 9, "Remove Deck"));
    REQUIRE(comp.decks.size() == 2);                   // bad index → nothing removed

    // SwitchDeckCmd with an out-of-range target index → active index unchanged.
    mgr.perform(std::make_unique<SwitchDeckCmd>(compResolverFor(comp), nullptr,
                0, 9, "Switch Deck"));
    REQUIRE(comp.activeDeckIndex == 0);                // stale target → no switch
}

// ===========================================================================
// Undo v1 step 7 — effect stacks (#27 add, #28 remove, #29 bypass toggle).
// EffectStackCmd stores a whole-vector before/after snapshot of ONE effect chain
// plus an EffectScope, re-resolved through the live Composition on every apply.
// EffectSlot equality is the file-scope operator== above (field-complete vs the
// HEAD struct: effectName / paramValues / dryWet / enabled / bypassed); vector
// comparison uses vecEq. Value-copy snapshots make bit-identical restores.
// ===========================================================================

static Clip::EffectSlot mkFx(const std::string& name, bool bypassed = false)
{
    Clip::EffectSlot fx;
    fx.effectName = name;
    fx.paramValues = { 0.1f, 0.25f };
    fx.dryWet = 0.5f;
    fx.bypassed = bypassed;
    return fx;
}

TEST_CASE("EffectStackCmd: whole-vector round-trip for all three scopes", "[undo][effect]")
{
    Composition comp = makeComp();                 // 1 deck, 3 layers, 12 empty cols
    comp.decks[0].getLayer(1)->clips[2] = Clip{};  // a clip for the Clip scope

    // add-shaped edit ({ripple} -> {ripple,blur}); deep-equal both ways + the UI
    // refresh hook must fire exactly on execute / undo / redo.
    auto roundTrip = [&](EffectScope scope, std::vector<Clip::EffectSlot>* target)
    {
        *target = std::vector<Clip::EffectSlot>{ mkFx("ripple") };
        const std::vector<Clip::EffectSlot> before = *target;
        const std::vector<Clip::EffectSlot> after = { mkFx("ripple"), mkFx("blur") };

        UndoManager mgr;
        int refreshCalls = 0;
        mgr.perform(std::make_unique<EffectStackCmd>(
            compResolverFor(comp), noopFence(), scope, before, after,
            [&refreshCalls]{ ++refreshCalls; }, "Add Effect 'blur'"));
        REQUIRE(vecEq(*target, after));            // execute applied after
        REQUIRE(mgr.undo());
        REQUIRE(vecEq(*target, before));           // undo restored before
        REQUIRE(mgr.redo());
        REQUIRE(vecEq(*target, after));            // redo re-applied after
        REQUIRE(refreshCalls == 3);                // execute + undo + redo
    };

    SECTION("global") { roundTrip(EffectScope::global(), &comp.globalEffects); }
    SECTION("layer")  { roundTrip(EffectScope::layer(0, 1),
                                  &comp.decks[0].getLayer(1)->layerEffects); }
    SECTION("clip")   { roundTrip(EffectScope::clip(0, 1, 2),
                                  &comp.decks[0].getClip(1, 2)->effects); }
}

TEST_CASE("EffectStackCmd: add / remove / bypass shapes each round-trip", "[undo][effect]")
{
    Composition comp = makeComp();
    auto& target = comp.globalEffects;

    // Null refresh hook is valid (headless / no undo host): apply must not deref it.
    auto roundTrip = [&](std::vector<Clip::EffectSlot> before,
                         std::vector<Clip::EffectSlot> after,
                         const std::string& desc)
    {
        target = before;
        UndoManager mgr;
        mgr.perform(std::make_unique<EffectStackCmd>(
            compResolverFor(comp), noopFence(), EffectScope::global(), before, after, nullptr, desc));
        REQUIRE(vecEq(target, after));
        REQUIRE(mgr.undo()); REQUIRE(vecEq(target, before));
        REQUIRE(mgr.redo()); REQUIRE(vecEq(target, after));
    };

    SECTION("add")    { roundTrip({ mkFx("a") }, { mkFx("a"), mkFx("b") }, "Add Effect 'b'"); }
    SECTION("remove") { roundTrip({ mkFx("a"), mkFx("b") }, { mkFx("a") }, "Remove Effect 'b'"); }
    SECTION("bypass") { roundTrip({ mkFx("a", false) }, { mkFx("a", true) }, "Bypass Effect 'a'"); }
}

TEST_CASE("EffectStackCmd: stale coordinate is a safe no-op (never crash)", "[undo][effect][resolve]")
{
    Composition comp = makeComp();
    const std::vector<Clip::EffectSlot> before;                    // empty
    const std::vector<Clip::EffectSlot> after = { mkFx("ghost") };

    UndoManager mgr;
    int refreshCalls = 0;
    auto refresh = [&refreshCalls]{ ++refreshCalls; };

    SECTION("bad layer index → nullptr vector, no-op, no refresh")
    {
        mgr.perform(std::make_unique<EffectStackCmd>(
            compResolverFor(comp), noopFence(), EffectScope::layer(0, 99), before, after, refresh, "x"));
        REQUIRE(comp.globalEffects.empty());     // model untouched
        REQUIRE(refreshCalls == 0);              // stale → refresh never fired
    }
    SECTION("bad column (empty cell) → nullptr vector, no-op, no refresh")
    {
        mgr.perform(std::make_unique<EffectStackCmd>(
            compResolverFor(comp), noopFence(), EffectScope::clip(0, 0, 99), before, after, refresh, "x"));
        REQUIRE(refreshCalls == 0);
    }
    SECTION("null composition → nullptr vector, no-op, no refresh")
    {
        CompositionResolver nullComp = []() -> Composition* { return nullptr; };
        mgr.perform(std::make_unique<EffectStackCmd>(
            nullComp, noopFence(), EffectScope::global(), before, after, refresh, "x"));
        REQUIRE(refreshCalls == 0);
    }
}

TEST_CASE("EffectStackCmd: two scopes in history undo to their own vectors", "[undo][effect]")
{
    Composition comp = makeComp();
    comp.decks[0].getLayer(1)->clips[2] = Clip{};
    comp.globalEffects.clear();
    comp.decks[0].getClip(1, 2)->effects.clear();

    const std::vector<Clip::EffectSlot> empty;
    const std::vector<Clip::EffectSlot> globalAfter = { mkFx("g") };
    const std::vector<Clip::EffectSlot> clipAfter   = { mkFx("c") };

    UndoManager mgr;
    mgr.perform(std::make_unique<EffectStackCmd>(
        compResolverFor(comp), noopFence(), EffectScope::global(), empty, globalAfter, nullptr, "Add Effect 'g'"));
    mgr.perform(std::make_unique<EffectStackCmd>(
        compResolverFor(comp), noopFence(), EffectScope::clip(0, 1, 2), empty, clipAfter, nullptr, "Add Effect 'c'"));

    REQUIRE(vecEq(comp.globalEffects, globalAfter));
    REQUIRE(vecEq(comp.decks[0].getClip(1, 2)->effects, clipAfter));

    // Undo the clip add → only the clip chain empties; global is left alone.
    REQUIRE(mgr.undo());
    REQUIRE(vecEq(comp.decks[0].getClip(1, 2)->effects, empty));
    REQUIRE(vecEq(comp.globalEffects, globalAfter));   // scope isolation

    // Undo the global add → global empties too.
    REQUIRE(mgr.undo());
    REQUIRE(vecEq(comp.globalEffects, empty));
}

TEST_CASE("EffectStackCmd: refresh hook fires once per execute/undo/redo", "[undo][effect]")
{
    Composition comp = makeComp();
    comp.globalEffects.clear();
    const std::vector<Clip::EffectSlot> before;
    const std::vector<Clip::EffectSlot> after = { mkFx("a") };

    int calls = 0;
    UndoManager mgr;
    mgr.perform(std::make_unique<EffectStackCmd>(
        compResolverFor(comp), noopFence(), EffectScope::global(), before, after,
        [&calls]{ ++calls; }, "Add Effect 'a'"));
    REQUIRE(calls == 1);   // execute
    REQUIRE(mgr.undo());
    REQUIRE(calls == 2);   // undo
    REQUIRE(mgr.redo());
    REQUIRE(calls == 3);   // redo
}

// Fence invocation count (family-fence fix round 2, 2026-07-28): EffectStackCmd
// routes every execute/undo/redo through the fence exactly once, same shape as
// the deck/column/clear commands' fence tests.
TEST_CASE("EffectStackCmd: fence fires once per execute/undo/redo", "[undo][effect][fence]")
{
    Composition comp = makeComp();
    comp.globalEffects.clear();
    const std::vector<Clip::EffectSlot> before;
    const std::vector<Clip::EffectSlot> after = { mkFx("a") };

    int fenceCalls = 0;
    DeckFenceHook countingFence =
        [&fenceCalls](const std::function<void()>& m) { ++fenceCalls; if (m) m(); };

    UndoManager mgr;
    mgr.perform(std::make_unique<EffectStackCmd>(
        compResolverFor(comp), countingFence, EffectScope::global(), before, after,
        nullptr, "Add Effect 'a'"));
    REQUIRE(fenceCalls == 1);                           // execute fenced
    mgr.undo();
    REQUIRE(fenceCalls == 2);                           // undo fenced
    mgr.redo();
    REQUIRE(fenceCalls == 3);                           // redo fenced
    REQUIRE(vecEq(comp.globalEffects, after));
}

// ===========================================================================
// Undo v1 step 8 — triggers (#1 TriggerClipCmd, #2 TriggerColumnCmd) + merge.
// Commands are MUTATE-THEN-PUSH: each test performs the live Layer::triggerClip /
// Deck::triggerColumn (as the handler does), captures before/after, then wraps —
// so perform()'s execute() re-applies `after` idempotently. Deep-equal uses the
// file-scope Layer operator== (clips row + the four runtime fields; clip runtime
// like `playing` is excluded there, so those are asserted directly).
// ===========================================================================

// ---------------------------------------------------------------------------
// TriggerClipCmd (#1): trigger a clip; undo → initial, redo → post (deep-equal).
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: trigger activates clip, undo restores runtime + playing, redo re-applies", "[undo][trigger]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);
    deck.setClip(0, 3, richClip(1, "c3"));   // nothing active yet (activeClipColumn -1)

    const Layer initial = L;                 // full-layer snapshot BEFORE the trigger

    // Mirror handleClipTrigger's mutate-then-push capture.
    const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(L);
    std::optional<bool> playBefore;
    if (const Clip* tc = L.getClipAt(3)) playBefore = tc->playing;   // false
    L.triggerClip(3);                        // live: activate col 3, playing -> true
    const LayerRuntimeSnapshot rtAfter = captureLayerRuntime(L);
    std::optional<bool> playAfter;
    if (const Clip* tc = L.getClipAt(3)) playAfter = tc->playing;    // true
    REQUIRE_FALSE(rtBefore == rtAfter);      // runtime changed (active -1 -> 3)

    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 3,
                rtBefore, rtAfter, playBefore, playAfter, "Trigger Clip"));
    const Layer post = L;                    // snapshot AFTER (execute idempotent w/ live)
    REQUIRE(L.activeClipColumn == 3);
    REQUIRE(L.getClipAt(3)->playing == true);
    REQUIRE(mgr.undoDescription() == "Trigger Clip");

    mgr.undo();
    REQUIRE(L == initial);                   // execute→undo == initial (deep-equal)
    REQUIRE(L.getClipAt(3)->playing == false);   // target clip `playing` restored

    mgr.redo();
    REQUIRE(L == post);                      // execute→undo→redo == post (deep-equal)
    REQUIRE(L.getClipAt(3)->playing == true);
}

// ---------------------------------------------------------------------------
// TriggerClipCmd: empty-cell trigger clears the active clip (runtime-only), undo
// restores the layer runtime. (The previously-active clip's `playing` flag is
// NOT restored — target cell empty → nullopt; accepted per spec risk #5, same as
// ClearActiveClipCmd.)
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: empty-cell trigger clears active clip, undo restores runtime", "[undo][trigger]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);
    deck.setClip(0, 3, richClip(1, "active"));
    L.activeClipColumn = 3;
    L.previousClipColumn = 0;
    L.crossfadeProgress = 0.5f;

    const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(L);
    std::optional<bool> playBefore;
    if (const Clip* tc = L.getClipAt(7)) playBefore = tc->playing;   // empty → nullopt
    L.triggerClip(7);                        // empty col 7 → clearActiveClip
    const LayerRuntimeSnapshot rtAfter = captureLayerRuntime(L);
    std::optional<bool> playAfter;
    if (const Clip* tc = L.getClipAt(7)) playAfter = tc->playing;    // nullopt
    REQUIRE_FALSE(rtBefore == rtAfter);      // active 3 -> -1

    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 7,
                rtBefore, rtAfter, playBefore, playAfter, "Trigger Clip"));
    REQUIRE(L.activeClipColumn == -1);

    mgr.undo();
    REQUIRE(L.activeClipColumn == 3);        // runtime restored
    REQUIRE(L.previousClipColumn == 0);
    REQUIRE(L.crossfadeProgress == 0.5f);

    mgr.redo();
    REQUIRE(L.activeClipColumn == -1);
}

// ---------------------------------------------------------------------------
// Merge (spec §3): consecutive SAME-layer triggers coalesce to one slot, keeping
// the ORIGINAL before-state and adopting the LATEST after-state.
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: consecutive same-layer triggers merge (original before, latest after)", "[undo][trigger][merge]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);
    deck.setClip(0, 2, richClip(1, "c2"));
    deck.setClip(0, 5, richClip(2, "c5"));

    // First trigger: col 2.
    const LayerRuntimeSnapshot b0 = captureLayerRuntime(L);
    std::optional<bool> p0; if (const Clip* tc = L.getClipAt(2)) p0 = tc->playing;
    L.triggerClip(2);
    const LayerRuntimeSnapshot a0 = captureLayerRuntime(L);
    std::optional<bool> pa0; if (const Clip* tc = L.getClipAt(2)) pa0 = tc->playing;
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 2,
                b0, a0, p0, pa0, "Trigger Clip"));
    REQUIRE(mgr.historySize() == 1);
    REQUIRE(L.activeClipColumn == 2);

    // Second trigger: col 5, SAME layer → merges into the first slot.
    const LayerRuntimeSnapshot b1 = captureLayerRuntime(L);
    std::optional<bool> p1; if (const Clip* tc = L.getClipAt(5)) p1 = tc->playing;
    L.triggerClip(5);
    const LayerRuntimeSnapshot a1 = captureLayerRuntime(L);
    std::optional<bool> pa1; if (const Clip* tc = L.getClipAt(5)) pa1 = tc->playing;
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 5,
                b1, a1, p1, pa1, "Trigger Clip"));

    REQUIRE(mgr.historySize() == 1);         // MERGED — still one slot
    REQUIRE(L.activeClipColumn == 5);        // model at the latest after

    mgr.undo();
    REQUIRE(L.activeClipColumn == -1);       // keep-original-before (start of the run, NOT 2)
    REQUIRE(L.previousClipColumn == -1);

    mgr.redo();
    REQUIRE(L.activeClipColumn == 5);        // update-latest-after
    REQUIRE(L.getClipAt(5)->playing == true);
}

// ---------------------------------------------------------------------------
// Different-layer triggers do NOT merge (each layer run is its own slot).
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: different-layer triggers do NOT merge", "[undo][trigger][merge]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L0 = *deck.getLayer(0);
    Layer& L1 = *deck.getLayer(1);
    deck.setClip(0, 2, richClip(1, "l0c2"));
    deck.setClip(1, 4, richClip(2, "l1c4"));

    const LayerRuntimeSnapshot b0 = captureLayerRuntime(L0);
    L0.triggerClip(2);
    const LayerRuntimeSnapshot a0 = captureLayerRuntime(L0);
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 2,
                b0, a0, std::optional<bool>(false), std::optional<bool>(true), "Trigger Clip"));

    const LayerRuntimeSnapshot b1 = captureLayerRuntime(L1);
    L1.triggerClip(4);
    const LayerRuntimeSnapshot a1 = captureLayerRuntime(L1);
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 1, 4,
                b1, a1, std::optional<bool>(false), std::optional<bool>(true), "Trigger Clip"));

    REQUIRE(mgr.historySize() == 2);         // two slots — no cross-layer merge

    mgr.undo();                              // undo layer-1 trigger only
    REQUIRE(L1.activeClipColumn == -1);
    REQUIRE(L0.activeClipColumn == 2);       // layer 0 still active
    mgr.undo();                              // undo layer-0 trigger
    REQUIRE(L0.activeClipColumn == -1);
}

// ---------------------------------------------------------------------------
// Retrigger of the already-active cell pushes NOTHING (the handler's guard skips
// it: no runtime and no target-`playing` change).
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: retrigger of the already-active cell pushes nothing", "[undo][trigger][merge]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);
    deck.setClip(0, 3, richClip(1, "c3"));

    const LayerRuntimeSnapshot b0 = captureLayerRuntime(L);
    std::optional<bool> p0; if (const Clip* tc = L.getClipAt(3)) p0 = tc->playing;
    L.triggerClip(3);
    const LayerRuntimeSnapshot a0 = captureLayerRuntime(L);
    std::optional<bool> pa0; if (const Clip* tc = L.getClipAt(3)) pa0 = tc->playing;
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 3,
                b0, a0, p0, pa0, "Trigger Clip"));
    REQUIRE(mgr.historySize() == 1);

    // Retrigger the SAME active cell → only a playhead reset. The handler guard
    // (replicated here) sees no change and pushes nothing.
    const LayerRuntimeSnapshot b1 = captureLayerRuntime(L);
    std::optional<bool> p1; if (const Clip* tc = L.getClipAt(3)) p1 = tc->playing;
    L.triggerClip(3);
    const LayerRuntimeSnapshot a1 = captureLayerRuntime(L);
    std::optional<bool> pa1; if (const Clip* tc = L.getClipAt(3)) pa1 = tc->playing;

    REQUIRE(b1 == a1);                        // runtime unchanged
    REQUIRE(p1 == pa1);                       // target `playing` unchanged
    if (!(b1 == a1) || p1 != pa1)             // the handler guard
        mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 3,
                    b1, a1, p1, pa1, "Trigger Clip"));
    REQUIRE(mgr.historySize() == 1);          // still ONE slot — nothing pushed
}

// ---------------------------------------------------------------------------
// TriggerColumnCmd (#2): composite of one TriggerClipCmd per NON-ignoring layer;
// ignoring layers are excluded; whole column is one undo slot.
// ---------------------------------------------------------------------------

TEST_CASE("TriggerColumnCmd composite: triggers all non-ignoring layers, excludes ignoring, one slot", "[undo][trigger][composite]")
{
    Composition comp = makeComp();            // 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    deck.setClip(0, 4, richClip(1, "l0"));
    deck.setClip(1, 4, richClip(2, "l1"));
    deck.setClip(2, 4, richClip(3, "l2"));
    deck.getLayer(1)->ignoreColumnTrigger = true;   // layer 1 opts out of column triggers

    // Mirror handleColumnTrigger: snapshot considered layers, triggerColumn, build.
    const int numLayers = deck.getNumLayers();
    std::vector<LayerRuntimeSnapshot> before(static_cast<size_t>(numLayers));
    std::vector<bool> considered(static_cast<size_t>(numLayers), false);
    for (int l = 0; l < numLayers; ++l)
    {
        auto* layer = deck.getLayer(l);
        if (!layer || layer->ignoreColumnTrigger) continue;
        considered[static_cast<size_t>(l)] = true;
        before[static_cast<size_t>(l)] = captureLayerRuntime(*layer);
    }
    deck.triggerColumn(4);

    auto composite = std::make_unique<CompositeCommand>("Trigger Column");
    for (int l = 0; l < numLayers; ++l)
    {
        if (!considered[static_cast<size_t>(l)]) continue;
        auto* layer = deck.getLayer(l);
        const LayerRuntimeSnapshot after = captureLayerRuntime(*layer);
        if (!(before[static_cast<size_t>(l)] == after))
            composite->add(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, l, 4,
                           before[static_cast<size_t>(l)], after,
                           std::nullopt, std::nullopt, "Trigger Column"));
    }
    REQUIRE(composite->size() == 2);          // only layers 0 and 2 (layer 1 excluded)
    mgr.perform(std::move(composite));

    REQUIRE(deck.getLayer(0)->activeClipColumn == 4);
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);   // ignoring layer untouched
    REQUIRE(deck.getLayer(2)->activeClipColumn == 4);
    REQUIRE(mgr.historySize() == 1);          // one gesture, one slot

    mgr.undo();
    REQUIRE(deck.getLayer(0)->activeClipColumn == -1);   // both restored
    REQUIRE(deck.getLayer(1)->activeClipColumn == -1);
    REQUIRE(deck.getLayer(2)->activeClipColumn == -1);

    mgr.redo();
    REQUIRE(deck.getLayer(0)->activeClipColumn == 4);
    REQUIRE(deck.getLayer(2)->activeClipColumn == 4);
}

// ---------------------------------------------------------------------------
// Stale-coordinate no-op safety for both commands (never crash).
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: stale coordinate is a safe no-op (never crash)", "[undo][trigger][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    comp.decks[0].setClip(0, 0, richClip(1, "keep"));

    LayerRuntimeSnapshot rt;                   // default runtime (active -1)
    LayerRuntimeSnapshot rt2; rt2.activeClipColumn = 5;

    // Stale LAYER index → resolver returns nullptr → apply is a safe no-op.
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 9, 0,
                rt, rt2, std::optional<bool>(false), std::optional<bool>(true), "Trigger Clip"));
    REQUIRE(comp.decks[0].getLayer(0)->activeClipColumn == -1);   // real layer untouched
    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr);             // deck intact

    // Stale DECK index → same.
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 9, 0, 0,
                rt, rt2, std::nullopt, std::nullopt, "Trigger Clip"));
    REQUIRE(comp.decks[0].getLayer(0)->activeClipColumn == -1);
}

TEST_CASE("TriggerColumnCmd composite: stale coordinates are safe no-ops (never crash)", "[undo][trigger][composite][resolve]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    comp.decks[0].setClip(0, 0, richClip(1, "keep"));

    LayerRuntimeSnapshot rt;
    LayerRuntimeSnapshot rt2; rt2.activeClipColumn = 4;

    // A column composite whose children target a STALE deck index → each child
    // apply resolves nullptr → the whole gesture is a safe no-op.
    auto composite = std::make_unique<CompositeCommand>("Trigger Column");
    composite->add(std::make_unique<TriggerClipCmd>(resolverFor(svc), 9, 0, 4,
                   rt, rt2, std::nullopt, std::nullopt, "Trigger Column"));
    composite->add(std::make_unique<TriggerClipCmd>(resolverFor(svc), 9, 2, 4,
                   rt, rt2, std::nullopt, std::nullopt, "Trigger Column"));
    mgr.perform(std::move(composite));
    REQUIRE(comp.decks[0].getLayer(0)->activeClipColumn == -1);   // untouched
    REQUIRE(comp.decks[0].getClip(0, 0) != nullptr);

    mgr.undo();                                // no-op undo → still safe
    REQUIRE(comp.decks[0].getLayer(0)->activeClipColumn == -1);
}

// ===========================================================================
// Undo v1 step 9 — spec §7 remainder + lane folds.
// (Manager-level merge + cap-eviction already live in test_composition.cpp:
// "UndoManager merges consecutive mergeable commands" and "UndoManager caps
// history at kMaxHistory (100)" — not duplicated here.)
// ===========================================================================

// ---------------------------------------------------------------------------
// Property test (spec §7): a SEEDED random sequence of mixed value-assignment
// commands (SetClipCmd / ToggleLayerFlagCmd / SwapClipsCmd, all in-grid so no
// column growth) — undo ALL → deep-equal initial; redo ALL → deep-equal final.
// Deterministic: fixed seed logged below. Only bit-identical-round-trip commands
// are used so the full Deck operator== is a valid oracle (SetColumnCountCmd is
// intentionally excluded — its grow-only clips vector is not bit-restored, by
// design, so it round-trips VISUALLY but not via deep-equal).
// ---------------------------------------------------------------------------

TEST_CASE("Property: random mixed-command sequence undoes to initial / redoes to final", "[undo][property]")
{
    constexpr unsigned kSeed = 0xC0FFEEu;      // deterministic — change to reproduce
    INFO("RNG seed = " << kSeed);
    std::mt19937 rng(kSeed);

    Composition comp = makeComp();             // 1 deck, 3 layers, 12 empty columns
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];

    const Deck initial = deck;                 // deep snapshot BEFORE any command

    const int numLayers = deck.getNumLayers();
    const int numCols = deck.numColumns;
    auto pick = [&rng](int loIncl, int hiIncl) {
        return std::uniform_int_distribution<int>(loIncl, hiIncl)(rng);
    };

    constexpr int N = 50;
    for (int i = 0; i < N; ++i)
    {
        switch (pick(0, 2))
        {
            case 0:   // SetClipCmd — set a fresh clip or clear a random in-grid cell
            {
                const int l = pick(0, numLayers - 1);
                const int c = pick(0, numCols - 1);
                std::optional<Clip> before = deck.getClip(l, c)
                    ? std::optional<Clip>(*deck.getClip(l, c)) : std::nullopt;
                std::optional<Clip> after = (pick(0, 1) == 0)
                    ? std::optional<Clip>(richClip(static_cast<uint32_t>(2000 + i),
                                                   "p" + std::to_string(i)))
                    : std::nullopt;
                mgr.perform(std::make_unique<SetClipCmd>(resolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                            0, l, c, before, after, "set"));
                break;
            }
            case 1:   // ToggleLayerFlagCmd — flip a random flag on a random layer
            {
                const int l = pick(0, numLayers - 1);
                const auto flag = static_cast<ToggleLayerFlagCmd::Flag>(pick(0, 2));
                Layer* L = deck.getLayer(l);
                const bool cur = (flag == ToggleLayerFlagCmd::Flag::Bypassed) ? L->bypassed
                               : (flag == ToggleLayerFlagCmd::Flag::Solo)     ? L->solo
                                                                              : L->folded;
                mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, l,
                            flag, cur, !cur, "flag"));
                break;
            }
            default:  // SwapClipsCmd — swap two random in-grid cells (no column change)
            {
                const int sl = pick(0, numLayers - 1), sc = pick(0, numCols - 1);
                const int dl = pick(0, numLayers - 1), dc = pick(0, numCols - 1);
                std::optional<Clip> sB = deck.getClip(sl, sc)
                    ? std::optional<Clip>(*deck.getClip(sl, sc)) : std::nullopt;
                std::optional<Clip> dB = deck.getClip(dl, dc)
                    ? std::optional<Clip>(*deck.getClip(dl, dc)) : std::nullopt;
                mgr.perform(std::make_unique<SwapClipsCmd>(deckResolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                            0, sl, sc, dl, dc, sB, dB, dB, sB, numCols, numCols, "swap"));
                break;
            }
        }
    }

    const Deck finalState = deck;              // deep snapshot AFTER the whole sequence

    while (mgr.canUndo()) mgr.undo();
    REQUIRE(deck == initial);                  // undo ALL → back to initial (deep-equal)

    while (mgr.canRedo()) mgr.redo();
    REQUIRE(deck == finalState);               // redo ALL → back to final (deep-equal)
}

// ---------------------------------------------------------------------------
// Coordinate resolution (spec §7): remove a MIDDLE layer (later layers shift
// down), undo, and confirm a command targeting a LATER layer index still
// resolves + applies correctly under linear history. (The existing "Layer index
// consistency" test removes the LAST layer; this covers the later-layer gap.)
// ---------------------------------------------------------------------------

TEST_CASE("Coordinate resolution: remove MIDDLE layer + undo keeps later-layer command resolvable", "[undo][layer][resolve]")
{
    Composition comp = makeComp();             // layers 0,1,2
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    deck.layers[0].name = "A"; deck.layers[1].name = "B"; deck.layers[2].name = "C";

    // Command A targets the LATER layer (index 2): bypass it.
    deck.layers[2].bypassed = true;
    mgr.perform(std::make_unique<ToggleLayerFlagCmd>(resolverFor(svc), 0, 2,
                ToggleLayerFlagCmd::Flag::Bypassed, false, true, "Bypass Layer"));

    // Command B removes the MIDDLE layer (index 1) → "C" shifts from index 2 to 1.
    // Coordinate-shift test, no clip content on this layer — noopDispose() deliberate.
    Layer removed = deck.layers[1];
    mgr.perform(std::make_unique<RemoveLayerCmd>(deckResolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                0, 1, removed, "Remove Layer"));
    REQUIRE(deck.getNumLayers() == 2);
    REQUIRE(deck.layers[1].name == "C");       // survivor shifted down

    // Linear history undoes B before A: undo B restores the middle layer, so the
    // later-layer command (index 2) resolves again when it is undone next.
    mgr.undo();                                // undo remove
    REQUIRE(deck.getNumLayers() == 3);
    REQUIRE(deck.layers[2].name == "C");       // "C" back at index 2
    REQUIRE(svc.resolveLayer(0, 2) != nullptr);// later index resolves again
    REQUIRE(deck.layers[2].bypassed == true);  // A's effect intact after the remove-undo

    mgr.undo();                                // undo bypass → resolves at index 2, reverts
    REQUIRE(deck.layers[2].bypassed == false);
}

// ---------------------------------------------------------------------------
// RemoveLayerCmd own stale-coordinate no-op (step-5 fold — its 2 siblings
// AddLayerCmd / MoveLayerCmd are already covered by "Layer commands no-op on
// stale coordinates"). RemoveLayerCmd has no ctor invariant, so any values are
// in-invariant; a stale DECK index makes apply resolve nullptr → safe no-op.
// ---------------------------------------------------------------------------

TEST_CASE("RemoveLayerCmd: stale coordinate is a safe no-op (never crash)", "[undo][layer][resolve]")
{
    Composition comp = makeComp();             // 3 layers
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;

    Layer dummy = comp.decks[0].layers[0];     // a valid Layer value; deck index is stale
    mgr.perform(std::make_unique<RemoveLayerCmd>(deckResolverFor(svc), noopFence(), noopMedia(), noopDispose(),
                9, 0, dummy, "Remove Layer"));
    REQUIRE(comp.decks[0].getNumLayers() == 3);// untouched (bad deck index → no erase)

    mgr.undo();                                // undo is a safe no-op too
    REQUIRE(comp.decks[0].getNumLayers() == 3);
}

// ---------------------------------------------------------------------------
// pendingTriggerColumn-only change (step-8 fold, COMMAND level): a trigger whose
// snapshot differs ONLY in pendingTriggerColumn (the beat-snap queue edge — a
// queued trigger with no active-clip change) still pushes, merges same-layer, and
// round-trips. Constructed at the snapshot level (no MainComponent needed).
// ---------------------------------------------------------------------------

TEST_CASE("TriggerClipCmd: pendingTriggerColumn-only change pushes, merges, round-trips", "[undo][trigger][merge]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);

    // Snapshot pair differing ONLY in pendingTriggerColumn (-1 -> 5).
    const LayerRuntimeSnapshot before = captureLayerRuntime(L);
    LayerRuntimeSnapshot after = before; after.pendingTriggerColumn = 5;
    REQUIRE_FALSE(before == after);            // differs, so the handler guard would push

    applyLayerRuntime(L, after);              // mutate-then-push: live-apply, then wrap
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 5,
                before, after, std::nullopt, std::nullopt, "Trigger Clip"));
    REQUIRE(mgr.historySize() == 1);           // pushed (NOT skipped as a no-op)
    REQUIRE(L.pendingTriggerColumn == 5);

    // A second pending-only change on the SAME layer merges into the slot.
    const LayerRuntimeSnapshot before2 = captureLayerRuntime(L);
    LayerRuntimeSnapshot after2 = before2; after2.pendingTriggerColumn = 8;
    applyLayerRuntime(L, after2);
    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 8,
                before2, after2, std::nullopt, std::nullopt, "Trigger Clip"));
    REQUIRE(mgr.historySize() == 1);           // MERGED — still one slot

    mgr.undo();
    REQUIRE(L.pendingTriggerColumn == -1);     // keep-original-before (run start was -1)
    mgr.redo();
    REQUIRE(L.pendingTriggerColumn == 8);      // update-latest-after
}

// ===========================================================================
// L5 Quantize fix round — the pending-trigger queue itself (Layer/Deck level,
// no MainComponent needed) plus the two cancellation fixes (deck-switch,
// clearActiveClip). Round 1 shipped the wiring with zero automated coverage on
// exactly this surface; these five close that gap.
// ===========================================================================

TEST_CASE("Layer::triggerClip: forced snap queues a non-active column instead of firing immediately", "[layer][trigger][quantize]")
{
    Layer L;
    L.ensureColumns(4);
    L.clips[2] = richClip(1, "target");
    REQUIRE(L.activeClipColumn == -1);

    L.triggerClip(2, Clip::BeatSnapMode::Beat);
    REQUIRE(L.pendingTriggerColumn == 2);               // queued, not fired
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Beat);
    REQUIRE(L.activeClipColumn == -1);                  // did NOT activate immediately
    REQUIRE_FALSE(L.clips[2]->playing);                 // never started playing
}

TEST_CASE("Layer::processPendingTrigger: forced override picks granularity independent of the clip's own beatSnapMode", "[layer][trigger][quantize]")
{
    Layer L;
    L.ensureColumns(4);
    L.clips[2] = richClip(1, "target");
    REQUIRE(L.clips[2]->beatSnapMode == Clip::BeatSnapMode::Off);   // clip itself has no snap set

    // Beat override: fires on ANY beat, regardless of beatInBar.
    L.triggerClip(2, Clip::BeatSnapMode::Beat);
    L.processPendingTrigger(2, 0);                      // beatInBar=2 — NOT a downbeat
    REQUIRE(L.activeClipColumn == 2);                   // fired anyway: Beat granularity
    REQUIRE(L.pendingTriggerColumn == -1);

    // Bar override: only fires on beatInBar == 0, even though THIS clip's own
    // beatSnapMode is Off (proves the override, not the clip field, drives it).
    L.clips[3] = richClip(2, "target2");
    L.triggerClip(3, Clip::BeatSnapMode::Bar);
    L.processPendingTrigger(2, 0);                      // NOT beat 0 — must NOT fire
    REQUIRE(L.pendingTriggerColumn == 3);                // still queued
    REQUIRE(L.activeClipColumn == 2);                    // unchanged — no premature fire

    L.processPendingTrigger(0, 0);                       // beat 0 of the bar — fires now
    REQUIRE(L.activeClipColumn == 3);
    REQUIRE(L.pendingTriggerColumn == -1);
}

TEST_CASE("Deck::triggerColumn: forced snap queues on every non-ignoring layer, skips ignoring ones", "[deck][trigger][quantize]")
{
    Deck d;
    d.initDefault();                                     // 3 layers, 12 columns
    for (auto& layer : d.layers)
        layer.clips[4] = richClip(1, "col4");
    d.layers[1].ignoreColumnTrigger = true;               // must be skipped

    d.triggerColumn(4, Clip::BeatSnapMode::Bar);

    REQUIRE(d.layers[0].pendingTriggerColumn == 4);
    REQUIRE(d.layers[0].pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);
    REQUIRE(d.layers[1].pendingTriggerColumn == -1);      // skipped entirely — untouched
    REQUIRE(d.layers[2].pendingTriggerColumn == 4);
    REQUIRE(d.layers[2].pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);
}

TEST_CASE("TriggerClipCmd: undo of a queued forced-snap trigger restores pendingTriggerSnapOverride too", "[undo][trigger][quantize]")
{
    Composition comp = makeComp();
    UndoService svc; svc.setCollaborators(&comp, nullptr, nullptr);
    UndoManager mgr;
    Deck& deck = comp.decks[0];
    Layer& L = *deck.getLayer(0);
    L.clips[6] = richClip(1, "queued");

    // Snapshot pair differing in BOTH pendingTriggerColumn AND the new
    // pendingTriggerSnapOverride field (-1/Off -> 6/Bar) — TRAP #3's exact
    // shape, now for the override field specifically. Without the field in
    // LayerRuntimeSnapshot's operator==/capture/apply, undo would restore
    // pendingTriggerColumn but silently leave pendingTriggerSnapOverride stuck
    // at Bar (the field would never have been captured/restored at all).
    const LayerRuntimeSnapshot before = captureLayerRuntime(L);
    L.triggerClip(6, Clip::BeatSnapMode::Bar);           // live: queues (mutate-then-push)
    const LayerRuntimeSnapshot after = captureLayerRuntime(L);
    REQUIRE(after.pendingTriggerColumn == 6);
    REQUIRE(after.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.perform(std::make_unique<TriggerClipCmd>(resolverFor(svc), 0, 0, 6,
                before, after, std::nullopt, std::nullopt, "Trigger Clip"));
    REQUIRE(L.pendingTriggerColumn == 6);
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.undo();
    REQUIRE(L.pendingTriggerColumn == -1);
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);

    mgr.redo();
    REQUIRE(L.pendingTriggerColumn == 6);
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);
}

TEST_CASE("SwitchDeckCmd: cancels a pending trigger on the deck being left; undo restores it, redo re-cancels",
          "[undo][deck][trigger][quantize]")
{
    Composition comp = makeComp();                 // deck 0: 3 layers, 12 cols
    comp.decks.push_back(richDeck("Deck 2", 2));    // deck 1: switch target
    comp.activeDeckIndex = 0;
    UndoManager mgr;

    Layer& L0 = *comp.decks[0].getLayer(0);
    L0.clips[5] = richClip(99, "queued");
    L0.triggerClip(5, Clip::BeatSnapMode::Bar);     // queues: col(5) != active(-1), forced snap
    REQUIRE(L0.pendingTriggerColumn == 5);
    REQUIRE(L0.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    // Mirror the app's onDeckSwitched shape: capture what's about to be
    // cancelled BEFORE the live switch (MainComponent isn't linked into this
    // test target, so handleDeckSwitch's cancellation loop is reproduced
    // headlessly here) — same idiom as handleClipTrigger's rtBefore/rtAfter
    // capture around a live mutation, not the mutation reporting itself.
    std::vector<PendingTriggerSnapshot> cancelled;
    for (int l = 0; l < comp.decks[0].getNumLayers(); ++l)
    {
        auto* layer = comp.decks[0].getLayer(l);
        if (layer->pendingTriggerColumn >= 0)
            cancelled.push_back({ l, layer->pendingTriggerColumn, layer->pendingTriggerSnapOverride });
    }
    REQUIRE(cancelled.size() == 1);

    // Live cancellation + switch (what handleDeckSwitch performs for every
    // switch path — user tab click, REST, OSC, MIDI, genre auto-switch alike).
    for (auto& layer : comp.decks[0].layers)
    {
        layer.pendingTriggerColumn = -1;
        layer.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
    }
    comp.activeDeckIndex = 1;

    mgr.perform(std::make_unique<SwitchDeckCmd>(compResolverFor(comp), nullptr,
                0, 1, "Switch Deck", std::move(cancelled)));
    REQUIRE(comp.activeDeckIndex == 1);
    REQUIRE(L0.pendingTriggerColumn == -1);          // stays cancelled after execute (idempotent replay)

    mgr.undo();
    REQUIRE(comp.activeDeckIndex == 0);
    REQUIRE(L0.pendingTriggerColumn == 5);           // restored — not stranded by the switch's undo
    REQUIRE(L0.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.redo();
    REQUIRE(comp.activeDeckIndex == 1);
    REQUIRE(L0.pendingTriggerColumn == -1);          // re-cancelled
    REQUIRE(L0.pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);
}

// Fix round 2 (review-named gap): AddDeckCmd is a SECOND deck-deactivation path
// that bypassed handleDeckSwitch's cancel entirely — Add Deck deactivates
// whichever deck was active without touching its pending trigger. Covers
// AddDeckCmd's OWN command-owns-the-mutation capture/cancel (execute's first-do
// branch), not a caller-side capture like the SwitchDeckCmd test above (there is
// no caller-side capture here — AddDeckCmd does it all internally).
//
// NOTE: AddDeckCmd's execute()/undo() push_back/insert/erase on comp.decks,
// which can reallocate the vector and move every Deck (and its Layer objects)
// to a new address — so this test deliberately never caches a Layer&/Deck&
// across those calls; it re-resolves comp.decks[0].getLayer(0) fresh at each
// assertion instead.
TEST_CASE("AddDeckCmd: cancels a pending trigger on the deck being left; undo restores it, redo re-cancels",
          "[undo][deck][trigger][quantize]")
{
    Composition comp = makeComp();                   // 1 deck (index 0), active 0
    UndoManager mgr;

    comp.decks[0].getLayer(0)->clips[5] = richClip(99, "queued");
    comp.decks[0].getLayer(0)->triggerClip(5, Clip::BeatSnapMode::Bar);   // queues: col(5) != active(-1)
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerColumn == 5);
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.perform(std::make_unique<AddDeckCmd>(compResolverFor(comp), noopFence(), "Add Deck"));
    REQUIRE(comp.activeDeckIndex == 1);                                          // new deck active
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerColumn == -1);              // cancelled by the add
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);

    mgr.undo();
    REQUIRE(comp.activeDeckIndex == 0);
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerColumn == 5);               // restored — not stranded
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.redo();
    REQUIRE(comp.activeDeckIndex == 1);
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerColumn == -1);              // re-cancelled
    REQUIRE(comp.decks[0].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);
}

// Fix round 4 (review-named gap #4): RemoveDeckCmd::undo() is a SECOND deck-
// deactivation path missed by rounds 2 and 3 — reactivating the restored deck
// deactivates whatever deck the removal's clamp had made active, with no call
// to the cancellation helper. Repro is the reviewer's exact scenario: remove
// the active deck (clamps active elsewhere), arm a Quantize trigger on THAT
// deck, undo the removal — the deactivated deck's trigger must not stay armed.
//
// NOTE: RemoveDeckCmd's execute()/undo() erase/insert on comp.decks, which can
// reallocate the vector — same discipline as the AddDeckCmd test above: never
// cache a Layer&/Deck& across mgr.perform/undo/redo, re-resolve fresh instead.
TEST_CASE("RemoveDeckCmd: undo cancels a pending trigger on the deck the reactivation deactivates",
          "[undo][deck][trigger][quantize]")
{
    Composition comp = makeComp();                   // deck 0
    comp.decks.push_back(richDeck("Deck 2", 2));      // deck 1
    comp.decks.push_back(richDeck("Deck 3", 3));      // deck 2
    comp.activeDeckIndex = 2;                         // active == last (the edge — clamps on removal)
    UndoManager mgr;

    const int removeIdx = comp.activeDeckIndex;       // 2
    Deck removedCopy = comp.decks[static_cast<size_t>(removeIdx)];

    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), noopMedia(), noopDispose(),
                removeIdx, std::move(removedCopy), removeIdx, "Remove Deck"));
    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.activeDeckIndex == 1);               // clamped to deck 1 ("Deck 2")

    // Arm a Quantize trigger on the now-active deck (1) — the reviewer's exact
    // "performer keeps working on the clamped-to deck" scenario.
    comp.decks[1].getLayer(0)->clips[5] = richClip(77, "queued");
    comp.decks[1].getLayer(0)->triggerClip(5, Clip::BeatSnapMode::Bar);
    REQUIRE(comp.decks[1].getLayer(0)->pendingTriggerColumn == 5);
    REQUIRE(comp.decks[1].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    mgr.undo();
    REQUIRE(comp.decks.size() == 3);
    REQUIRE(comp.activeDeckIndex == 2);               // deck 2 restored + reactivated
    // Deck 1 (deactivated by this reactivation) must not keep an armed
    // trigger — without the fix this reads 5/Bar, not -1/Off.
    REQUIRE(comp.decks[1].getLayer(0)->pendingTriggerColumn == -1);
    REQUIRE(comp.decks[1].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);
}

// Second sub-case of the same fix, found while re-deriving it (not reviewer-
// named): a NON-last removal leaves activeDeckIndex numerically UNCHANGED
// across execute() (no clamp needed), because the survivor deck shifts DOWN to
// fill the gap and keeps the same index number. An index-equality guard
// ("only cancel if activeDeckIndex changed") would silently miss this case —
// the deck NUMBER stays the same but the deck OBJECT at that number changes
// when undo's insert() shifts the survivor back off the active slot. This test
// specifically falsifies that guard shape (an earlier draft of this fix used
// `comp->activeDeckIndex != priorActiveIndex_` and passed the OTHER new test
// above while silently failing this one).
TEST_CASE("RemoveDeckCmd: undo cancels a pending trigger even when activeDeckIndex numerically stays the same",
          "[undo][deck][trigger][quantize]")
{
    Composition comp = makeComp();                    // deck 0
    comp.decks.push_back(richDeck("Deck 2", 2));       // deck 1
    comp.decks.push_back(richDeck("Deck 3", 3));       // deck 2
    comp.activeDeckIndex = 1;                          // active is the MIDDLE deck
    UndoManager mgr;

    const int removeIdx = comp.activeDeckIndex;        // 1
    Deck removedCopy = comp.decks[static_cast<size_t>(removeIdx)];

    mgr.perform(std::make_unique<RemoveDeckCmd>(compResolverFor(comp), noopFence(), noopMedia(), noopDispose(),
                removeIdx, std::move(removedCopy), removeIdx, "Remove Deck"));
    REQUIRE(comp.decks.size() == 2);
    REQUIRE(comp.activeDeckIndex == 1);                // NOT clamped — "Deck 3" shifted down into slot 1

    // Arm a Quantize trigger on the survivor now occupying slot 1 ("Deck 3").
    comp.decks[1].getLayer(0)->clips[5] = richClip(88, "queued");
    comp.decks[1].getLayer(0)->triggerClip(5, Clip::BeatSnapMode::Beat);
    REQUIRE(comp.decks[1].getLayer(0)->pendingTriggerColumn == 5);

    mgr.undo();
    REQUIRE(comp.decks.size() == 3);
    REQUIRE(comp.activeDeckIndex == 1);                // index UNCHANGED (1 -> 1)...
    // ...but the deck now AT slot 1 is the restored "Deck 2" — "Deck 3" shifted
    // back up to slot 2 and was deactivated by this undo. Without the fix (or
    // with the falsified index-equality guard), this would still read 5.
    REQUIRE(comp.decks[2].getLayer(0)->pendingTriggerColumn == -1);
    REQUIRE(comp.decks[2].getLayer(0)->pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);
}

// ---------------------------------------------------------------------------
// clearActiveClip() cancellation (L5 Quantize fix, the direct-contract test
// the original round shipped without): the X-button clear, and everything
// that routes through it (Clear Deck / Clear Layer Clips), must not leave a
// pending trigger to outlive the clear. An independent reviewer proved this
// was UNCOVERED by reverting the two lines at the end of clearActiveClip()
// (pendingTriggerColumn = -1; pendingTriggerSnapOverride = Off;) and
// re-running the whole existing L5 Quantize test set — all 36 assertions
// still passed. This test is built to fail on that exact revert.
//
// The MIDI momentary-release scenario that originally motivated the fix is
// NOT exercised here — it turned out structurally unreachable (queuing only
// happens when column != activeClipColumn, but the release path guards on
// activeClipColumn == resolvedColumn, so the two conditions can never both
// hold). This drives clearActiveClip() directly instead.
//
// Also pins down a real, previously-undiscussed behavior: Layer has ONE
// pending slot, not one per column, so clearing the ACTIVE clip (column 0)
// also drops a QUEUED trigger on a completely unrelated column (2) — the
// cancel is layer-wide, not scoped to the column being cleared.
// ---------------------------------------------------------------------------

TEST_CASE("Layer::clearActiveClip: cancels a pending trigger too, even one queued on an unrelated column", "[layer][trigger][quantize]")
{
    Layer L;
    L.ensureColumns(4);
    L.clips[0] = richClip(1, "active");
    L.clips[2] = richClip(2, "queued");

    L.triggerClip(0);                                   // no snap on this clip -> fires immediately
    REQUIRE(L.activeClipColumn == 0);
    REQUIRE(L.clips[0]->playing == true);

    L.triggerClip(2, Clip::BeatSnapMode::Bar);           // unrelated column — queues (2 != active 0)
    REQUIRE(L.pendingTriggerColumn == 2);
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Bar);

    L.clearActiveClip();                                 // clears column 0 by contract

    REQUIRE(L.activeClipColumn == -1);
    REQUIRE(L.clips[0]->playing == false);
    // The queued trigger on column 2 — never itself cleared — is dropped too.
    REQUIRE(L.pendingTriggerColumn == -1);
    REQUIRE(L.pendingTriggerSnapOverride == Clip::BeatSnapMode::Off);
}
