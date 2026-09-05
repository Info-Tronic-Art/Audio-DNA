# Lane 4 — Control Plane — Re-normalize Audit (2026-09-05)

Scope: `src/mapping`, `src/routing`, `src/signal`, `src/binding`, `src/midi`, `src/osc`,
`src/sync`, `src/model`, `src/core`. Baseline: `9139dd4` (2026-07-16). HEAD at time of audit.

## 0. Scope of drift

```
git log --oneline 9139dd4..HEAD -- src/mapping src/routing src/signal src/binding src/midi src/osc src/sync src/model src/core
```
→ **19 commits** touch these paths (counted by hand from the log, not carried from any
prior number). List:

```
e76ca9f docs(mapping): correct MappingEngine.h threading comment for C3
c51aff7 feat(mapping): message-thread mapping tick @120Hz (OW arc C3: W5+A4+A6)
fcad6d0 fix(mapping): single-store processFrame + test-mode mapping routes (OW arc C2: W4+W6)
43ff194 Fix round: replace stringly-typed layer-move detection with Command::affectsLayerOrder
ca068e4 Remove dead ReinspectTarget code path (B8)
201654a Fix autopilot advance-gate stall for non-playable clips (Source/Image/Camera)
a718572 Fix X-clear renderer purge and kClipClear blank-cell bug (bundle A)
8bd09ba fix(render): GL-fence family — fence all message-thread model mutations against unlocked GL reads
0a1c882 test(undo): Undo v1 step 9 — spec §7 test remainder + cleanup folds
4ee2dac feat(undo): Undo v1 step 8 — clip/column triggers (#1, #2) with same-layer merge
d90e953 feat(undo): Undo v1 step 7 — effect stacks (#27-29) via performEdit + EffectScope
316a2bf feat(undo): Undo v1 step 6 — deck ops (#21, #22, #24)
7f87094 feat(undo): Undo v1 step 5 — layer ops (#13-18, #20) with GL fence
6d2def4 feat(undo): Undo v1 step 4 — composites (multi-drops, column ops, layer/deck clear)
daa9361 feat(undo): Undo v1 step 3 — SwapClipsCmd (clip move/swap with column-count restore)
7921572 feat(undo): Undo v1 step 2 — SetClipCmd + single-cell sites, replace-undo media fix
7c8d286 feat(undo): Undo v1 step 1 — plumbing (GL fence validated, CompositeCommand, UndoService, dynamic menu)
6484aba Wave 1-C: serialize all dropped Clip/Layer/Composition fields (fix lossy presets)
368d621 refactor(cleanup): Wave 0 Group 1 — purge dead code (13 items)
```

`git diff --stat 9139dd4..HEAD -- <lane paths>`: 25 files, +2015/-480. By far the largest
chunk is `src/core` (new: ClipCommands.h, CompositeCommand.h, DeckCommands.h 759 lines,
EffectCommands.h, EffectScope.h, MediaReconnect.h, TriggerCommands.h, UndoService.h/.cpp).
`src/routing`, `src/binding`, `src/midi`, `src/sync` show **zero** commits and zero diff in
this window — confirmed by the same log/diff command above returning nothing for those
subtrees specifically (re-run individually to be sure: each of
`git log --oneline 9139dd4..HEAD -- src/routing`, `-- src/binding`, `-- src/midi`,
`-- src/osc`, `-- src/sync` returns empty).

---

## 1. CRITICAL — "Undo/redo remains a no-op" is false; a full Undo v1 landed

**FEATURES.md says:** Top banner (line 7): *"Undo/redo remains a no-op (Wave 2)."* Section 8
also lists `UndoManager` as "Command pattern undo/redo" and "supports undo/redo chain" — but
without the banner's caveat readers would assume that description was already live; the
banner is the operative, more recent-sounding claim and it says no-op.

**Source says:** Undo/redo is now a substantial, wired, tested subsystem ("Undo v1", 9 build
steps across 11 commits, `7c8d286`..`0a1c882`).

