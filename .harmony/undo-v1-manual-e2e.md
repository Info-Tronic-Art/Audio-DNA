# Undo v1 — Manual E2E Checklist (Boris-assisted)

Consolidated from the lane ledger (steps 3-8 additions + known-gap items).
Run against HEAD ≥ 4ee2dac. Harmony drives where possible; items marked 👁 need
Boris's eyes/hands. Check off in place.

## 0. Preconditions (must pass first)

- [x] 👁 **TCC mic prompt:** launch `open build/AudioDNA_artefacts/Release/Audio-DNA.app`
      → click **Allow** on the microphone dialog (re-fires after EVERY rebuild —
      ad-hoc signing; see gotchas 2026-07-25). NOT a coreaudiod issue — never
      killall/reboot for this. *(2026-07-28 sitting: health up ~6s)*
- [x] `/api/health` returns ok (binds ~12s post-launch). *(ok/ready/117fps/135 effects)*

## 1. Quick smoke (10s UI eyeball additions)

- [ ] 👁 Edit menu shows dynamic "Undo <desc>" / "Redo <desc>" with correct
      greyed/enabled state; updates as history changes.
- [x] Drop a clip → quick Cmd+Z → cell restores. *(PASS 2026-07-28)*
- [x] 👁 Drag-move a clip (incl. to a FAR empty column) → Cmd+Z → both cells AND
      column count restore (step-3 gesture path). *(PASS 2026-07-28)*
- [x] Menus / 3-tab Prefs / new Sources rows render sane (general eyeball).
      *(PASS 2026-07-28 — 3 tabs confirmed, sparse but real. NOTE cosmetic:
      menu bar shows "Audio-DNA" twice — native app menu + custom menu of the
      same name; polish item for the design connect phase.)*

## 2. Drops & cells (steps 2-4)

- [x] Multi-video drop near RIGHT EDGE → Cmd+Z restores cells AND column count. *(PASS 2026-07-28)*
- [x] Multi-SOURCE drop → Cmd+Z restores (was NOT undoable at all pre-step-4).
      *(PASS 2026-07-28)*
- [x] Multi-FX drop onto empty far cells → Cmd+Z restores columns. *(PASS 2026-07-28)*
- [ ] MilkDrop single + playlist drop → Cmd+Z (playlist survives — value-copy).
- [ ] Menu Column → New / Column → Remove → Cmd+Z. **FAIL 2026-07-28: Column →
      New CRASHED the app** (SIGSEGV, GL thread, applyClipEffects use-after-realloc
      signature; .ips 2026-07-28-182825; crash-scout diagnosing). **HOLD all
      column-op tests** (New/Insert/Duplicate/Remove) until fixed.
- [ ] Clear Layer Clips + Clear Deck Clips → Cmd+Z restores clips AND layer
      active/crossfade state.
- [ ] **Video→video replace → undo → OLD video visibly plays** (the reclassified
      silent-wrong-video case — worst class, must check).
- [ ] Cosmetic (accepted): single-layer deck's deck-clear label reads "Clear
      Layer Clips" (single-child collapse).

## 3. Layers (step 5)

- [ ] 👁 Layer add / remove / move WHILE RENDERING → no crash/torn frame (GL
      fence live). Each → Cmd+Z restores count, order, full state incl. clips.
