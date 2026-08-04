# SURFACE AUDIT — 2026-08-04d

First systematic surface-by-surface sweep of Audio-DNA. Two independent recon agents
(top-down from the UI, bottom-up from the wiring), then Harmony receiver-verified every
load-bearing claim on disk. HEAD `3b940a0`.

**Why this exists:** every previously-known defect was found by accident. Boris asked "what
is left to get all main surfaces working" and no one could answer, because a defect list is
not a surface map. This is the map.

---

## THE HEADLINE — HARMONY-VERIFIED, NOT RELAYED

### The output window cannot show the deck. Syphon is the only path to an audience.

`OutputRenderer`'s constructor takes exactly three things — `FeatureBus`, `MappingEngine`,
`EffectChain` (`src/ui/OutputWindow.h`, ctor). **No Compositor. No Deck. No Composition.**
Its only content inputs are `loadImage()` and `queueCameraFrame()` — the legacy v1
image/camera path. A case-insensitive grep for `compositor|deck|composition` across
`OutputWindow.cpp` + `OutputWindow.h` returns **ONE hit**, and it is a comment
(`OutputWindow.cpp:132`) stating the deck/composition surface is *"excluded from this arc."*

**Consequence:** open the output window on a projector today and you get the legacy image (or
nothing), with global FX — not your clips. VERIFIED by Harmony on disk. **Behavioral
confirmation still pending** (Boris has offered to run an output test).

**This reframes the whole OutputWindow C1/C2/C3 arc:** that arc hardened the window's
threading and lifecycle correctly, and none of it is wasted — but it was never about making
the window show the deck. That work was never done.

### Syphon DOES work, and it is real
`Renderer::publishSyphonFrame` (`Renderer.cpp:1850`) is called at `Renderer.cpp:693` from the
**main** Renderer — the one that composites the deck plus global FX. It blits the finished
default-framebuffer region and publishes it. VERIFIED call site.
=> **Today, driving a projector means Syphon into VDMX/Resolume.** That path is live.

---

## AUDIO REACTIVITY — the core is REAL. The shaders are the gap.

**Path (VERIFIED):** AudioEngine → RingBuffer → AnalysisThread → FeatureBus →
`Renderer.cpp:446` `setLatestSnapshot` → `CompositorEngine::uploadAudioUniforms`
(`CompositorEngine.cpp:1398-1456`, call site `:369`). **28 uniforms per effect per frame** —
rms, bass/mid/high, beatPhase, barPhase, phrasePhase, spectralCentroid/Flux,
onsetStrength/Detected, bpm, bandEnergies[7], chromagram[12], mfccs[13], key/genre/energy/
sidechain/swing/formant/resonance. **THIS IS THE ASSET TO PROTECT — do not rebuild it.**

**The gap is shader authoring, not plumbing.** Harmony-measured counts over
`src/render/EmbeddedShaders.h` (textual, so "used" is an UPPER BOUND):

| Uniform | Declared | Used | Verdict |
|---|---|---|---|
| `u_rms` | 96 | ~95 | Real — loudness reactivity is wired |
| `u_onsetStrength` | 21 | 19 | Real |
| `u_bass` | 26 | 18 | Mostly real |
| `u_spectralCentroid` | 11 | 6 | Half hollow |
| **`u_beatPhase`** | **56** | **~12** | **~44 shaders declare beat phase and ignore it** |

**User-visible meaning: everything pulses with VOLUME; almost nothing responds to the BEAT.**
The beat data is already uploaded to every one of those shaders and simply goes unused.

**CORRECTS AN INHERITED FACT (the 7th broken one):** prior handoffs said `u_beatPhase` was
"declared in 3 shaders, used in ZERO." Both halves wrong — 56 declared, ~12 used.

Also: `u_energyState` is uploaded by two subsystems and declared by ZERO shaders — a uniform
with no consumer. Harmless (location < 0), but it is dead freight.

---

## CONFIRMED DEAD — the app presents these and they cannot work

Harmony re-derived every zero-caller count below.

