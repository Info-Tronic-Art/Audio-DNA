#pragma once
#include "model/Deck.h"
#include "model/Layer.h"
#include "model/ClipRef.h"
#include "model/ShowMigration.h"
#include "model/Routine.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "connect/ConnSerialization.h"
#include <juce_core/juce_core.h>
#include <array>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

struct Composition;

// C2 (lane bf9b): where Composition::forEachClip found a clip.
struct ClipSite
{
    int deckIndex = -1;     // -1 = a retired deck
    uint32_t deckId = 0;
    int row = -1;           // = the shared layer the row feeds
    int column = -1;
    bool retired = false;
};

// The only place that names which Composition field backs each CompScalar
// (s166 spec section 2.2's exact phrasing). Forward-declared here (defined
// below the struct, where its fields are visible) so Composition::eff() --
// an inline member defined inside the class body -- can call it; ordinary
// name lookup for a free function needs the declaration to precede its use
// textually, unlike a class's own later-declared members.
RelaxedFloat& manualRef(Composition& c, CompScalar s);

// Composition: the complete app state saved to disk.
// Contains the shared layer stack, all decks (boxes of clips), global effects, global settings. Lane bf9b
// (s-rta-1002b): ONE shared layer stack plays whatever deck the grid shows; a deck switch changes only the grid.
struct Composition
{
    // === Identity ===
    std::string name = "Untitled";
    juce::File filePath; // Where this composition is saved

    // === The shared layer stack (lane bf9b) ===
    // Every layer's settings and its playing tuple; no clips (row N of every deck feeds layer N). Index 0 = bottom.
    // Written only on the message thread inside a fence for structure (insertLayer / eraseLayer / moveLayer); the GL
    // thread walks it every frame (CompositorEngine::compositeShow, the show autopilot).
    std::vector<Layer> layers;

    // === Decks (boxes of clips) ===
    std::vector<Deck> decks;
    // Written by the message thread (deck switch, deck remove / undo, load); read by the httplib thread (/api/status,
    // /api/composition, /api/state): a relaxed atomic (Pitfall 63). The GL thread never reads it -- it derives its
    // index from the acquire-loaded deck pointer (Renderer::renderOpenGL).
    RelaxedInt activeDeckIndex = 0;

    // Set by fromVar when an old show was converted (ShowMigration::convertShow, ruling-bf9b amendment 9) -- logged
    // once by the load; never silent. Not serialized (like routineLoadNote).
    std::string migrationNote;

    // Deck ids at or above this renumber at load (amendment 7(b)): far below ClipRef::kMaxDeckId, so a session never
    // runs out of deck numbers by accident.
    static constexpr uint32_t kDeckIdCompactAt = 8192;

    // === Global Effects (post-composite chain) ===
    std::vector<Clip::EffectSlot> globalEffects;

    // === Routines (s-rta-0926 routines slice 1, plan-routines-s1-final.md 3.1-3.2) ===
    // Composition-owned routines (D9 "inside the thing it belongs to") and the
    // pad bank that fires them. The bank references a routine by uuid; a
    // routine no slot references is erased by the slot helpers below.
    static constexpr int kRoutineBankSize = 8;   // matches MacroBank::kNumMacros
    std::vector<Routine> routines;
    std::vector<RoutineSlot> routineBank;
    // Set by fromVar when a bank entry had to be dropped (its routine is not in
    // the file, its slot is out of range or taken twice) -- the app shows it
    // once; never silent. Not serialized.
    std::string routineLoadNote;

    // === Composition Master ===
    // Lane tsan (s-rta-1002; Pitfall 63): the 9 manualRef scalars (master opacity / speed / signal, the comp
    // transform) are RelaxedFloat -- message-thread writers, GL-thread reads through eff().
    RelaxedFloat masterOpacity = 1.0f;
    RelaxedFloat masterSpeed = 1.0f;       // Global speed multiplier
    // Master Signal depth (s-rta-0925 mastersignal Step 1): 1 = every
    // signal->parameter connection moves the controls it drives fully;
    // 0 = every one of those controls sits at its hand value. Backs
    // CompScalar::Signal; scaled in via SignalDepth.h::applyDepth at the
    // one point where a signal enters each chain (ConnectionEngine::
    // evaluate for non-Macro sources, MacroBank::updateValues, v1
    // MappingEngine::processFrame) -- never on any GL thread (Boris Q2:
    // effects/sources reading the beat clock or audio uniforms directly
    // keep pulsing at 0%).
    RelaxedFloat masterSignal = 1.0f;

    // === CrossFader ===
    float crossfaderPhase = 0.5f;   // [0,1] A↔B
    enum class CrossfaderBlendMode : uint8_t { Alpha, Add, Multiply };
    CrossfaderBlendMode crossfaderBlendMode = CrossfaderBlendMode::Alpha;
    enum class CrossfaderBehaviour : uint8_t { Cut, Smooth };
    CrossfaderBehaviour crossfaderBehaviour = CrossfaderBehaviour::Cut;
    enum class CrossfaderCurve : uint8_t { Linear, EaseInOut, SCurve };
    CrossfaderCurve crossfaderCurve = CrossfaderCurve::Linear;

    // === Transform (composition-level, applied to final output) ===
    RelaxedFloat compPositionX = 0.0f;
    RelaxedFloat compPositionY = 0.0f;
    RelaxedFloat compScale = 1.0f;         // 1.0 = 100%
    RelaxedFloat compRotation = 0.0f;      // Degrees
    RelaxedFloat compAnchorX = 0.0f;
    RelaxedFloat compAnchorY = 0.0f;

    // === Connections (s167-l2) ===
    // One ParamConnection + LiveValue twin per CompScalar. Opacity targets
    // masterOpacity -- an owner amendment received during this lane rules
    // Composition opacity is ONE knob (final = masterOpacity * layerOpacity
    // * clipOpacity); the model's old separate per-composition opacity
    // field merged into masterOpacity and was removed (s-rta-0923 lane 3
    // plan section 4.6; see ScalarParams.h's CompScalar comment).
    // eff()/manualRef() are the only places that name which struct field
    // backs each CompScalar.
    std::array<ParamConnection, static_cast<size_t>(CompScalar::Count)> scalarConns;
    std::array<LiveValue, static_cast<size_t>(CompScalar::Count)> scalarLive;
    float eff(CompScalar s) const
    {
        return scalarLive[static_cast<size_t>(s)].effective(manualRef(const_cast<Composition&>(*this), s));
    }

    // === Connect settings (s167-l2) ===
    // Composition-level "connect" settings (owner D14/D15): how long a
    // release-less grip (MIDI/OSC/HTTP) survives after its last write before
    // the signal takes back over, and how long a hand-back glide runs after
    // any grip releases. Global to the whole composition, not per-connection
    // -- keeps ParamConnection to exactly the owner's per-connection list.
    float gripHoldMs = 250.0f;
    float handBackGlideMs = 120.0f;