- [ ] Bypass / solo / fold toggle → Cmd+Z flips back.
- [ ] X-button layer-clear → Cmd+Z restores active-clip runtime (played clip's
      `playing` flag NOT restored — accepted risk #5).
- [ ] 👁 #16-18 are command-owned: UI result appears only after perform() —
      confirm no visible double-apply or lag.

## 4. Decks (step 6)

- [ ] Deck add / remove / tab-switch → Cmd+Z / Cmd+Shift+Z restores deck set,
      active deck, grid content.
- [ ] REST/OSC/MIDI/genre-auto deck switches must NOT appear in undo history
      (only tab clicks do).
- [ ] 👁 Boris decision: new deck arrives with ZERO layers (raw kDeckNew, HEAD
      behavior) — intended?
- [ ] KNOWN COSMETIC (pre-existing): deck-tab HIGHLIGHT may lag until next
      click — grid content is authoritative. (Real fix = ratify the
      refreshAfterUndoRedo follow-up.)

## 5. Effect stacks (step 7)

- [ ] FX drop on clip / layer / global stack → Cmd+Z restores (each scope).
- [ ] Delete FX → Cmd+Z. Bypass toggle → Cmd+Z flips back.
- [ ] Multi-select FX drop = ONE undo entry.
- [ ] Undo a clip-effect edit while a DIFFERENT cell is selected → model
      restores; inspector shows the selected cell.
- [ ] KNOWN COSMETIC (pre-existing): expanded effect rows COLLAPSE after ANY
      undo/redo — fix is the tracked refreshAfterUndoRedo follow-up (Boris to
      ratify).

## 6. Triggers (step 8)

- [ ] Trigger a cell → Cmd+Z restores previous active clip + crossfade state.
- [ ] MASH several cells on ONE layer → a single Cmd+Z undoes the whole run
      (merge).
- [ ] Trigger cells on TWO layers → two undo entries (no cross-layer merge).
- [ ] Column trigger → Cmd+Z restores ALL non-ignoring layers at once; ignoring
      layer stays put.
- [ ] Retrigger the already-active cell → history does NOT grow (Edit menu).
- [ ] REST/OSC/MIDI trigger → DOES appear in undo history (spec row 8 —
      remote user actions are undoable).
- [ ] Autopilot triggers → NEVER appear in history.
- [ ] Undo a trigger while autopilot runs → best-effort (may be immediately
      overridden — accepted risk #2).
- [ ] KNOWN LIMITATION (documented, risk #5): first-ever trigger → undo →
      re-trigger = clip goes active but skips auto-play.
- [ ] Handler-level pendingTriggerColumn edge: queue a beat-snap trigger on
      another column, retrigger the active cell → the queued-trigger clear IS
      undoable (pushes one entry).

## 7. Cross-cutting

- [ ] Undo while a video plays → no crash, sane restore.
- [ ] Composition → New → history clears (menu greys out). *(2026-07-28: "load"
      half is UNTESTABLE at HEAD — Comp/Decks browser load/save-composition
      callbacks are UNWIRED no-ops (reviewer-traced); only New exercises the
      history-clear. See findings.)*
- [ ] 👁 Boris UX check: undo with NO cell selected clears the clip inspector
      (safety over UX) — acceptable?

## Deferred non-lane (same Boris session, after checklist)

- [ ] Syphon.framework install + rebuild `-DAUDIODNA_BUILD_SYPHON=ON` → verify
      real publish.

## Sitting findings (2026-07-28)

- **CRASH (blocker) — DIAGNOSED 2026-07-28, scout-confirmed UAF, PRE-EXISTING:**
  Column → New: message thread `deck->addColumn()` (MainComponent.cpp:3712 →
  Layer.h:300-303 clips.resize) reallocates while GL thread holds an interior
  Clip* (CompositorEngine.cpp:694 getActiveClip → :734 applyClipEffects; crash
  reg = clip.effects begin/end overwritten with geometry DOUBLES from
  rebuildGrid). renderOpenGL runs with NO MessageManager lock (Renderer.cpp:28
  setComponentPaintingEnabled(false) — verified pre-existing at fb271e3).
  Pre-existing verdict: handler + resize byte-identical pre-lane; lane adds only
  a no-op ensureColumns after the realloc. Fence (makeDeckFence/withDeckDetached)
  WOULD cover it — exclusion was pure spec scope choice, now falsified.
  Fix candidate (scout rec (a)): fence live column mutations + DeckFenceHook into
  SetColumnCountCmd/RemoveColumnCmd so undo/redo replay is fenced too (4 ctor
  sites; headless tests use pass-through hook per AddLayerCmd pattern). ~3-16ms
  per menu action. Boris nod pending. Evidence: .ips 2026-07-28-182825, binary
  UUID matched, disassembly-verified.
- **EXPANDED HOLD LIST (same root cause, scout-enumerated):** Column
  New/Insert/Remove/Duplicate · Deck → Clear Clips (:3557) · Layer → Clear
  (:3621) — MORE deterministic than the column case · Clip → Clear (:3759) ·
  undo/redo replay of ALL of the above (DeckCommands.h:55,97,108-111;
  ClipCommands.h:56,160). NOT exposed (already fenced): layer add/remove/move,
  deck add/remove. NOTE: already-PASSED drop tests (multi-video edge, multi-FX
  far, drag-move far, multi-SOURCE) ride the SAME exposed column-growth paths —
  their PASSes stand but were race-lucky; RE-RUN after the fence fix.
- **NEW BUG (not undo's fault, queued fix candidate):** external Finder drop of a
  MIXED batch (videos + images) silently discards the images — ClipCell.cpp
  filesDropped video-first early-return branches; internal drag path handles the
  same batch correctly. Scouted + logged in notebook.md 2026-07-28. Boris nod
  needed to fix.
- **CRASH #2 (separate bug, message thread, NOT the UAF family):** clicking the
  arrow button right of the signal level → layout cascade (MainComponent::resized
  → BrowserPanel::resized → MilkDropBrowser::resized → calculateContentHeight →
  **getCuratedPresets() +104, SIGSEGV at 0x61** — near-null deref). Likely the
  empty state: preset dir never configured / no engine (libprojectM absent) →
  null member dereferenced during layout. Deterministic-looking. .ips
  2026-07-28-190701. Fix candidate: null-guard MilkDropBrowser empty state —
  natural bundle with Boris's "presets loaded by default" ask (default preset
  dir → bundled resources/projectm_presets). Queued AFTER fence-f1 lane closes
  (serial integration). Boris nod pending.
- **Boris UX/feature requests (2026-07-28, backlog candidates):**
  (1) Cmd+X should cut/clear the selected clip cell (Clip menu lists Cut — verify
  wiring + shortcut); (2) MilkDrop presets should be loaded by default (default
  preset dir → bundled resources/projectm_presets — browser currently empty until
  dir is set manually).
- **Cosmetic:** menu bar shows "Audio-DNA" twice (native app menu + custom menu
  with same title) — polish item for the design connect phase.
- **Product gap (pre-existing, reviewer-traced 2026-07-28):** Comp/Decks browser
  "load composition" / "load deck" / "Save Composition" are UNWIRED — callbacks
  never assigned, buttons silently do nothing (Save Deck + right-click delete DO
  work). Future-fence requirement recorded in source + notebook: wiring the load
  MUST use withDeckDetached + undoManager_.clear() (kCompNew precedent). Boris
  decision: wire it (small lane) or leave for the recorder/persistence wave.

## Accepted/known items — do NOT file as undo bugs

Played-clip `playing` not restored (risk #5) · first-trigger auto-play skip
(risk #5 family) · deck-tab highlight lag (pre-existing) · expanded-row
collapse (pre-existing; follow-up tracked) · zero-layer Deck-New (HEAD
behavior, pending Boris intent) · deck-clear label collapse (cosmetic).