Evidence:
- `src/core` grew from 3 files (`Command.h`, `UndoManager.h/.cpp` — infra only, no concrete
  commands) at `9139dd4` to 12 files at HEAD: `ClipCommands.h`, `CompositeCommand.h`,
  `DeckCommands.h` (759 lines), `EffectCommands.h`, `EffectScope.h`, `MediaReconnect.h`,
  `TriggerCommands.h`, `UndoService.h/.cpp` are all new (`git ls-tree -r --name-only 9139dd4
  -- src/core` vs `HEAD` diff).
- Concrete `Command` subclasses are actually constructed and pushed at 22+ call sites in
  `src/MainComponent.cpp` (`grep -on 'std::make_unique<[A-Za-z]*Cmd>' src/MainComponent.cpp`):
  `SetClipCmd`, `SwapClipsCmd`, `SwitchDeckCmd`, `TriggerClipCmd` (x2), `AddDeckCmd`,
  `RemoveDeckCmd`, `AddLayerCmd`, `RemoveLayerCmd`, `MoveLayerCmd` (x2), `ClearLayerClipsCmd`
  (x2), `ToggleLayerFlagCmd` (x3), `SetColumnCountCmd` (x5), `RemoveColumnCmd`,
  `ClearActiveClipCmd` (x2), `ToggleClipLockCmd`, `EffectStackCmd` (x2).
- All route through `MainComponent::pushCommands()` → `undoManager_.perform(...)`
  (`src/MainComponent.cpp:3597-3612`), which wraps multi-child edits in `CompositeCommand`.
- The Edit menu is dynamically labelled from live state:
  `undoManager_.undoDescription()` / `redoDescription()` at `src/MainComponent.cpp:1576,1580`.
- GL-fenced mutations (`UndoService::withDeckDetached`, `src/core/UndoService.h`) and a
  structural `Command::affectsLayerOrder()` flag (added this window, commit `43ff194`) drive
  post-undo UI reconciliation (`refreshAfterUndoRedo`, `src/MainComponent.cpp:3599+`).
- `tests/test_undo_commands.cpp` — 2222 lines, 59 `TEST_CASE`s (new this window, commit
  `0a1c882` is "test remainder + cleanup folds", i.e. step 9 of 9).

**Nuance on the task brief's premise:** `src/core` is not wholly new — `Command.h` and
`UndoManager.h/.cpp` already existed at `9139dd4` (infrastructure only: perform/undo/redo/
canUndo/canRedo, but **zero** concrete `Command` subclasses anywhere in the repo at that
sha, so `undo()` always found an empty history — which is exactly what made the old banner
claim true then). What's new is the 8 files that gave the infrastructure something to do.
The banner needs updating to reflect that Undo v1 now covers: clip/column triggers, clip
set/swap, column count changes, layer ops (add/remove/move/clear/toggle-flag), deck ops
(add/remove/switch/clear), and effect stacks — not "no-op."

**Not covered / could not fully verify:** I did not exhaustively enumerate every mutating
call site in MainComponent.cpp to confirm 100% coverage (e.g. whether macro-knob assignment
or binding edits push undo commands) — the 22+ call sites above are a lower bound, not a
closed set. Treat "Undo v1 covers clip/deck/layer/effect/trigger ops" as confirmed; "covers
literally everything mutable" as unconfirmed.

---

## 2. CRITICAL — Mapping pipeline: wrong method name AND wrong threading model

**FEATURES.md says (§6, "Implementation chain" step 3):** *"Each render frame:
`MappingEngine::processAll(snapshot)`"* — implying mapping evaluation happens on the
render/GL callback, once per rendered frame, and that the method is named `processAll`.

**Source says both are wrong:**
- The method is `processFrame`, not `processAll`. `grep -rn "processAll" src/
  .harmony/FEATURES.md` returns **only** the FEATURES.md line itself — the symbol does not
  exist anywhere in source. (Second pattern: `grep -rn "MappingEngine::" src/mapping/
  MappingEngine.h` shows only `processFrame` as the per-tick entry point.)
- It is explicitly **not** called from the render path any more. `src/mapping/
  MappingEngine.h`'s class comment (rewritten this window, commit `e76ca9f`, "docs(mapping):
  correct MappingEngine.h threading comment for C3"):
  > "Since C3 ... processFrame() is called on the MESSAGE thread by MainComponent's mapping
  > tick timer (MappingTickTimer, kMappingTickHz) — NOT a render/GL thread. The tick runs
  > UNCONDITIONALLY, independent of either GL context's attach state..."
