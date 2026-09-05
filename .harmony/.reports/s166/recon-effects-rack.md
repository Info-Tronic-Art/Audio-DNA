# Recon: EffectsRackPanel — reachability, capability, duplication, curve-shaping fate

Read-only recon, s166. No edits/builds/runs performed.

## 1. Definition and reachability

**Defined:** `src/ui/EffectsRackPanel.h:20` (class decl), `src/ui/EffectsRackPanel.cpp:5` (ctor).
Built via CMake (`CMakeLists.txt:254-255`), so it's compiled into the app, not dead-at-build-time.

**Instantiated:** `src/MainComponent.cpp:451-455`:
```
effectsRackPanel_ = std::make_unique<EffectsRackPanel>(
    previewPanel_.getMappingEngine(), previewPanel_.getEffectChain(), effectLibrary_);
addAndMakeVisible(effectsRackPanel_.get());
```
So it exists, is constructed, and is added as a child component. This is *not* zero call sites — it
is reachable as an object. The question is whether a user can ever see/use it.

**Two greps of different shape, both say "no":**

- Grep A (positioning): `grep -n "effectsRackPanel_->setBounds" src/MainComponent.cpp` →
  **zero matches**, anywhere in the file. A JUCE `Component` with no `setBounds()` call renders at
  its default 0×0 size regardless of visibility — so even absent everything below, it could never
  occupy screen space.
- Grep B (visibility): `grep -n "effectsRackPanel_" src/MainComponent.cpp` → the only visibility
  calls found are two `setVisible(false)`, at `src/MainComponent.cpp:2137` and `:2216`. There is
  **no** `effectsRackPanel_->setVisible(true)` anywhere in the file (confirmed by a separate grep
  for `setVisible(true)` across the whole file, whose hit list contains `signalBar_`, `deckView_`,
  `previewPanel_`, `waveformDisplay_`, `timingWindow_`, `inspectorPanel_`, `browserPanel_` — never
  `effectsRackPanel_`).

The `resized()` override hides it explicitly and unconditionally, in two separate layout branches:
- Signal-bar-expanded branch (`src/MainComponent.cpp:~2137`): hides everything else too when the
  signal bar takes over the screen.
- Normal v2 layout (`src/MainComponent.cpp:2214-2216`), under the comment
  `// Hide v1 panels removed from v2 layout` — grouped with `audioReadoutPanel_.setVisible(false)`
  and `spectrumDisplay_.setVisible(false)`, two other panels the codebase's own design docs already
  label `[HIDDEN-V1]`.

**Verdict on reachability: UNREACHABLE.** It is instantiated (so "call sites = 0" would be the
wrong claim) but never sized and never made visible in any code path — no button, menu item, or
tab flips it back to visible anywhere in `MainComponent.cpp`, `EffectStackView`, `InspectorPanel`,
or elsewhere (grepped `EffectsRackPanel(` for a second constructor site — none). A user cannot
reach it today under any UI action.

## 2. What it presents / does (src/ui/EffectsRackPanel.cpp, full read)

A vertical, scrollable rack (`viewport_` + `contentComponent_`) that lists **every effect in the
chain**, grouped under category headers (3D, Warp, Color, Glitch, Pattern, Animation, Blend, Blur —
`EffectsRackPanel.cpp:283-305`). Per effect (`EffectsRackPanel.cpp:307-420`):
- Enable/disable toggle (`enableToggle`)
- Lock button ("L") — protects the effect from the randomizer (`isEffectLocked()`,
  `EffectsRackPanel.h:36`)
- Per-effect "R" randomize button — randomizes that effect's own params and rebuilds 1-2 random
  audio-reactive mappings for it (`EffectsRackPanel.cpp:389-427`)
- One rotary `Knob` per parameter, each with a "+"/"M" button that opens a `MappingEditor` popup
  (`openMappingEditor()`, `EffectsRackPanel.cpp:466-495`) to bind that parameter to an audio
  feature (`MappingSource`: RMS, Peak, spectral centroid/flux, 7-band energies, onset, beat/bar/
  phrase phase, harmonic change, MFCCs, etc. — `src/mapping/MappingTypes.h:6-60`), a response curve
  (see §4), an output range, and a smoothing amount.
- A panel-wide "Random" button (`randomizeButton_`) that re-randomizes 3-8 unlocked effects at once
  with fresh mappings (`EffectsRackPanel.cpp:16-81`).

A 10 Hz timer (`timerCallback()`, `EffectsRackPanel.cpp:194-232`) keeps knob positions and mapping
indicators in sync with live parameter values, and rebuilds the whole UI when the effect count
changes (e.g. after a preset load).

## 3. Duplicate of EffectStackView, or distinct?

`EffectStackView` (`src/ui/EffectStackView.h`) is the **v2, currently-wired** effect list: header
comment `EffectStackView.h:17-19` shows collapsible rows with inline parameter sliders and a
"driven-by" viz line (e.g. `← Volume ▓▓▓▓▓░░░░░░`, `← Beat Position`, `← Manual`). It supports
drag-and-drop reorder (`DragAndDropTarget`), bypass, and operates on `Clip::EffectSlot` scoped to
clip/layer/global with undo (`EffectScope`, `setEffects()` comment, `EffectStackView.h:37-42`). It
takes a `SignalRegistry` for "source pickers" and a `MacroBank` for macro-driven params
(`EffectStackView.h:48-51`).

Grepping `EffectStackView.h`/`.cpp` for `Mapping|Knob|randomize|Random|lock` returns **zero
matches**. It has no per-parameter rotary knob widget, no lock-from-randomize concept, and no
"randomize this effect" button — those exist only in `EffectsRackPanel`.

