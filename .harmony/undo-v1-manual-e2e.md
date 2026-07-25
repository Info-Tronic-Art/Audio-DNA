# Undo v1 — Manual E2E Checklist (Boris-assisted)

Consolidated from the lane ledger (steps 3-8 additions + known-gap items).
Run against HEAD ≥ 4ee2dac. Harmony drives where possible; items marked 👁 need
Boris's eyes/hands. Check off in place.

## 0. Preconditions (must pass first)

- [ ] 👁 **TCC mic prompt:** launch `open build/AudioDNA_artefacts/Release/Audio-DNA.app`
      → click **Allow** on the microphone dialog (re-fires after EVERY rebuild —
      ad-hoc signing; see gotchas 2026-07-25). NOT a coreaudiod issue — never
      killall/reboot for this.
- [ ] `/api/health` returns ok (binds ~12s post-launch).

## 1. Quick smoke (10s UI eyeball additions)

- [ ] 👁 Edit menu shows dynamic "Undo <desc>" / "Redo <desc>" with correct
      greyed/enabled state; updates as history changes.
- [ ] Drop a clip → quick Cmd+Z → cell restores.
- [ ] 👁 Drag-move a clip (incl. to a FAR empty column) → Cmd+Z → both cells AND
      column count restore (step-3 gesture path).
- [ ] Menus / 3-tab Prefs / new Sources rows render sane (general eyeball).

## 2. Drops & cells (steps 2-4)

- [ ] Multi-video drop near RIGHT EDGE → Cmd+Z restores cells AND column count.
- [ ] Multi-SOURCE drop → Cmd+Z restores (was NOT undoable at all pre-step-4).
- [ ] Multi-FX drop onto empty far cells → Cmd+Z restores columns.
- [ ] MilkDrop single + playlist drop → Cmd+Z (playlist survives — value-copy).
- [ ] Menu Add Column / Remove Column → Cmd+Z.
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
- [ ] Composition load / New → history clears (menu greys out).
- [ ] 👁 Boris UX check: undo with NO cell selected clears the clip inspector
      (safety over UX) — acceptable?

## Deferred non-lane (same Boris session, after checklist)

- [ ] Syphon.framework install + rebuild `-DAUDIODNA_BUILD_SYPHON=ON` → verify
      real publish.

## Accepted/known items — do NOT file as undo bugs

Played-clip `playing` not restored (risk #5) · first-trigger auto-play skip
(risk #5 family) · deck-tab highlight lag (pre-existing) · expanded-row
collapse (pre-existing; follow-up tracked) · zero-layer Deck-New (HEAD
behavior, pending Boris intent) · deck-clear label collapse (cosmetic).