| Surface | What the user sees | Reality | Size |
|---|---|---|---|
| **Clip > Clear** | Clip clears | **Leaks a VideoPlayer every time.** `Renderer::closeMediaForClip` has ZERO call sites and holds the ONLY two `.erase()` calls on the media maps; `Deck::clearCell` never touches the Renderer; `s_nextClipId` never reuses ids ⇒ unbounded orphaned FFmpeg decoders + GL textures + decode threads for the life of the process | **one call** |
| Layer **Solo** | Toggles, undoes, repaints | Compositor never reads `Layer::solo` — sole hit in `src/render` + `Layer.h` is the declaration | one-liner |
| Top bar **BPM multiplier** | /4 /2 x1 x2 x4 | `onBpmMultiplierChanged` never assigned; `bpmMultiplier` never written or read | one-liner |
| Top bar **Quantize** | Sets a mode | No consumer outside JSON | one-liner |
| **Composition Inspector — Autopilot block** | Direction / Duration mode / Clip Loops / Loop / Master Layer | Zero readers; Autopilot reads only Layer + Clip | medium |
| **Composition Inspector — global FX stack** | An FX stack | `Composition::globalEffects` never read by any renderer | medium |
| **Composition > Open / Save / Save As** | Save and load a composition | Operate on the **v1 FX preset**, not the composition. A composition can be saved (only as a side effect of Collect Media) and **never loaded** | medium |
| **Comp/Decks browser** | Click a comp/deck to load; Save Comp | `onCompositionLoad` / `onDeckLoad` / `onCompositionSave` never assigned. Only Save Deck + right-click delete work | small (blocked by the above) |
| **Record tab** | Session record + playback | Only `recordClipTrigger` is called; 6 other `record*` methods and `advancePlayback()` have ZERO callers — records almost nothing, replays nothing | medium |
| **Effects rack v1 + MappingEditor** | — | Panel is `setVisible(false)` unconditionally on every `resized()`. It holds the **ONLY** mapping-curve editor | small-medium |
| **Signal routing (RoutingEngine)** | — | `addRoute` has one caller: the **test server**. No UI at all. Clip/Layer scope is a TODO | large |
| **Timing window** (BPM/Routing/Oscillators) | A tabbed panel | Placeholder — paints the tab name and nothing else, while holding a permanent quarter of the bottom row | medium |
| **Prefs > Video MilkDrop folder** | Browse dialog fills a path | `setPresetDirectories`/`rescan()` ZERO callers; value never read. Empty-state text tells users to use it | small |
| **Genre / P23 config family** | — | `Composition::loadFromFile` has ZERO callers, stranding `autoPresetOnGenre`, `smartAutopilotEnabled`, `structuralSceneEnabled`, `genreDeckAssignment`, `genrePresetNames`. **ONE root cause, not five.** Structural-scene handler body is a bare log — behavior absent, not just gated | medium |
| **Ableton Link** | — | `LinkSync::setEnabled` zero callers | one-liner→small |
| **MIDI output feedback** | — | `openDevice` zero callers ⇒ `isOpen()` always false | one-liner→small |
| **MilkDrop jukebox Pool / Mode / Blend** | Three controls | No `onChange` handler | small |
| **Param source BPMSync / ClipPosition / Timeline** | Appear in picker, mark param "connected" | Handled by none — and the param then stops responding to its own slider | small |

## PARTIAL — works, with a named gap
- **Effect param modulation freezes when you look away.** Modulation lives inside
  `EffectStackView::refresh()`, called only for the **active tab's inspected** clip/layer, at
  **10Hz**. (Severity is INFERRED — not confirmed at runtime.)
- **Signal inspector** promises a "routes list" in its header that does not exist.
- **Prefs > About** opens on the General tab (TODO in source).

## RULED OUT — do not re-investigate
- **Cut/Copy/Paste menu enums** — future-reserved, never shown to the user, self-documented.
- **The genre auto-switch handler itself** is correctly written; only the flag is unreachable.
- **Clip-replace does NOT leak** — `unique_ptr` reassignment destroys the old player and
  `~VideoPlayer` closes it. Only the *clear* path leaks.

## WIRED — confirmed working, leave alone
Deck/clip grid (trigger, select, all drag-drop forms, move, deck switch, undoable) · undo/redo ·
clip inspector · layer inspector · browser Files/FX/Sources · top bar transport/tap/manual BPM/
fade/master/audio-source · bindings + keyboard + MIDI learn (all 19 actions) · OSC/HTTP API ·
Syphon · video recorder · snapshot · all menus.

---

## OPEN QUESTIONS THAT DRIVE PRIORITY — Boris's call, not an agent's
1. **Is the output window MEANT to mirror the deck?** Its own comment says the deck surface was
   "excluded from this arc." Staged gap vs bug. This decides whether it is the #1 item or a
   deliberate later phase.
2. **Is the hidden EffectsRackPanel retired (v1 legacy) or accidentally hidden?** Decides
   whether the mapping-editor and routing-UI items are one fix or two.
3. **Are the legacy row-1 controls and the 10 bottom preset slots still wanted?** They are
   visible and their handlers work, but they operate on the v1 image/FX model the deck
   superseded.

## NOT REACHED — route elsewhere if needed
Per-effect correctness of the ~200 embedded shaders beyond uniform counts · the ISF import path
end to end · ProceduralSource / ProjectMSource internals · `design/FEATURE_INVENTORY.md`
audited against this map (known wrong at ~:2126).