Conversely, `MappingEditor` — the popup that actually lets a user pick a source + curve + range +
smoothing for one parameter — is used **only** by `EffectsRackPanel`
(`grep -rln "MappingEditor\b" src/` → `MappingEditor.cpp`, `MappingEditor.h`, `EffectsRackPanel.h`,
`EffectsRackPanel.cpp`; no other caller).

**This is not a plain duplicate.** It's the UI for a different, older modulation model
(`MappingEngine`/`Mapping`, source+curve+range+smoothing, `src/mapping/`) that `RoutingEngine`
explicitly documents itself as superseding: `src/routing/RoutingEngine.h:12` —
*"RoutingEngine: processes all routes each render frame. **Replaces MappingEngine for v2.** Called
by the render thread."* `EffectStackView`'s inline source-pickers/macros are the front end for
`RoutingEngine`+`Route` (`src/routing/Route.h:8-41`), which has no curve field at all (only
outputMin/Max, inverted, dialRange, threshold, gain, falloff — no `MappingCurve` analogue).

Both engines are live simultaneously: `RoutingEngine::processFrame()` runs every GL frame
(`src/render/Renderer.cpp:236`), and `MappingEngine::processFrame()` also still runs, unconditionally,
on a message-thread timer (`MainComponent.cpp:3019`; documented at
`src/mapping/MappingEngine.h:21-24`: *"processFrame() is called on the MESSAGE thread by
MainComponent's mapping tick timer... runs UNCONDITIONALLY... keeps mapped params updating even
while the preview GL context is detached"*). So the v1 `Mapping` data model is not inert dead
weight — it is actively evaluated every frame, and its curve-shaped output competes/sums with
whatever `RoutingEngine` writes to the same parameter (`MappingEngine.h:16-17`: multiple mappings
targeting the same param are summed and clamped).

`Mapping`s also round-trip through the live preset system, not just runtime: `PresetManager.cpp`
serializes them (`sourceToString`/`curveToString`, lines 35-64; write loop lines 106-151, `root->
setProperty("mappings", ...)`) and deserializes them on load (`PresetManager::loadPreset(...,
MappingEngine& engine, ...)`, `PresetManager.cpp:160-163`), called from the still-visible
Save/Load Preset buttons wired at `MainComponent.cpp:71-72` and used at `MainComponent.cpp:2368,
2395`. So a preset saved back when the rack was reachable — or one authored by hand-editing that
JSON — can carry curve-shaped mappings that silently apply today, even though no UI can currently
create or inspect a new one.

## 4. Curve shaping — implemented? where does it live?

`MappingCurve` (`src/mapping/MappingTypes.h:88-116`) has ~20 members: Linear, Exponential
(`x^2.0`), Logarithmic (`log(1+9x)/log(10)`), SCurve (smoothstep), Stepped, and a P24 batch of
easing curves (Circular/Back/Elastic/Bounce/Cubic/Sine in/out/inOut variants, plus Hold). All are
implemented: `MappingEngine::applyCurve()` (`src/mapping/MappingEngine.cpp:139`, declared
`MappingEngine.h:59`) and `src/mapping/CurveTransforms.h:221` (comment: *"Matches MappingCurve enum
order"*) supply the actual math — not stubs.

**This logic is 100% in the model/engine layer, not in `EffectsRackPanel`.** `EffectsRackPanel.cpp`
never computes a curve; it only lets the user *pick* a `MappingCurve` value inside `MappingEditor`
and writes it into a `Mapping` struct that `MappingEngine` owns and evaluates
(`Mapping::curve` field, `MappingTypes.h:130`). Deleting `EffectsRackPanel.{h,cpp}` deletes zero
curve math — `MappingEngine.{h,cpp}`, `MappingTypes.h`, and `CurveTransforms.h` are independent
files with no dependency on the panel, and `MappingEngine::processFrame()` keeps running from the
message-thread timer either way. `PresetManager`'s mapping serialization is likewise independent
and would keep round-tripping existing curve-mapped presets.

**What WOULD be lost:** the only UI path to *create or edit* a `Mapping` (`MappingEditor`, opened
exclusively from `EffectsRackPanel`'s "+"/"M" buttons) and the only UI path that shows lock/
randomize/per-effect knobs. `MappingEditor.{h,cpp}` would become a second orphan (0 remaining
callers) unless kept deliberately for reuse. Practically, this capability is *already* unreachable
today (see §1), so the marginal loss from deleting the files is: removing the dead source, not
removing a live user-facing capability that exists right now.

## 5. Verdict

**(b) — a half-built/orphaned-UI feature with a genuinely distinct purpose, not a dead duplicate.**

- It is not (a) a duplicate of `EffectStackView`: the two front ends drive two different backend
  modulation systems (v1 `MappingEngine`/curve-shaped `Mapping`, still ticking every frame and
  still saved/loaded in presets, vs. v2 `RoutingEngine`/`Route`, which has no curve concept at all).
  `EffectStackView` cannot do what the rack's `MappingEditor` does (pick a response curve); the
  rack cannot do what the stack does (inline collapsible rows, drag reorder, clip/layer/global
  scoping, macro bank).
- It is unreachable today by any UI action (§1), so calling it "working" would overstate it — the
  UI half is genuinely half-built/orphaned.
- The curve-shaping engine underneath is fully implemented, decoupled from the panel, and already
  live in the render/preset pipeline (§3, §4) — so it would survive panel deletion intact, but the
  only way to *drive* it (create a new curve-mapped parameter binding) would be gone unless
  something replaces `MappingEditor`'s role, or `RoutingEngine`/`EffectStackView` grows an
  equivalent curve field first.