    // === Global Settings ===
    int bpmMultiplier = 1; // -4 = ÷4, -2 = ÷2, 1 = ×1, 2 = ×2, 4 = ×4

    enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat };
    QuantizeMode quantizeMode = QuantizeMode::Off;

    // === Autopilot (composition-level) ===
    enum class AutopilotDirection : uint8_t { Rewind, Off, Forward, Random };
    AutopilotDirection autopilotDirection = AutopilotDirection::Off;
    enum class AutopilotDurationMode : uint8_t { LongestClip, ClipTransport, Custom };
    AutopilotDurationMode autopilotDurationMode = AutopilotDurationMode::LongestClip;
    int autopilotClipLoops = 1;
    bool autopilotLoop = false;
    int autopilotMasterLayer = -1;  // -1 = Off

    // === Per-Type Autopilot (P20) ===
    // Separate timers/settings for Opaque, Transparent, and FX layers.
    struct PerTypeAutopilotConfig
    {
        // Opaque layers
        int opaqueCycleBeats = 16;
        bool opaquePlayUntilEnd = false;

        // Transparent layers
        int transparentCycleBeats = 8;
        int transparentMaxLayers = 2;
        bool transparentRandomize = true;

        // Effect layers
        int effectCycleBeats = 4;
        int effectMaxLayers = 2;
        bool effectRandomize = true;

        // Global overrides
        bool perTypeEnabled = false;    // false = use existing per-layer autopilot
        bool globalRandomize = false;
        bool loopAutopilot = true;
    };
    PerTypeAutopilotConfig perTypeAutopilot;

    // === Genre-Aware Automation (P23) ===
    bool autoPresetOnGenre = false;         // Auto-switch visual preset on genre change
    bool smartAutopilotEnabled = false;     // Energy-aware clip selection
    bool structuralSceneEnabled = false;    // Auto-switch decks on structural transitions

    // Per-genre deck assignment: which deck to switch to when genre is detected
    // Index = genre ID (0-7), value = deck index (-1 = no switch)
    int genreDeckAssignment[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

    // Per-genre effect preset: name of FX preset to load when genre is detected
    std::string genrePresetNames[8] = {};

    // === Output Settings ===
    int outputWidth = 1920;
    int outputHeight = 1080;
    int outputDisplay = -1; // -1 = no external output

    // === Initialization ===
    // 3 shared layers (ids 0, 1, 2; layer 0 Opaque, the others Transparent) and one deck "Deck 1" with 3 rows.
    void initDefault()
    {
        name = "Untitled";
        filePath = juce::File();
        layers.clear();
        for (int i = 0; i < Deck::kDefaultLayers; ++i)
        {
            Layer layer;
            layer.name = "Layer " + std::to_string(i + 1);
            layer.id = static_cast<uint32_t>(i);
            layer.type = (i == 0) ? Layer::Type::Opaque : Layer::Type::Transparent;
            layers.push_back(std::move(layer));
        }
        nextLayerId_ = 100;
        decks.clear();
        retiredDecks_.clear();
        Deck deck;
        deck.name = "Deck 1";
        deck.id = 0;
        deck.initDefault(getNumLayers());
        decks.push_back(std::move(deck));
        activeDeckIndex = 0;
        globalEffects.clear();
        routines.clear();
        routineBank.clear();
        routineLoadNote.clear();
        migrationNote.clear();
        masterOpacity = 1.0f;
        masterSignal = 1.0f;
        bpmMultiplier = 1;
        quantizeMode = QuantizeMode::Off;
    }

    // === The shared layer stack (lane bf9b) ===
    Layer* getLayer(int index)
    {
        if (index >= 0 && index < static_cast<int>(layers.size()))
            return &layers[static_cast<size_t>(index)];
        return nullptr;
    }

    const Layer* getLayer(int index) const
    {
        if (index >= 0 && index < static_cast<int>(layers.size()))
            return &layers[static_cast<size_t>(index)];
        return nullptr;
    }

    int getNumLayers() const { return static_cast<int>(layers.size()); }
    int topLayerIndex() const { return static_cast<int>(layers.size()) - 1; }

    // === Active (SHOWN) Deck Access -- the grid's box, never what plays (Composition::playing) ===
    // ONE load of the index: the range check and the subscript see the same value.
    Deck* getActiveDeck()
    {
        const int idx = activeDeckIndex.load();
        if (idx >= 0 && idx < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(idx)];
        return nullptr;
    }

    const Deck* getActiveDeck() const
    {
        const int idx = activeDeckIndex.load();
        if (idx >= 0 && idx < static_cast<int>(decks.size()))
            return &decks[static_cast<size_t>(idx)];
        return nullptr;
    }

    // === Deck lookup by id (a ClipRef names a deck by id, never by index) ===
    // findDeckById: live decks, then retired ones. findDeckIndexById: live decks only, -1 = removed / retired / none.
    Deck* findDeckById(uint32_t id)
    {
        return const_cast<Deck*>(static_cast<const Composition&>(*this).findDeckById(id));
    }

    const Deck* findDeckById(uint32_t id) const
    {
        if (id == ClipRef::kNoDeck)
            return nullptr;
        for (const auto& d : decks)
            if (d.id == id)
                return &d;
        for (const auto& d : retiredDecks_)
            if (d.id == id)
                return &d;
        return nullptr;
    }

    int findDeckIndexById(uint32_t id) const
    {
        if (id == ClipRef::kNoDeck)
            return -1;
        for (size_t i = 0; i < decks.size(); ++i)
            if (decks[i].id == id)
                return static_cast<int>(i);
        return -1;
    }

    bool isRetiredDeck(uint32_t id) const
    {
        return std::any_of(retiredDecks_.begin(), retiredDecks_.end(), [id](const Deck& d) { return d.id == id; });
    }

    // Decks that were removed while one of their clips still plays (plan-bf9b F3): not shown, not saved, not
    // indexed; their ids stay reserved. Read-only outside the structure ops below.
    const std::vector<Deck>& retiredDecks() const { return retiredDecks_; }
    int getNumRetiredDecks() const { return static_cast<int>(retiredDecks_.size()); }

    // The clip a ref names in row `row` (live or retired deck); nullptr when the deck is gone or the cell is empty.
    Clip* clipAt(ClipRef ref, int row)
    {
        if (ref.column < 0)
            return nullptr;
        if (Deck* d = findDeckById(ref.deckId))
            return d->getClip(row, ref.column);
        return nullptr;
    }

    const Clip* clipAt(ClipRef ref, int row) const
    {
        if (ref.column < 0)
            return nullptr;
        if (const Deck* d = findDeckById(ref.deckId))
            return d->getClip(row, ref.column);
        return nullptr;
    }

    // === C1 (contract for bf6): what a shared layer plays ===
    // ONE runtime() load; the INCOMING (active) clip; nullptr when the layer is clear or its cell is empty; it may
    // point into ANY deck box, retired ones included. Message thread (and the GL thread inside its deckActive gate).
    struct PlayingClip
    {
        Clip* clip = nullptr;
        ClipRef ref;
        int deckIndex = -1;   // -1 = retired or none
        bool retired = false;
    };

    PlayingClip playing(int layerIndex) const
    {
        PlayingClip p;
        const Layer* layer = getLayer(layerIndex);
        if (layer == nullptr)
            return p;
        p.ref = layer->runtime().activeRef();
        if (p.ref.column < 0)
            return p;
        p.deckIndex = findDeckIndexById(p.ref.deckId);
        p.retired = p.deckIndex < 0 && isRetiredDeck(p.ref.deckId);
        p.clip = const_cast<Clip*>(clipAt(p.ref, layerIndex));
        return p;
    }

    Clip* playingClip(int layerIndex) { return playing(layerIndex).clip; }
    const Clip* playingClip(int layerIndex) const { return playing(layerIndex).clip; }

    // How shared layer `layerIndex` reaches the clips of its row in any deck (Layer's trigger API).
    RowClips rowClips(int layerIndex)
    {
        RowClips rc;
        rc.fn = &Composition::rowClipFn;
        rc.cellsFn = &Composition::rowCellsFn;
        rc.ctx = this;
        rc.row = layerIndex;
        return rc;
    }

    // === C2 (contract for bf6 / bf45): the walks ===
    // forEachLayer: fn(Layer&, int layerIndex), each shared layer exactly once, bottom to top.
    template <class Fn>
    void forEachLayer(Fn&& fn)
    {
        for (size_t i = 0; i < layers.size(); ++i)
            fn(layers[i], static_cast<int>(i));
    }

    template <class Fn>
    void forEachLayer(Fn&& fn) const
    {
        for (size_t i = 0; i < layers.size(); ++i)
            fn(layers[i], static_cast<int>(i));
    }

    // forEachClip: fn(Clip&, const ClipSite&), each clip of every deck box exactly once: live decks by index, then
    // retired decks; rows and columns ascending.
    template <class Fn>
    void forEachClip(Fn&& fn)
    {
        auto walk = [&](Deck& d, int deckIndex, bool retired) {
            for (size_t r = 0; r < d.rows.size(); ++r)
                for (size_t c = 0; c < d.rows[r].clips.size(); ++c)
                    if (d.rows[r].clips[c].has_value())
                        fn(*d.rows[r].clips[c], ClipSite{ deckIndex, d.id, static_cast<int>(r), static_cast<int>(c),
                                                          retired });
        };
        for (size_t i = 0; i < decks.size(); ++i)
            walk(decks[i], static_cast<int>(i), false);
        for (auto& d : retiredDecks_)
            walk(d, -1, true);
    }

    template <class Fn>
    void forEachClip(Fn&& fn) const
    {
        auto walk = [&](const Deck& d, int deckIndex, bool retired) {
            for (size_t r = 0; r < d.rows.size(); ++r)
                for (size_t c = 0; c < d.rows[r].clips.size(); ++c)
                    if (d.rows[r].clips[c].has_value())
                        fn(*d.rows[r].clips[c], ClipSite{ deckIndex, d.id, static_cast<int>(r), static_cast<int>(c),
                                                          retired });
        };
        for (size_t i = 0; i < decks.size(); ++i)
            walk(decks[i], static_cast<int>(i), false);
        for (const auto& d : retiredDecks_)
            walk(d, -1, true);
    }

    // === Firing (model level; MainComponent::handleClipTrigger / handleColumnTrigger stay the app's only entries) ===
    // Fire (deckIndex, row layerIndex, column) into shared layer layerIndex. An unknown layer / deck, or a ref the
    // tuple cannot hold (amendment 2(c)), is refused: the unchanged transition.
    LayerRuntimeTransition fire(int layerIndex, int deckIndex, int column,
                                Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off, bool immediate = false)
    {
        Layer* layer = getLayer(layerIndex);
        if (layer == nullptr)
            return {};
        const ClipRef ref = refFor(deckIndex, column);
        if (!ref.valid())
        {
            const auto r = layer->runtime();
            return { r, r, true };
        }
        return immediate ? layer->triggerClipImmediate(ref, rowClips(layerIndex))
                         : layer->triggerClip(ref, rowClips(layerIndex), forcedSnap);
    }

    // A column fire: the column of deck `deckIndex` into every shared layer that does not ignore column triggers; an
    // empty cell clears its layer (plan F12 / Q5's default). out (optional): one entry per layer -- the exact
    // transition, nullopt for a layer that ignores column triggers.
    void triggerColumn(int deckIndex, int column, Clip::BeatSnapMode forcedSnap = Clip::BeatSnapMode::Off,
                       std::vector<std::optional<LayerRuntimeTransition>>* out = nullptr)
    {
        if (out != nullptr)
            out->assign(layers.size(), std::nullopt);
        const ClipRef ref = refFor(deckIndex, column);
        if (!ref.valid())
            return;
        for (size_t i = 0; i < layers.size(); ++i)
        {
            auto& layer = layers[i];
            if (layer.ignoreColumnTrigger)
                continue;
            const auto t = layer.triggerClip(ref, rowClips(static_cast<int>(i)), forcedSnap);
            if (out != nullptr)
                (*out)[i] = t;
        }
    }

    // The ClipRef of (live deck index, column); an unknown deck gives a deck-less ref (refused by every trigger).
    ClipRef refFor(int deckIndex, int column) const
    {
        if (deckIndex < 0 || deckIndex >= static_cast<int>(decks.size()))
            return ClipRef{ ClipRef::kNoDeck, column };
        return ClipRef{ decks[static_cast<size_t>(deckIndex)].id, column };
    }

    // === Layer structure (every live AND retired deck keeps rows == layers) ===
    // Callers fence (UndoService::withDeckDetached): the GL thread walks `layers` and every deck's rows.
    // A new layer as Add Layer makes it: "Layer N", a fresh id, empty rows.
    Layer makeLayer(Layer::Type type = Layer::Type::Transparent)
    {
        Layer layer;
        layer.name = "Layer " + std::to_string(layers.size() + 1);
        layer.id = nextLayerId_++;
        layer.type = type;
        return layer;
    }

    // Insert `layer` at `at` (clamped) with an empty row in every deck (live and retired). Returns its index.
    int insertLayer(int at, Layer layer)
    {
        const size_t pos = static_cast<size_t>(std::clamp(at, 0, static_cast<int>(layers.size())));
        nextLayerId_ = std::max(nextLayerId_, layer.id + 1u);
        layers.insert(layers.begin() + static_cast<std::ptrdiff_t>(pos), std::move(layer));
        auto addRow = [pos](Deck& d) {
            ClipRow row;
            row.ensureColumns(d.numColumns);
            d.rows.insert(d.rows.begin() + static_cast<std::ptrdiff_t>(std::min(pos, d.rows.size())), std::move(row));
        };
        for (auto& d : decks)
            addRow(d);
        for (auto& d : retiredDecks_)
            addRow(d);
        return static_cast<int>(pos);
    }

    // Insert `layer` at `at` and the given row into each deck by id (live and retired); a deck the map lacks gets an
    // empty row (RemoveLayerCmd's undo, ruling-bf9b amendment 22).
    int insertLayerWithRows(int at, Layer layer, const std::vector<std::pair<uint32_t, ClipRow>>& rowsByDeckId)
    {
        const int pos = insertLayer(at, std::move(layer));
        auto fill = [&](Deck& d) {
            for (const auto& [id, row] : rowsByDeckId)
                if (id == d.id && pos < static_cast<int>(d.rows.size()))
                {
                    d.rows[static_cast<size_t>(pos)] = row;
                    d.rows[static_cast<size_t>(pos)].ensureColumns(d.numColumns);
                    return;
                }
        };
        for (auto& d : decks)
            fill(d);
        for (auto& d : retiredDecks_)
            fill(d);
        return pos;
    }

    // Erase layer `index` and row `index` of every deck. The show keeps >= 1 layer.
    bool eraseLayer(int index)
    {
        if (index < 0 || index >= static_cast<int>(layers.size()) || layers.size() <= 1)
            return false;
        layers.erase(layers.begin() + index);
        auto dropRow = [index](Deck& d) {
            if (index < static_cast<int>(d.rows.size()))
                d.rows.erase(d.rows.begin() + index);
        };
        for (auto& d : decks)
            dropRow(d);
        for (auto& d : retiredDecks_)
            dropRow(d);
        return true;
    }

    // P24.13: move a layer -- and row `from` of every deck in step, so every ref (active / previous / pending)
    // resolves to the same Clip afterwards.
    bool moveLayer(int fromIndex, int toIndex)
    {
        const int n = static_cast<int>(layers.size());
        if (fromIndex < 0 || fromIndex >= n || toIndex < 0 || toIndex >= n || fromIndex == toIndex)
            return false;
        Layer temp = std::move(layers[static_cast<size_t>(fromIndex)]);
        layers.erase(layers.begin() + fromIndex);
        layers.insert(layers.begin() + toIndex, std::move(temp));
        auto moveRow = [fromIndex, toIndex](Deck& d) {
            if (fromIndex >= static_cast<int>(d.rows.size()) || toIndex >= static_cast<int>(d.rows.size()))
                return;
            ClipRow row = std::move(d.rows[static_cast<size_t>(fromIndex)]);
            d.rows.erase(d.rows.begin() + fromIndex);
            d.rows.insert(d.rows.begin() + toIndex, std::move(row));
        };
        for (auto& d : decks)
            moveRow(d);
        for (auto& d : retiredDecks_)
            moveRow(d);
        return true;
    }

    // Every deck at exactly layers.size() rows: a short deck is padded with empty rows (sized to its columns).
    void padRows(Deck& d) const
    {
        if (d.rows.size() < layers.size())
        {
            const size_t from = d.rows.size();
            d.rows.resize(layers.size());
            for (size_t r = from; r < d.rows.size(); ++r)
                d.rows[r].ensureColumns(d.numColumns);
        }
    }

    // === Deck Management ===
    // Deck ids are never reused in a session (ruling-bf9b amendment 7): the mint refuses past ClipRef::kMaxDeckId.
    bool canMintDeckId() const { return nextDeckId_ <= ClipRef::kMaxDeckId; }

    bool addDeck(const std::string& deckName = "New Deck")
    {
        Deck deck;
        deck.name = deckName;
        deck.initDefault(getNumLayers());
        return appendDeck(std::move(deck)) >= 0;
    }

    // Append a fully-formed deck (e.g. loaded from a deck file) under a fresh id, padded to the show's layer count.
    // Returns its index, or -1 when the show has used all its deck numbers (nothing added). Caller is responsible
    // for the GL fence (push_back reallocates).
    int appendDeck(Deck deck)
    {
        if (!canMintDeckId())
            return -1;
        deck.id = nextDeckId_++;
        padRows(deck);
        decks.push_back(std::move(deck));
        return static_cast<int>(decks.size()) - 1;
    }

    // Insert a deck that keeps its id (undo / redo of a deck command) at `at` (clamped). Returns its index.
    int insertDeckKeepingId(int at, Deck deck)
    {
        padRows(deck);
        const size_t pos = static_cast<size_t>(std::clamp(at, 0, static_cast<int>(decks.size())));
        decks.insert(decks.begin() + static_cast<std::ptrdiff_t>(pos), std::move(deck));
        return static_cast<int>(pos);
    }

    // Does any shared layer's active ref, or the previous ref of a fade still running, name deck `id`? (a fading-out
    // clip keeps its deck alive until its fade completes -- ruling-bf9b amendment 4(b). A Cut or a clear leaves
    // `previous` set at progress 1: nothing draws that clip, so it keeps nothing alive.)
    bool deckIsPlaying(uint32_t id) const
    {
        for (const auto& l : layers)
        {
            const auto rt = l.runtime();
            if ((rt.activeClipColumn >= 0 && rt.activeDeckId == id)
                || (rt.previousClipColumn >= 0 && rt.previousDeckId == id && rt.crossfadeProgress < 1.0f))
                return true;
        }
        return false;
    }

    // Remove live deck `index` (plan-bf9b F3): while a layer plays from it (deckIsPlaying: its active ref, or a running
    // fade's previous ref, names it) it is RETIRED -- moved, with every clip at its address, to the retired list --
    // else erased. Returns true when it was retired; `erased` (optional) receives an erased deck. The caller fences
    // and adjusts activeDeckIndex.
    bool retireOrEraseDeck(int index, std::optional<Deck>* erased = nullptr)
    {
        if (index < 0 || index >= static_cast<int>(decks.size()))
            return false;
        const bool retire = deckIsPlaying(decks[static_cast<size_t>(index)].id);
        if (retire)
            retiredDecks_.push_back(std::move(decks[static_cast<size_t>(index)]));
        else if (erased != nullptr)
            *erased = std::move(decks[static_cast<size_t>(index)]);
        decks.erase(decks.begin() + index);
        return retire;
    }

    // Undo of a Remove Deck: move the retired deck `id` back to live index `at` (clamped), live playheads kept.
    // false when it is not retired any more (reaped).
    bool restoreRetiredDeck(uint32_t id, int at)
    {
        auto it = std::find_if(retiredDecks_.begin(), retiredDecks_.end(), [id](const Deck& d) { return d.id == id; });
        if (it == retiredDecks_.end())
            return false;
        Deck deck = std::move(*it);
        retiredDecks_.erase(it);
        insertDeckKeepingId(at, std::move(deck));
        return true;
    }

    // Every retired deck no layer plays from (deckIsPlaying) leaves the model; the caller disposes their media
    // (ruling-bf9b amendment 4(a): called inside every fenced edit, UndoService::withDeckDetached).
    std::vector<Deck> reapRetiredDecks()
    {
        std::vector<Deck> reaped;
        for (auto it = retiredDecks_.begin(); it != retiredDecks_.end();)
        {
            if (deckIsPlaying(it->id))
            {
                ++it;
                continue;
            }
            reaped.push_back(std::move(*it));
            it = retiredDecks_.erase(it);
        }
        return reaped;
    }

    // === Routine bank ===
    const Routine* routineInSlot(int slot) const
    {
        for (const auto& s : routineBank)
            if (s.slot == slot)
                for (const auto& r : routines)
                    if (r.uuid == s.uuid)
                        return &r;
        return nullptr;
    }

    Routine* routineInSlot(int slot)
    {
        return const_cast<Routine*>(static_cast<const Composition&>(*this).routineInSlot(slot));
    }

    // -1 when every pad is taken.
    int firstFreeRoutineSlot() const
    {
        for (int slot = 0; slot < kRoutineBankSize; ++slot)
            if (std::none_of(routineBank.begin(), routineBank.end(),
                             [slot](const RoutineSlot& s) { return s.slot == slot; }))
                return slot;
        return -1;
    }

    // Puts routine `uuid` (already in `routines`) on `slot`, replacing any occupant; the
    // replaced routine is erased unless another slot still references it.
    bool assignRoutineSlot(int slot, const std::string& uuid)
    {
        if (slot < 0 || slot >= kRoutineBankSize)
            return false;
        if (std::none_of(routines.begin(), routines.end(),
                         [&](const Routine& r) { return r.uuid == uuid; }))
            return false;

        std::string replaced;
        auto it = std::find_if(routineBank.begin(), routineBank.end(),
                               [slot](const RoutineSlot& s) { return s.slot == slot; });
        if (it != routineBank.end())
        {
            replaced = it->uuid;
            it->uuid = uuid;
        }
        else
        {
            routineBank.push_back(RoutineSlot{ slot, uuid });
        }
        if (!replaced.empty() && replaced != uuid)
            eraseRoutineIfUnreferenced(replaced);
        return true;
    }

    // Frees `slot`; its routine is erased unless another slot still references it.
    bool removeRoutineSlot(int slot)
    {
        auto it = std::find_if(routineBank.begin(), routineBank.end(),
                               [slot](const RoutineSlot& s) { return s.slot == slot; });
        if (it == routineBank.end())
            return false;
        const std::string uuid = it->uuid;
        routineBank.erase(it);
        eraseRoutineIfUnreferenced(uuid);
        return true;
    }

    // === Serialization ===
    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", juce::String(name));
        obj->setProperty("activeDeckIndex", activeDeckIndex.load());
        obj->setProperty("masterOpacity", static_cast<double>(masterOpacity));
        obj->setProperty("bpmMultiplier", bpmMultiplier);
        obj->setProperty("quantizeMode", static_cast<int>(quantizeMode));
        obj->setProperty("outputWidth", outputWidth);
        obj->setProperty("outputHeight", outputHeight);
        obj->setProperty("outputDisplay", outputDisplay);

        // Composition master + video
        obj->setProperty("masterSpeed", static_cast<double>(masterSpeed));
        obj->setProperty("masterSignal", static_cast<double>(masterSignal));

        // Crossfader
        obj->setProperty("crossfaderPhase", static_cast<double>(crossfaderPhase));
        obj->setProperty("crossfaderBlendMode", static_cast<int>(crossfaderBlendMode));
        obj->setProperty("crossfaderBehaviour", static_cast<int>(crossfaderBehaviour));
        obj->setProperty("crossfaderCurve", static_cast<int>(crossfaderCurve));

        // Transform (composition-level)
        obj->setProperty("compPositionX", static_cast<double>(compPositionX));
        obj->setProperty("compPositionY", static_cast<double>(compPositionY));
        obj->setProperty("compScale", static_cast<double>(compScale));
        obj->setProperty("compRotation", static_cast<double>(compRotation));
        obj->setProperty("compAnchorX", static_cast<double>(compAnchorX));
        obj->setProperty("compAnchorY", static_cast<double>(compAnchorY));

        // Autopilot (composition-level)
        obj->setProperty("autopilotDirection", static_cast<int>(autopilotDirection));
        obj->setProperty("autopilotDurationMode", static_cast<int>(autopilotDurationMode));
        obj->setProperty("autopilotClipLoops", autopilotClipLoops);
        obj->setProperty("autopilotLoop", autopilotLoop);
        obj->setProperty("autopilotMasterLayer", autopilotMasterLayer);

        // Per-Type Autopilot (P20)
        obj->setProperty("ptaOpaqueCycleBeats", perTypeAutopilot.opaqueCycleBeats);
        obj->setProperty("ptaOpaquePlayUntilEnd", perTypeAutopilot.opaquePlayUntilEnd);
        obj->setProperty("ptaTransparentCycleBeats", perTypeAutopilot.transparentCycleBeats);
        obj->setProperty("ptaTransparentMaxLayers", perTypeAutopilot.transparentMaxLayers);
        obj->setProperty("ptaTransparentRandomize", perTypeAutopilot.transparentRandomize);
        obj->setProperty("ptaEffectCycleBeats", perTypeAutopilot.effectCycleBeats);
        obj->setProperty("ptaEffectMaxLayers", perTypeAutopilot.effectMaxLayers);
        obj->setProperty("ptaEffectRandomize", perTypeAutopilot.effectRandomize);
        obj->setProperty("ptaPerTypeEnabled", perTypeAutopilot.perTypeEnabled);
        obj->setProperty("ptaGlobalRandomize", perTypeAutopilot.globalRandomize);
        obj->setProperty("ptaLoopAutopilot", perTypeAutopilot.loopAutopilot);

        // Genre-aware automation (P23)
        obj->setProperty("autoPresetOnGenre", autoPresetOnGenre);
        obj->setProperty("smartAutopilotEnabled", smartAutopilotEnabled);
        obj->setProperty("structuralSceneEnabled", structuralSceneEnabled);
        juce::Array<juce::var> genreDeckArray;
        for (int i = 0; i < 8; ++i)
            genreDeckArray.add(genreDeckAssignment[i]);
        obj->setProperty("genreDeckAssignment", genreDeckArray);
        juce::Array<juce::var> genrePresetArray;
        for (int i = 0; i < 8; ++i)
            genrePresetArray.add(juce::String(genrePresetNames[i]));
        obj->setProperty("genrePresetNames", genrePresetArray);

        // The shared layer stack (settings only; lane bf9b -- a file with this key is a bf9b show)
        juce::Array<juce::var> layerArray;
        for (const auto& layer : layers)
            layerArray.add(layer.toVar());
        obj->setProperty("layers", layerArray);

        // Decks (rows of clips)
        juce::Array<juce::var> deckArray;
        for (const auto& deck : decks)
            deckArray.add(deck.toVar());
        obj->setProperty("decks", deckArray);

        // Global effects
        juce::Array<juce::var> fxArray;
        for (const auto& fx : globalEffects)
        {
            auto* fxObj = new juce::DynamicObject();
            fxObj->setProperty("name", juce::String(fx.effectName));
            fxObj->setProperty("enabled", fx.enabled);
            fxObj->setProperty("bypassed", fx.bypassed);
            fxObj->setProperty("dryWet", static_cast<double>(fx.dryWet));
            juce::Array<juce::var> paramArray;
            for (float p : fx.paramValues)
                paramArray.add(static_cast<double>(p));
            fxObj->setProperty("params", paramArray);

            juce::Array<juce::var> connsArray;
            for (size_t p = 0; p < fx.paramConns.size(); ++p)
            {
                if (!fx.paramConns[p].isConnected())
                    continue;
                auto connVar = ConnSerialization::toVar(fx.paramConns[p]);
                connVar.getDynamicObject()->setProperty("p", static_cast<int>(p));
                connsArray.add(connVar);
            }
            if (!connsArray.isEmpty())
                fxObj->setProperty("conns", connsArray);
            if (fx.dryWetConn.isConnected())
                fxObj->setProperty("dryWetConn", ConnSerialization::toVar(fx.dryWetConn));

            fxArray.add(juce::var(fxObj));
        }
        obj->setProperty("globalEffects", fxArray);

        // s167-l2: per-scalar connection map, sparse -- only written if
        // something is connected.
        auto scalarConnsVar = ConnSerialization::scalarsToVar<CompScalar>(scalarConns, compScalarDefs());
        if (!scalarConnsVar.isVoid())
            obj->setProperty("conns", scalarConnsVar);

        // Composition-level connect settings (owner D14/D15).
        auto* connectObj = new juce::DynamicObject();
        connectObj->setProperty("gripHoldMs", static_cast<double>(gripHoldMs));
        connectObj->setProperty("handBackGlideMs", static_cast<double>(handBackGlideMs));
        obj->setProperty("connect", juce::var(connectObj));

        // Routines (s-rta-0926): both keys ALWAYS written (an empty array is fine).
        juce::Array<juce::var> routineArray;
        for (const auto& r : routines)
            routineArray.add(r.toVar());
        obj->setProperty("routines", routineArray);
        juce::Array<juce::var> bankArray;
        for (const auto& s : routineBank)
        {
            auto* slotObj = new juce::DynamicObject();
            slotObj->setProperty("slot", s.slot);
            slotObj->setProperty("uuid", juce::String(s.uuid));
            bankArray.add(juce::var(slotObj));
        }
        obj->setProperty("routineBank", bankArray);

        return juce::var(obj);
    }

    void fromVar(const juce::var& v)
    {
        if (auto* obj = v.getDynamicObject())
        {
            name = obj->getProperty("name").toString().toStdString();
            activeDeckIndex = static_cast<int>(obj->getProperty("activeDeckIndex"));
            masterOpacity = static_cast<float>(static_cast<double>(obj->getProperty("masterOpacity")));
            bpmMultiplier = static_cast<int>(obj->getProperty("bpmMultiplier"));
            quantizeMode = static_cast<QuantizeMode>(static_cast<int>(obj->getProperty("quantizeMode")));
            // s-rta-0926b plan4 S4: guarded like masterSpeed below -- the canvas size is the render
            // size now, and a JSON without these keys used to load 0x0.
            if (obj->hasProperty("outputWidth"))
                outputWidth = static_cast<int>(obj->getProperty("outputWidth"));
            if (obj->hasProperty("outputHeight"))
                outputHeight = static_cast<int>(obj->getProperty("outputHeight"));
            if (outputWidth <= 0 || outputHeight <= 0)
            {
                outputWidth = 1920;
                outputHeight = 1080;
            }
            outputDisplay = static_cast<int>(obj->getProperty("outputDisplay"));

            // Composition master + video (guarded for backward compatibility with old presets)
            if (obj->hasProperty("masterSpeed"))
                masterSpeed = static_cast<float>(static_cast<double>(obj->getProperty("masterSpeed")));
            // Guarded the same way (not masterOpacity's unguarded read at
            // masterOpacity's assignment above): an old composition without
            // this key must load 1.0 (Boris Q3), which the field's own
            // default already gives -- this line simply doesn't touch it.
            if (obj->hasProperty("masterSignal"))
                masterSignal = static_cast<float>(static_cast<double>(obj->getProperty("masterSignal")));
            // The old separate per-composition opacity key (pre-lane-3
            // presets) is a known, deliberately unrecognized key now --
            // s-rta-0923 lane 3 plan section 4.6: it merged into
            // masterOpacity (ruling 11). hasProperty-guarded loads simply
            // ignore unrecognized keys, so old files still load.

            // Crossfader
            if (obj->hasProperty("crossfaderPhase"))
                crossfaderPhase = static_cast<float>(static_cast<double>(obj->getProperty("crossfaderPhase")));
            if (obj->hasProperty("crossfaderBlendMode"))
                crossfaderBlendMode = static_cast<CrossfaderBlendMode>(static_cast<int>(obj->getProperty("crossfaderBlendMode")));
            if (obj->hasProperty("crossfaderBehaviour"))
                crossfaderBehaviour = static_cast<CrossfaderBehaviour>(static_cast<int>(obj->getProperty("crossfaderBehaviour")));
            if (obj->hasProperty("crossfaderCurve"))
                crossfaderCurve = static_cast<CrossfaderCurve>(static_cast<int>(obj->getProperty("crossfaderCurve")));

            // Transform (composition-level)
            if (obj->hasProperty("compPositionX"))
                compPositionX = static_cast<float>(static_cast<double>(obj->getProperty("compPositionX")));
            if (obj->hasProperty("compPositionY"))
                compPositionY = static_cast<float>(static_cast<double>(obj->getProperty("compPositionY")));
            if (obj->hasProperty("compScale"))
                compScale = static_cast<float>(static_cast<double>(obj->getProperty("compScale")));
            if (obj->hasProperty("compRotation"))
                compRotation = static_cast<float>(static_cast<double>(obj->getProperty("compRotation")));
            if (obj->hasProperty("compAnchorX"))
                compAnchorX = static_cast<float>(static_cast<double>(obj->getProperty("compAnchorX")));
            if (obj->hasProperty("compAnchorY"))
                compAnchorY = static_cast<float>(static_cast<double>(obj->getProperty("compAnchorY")));

            // Autopilot (composition-level)
            if (obj->hasProperty("autopilotDirection"))
                autopilotDirection = static_cast<AutopilotDirection>(static_cast<int>(obj->getProperty("autopilotDirection")));
            if (obj->hasProperty("autopilotDurationMode"))
                autopilotDurationMode = static_cast<AutopilotDurationMode>(static_cast<int>(obj->getProperty("autopilotDurationMode")));
            if (obj->hasProperty("autopilotClipLoops"))
                autopilotClipLoops = static_cast<int>(obj->getProperty("autopilotClipLoops"));
            if (obj->hasProperty("autopilotLoop"))
                autopilotLoop = static_cast<bool>(obj->getProperty("autopilotLoop"));
            if (obj->hasProperty("autopilotMasterLayer"))
                autopilotMasterLayer = static_cast<int>(obj->getProperty("autopilotMasterLayer"));

            // Per-Type Autopilot (P20)
            if (obj->hasProperty("ptaOpaqueCycleBeats"))
                perTypeAutopilot.opaqueCycleBeats = static_cast<int>(obj->getProperty("ptaOpaqueCycleBeats"));
            if (obj->hasProperty("ptaOpaquePlayUntilEnd"))
                perTypeAutopilot.opaquePlayUntilEnd = static_cast<bool>(obj->getProperty("ptaOpaquePlayUntilEnd"));
            if (obj->hasProperty("ptaTransparentCycleBeats"))
                perTypeAutopilot.transparentCycleBeats = static_cast<int>(obj->getProperty("ptaTransparentCycleBeats"));
            if (obj->hasProperty("ptaTransparentMaxLayers"))
                perTypeAutopilot.transparentMaxLayers = static_cast<int>(obj->getProperty("ptaTransparentMaxLayers"));
            if (obj->hasProperty("ptaTransparentRandomize"))
                perTypeAutopilot.transparentRandomize = static_cast<bool>(obj->getProperty("ptaTransparentRandomize"));
            if (obj->hasProperty("ptaEffectCycleBeats"))
                perTypeAutopilot.effectCycleBeats = static_cast<int>(obj->getProperty("ptaEffectCycleBeats"));
            if (obj->hasProperty("ptaEffectMaxLayers"))
                perTypeAutopilot.effectMaxLayers = static_cast<int>(obj->getProperty("ptaEffectMaxLayers"));
            if (obj->hasProperty("ptaEffectRandomize"))
                perTypeAutopilot.effectRandomize = static_cast<bool>(obj->getProperty("ptaEffectRandomize"));
            if (obj->hasProperty("ptaPerTypeEnabled"))
                perTypeAutopilot.perTypeEnabled = static_cast<bool>(obj->getProperty("ptaPerTypeEnabled"));
            if (obj->hasProperty("ptaGlobalRandomize"))
                perTypeAutopilot.globalRandomize = static_cast<bool>(obj->getProperty("ptaGlobalRandomize"));
            if (obj->hasProperty("ptaLoopAutopilot"))
                perTypeAutopilot.loopAutopilot = static_cast<bool>(obj->getProperty("ptaLoopAutopilot"));

            // Genre-aware automation (P23)
            if (obj->hasProperty("autoPresetOnGenre"))
                autoPresetOnGenre = static_cast<bool>(obj->getProperty("autoPresetOnGenre"));
            if (obj->hasProperty("smartAutopilotEnabled"))
                smartAutopilotEnabled = static_cast<bool>(obj->getProperty("smartAutopilotEnabled"));
            if (obj->hasProperty("structuralSceneEnabled"))
                structuralSceneEnabled = static_cast<bool>(obj->getProperty("structuralSceneEnabled"));
            if (auto* genreDeckArray = obj->getProperty("genreDeckAssignment").getArray())
            {
                int gi = 0;
                for (const auto& gd : *genreDeckArray)
                    if (gi < 8) genreDeckAssignment[gi++] = static_cast<int>(gd);
            }
            if (auto* genrePresetArray = obj->getProperty("genrePresetNames").getArray())
            {
                int gi = 0;
                for (const auto& gp : *genrePresetArray)
                    if (gi < 8) genrePresetNames[gi++] = gp.toString().toStdString();
            }

            // Lane bf9b: a bf9b show carries top-level "layers" (settings) + decks of clip rows; an old show (no
            // "layers") is converted by ShowMigration -- the first deck's layer settings win (plan-bf9b F7).
            layers.clear();
            decks.clear();
            retiredDecks_.clear();
            migrationNote.clear();
            if (ShowMigration::isLegacyShow(v))
            {
                migrationNote = ShowMigration::convertShow(v, layers, decks, nextLayerId_);
            }
            else
            {
                if (auto* layerArray = obj->getProperty("layers").getArray())
                    for (const auto& layerVar : *layerArray)
                    {
                        Layer layer;
                        layer.fromVar(layerVar);
                        layers.push_back(std::move(layer));
                    }
                if (auto* deckArray = obj->getProperty("decks").getArray())
                    for (const auto& deckVar : *deckArray)
                    {
                        Deck deck;
                        deck.fromVar(deckVar);
                        decks.push_back(std::move(deck));
                    }
                // Layer ids unique per show (GL history keys by layer id, Pitfall 35); id 0 stays valid (Pitfall 15).
                for (const auto& l : layers)
                    nextLayerId_ = std::max(nextLayerId_, l.id + 1u);
                for (size_t i = 0; i < layers.size(); ++i)
                    for (size_t j = 0; j < i; ++j)
                        if (layers[j].id == layers[i].id)
                        {
                            layers[i].id = nextLayerId_++;
                            break;
                        }
            }
            normalizeRows();

            // Deck ids (ruling-bf9b amendment 7(b)): any id above ClipRef::kMaxDeckId, or a largest id at or above
            // kDeckIdCompactAt, renumbers every deck 100, 101, ... in file order (safe at load: nothing outside the
            // file names a deck id, and a load clears undo). Otherwise ids are kept and duplicates re-minted below.
            {
                uint32_t maxId = 0;
                bool renumber = false;
                for (const auto& deck : decks)
                {
                    renumber = renumber || deck.id > ClipRef::kMaxDeckId;
                    maxId = std::max(maxId, deck.id);
                }
                if (renumber || maxId >= kDeckIdCompactAt)
                {
                    uint32_t next = 100;
                    for (auto& deck : decks)
                        deck.id = next++;
                    nextDeckId_ = next;
                }
            }

            // L3: nextDeckId_ resets to its default on every Composition constructed
            // by fromVar; without this, a post-load addDeck()/appendDeck() re-mints an
            // id a loaded deck already holds.
            for (const auto& deck : decks)
                nextDeckId_ = std::max(nextDeckId_, deck.id + 1u);

            // F1 (s-rta-0926b plan6): a file saved by a build whose New Deck left every deck at id 0 carries
            // DUPLICATE deck ids; a ClipRef names a deck by id, so two decks sharing an id would alias each other's
            // clips. Re-mint any repeat (the bump above already put nextDeckId_ past every loaded id).
            {
                std::vector<uint32_t> seen;
                for (auto& deck : decks)
                {
                    if (std::find(seen.begin(), seen.end(), deck.id) != seen.end())
                        deck.id = nextDeckId_++;
                    seen.push_back(deck.id);
                }
            }

            globalEffects.clear();
            if (auto* fxArray = obj->getProperty("globalEffects").getArray())
            {
                for (const auto& fxVar : *fxArray)
                {
                    if (auto* fxObj = fxVar.getDynamicObject())
                    {
                        Clip::EffectSlot slot;
                        slot.effectName = fxObj->getProperty("name").toString().toStdString();
                        slot.enabled = static_cast<bool>(fxObj->getProperty("enabled"));
                        slot.bypassed = static_cast<bool>(fxObj->getProperty("bypassed"));
                        if (fxObj->hasProperty("dryWet"))
                            slot.dryWet = static_cast<float>(static_cast<double>(fxObj->getProperty("dryWet")));
                        if (auto* paramArray = fxObj->getProperty("params").getArray())
                            for (const auto& p : *paramArray)
                                slot.paramValues.push_back(static_cast<float>(static_cast<double>(p)));
                        slot.resizeParams(slot.paramValues.size());
                        if (auto* connsArray = fxObj->getProperty("conns").getArray())
                        {
                            for (const auto& cv : *connsArray)
                            {
                                if (auto* cvObj = cv.getDynamicObject())
                                {
                                    int p = static_cast<int>(cvObj->getProperty("p"));
                                    if (p >= 0 && static_cast<size_t>(p) < slot.paramConns.size())
                                        ConnSerialization::fromVar(slot.paramConns[static_cast<size_t>(p)], cv);
                                }
                            }
                        }
                        if (fxObj->hasProperty("dryWetConn"))
                            ConnSerialization::fromVar(slot.dryWetConn, fxObj->getProperty("dryWetConn"));
                        globalEffects.push_back(std::move(slot));
                    }
                }
            }

            if (obj->hasProperty("conns"))
                ConnSerialization::scalarsFromVar<CompScalar>(scalarConns, compScalarDefs(), obj->getProperty("conns"));

            if (auto* connectObj = obj->getProperty("connect").getDynamicObject())
            {
                if (connectObj->hasProperty("gripHoldMs"))
                    gripHoldMs = static_cast<float>(static_cast<double>(connectObj->getProperty("gripHoldMs")));
                if (connectObj->hasProperty("handBackGlideMs"))
                    handBackGlideMs = static_cast<float>(static_cast<double>(connectObj->getProperty("handBackGlideMs")));
            }

            // Routines (s-rta-0926): cleared first, then read guarded -- a file saved before
            // routines existed loads with none. A bank entry whose routine is not in the file
            // (or whose slot is out of range / already taken) is dropped and counted into
            // routineLoadNote, never silently.
            routines.clear();
            routineBank.clear();
            routineLoadNote.clear();
            if (auto* routineArray = obj->getProperty("routines").getArray())
                for (const auto& rv : *routineArray)
                    routines.push_back(Routine::fromVar(rv));
            int droppedSlots = 0;
            if (auto* bankArray = obj->getProperty("routineBank").getArray())
            {
                for (const auto& sv : *bankArray)
                {
                    auto* slotObj = sv.getDynamicObject();
                    if (!slotObj) { ++droppedSlots; continue; }
                    RoutineSlot s;
                    s.slot = static_cast<int>(slotObj->getProperty("slot"));
                    s.uuid = slotObj->getProperty("uuid").toString().toStdString();
                    const bool slotOk = s.slot >= 0 && s.slot < kRoutineBankSize
                        && std::none_of(routineBank.begin(), routineBank.end(),
                                        [&](const RoutineSlot& o) { return o.slot == s.slot; });
                    const bool routineOk = std::any_of(routines.begin(), routines.end(),
                                                       [&](const Routine& r) { return r.uuid == s.uuid; });
                    if (slotOk && routineOk)
                        routineBank.push_back(std::move(s));
                    else
                        ++droppedSlots;
                }
            }
            if (droppedSlots > 0)
                routineLoadNote = std::to_string(droppedSlots)
                    + (droppedSlots == 1 ? " routine pad was" : " routine pads were")
                    + " left empty: the routine it pointed at is not in this file";
        }
    }

    // === File I/O ===
    bool saveToFile(const juce::File& file) const
    {
        auto json = juce::JSON::toString(toVar());
        return file.replaceWithText(json);
    }

    bool loadFromFile(const juce::File& file)
    {
        auto json = file.loadFileAsString();
        if (json.isEmpty()) return false;
        auto parsed = juce::JSON::parse(json);
        if (parsed.isVoid()) return false;
        fromVar(parsed);
        filePath = file;
        return true;
    }

    // Every deck at exactly layers.size() rows: a deck with MORE rows than the show has layers adds shared layers
    // (plan-bf9b F6: never drop clips), then every short deck is padded.
    void normalizeRows()
    {
        size_t most = 0;
        for (const auto& d : decks)
            most = std::max(most, d.rows.size());
        while (layers.size() < most)
            layers.push_back(makeLayer());
        for (auto& d : decks)
            padRows(d);
        for (auto& d : retiredDecks_)
            padRows(d);
    }