- `src/render/Renderer.cpp:241` carries the removal comment in place: *"mappingEngine_.
  processFrame() moved OFF this GL callback. A message-thread juce::Timer ... is now the
  SOLE caller, running unconditionally so mapped params keep updating even while this GL
  context is detached."*
- Actual call site: `src/MainComponent.cpp:2483`,
  `previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain());`,
  driven by `mappingTickTimer_.startTimerHz(kMappingTickHz)` at `src/MainComponent.cpp:261`,
  with `kMappingTickHz = 120` (`src/MainComponent.h:240`).

This is a behavior-reachable error, not phrasing: a reader would conclude mapped params stop
updating whenever the render/GL path stalls or the preview context detaches. The opposite is
now true by design — mapping updates at a fixed 120Hz on the message thread regardless of
render state. Two related commits this window (`fcad6d0` "single-store processFrame", `c51aff7`
"message-thread mapping tick @120Hz") plus the doc-only follow-up `e76ca9f` show this was a
deliberate, recent architectural change (part of an "outputwindow-arc-design.md" arc, steps
C2/C3), not a typo — FEATURES.md simply wasn't updated for it.

---

## 3. MAJOR — Dead surface: BPM Multiplier buttons (/4, /2, x1, x2, x4)

**FEATURES.md says (§26a):** *"BPM Multiplier: /4, /2, x1, x2, x4 buttons — multiply detected
BPM"* — presented as working, no caveat.

**Source says:** the control writes a model field and fires a callback; nothing reads either.

- `TopBar::handleMultiplierButton()` (`src/ui/TopBar.cpp:286-313`) does two things: sets
  `composition_.bpmMultiplier = multiplier;` and, if `onBpmMultiplierChanged` is set, invokes
  it.
