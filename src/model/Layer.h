#pragma once
#include "model/Clip.h"
#include "model/ClipRef.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <cstdint>
#include <utility>

// FeedbackConfig: per-layer feedback parameters.
// Feedback routes a layer's output back to its input with transformation.
struct FeedbackConfig
{
    bool enabled = false;
    float amount = 0.5f;      // [0,1] — how much of prev frame bleeds through
    float scaleX = 0.98f;     // Per-frame scale X (< 1 = zoom in, > 1 = zoom out)
    float scaleY = 0.98f;     // Per-frame scale Y
    float rotation = 0.0f;    // Per-frame rotation in degrees
    float offsetX = 0.0f;     // Per-frame horizontal drift [-0.5, 0.5]
    float offsetY = 0.0f;     // Per-frame vertical drift [-0.5, 0.5]
    float lumaKey = 0.0f;     // Fade out dark areas to prevent muddiness [0,1]
    std::string presetName;   // Preset name (empty = custom)
};

// === The Layer trigger tuple (lane tsan, s-rta-1002; Pitfall 63) ===
// A layer's per-layer trigger state: which clip is active, the clip it fades from, the fade's progress, and a
// quantized trigger waiting for its beat. The message thread (clicks / REST / OSC / MIDI / undo) AND the GL thread
// (the fade clock, autopilot, a queued trigger firing on its beat) both write it, and the GL thread reads it every
// frame. So it is ONE 16-byte atomic word (LayerRuntimeCell), never five fields: every reader loads a consistent
// tuple with one load, and every transition from either thread is a compare-exchange of the whole word, so no
// transition is lost (a fade tick that lost the race to a trigger adopts the trigger's tuple).

// A value copy of the tuple (same 5 fields as before the lane; an aggregate, so a test can write
// layer.setRuntime({ .activeClipColumn = 1, .crossfadeProgress = 0.5f })).
// bf9b S1: each clip slot also carries the id of the deck it names (activeRef() / previousRef() / pendingRef() =
// a ClipRef); the deck ids default to ClipRef::kNoDeck and come last, so a column-only tuple is written as before.
// pendingTriggerSnapOverride rides alongside pendingTriggerColumn (L5 Quantize): Off means "derive the granularity
// from the target clip's own beatSnapMode"; non-Off means a caller (global Quantize) FORCED a granularity for this
// one queued trigger (the clip's own beatSnapMode is never mutated).
struct LayerRuntimeSnapshot
{
    int activeClipColumn = -1;       // -1 = no active clip
    int previousClipColumn = -1;     // the clip a crossfade fades from (-1 = none)
    float crossfadeProgress = 1.0f;  // 1.0 = fully transitioned
    int pendingTriggerColumn = -1;   // beat snap: a queued trigger awaiting its beat / bar (-1 = none)
    Clip::BeatSnapMode pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
    uint32_t activeDeckId = ClipRef::kNoDeck;     // the deck box each slot's column is in (kNoDeck = none)
    uint32_t previousDeckId = ClipRef::kNoDeck;
    uint32_t pendingDeckId = ClipRef::kNoDeck;

    ClipRef activeRef() const { return { activeDeckId, activeClipColumn }; }
    ClipRef previousRef() const { return { previousDeckId, previousClipColumn }; }
    ClipRef pendingRef() const { return { pendingDeckId, pendingTriggerColumn }; }
};

inline bool operator==(const LayerRuntimeSnapshot& a, const LayerRuntimeSnapshot& b)
{
    return a.activeClipColumn == b.activeClipColumn
        && a.previousClipColumn == b.previousClipColumn
        && a.crossfadeProgress == b.crossfadeProgress
        && a.pendingTriggerColumn == b.pendingTriggerColumn
        && a.pendingTriggerSnapOverride == b.pendingTriggerSnapOverride
        && a.activeDeckId == b.activeDeckId
        && a.previousDeckId == b.previousDeckId
        && a.pendingDeckId == b.pendingDeckId;
}

// One transition of the tuple: `before` is the tuple the compare-exchange replaced and `after` the tuple it
// installed (the exact pair, so undo captures exactly what happened). before == after when the transition changed
// nothing. applied is false only when a BOUNDED update ran out of attempts (before == after == the concurrent tuple).
struct LayerRuntimeTransition
{
    LayerRuntimeSnapshot before;
    LayerRuntimeSnapshot after;
    bool applied = true;
    bool changed() const { return !(before == after); }
};