private:
    uint32_t nextDeckId_ = 100;
    uint32_t nextLayerId_ = 100;
    std::vector<Deck> retiredDecks_;

    static Clip* rowClipFn(void* ctx, uint32_t deckId, int row, int column)
    {
        auto* self = static_cast<Composition*>(ctx);
        if (Deck* d = self->findDeckById(deckId))
            return d->getClip(row, column);
        return nullptr;
    }

    static int rowCellsFn(void* ctx, uint32_t deckId, int row)
    {
        auto* self = static_cast<Composition*>(ctx);
        if (const Deck* d = self->findDeckById(deckId))
            if (const ClipRow* r = d->getRow(row))
                return r->getNumColumns();
        return -1;
    }

    void eraseRoutineIfUnreferenced(const std::string& uuid)
    {
        if (std::any_of(routineBank.begin(), routineBank.end(),
                        [&](const RoutineSlot& s) { return s.uuid == uuid; }))
            return;
        routines.erase(std::remove_if(routines.begin(), routines.end(),
                                      [&](const Routine& r) { return r.uuid == uuid; }),
                       routines.end());
    }
};

inline RelaxedFloat& manualRef(Composition& c, CompScalar s)
{
    switch (s)
    {
        case CompScalar::Opacity:  return c.masterOpacity;   // see the CompScalar comment above
        case CompScalar::Speed:    return c.masterSpeed;
        case CompScalar::PosX:     return c.compPositionX;
        case CompScalar::PosY:     return c.compPositionY;
        case CompScalar::Scale:    return c.compScale;
        case CompScalar::Rotation: return c.compRotation;
        case CompScalar::AnchorX:  return c.compAnchorX;
        case CompScalar::AnchorY:  return c.compAnchorY;
        case CompScalar::Signal:   return c.masterSignal;
        case CompScalar::Count:    break;
    }
    static RelaxedFloat dummy = 0.0f;   // unreachable for a valid enumerator
    return dummy;
}