- Pattern 1 — is `composition_.bpmMultiplier` ever *read* anywhere besides that one write
  site and Composition's own default/serialize/deserialize? `grep -rn "\.bpmMultiplier"
  src/**/*.cpp` → only the write site (`TopBar.cpp:290`). No consumer multiplies a displayed
  or detected BPM by it anywhere.
- Pattern 2 — is `onBpmMultiplierChanged` (the callback `TopBar` fires) ever assigned to a
  handler? `grep -rn "onBpmMultiplierChanged\s*=" src/` → **zero results**, including in
  `MainComponent.cpp` where `TopBar`'s other callbacks (`onTapTempo`, `onManualBpmChanged`,
  etc.) get wired. It is declared (`TopBar.h:27`), fired (`TopBar.cpp:312-313`), and never
  listened to.
- The field does round-trip through `Composition::toVar()/fromVar()` (`Composition.h:169,
  260`), so a saved preset will silently carry a `bpmMultiplier` value that has never done
  anything and never will on reload either.

Net effect: clicking any of the five multiplier buttons highlights the button and does
nothing else. FEATURES.md's "multiply detected BPM" is false as user-reachable behavior.

---

## 4. MAJOR — Dead surface: Quantize selector (Off / Next-Beat / Next-Downbeat)

**FEATURES.md says (§26a, §8 Config):** Lists "Quantize: dropdown selector for beat snap
modes" and, separately in §8 Config, "Beat snap modes: Off, Beat, Bar, TwoBar, FourBar" (note:
that's a *different*, five-value enum — see §5 below on why these two "quantize" mentions
don't even agree with each other or with source).

**Source says:** `Composition::QuantizeMode` (`src/model/Composition.h:51`) is
`enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat };` — three values, matching
the task brief's candidate list, not the five-value "Off, Beat, Bar, TwoBar, FourBar" list
FEATURES.md's §8 Config attributes to "beat snap modes." `quantizeMode` is:
- Written by `TopBar.cpp:159-160` from the dropdown selection.
- Serialized/deserialized (`Composition.h:170, 261`).
- **Never read anywhere else.** `grep -rn "quantizeMode" src/` outside `TopBar.cpp` and
  `Composition.h` → zero hits. No clip-trigger, autopilot, or transport code path consults it
  to actually delay a trigger to the next beat/downbeat.
- `onQuantizeChanged` (declared `TopBar.h:28`) is likewise never assigned
  (`grep -rn "onQuantizeChanged\s*=" src/` → zero).

Separately, §8's "Beat snap modes: Off, Beat, Bar, TwoBar, FourBar" appears to describe
something else entirely (possibly autopilot beat-snap, not the TopBar quantize selector) —
I could not find a 5-value enum matching that list anywhere in `src/model` or `src/render`
under a second pattern search (`grep -rn "TwoBar\|FourBar" src/`); that line in §8 may itself
be describing a feature that doesn't exist under that name, or one outside this lane's paths.
Flagging as **could-not-determine** rather than asserting it's wrong outright.

---

## 5. MAJOR — Dead surface: "Smart Autopilot" toggle is disconnected from the code that implements it, and that code has no caller either

**FEATURES.md says (§16):** *"Real-time 8-genre classification ... with smart autopilot and
structural scene triggering,"* step 5: *"Smart random autopilot uses structural state +
energy level for clip selection,"* Config: `composition.smartAutopilotEnabled`.

**Source says:** two unrelated booleans exist, and neither is reachable by a user.

- `Composition::smartAutopilotEnabled` (`src/model/Composition.h:90`) — the field
  FEATURES.md's Config line names. `grep -rn "smartAutopilotEnabled" .` (whole repo, both
  `.cpp`/`.h`) → only `Composition.h` (default/serialize/deserialize) and
  `tests/test_composition.cpp` (round-trip test). **No runtime code reads it.** No UI control
  sets it (`grep -rn -i "smartAutopilot" src/ui/` → zero).
- The actual smart-random clip-selection logic lives behind a *different* flag,
  `Autopilot::smartRandomEnabled_` (`src/model/Autopilot.h:62`, setter
  `setSmartRandomEnabled()` at line 25), and is genuinely wired into `Autopilot::processFrame`
  (`src/model/Autopilot.cpp:33-34, 101-103` gate calls to `smartAdvanceClip()`). This part is
  real, implemented logic — not a stub.
- But `Autopilot::setSmartRandomEnabled()` has exactly one caller in the whole tree:
  `Renderer::setSmartRandomEnabled()` (`src/render/Renderer.h:113`), which just forwards to
  it — and **that** has zero callers. `grep -rn "setSmartRandomEnabled" src/ tests/` → only
  the two declarations above, no invocation anywhere (not `MainComponent.cpp`, not
  `src/api`, not `src/osc`, not `src/ui`). Second pattern
  (`grep -rn -i "smartrandom\|smart_random\|smart-random" src/api src/osc src/ui`) also
  empty.

Net effect: the model field a preset would persist (`smartAutopilotEnabled`) and the runtime
flag that actually changes autopilot behavior (`smartRandomEnabled_`) are two different
variables that never talk to each other, and the one with real logic behind it defaults to
`false` and can never be set to `true` by anything reachable from the UI, REST API, or OSC.
"Smart random autopilot" as described in §16 cannot currently be turned on by a user through
any path I could find.

---

## 6. MAJOR — Dead surface: 3 of 8 UniversalParamControl "source modes" do nothing (BPMSync, ClipPosition, Timeline)

**FEATURES.md says (§26m):** *"Supports 8 source modes: Manual, Signal, BPMSync, Oscillator,
Envelope, ClipPosition, Timeline, and Macro"* — listed as equally-supported options with no
caveat.

**Source says:** `UniversalParamControl::SourceMode` (`src/ui/UniversalParamControl.h:61-68`)
does have all 8 enumerators, and the source-picker popup does let a user select "BPMSync"
(line ~514), "Clip Position" (line ~472), and "Timeline" (line ~478) via
`UniversalParamControl.cpp`. But the two places that actually *drive* a param from whatever
`sourceMode_` is currently selected — `EffectStackView::refresh()`
(`src/ui/EffectStackView.cpp:150-215`) and `ClipInspector::refresh()`
(`src/ui/ClipInspector.cpp:833-877`) — both branch only on:
```
mode == SourceMode::Signal || mode == SourceMode::Oscillator || mode == SourceMode::Envelope
   (→ pulls from SignalRegistry)
mode == SourceMode::Macro
   (→ pulls from MacroBank)