// The tuple as one lock-free 16-byte atomic word: int32 active, int32 previous, float progress, uint32 pending.
// Each clip slot packs a ClipRef (bf9b S1): active / previous = (deck << 16) | uint16(column) -- a deck-less ref
// stores deck field 0xFFFF, so "no clip" is -1 as before; pending = the low 28 bits ((deck << 14) | (column + 1),
// deck field 0x3FFF = no deck) under the snap override in the high 4. Columns -1 .. ClipRef::kMaxColumn, deck ids
// 0 .. ClipRef::kMaxDeckId; every value in those ranges round-trips (the bijection test). No padding bits, so the
// compare-exchange compares exactly the tuple. Apple clang arm64: ldp + dmb (acquire load), stp (release store),
// caspal (CAS) -- no lock, and a reader never writes the cache line. Apple clang x86_64 is lock-free too, without
// -mcx16, but its load is a lock cmpxchg16b (a reader writes the line). Elsewhere a 16-byte atomic may
// not be lock-free (GCC x86_64 needs -mcx16 and reports it not always-lock-free; MSVC's are not): the static_assert
// below guards the Apple build only.
class LayerRuntimeCell
{
public:
    struct alignas(16) Word
    {
        int32_t active;
        int32_t previous;
        float progress;
        uint32_t pendingPacked;
    };
    static constexpr int kMaxPendingColumn = ClipRef::kMaxColumn;   // 16,382

    static Word pack(const LayerRuntimeSnapshot& r) noexcept
    {
        return { packSlot(r.activeDeckId, r.activeClipColumn), packSlot(r.previousDeckId, r.previousClipColumn),
                 r.crossfadeProgress,
                 packPending(r.pendingDeckId, r.pendingTriggerColumn)
                     | (static_cast<uint32_t>(r.pendingTriggerSnapOverride) << 28) };
    }
    static LayerRuntimeSnapshot unpack(const Word& w) noexcept
    {
        const uint32_t pendingDeck = (w.pendingPacked >> 14) & 0x3FFFu;
        return { slotColumn(w.active), slotColumn(w.previous), w.progress,
                 static_cast<int>(w.pendingPacked & 0x3FFFu) - 1,
                 static_cast<Clip::BeatSnapMode>(w.pendingPacked >> 28),
                 slotDeck(w.active), slotDeck(w.previous),
                 pendingDeck == 0x3FFFu ? ClipRef::kNoDeck : pendingDeck };
    }