```
There is no `else if` arm for `SourceMode::BPMSync`, `SourceMode::ClipPosition`, or
`SourceMode::Timeline` in either file (confirmed by reading both `if`/`else if` chains in
full, and by `grep -rln "isConnected()" src/ui/ | xargs grep -l "getSourceMode"` → only these
two consumer files exist at all). Selecting one of these three modes flips the connect
triangle to "connected" (cyan) and stores the mode, but `found` never becomes `true` for
them, so the parameter value is never updated from any live source — it just freezes at
whatever it last was. This is despite `ClipPositionSignal` itself being fully implemented and
already live-wired as a *SignalRegistry* signal (§7a) — the disconnect is specifically that
`UniversalParamControl`'s own `ClipPosition`/`BPMSync`/`Timeline` source-mode branch (as
opposed to routing through SignalRegistry's own "Clip Position" named signal) was never
finished.

**Could not determine:** whether this is a regression (these 3 modes worked pre-9139dd4 and
broke) or always-been-a-stub — `src/ui` is outside this lane's path scope so I did not run
`git log` on `UniversalParamControl.cpp`/`EffectStackView.cpp`/`ClipInspector.cpp`. Reporting
current-state-only, which is squarely this lane's job (BPMSync/ClipPosition/Timeline were
named as suspects in the task brief).

---

## 7. MAJOR — "Structural scene triggering" callback is wired but its body only logs; it does not trigger a scene

**FEATURES.md says (§16 step 6):** *"Structural scene triggering: `Renderer::
onStructuralStateChanged_` on transitions,"* with Config listing
`composition.structuralSceneEnabled — structural scene triggers`.

**Source says:** `composition_.structuralSceneEnabled` **is** read (unlike the other four
dead surfaces above) — `src/MainComponent.cpp:628`, inside the handler passed to
`previewPanel_.getRenderer().setOnStructuralStateChanged(...)`. But the handler body,
gated by that flag, is:
```cpp
std::cerr << "[P23] Structural state: " << static_cast<int>(state)
          << (state == 0 ? " (normal)" : state == 1 ? " (buildup)"
          : state == 2 ? " (drop)" : " (breakdown)") << std::endl;
```
That's it — a debug print, nothing else. Contrast with the sibling callback immediately
above it in the same file, `setOnGenreChanged` (`src/MainComponent.cpp:601-622`), which is
gated by `composition_.autoPresetOnGenre` and actually calls `handleDeckSwitch(deckIdx)` to
switch decks. The structural-transition path has the same shape (flag-gated callback wired
by `MainComponent`, fired from `Renderer.cpp:279-281`) but was never given the corresponding
"switch scene/deck" body. §16's phrase "structural scene triggers" reads as if this does the
same kind of thing the genre auto-switch does; it currently only writes to stderr.

---

## 8. Recounted numbers (all re-derived from source, not carried forward)

| What | Doc says | Source says | Changed? | How counted |
|---|---|---|---|---|
| Mapping sources (`MappingSource` enum) | 58 (excl. `Count`) | 58 (excl. `Count`) | No | Read full enum `src/mapping/MappingTypes.h`; counted every non-comment enumerator line between `enum class MappingSource` and the following `enum class MappingCurve`, minus the `Count` sentinel (59 lines incl. `Count` → 58). |
| Curve types (`MappingCurve` enum) | 24 | 24 | No | Read full enum body; `Linear`(0) through `Hold`(23) = 24 entries before `Count`. |
| Signal Registry inventory | 32 (8 visible + 21 hidden + 2 mod + 1 clip-pos) | 32 (8+21+2+1) | No | Read `SignalRegistry::initDefaults()` in full (`src/signal/SignalRegistry.cpp`) and counted each `addAudio`/`addHiddenAudio`/direct-push call: 8 `addAudio`, 21 `addHiddenAudio`, 1 `OscillatorSignal` ("Mod 1"), 1 `EnvelopeSignal` ("Mod 2"), 1 `ClipPositionSignal`. |
| OSC patterns | 11, "LIVE... 11/11 callbacks wired" | 11 `if (address...)` branches in `OscHandler::oscMessageReceived` | No | Counted `if (address.startsWith(...` / `address ==` branches in `src/osc/OscHandler.cpp` (lines 58-175): clip, layer/opacity, layer/bypass, layer/solo, layer/mute, deck, master, bpm, snapshot, macro, effect = 11. `src/osc` has zero commits since `9139dd4` (`git log --oneline 9139dd4..HEAD -- src/osc` empty), so this section is unchanged and still accurate — **not** a drift finding. |
| Commits touching lane-4 paths since `9139dd4` | (not previously stated) | 19 | N/A | `git log --oneline 9139dd4..HEAD -- <9 lane paths>`, counted lines by hand (matches `wc -l`). |
| `src/core` file count | (not previously stated as a count) | 3 at `9139dd4` → 12 at HEAD | N/A | `git ls-tree -r --name-only <sha> -- src/core` at both revisions. |

---

## 9. Undocumented (real things in source, absent from FEATURES.md)

- **Undo v1's actual command inventory** — FEATURES.md doesn't name a single concrete
  `Command` subclass (`SetClipCmd`, `SwapClipsCmd`, `TriggerClipCmd`, deck/layer/effect
  commands, `CompositeCommand`, `EffectScope`, `UndoService`, the GL-fence
  (`withDeckDetached`) mechanism, or `Command::affectsLayerOrder()`). All of `src/core`'s 8
  new files are absent from the doc by name.
- **`MappingTickTimer` / `kMappingTickHz`** (`src/MainComponent.h:229-241`) — the actual
  120Hz message-thread driver for mapping evaluation is not named anywhere in §6; the doc
  still describes a render-frame-driven model that no longer exists.
- **GL-fence family for message-thread model mutations** (commit `8bd09ba`,
  `UndoService::withDeckDetached`) — a repo-wide invariant ("all message-thread model
  mutations fenced against unlocked GL reads") with no mention in FEATURES.md's gotchas for
  §8 or a dedicated entry.

## 10. Dead surfaces (summary — see §3-6 above for full trace)

- `composition_.bpmMultiplier` + `TopBar::onBpmMultiplierChanged` — written/fired, zero
  consumers. (§3)
- `composition_.quantizeMode` + `TopBar::onQuantizeChanged` — written/fired, zero consumers.
  (§4)
- `Composition::smartAutopilotEnabled` — written/serialized, zero consumers; and the
  *actually-wired* `Autopilot::smartRandomEnabled_` / `Renderer::setSmartRandomEnabled()` has
  zero callers, so real smart-random logic is unreachable from any UI/API/OSC path. (§5)
- `UniversalParamControl::SourceMode::{BPMSync, ClipPosition, Timeline}` — selectable in the
  UI, never handled by either of the two consumer sites that drive live param values. (§6)
- `composition_.structuralSceneEnabled` — this one is NOT a full dead surface (it is read and
  gates a real callback), but the gated behavior is log-only, not the deck/scene switch the
  doc's phrasing implies. Listed separately in §7 rather than here since it has a consumer,
  just not the promised one.

## 11. Confidence

**HIGH** on all seven numbered findings above — each has at least two independent grep
patterns and/or full-file reads behind it, plus (for §1, §2) direct commit-message/comment
corroboration that the change was deliberate and recent. **MEDIUM** on the §4 cross-reference
to §8's "Beat snap modes: Off, Beat, Bar, TwoBar, FourBar" — I could not locate that 5-value
enum in my lane's paths and did not chase it outside them.

## 12. Could not determine

- Whether §8's "Beat snap modes: Off, Beat, Bar, TwoBar, FourBar" describes a real, different
  feature (autopilot beat-snap, distinct from TopBar's 3-value `QuantizeMode`) that lives
  outside my lane's paths, or is itself stale text. Needs a search of `src/model/Autopilot.*`
  callers / `src/render` beyond what this lane covers, or a nudge to whichever lane owns
  autopilot/render UI text.
- Full closure on Undo v1 coverage — confirmed for clip/column/deck/layer/effect/trigger
  operations via 22+ call sites, but I did not enumerate every mutating UI action in
  `MainComponent.cpp` to certify there are zero remaining un-undoable mutations.
- Whether the BPMSync/ClipPosition/Timeline UniversalParamControl gap (§6) is a regression or
  a pre-existing stub, since the relevant files live in `src/ui`, outside this lane's git-log
  scope.