    LayerRuntimeCell() noexcept : w_(pack(LayerRuntimeSnapshot{})) {}
    // Copyable like LiveValue (Layer / Deck / Composition are value types): one relaxed load + one relaxed store.
    // A live Layer copy is per-field atomic, never a snapshot of the whole Layer: use runtime() for a tuple.
    LayerRuntimeCell(const LayerRuntimeCell& o) noexcept : w_(o.w_.load(std::memory_order_relaxed)) {}
    LayerRuntimeCell(LayerRuntimeCell&& o) noexcept : w_(o.w_.load(std::memory_order_relaxed)) {}
    LayerRuntimeCell& operator=(const LayerRuntimeCell& o) noexcept
    {
        w_.store(o.w_.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }
    LayerRuntimeCell& operator=(LayerRuntimeCell&& o) noexcept
    {
        w_.store(o.w_.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    LayerRuntimeSnapshot load() const noexcept { return unpack(w_.load(std::memory_order_acquire)); }
    void store(const LayerRuntimeSnapshot& r) noexcept { w_.store(pack(r), std::memory_order_release); }
    // ONE strong compare-exchange (acq_rel; acquire on failure). On failure `expected` receives the current tuple.
    bool compareExchange(LayerRuntimeSnapshot& expected, const LayerRuntimeSnapshot& desired) noexcept
    {
        Word e = pack(expected);
        if (w_.compare_exchange_strong(e, pack(desired), std::memory_order_acq_rel, std::memory_order_acquire))
            return true;
        expected = unpack(e);
        return false;
    }

private:
    // One ClipRef slot <-> its packed field (see the word layout above).
    static int32_t packSlot(uint32_t deckId, int column) noexcept
    {
        const uint32_t deck = deckId == ClipRef::kNoDeck ? 0xFFFFu : (deckId & 0xFFFFu);
        return static_cast<int32_t>((deck << 16) | (static_cast<uint32_t>(column) & 0xFFFFu));
    }
    static uint32_t slotDeck(int32_t v) noexcept
    {
        const uint32_t deck = static_cast<uint32_t>(v) >> 16;
        return deck == 0xFFFFu ? ClipRef::kNoDeck : deck;
    }
    static int slotColumn(int32_t v) noexcept { return static_cast<int16_t>(static_cast<uint32_t>(v) & 0xFFFFu); }
    static uint32_t packPending(uint32_t deckId, int column) noexcept
    {
        const uint32_t deck = deckId == ClipRef::kNoDeck ? 0x3FFFu : (deckId & 0x3FFFu);
        return (deck << 14) | (static_cast<uint32_t>(column + 1) & 0x3FFFu);
    }

    std::atomic<Word> w_;
#if defined(__APPLE__)
    static_assert(std::atomic<Word>::is_always_lock_free, "the Layer trigger tuple must be one lock-free word");
#endif
};
static_assert(sizeof(LayerRuntimeCell::Word) == 16, "the tuple word is 16 bytes with no padding");
static_assert(static_cast<int>(Clip::BeatSnapMode::FourBar) < 16, "the snap override packs into 4 bits");

// Layer: a row in the deck. Contains clips across columns.
// One clip is active per layer at a time.
struct Layer
{
    // === Identity ===
    std::string name = "Layer";
    uint32_t id = 0;

    // === Layer Type ===
    enum class Type : uint8_t
    {
        Opaque,         // One clip at a time, replaces everything below
        Transparent,    // Composited over layers below with blend/keying
        FXOnly,         // Effects applied to accumulator (no media)
        ThreeD,         // 3D model/surface rendering
        Mask            // Content becomes alpha mask for layers below
    };
    Type type = Type::Opaque;

    // === Layer Controls ===
    RelaxedFloat opacity = 1.0f;   // a manualRef scalar (lane tsan: the GL thread reads it via eff(); Pitfall 63)
    bool visible = true;
    bool bypassed = false;
    bool solo = false;
    bool muted = false;          // Audio mute
    bool autopilotEnabled = false;
    bool ignoreColumnTrigger = false;
    bool folded = false;            // P24.12: If true, layer row is collapsed in DeckView

    // === Mix Mode — unified list for both layer blending and clip transitions ===
    // V dropdown picks a MixMode for the layer's blend.
    // F dropdown picks a MixMode for momentary clip-to-clip transitions.
    // Same list, different contexts: V = "the look", F = "the flash".
    enum class MixMode : uint8_t
    {
        // === Standard Compositing (layer blend modes) ===
        // Basic
        Normal, Additive, Screen, Multiply, Overlay,
        // Light
        SoftLight, HardLight, VividLight, LinearLight, PinLight, HardMix,
        // Dark/Light compare
        Darken, Lighten, DarkerColor, LighterColor,
        // Dodge/Burn
        ColorDodge, ColorBurn,
        // Inversion
        Difference, Exclusion, Subtract,
        // Component (HSL)
        Hue, Saturation, Color, Luminosity,
        // Special blend
        Dissolve,

        // === Transitions (momentary clip changes, also usable as blend) ===
        // Instant
        Cut,
        // Directional wipes
        WipeLeft, WipeRight, WipeUp, WipeDown, WipeEllipse, WipeDiagonal,
        // Push (content slides in/out)
        PushLeft, PushRight, PushUp, PushDown,
        // Zoom
        ZoomIn, ZoomOut,
        // 3D rotation
        RotateX, RotateY, Spin, Cube, Flip, Fold,
        // Fade through color
        ToBlack, ToWhite,
        // Creative / VJ
        Pixelate, Blur, Noise, RGBSplit, GlitchBlocks, Strobe,
        Slide, Stretch, Displace
    };
    MixMode blendMode = MixMode::Additive;

    // === Keying Mode (Transparent type) ===
    enum class KeyingMode : uint8_t
    {
        Alpha, LumaKey, InvertedLumaKey, LumaIsAlpha, InvertedLumaIsAlpha,
        ChromaKey, MaxRGB, SaturationKey, EdgeDetection, ThresholdMask,
        ChannelR, ChannelG, ChannelB
    };
    KeyingMode keyingMode = KeyingMode::Alpha;
    float keyThreshold = 0.1f;
    float keySoftness = 0.1f;
    float chromaKeyR = 0.0f, chromaKeyG = 1.0f, chromaKeyB = 0.0f;
    float chromaKeyTolerance = 0.2f;

    // === FX Only ===
    float dryWetMix = 1.0f;

    // === 3D Controls ===
    float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
    float rotationSpeed = 0.0f;
    float scale3D = 1.0f;

    // === Video Properties (per-layer) ===
    int layerWidth = 1920;
    int layerHeight = 1080;
    enum class AutoSizeMode : uint8_t { Off, Fill, Fit, Stretch, Original };
    AutoSizeMode autoSize = AutoSizeMode::Off;

    // === Transition ===
    MixMode transitionMode = MixMode::Dissolve;  // F dropdown — momentary clip change style
    MixMode transitionBlendMode = MixMode::Normal; // Transition blend method
    float transitionSpeed = -1.0f; // -1 = use global default

    // === Transform (per-layer, applied after clip compositing) ===
    // Lane tsan (s-rta-1002; Pitfall 63): the manualRef scalars are RelaxedFloat -- the message thread writes them
    // (manualWriteCore, inspectors, REST / OSC / MIDI), the GL thread reads them through eff().
    RelaxedFloat positionX = 0.0f;
    RelaxedFloat positionY = 0.0f;
    RelaxedFloat layerScale = 1.0f;        // 1.0 = 100%
    RelaxedFloat layerRotation = 0.0f;     // Degrees
    RelaxedFloat layerAnchorX = 0.0f;
    RelaxedFloat layerAnchorY = 0.0f;

    // === Connections (s167-l2) ===
    // One ParamConnection + LiveValue twin per LayerScalar (opacity -- the
    // SAME field both the "Master" and "Opacity" widgets write today,
    // LayerInspector.cpp:149,186 -- the five transform fields above, and
    // anchorY). eff()/manualRef() are the only places that name which
    // struct field backs each LayerScalar.
    std::array<ParamConnection, static_cast<size_t>(LayerScalar::Count)> scalarConns;
    std::array<LiveValue, static_cast<size_t>(LayerScalar::Count)> scalarLive;
    float eff(LayerScalar s) const;

    // === Feedback (Larsen loop) ===
    FeedbackConfig feedback;

    // === Per-layer Effect Chain ===
    std::vector<Clip::EffectSlot> layerEffects;

    // === Autopilot Defaults ===
    Clip::AutopilotAction defaultAutopilotAction = Clip::AutopilotAction::PlayNext;
    Clip::AutopilotDuration defaultAutopilotDuration = Clip::AutopilotDuration::Beat4;
    int defaultAutopilotCustomBeats = 4;
    int autopilotLoops = 1;  // Number of clip loops before advancing (1 = advance after first play)
    bool autopilotEndOfVideo = false;  // true = advance when video playhead reaches outPoint

    // === Clips (one per column) ===
    // Indexed by column. Use std::optional so empty cells are explicit.
    std::vector<std::optional<Clip>> clips;

    // === Runtime State: the trigger tuple (one atomic word; see LayerRuntimeCell above) ===
    // runtime(): ONE acquire load. setRuntime(): a release store (a value restore -- undo / redo, a load, tests).
    // casRuntime(): ONE compare-exchange; on failure `expected` receives the current tuple.
    // updateRuntime(fn, maxAttempts): fn is a PURE function of the current tuple; retried until its CAS lands
    // (maxAttempts 0 = unbounded: the message thread; the GL thread always passes a bound and never waits).
    LayerRuntimeSnapshot runtime() const noexcept { return runtime_.load(); }
    void setRuntime(const LayerRuntimeSnapshot& r) noexcept { runtime_.store(r); }
    bool casRuntime(LayerRuntimeSnapshot& expected, const LayerRuntimeSnapshot& desired) noexcept
    {
        return runtime_.compareExchange(expected, desired);
    }

    template <class Fn>
    LayerRuntimeTransition updateRuntime(Fn&& fn, int maxAttempts = 0)
    {
        return updateRuntime(std::forward<Fn>(fn), maxAttempts,
                             [](const LayerRuntimeSnapshot&, const LayerRuntimeSnapshot&) {});
    }

    // beforeCas(from, to) runs before EVERY compare-exchange attempt that would install `to` (recomputed per
    // attempt), so whatever it writes is published by the CAS that installs `to`.
    template <class Fn, class BeforeCas>
    LayerRuntimeTransition updateRuntime(Fn&& fn, int maxAttempts, BeforeCas&& beforeCas)
    {
        LayerRuntimeSnapshot cur = runtime();
        for (int attempt = 0; maxAttempts <= 0 || attempt < maxAttempts; ++attempt)
        {
            const LayerRuntimeSnapshot next = fn(cur);
            if (next == cur)
                return { cur, cur, true };
            beforeCas(cur, next);
            LayerRuntimeSnapshot seen = cur;
            if (casRuntime(seen, next))
                return { cur, next, true };
            cur = seen;
        }
        return { cur, cur, false };
    }

    // === Helpers ===
    // ONE load, then the clip at that column.
    Clip* getActiveClip() { return getClipAt(runtime().activeClipColumn); }
    const Clip* getActiveClip() const { return getClipAt(runtime().activeClipColumn); }

    Clip* getClipAt(int column)
    {
        if (column >= 0 && column < static_cast<int>(clips.size()))
        {
            if (clips[static_cast<size_t>(column)].has_value())
                return &clips[static_cast<size_t>(column)].value();
        }
        return nullptr;
    }

    const Clip* getClipAt(int column) const
    {
        if (column >= 0 && column < static_cast<int>(clips.size()))
        {
            if (clips[static_cast<size_t>(column)].has_value())
                return &clips[static_cast<size_t>(column)].value();
        }
        return nullptr;
    }

    // The pure tuple functions (no clip effects): an immediate trigger of `column` (a retrigger when it is already
    // active: only the queue is cleared, the fade keeps running), and a clear.
    static LayerRuntimeSnapshot immediateNext(LayerRuntimeSnapshot r, int column, float transitionSpeedSeconds)
    {
        r.pendingTriggerColumn = -1;
        r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
        if (column == r.activeClipColumn)
            return r;
        r.previousClipColumn = r.activeClipColumn;
        r.activeClipColumn = column;
        r.crossfadeProgress = (transitionSpeedSeconds <= 0.0f) ? 1.0f : 0.0f;
        return r;
    }

    // L5 Quantize fix: a clear also cancels any quantized trigger still queued on this layer (ONE pending slot per
    // layer, so a trigger queued on an unrelated column is dropped too). Without this a pending trigger outlives the
    // clear and fires on the next beat / bar crossing, reactivating a layer the caller just deactivated. It lives in
    // the tuple function, not at call sites, so every caller inherits it.
    static LayerRuntimeSnapshot clearedNext(LayerRuntimeSnapshot r)
    {
        r.previousClipColumn = r.activeClipColumn;
        r.activeClipColumn = -1;
        r.crossfadeProgress = 1.0f;
        r.pendingTriggerColumn = -1;
        r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
        return r;
    }

    // Every trigger below returns the exact transition (statement callers ignore it). maxAttempts 0 = unbounded (the
    // message thread); the GL thread (autopilot, beat snap) passes 16 and never waits: if 16 consecutive
    // message-thread CASes on this layer beat it inside one call, the trigger retries at the next beat crossing (a
    // Bar / TwoBar / FourBar trigger slips to its next qualifying edge).

    // Trigger a clip: queue it for its beat / bar when beat snap is on (or a caller FORCES a granularity, global
    // Quantize) and it is not already active; otherwise trigger it immediately. An empty cell clears the layer.
    // onlyIfActive: a GL-thread trigger DECIDED from a tuple snapshot (the autopilot advance) passes the active column
    // it decided from; the trigger then applies only while the tuple still names that column -- tested inside the
    // pure function of every CAS attempt, so a clear, or an IMMEDIATE user trigger of another column, that landed since
    // the snapshot stands and the call is a no-op (no tuple change, no clip tail). The guard compares the ACTIVE column
    // only: a user trigger QUEUED by beat snap after the snapshot, or a retrigger of the same active column, leaves it
    // unchanged, so the advance still applies and (immediateNext) cancels that queue -- as before this guard existed.
    LayerRuntimeTransition triggerClip(int column, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off,
                                       int maxAttempts = 0, std::optional<int> onlyIfActive = std::nullopt)
    {
        if (column < 0 || column >= static_cast<int>(clips.size()))
            return unchanged();
        if (!clips[static_cast<size_t>(column)].has_value())
            return clearActiveClip(onlyIfActive, maxAttempts);

        const Clip& target = *clips[static_cast<size_t>(column)];
        const bool snapEnabled = forcedSnap != Clip::BeatSnapMode::Off
                              || target.beatSnapMode != Clip::BeatSnapMode::Off || target.beatSnap;
        const float speed = transitionSpeed;
        return activate(column, maxAttempts, onlyIfActive, [&](LayerRuntimeSnapshot r) {
            if (snapEnabled && column != r.activeClipColumn)
            {
                r.pendingTriggerColumn = column;
                r.pendingTriggerSnapOverride = forcedSnap;
                return r;
            }
            return immediateNext(r, column, speed);
        });
    }

    // Execute a clip trigger immediately (bypasses beat snap). A retrigger of the active column restarts its clip
    // from the in-point and keeps its play / pause state.
    LayerRuntimeTransition triggerClipImmediate(int column, int maxAttempts = 0)
    {
        if (column < 0 || column >= static_cast<int>(clips.size()))
            return unchanged();
        const float speed = transitionSpeed;
        return activate(column, maxAttempts, std::nullopt,
                        [&](const LayerRuntimeSnapshot& r) { return immediateNext(r, column, speed); });
    }

    // Fire the queued trigger if this beat qualifies. Call on each beat crossing. beatInBar: which beat within the bar
    // (0-3). barCount: total bars elapsed. One CAS-guarded transition, so a trigger cancelled on the message thread
    // (deck switch, clear, momentary release) can never fire afterwards.
    LayerRuntimeTransition processPendingTrigger(int beatInBar = 0, int barCount = 0, int maxAttempts = 16)
    {
        const float speed = transitionSpeed;
        auto fire = [&](LayerRuntimeSnapshot r) {
            const int col = r.pendingTriggerColumn;
            if (col < 0 || col >= static_cast<int>(clips.size()))
                return r;
            // A forced override (global Quantize) wins outright; otherwise the pending clip's own beatSnapMode.
            auto snapMode = Clip::BeatSnapMode::Beat;
            const auto& clipOpt = clips[static_cast<size_t>(col)];
            if (r.pendingTriggerSnapOverride != Clip::BeatSnapMode::Off)
                snapMode = r.pendingTriggerSnapOverride;
            else if (clipOpt.has_value())
                snapMode = (clipOpt->beatSnapMode != Clip::BeatSnapMode::Off) ? clipOpt->beatSnapMode
                                                                              : Clip::BeatSnapMode::Beat;
            bool shouldTrigger = false;
            switch (snapMode)
            {
                case Clip::BeatSnapMode::Off:
                case Clip::BeatSnapMode::Beat:
                    shouldTrigger = true; // Every beat
                    break;
                case Clip::BeatSnapMode::Bar:
                    shouldTrigger = (beatInBar == 0); // First beat of bar
                    break;
                case Clip::BeatSnapMode::TwoBar:
                    shouldTrigger = (beatInBar == 0 && (barCount % 2) == 0);
                    break;
                case Clip::BeatSnapMode::FourBar:
                    shouldTrigger = (beatInBar == 0 && (barCount % 4) == 0);
                    break;
            }
            return shouldTrigger ? immediateNext(r, col, speed) : r;
        };
        return updateRuntime(fire, maxAttempts, [this](const LayerRuntimeSnapshot& from, const LayerRuntimeSnapshot& to) {
            if (from.pendingTriggerColumn >= 0 && to.activeClipColumn == from.pendingTriggerColumn)
                applyActivationTail(from, to);
        });
    }

    // Clear the layer (and cancel its queued trigger, L5). onlyIfActive: clear only if that column is the active one
    // (one CAS: no separate check first). The old active clip stops playing.
    LayerRuntimeTransition clearActiveClip(std::optional<int> onlyIfActive = std::nullopt, int maxAttempts = 0)
    {
        auto t = updateRuntime([&](const LayerRuntimeSnapshot& r) {
            if (onlyIfActive.has_value() && r.activeClipColumn != *onlyIfActive)
                return r;
            return clearedNext(r);
        }, maxAttempts);
        applyClearTail(t);
        return t;
    }

    // A momentary pad released (MIDI / keyboard): ONE CAS. Its column active -> today's clear (incl. the pending
    // cancel); else its column still QUEUED (released before the beat) -> the queued trigger is cancelled, the
    // active clip untouched; else nothing.
    LayerRuntimeTransition releaseMomentary(int column)
    {
        auto t = updateRuntime([&](LayerRuntimeSnapshot r) {
            if (r.activeClipColumn == column)
                return clearedNext(r);
            if (r.pendingTriggerColumn == column)
            {
                r.pendingTriggerColumn = -1;
                r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
            }
            return r;
        });
        applyClearTail(t);
        return t;
    }

    void ensureColumns(int count)
    {
        if (static_cast<int>(clips.size()) < count)
            clips.resize(static_cast<size_t>(count));
    }

    // === Serialization ===
    juce::var toVar() const;
    void fromVar(const juce::var& v);

private:
    LayerRuntimeTransition unchanged() const
    {
        const auto r = runtime();
        return { r, r, true };
    }

    // The activation tail of a transition that makes a column active (a new activation, or a retrigger of the
    // active column): the clip's playhead back to its in-point, its beat count to 0, and -- only on a NEW activation
    // of a clip never triggered -- playing = true. Written BEFORE each CAS attempt that would install the
    // transition (idempotent, recomputed per attempt), so the acq_rel CAS publishes it: a GL-thread load that names
    // the column happens-after the reset (autopilot never counts a fresh trigger from a stale beatsPlayed). The
    // clip runtime fields are per-field atomics; the tuple word is the only consistent unit. Not reverted when the
    // CAS never lands (a concurrent cancel, or a bounded update out of attempts): harmless -- the clip is inactive,
    // nothing reads its playhead / beat count until its next activation re-runs this tail (a never-triggered clip
    // may be left playing = true, which its next first activation sets anyway).
    void applyActivationTail(const LayerRuntimeSnapshot& from, const LayerRuntimeSnapshot& to)
    {
        if (Clip* clip = getClipAt(to.activeClipColumn))
        {
            clip->playheadPosition = static_cast<double>(clip->inPoint);
            clip->beatsPlayed = 0;
            if (to.activeClipColumn != from.activeClipColumn && !clip->hasBeenTriggered)
                clip->playing = true;   // only auto-play on first activation; returning clips keep their state
        }
    }

    // fn may queue or activate `column`; the tail runs before each CAS that would make `column` active. A retrigger
    // with nothing queued changes no tuple field (no CAS), so its tail runs once after. onlyIfActive (see
    // triggerClip): fn runs only while the tuple's active column is that one; otherwise the attempt is a no-op.
    template <class Fn>
    LayerRuntimeTransition activate(int column, int maxAttempts, std::optional<int> onlyIfActive, Fn&& fn)
    {
        auto guarded = [&](const LayerRuntimeSnapshot& r) -> LayerRuntimeSnapshot {
            if (onlyIfActive.has_value() && r.activeClipColumn != *onlyIfActive)
                return r;   // decided from a stale snapshot: a changed active column (clear / immediate trigger) stands
            return fn(r);
        };
        auto t = updateRuntime(guarded, maxAttempts,
                               [&](const LayerRuntimeSnapshot& from, const LayerRuntimeSnapshot& to) {
                                   if (to.activeClipColumn == column)
                                       applyActivationTail(from, to);
                               });
        if (t.applied && !t.changed() && t.after.activeClipColumn == column
            && (!onlyIfActive.has_value() || *onlyIfActive == column))
            applyActivationTail(t.before, t.after);
        return t;
    }

    // The clear tail: the OLD active clip stops playing, applied once after the successful CAS. No guard needed: no
    // GL-thread path re-activates a cleared layer -- the autopilot needs an active playing clip, and an advance it
    // decided before the clear landed is conditioned on the column it decided from (triggerClip's onlyIfActive), so
    // it is a no-op on the cleared tuple; a queued trigger was cancelled in the same word.
    void applyClearTail(const LayerRuntimeTransition& t)
    {
        if (t.applied && t.before.activeClipColumn >= 0 && t.after.activeClipColumn < 0)
            if (Clip* clip = getClipAt(t.before.activeClipColumn))
                clip->playing = false;
    }

    // Declared after `clips` (member order = copy order).
    LayerRuntimeCell runtime_;
};

// The only place that names which Layer field backs each LayerScalar (s166
// spec section 2.2's exact phrasing). Used by Layer::eff() and by
// ConnectionEngine when publishing a shaped value into scalarLive.
RelaxedFloat& manualRef(Layer& l, LayerScalar s);
