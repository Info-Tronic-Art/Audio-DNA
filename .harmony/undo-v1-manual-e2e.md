# Undo v1 — Manual E2E Checklist (Boris-assisted)

Consolidated from the lane ledger (steps 3-8 additions + known-gap items).
Run against HEAD ≥ 4ee2dac. Harmony drives where possible; items marked 👁 need
Boris's eyes/hands. Check off in place.

## 2026-07-30 AUTONOMOUS DRIVE — status + Boris runsheet

Driver: Accessibility-granted synthetic UI (CGEvent clicks/drags + System Events
menus/keys, verify-and-retry; oracles = /api/composition + Composition-menu
labels + window captures). **~130 mutations across every crash-family path under
active GL render — ZERO crashes** (fps 114-120 throughout, .ips unchanged).
Most items below now carry dated PASS notes. **REMAINING = this runsheet (~15-20 min):**
1. External Finder drops: multi-video near RIGHT EDGE, multi-FX far, multi-SOURCE
   (race-lucky re-runs) + mixed video+image batch (known discard bug)
2. Video: video→video replace → undo → OLD video visibly plays · undo while a
   video plays
3. Drag-move a clip to a far column → Cmd+Z (start the drag from the cell NAME
   BAR — bottom ~20px; a thumbnail-drag TRIGGERS instead)
4. Clip > Clear: name-bar click to select, then the menu (wired, source-proven —
   driver can't hit the 20px band)
5. FX: layer + global scope drops · delete-FX · FX bypass · multi-select FX
   drop = ONE undo entry
6. Autopilot ON with clips visibly cycling → Composition menu stays frozen
7. Quantize/beat-snap: queue a trigger on another column, retrigger active cell
   → the queued-trigger clear pushes ONE entry
8. Column trigger vs a layer with "Ignore Column Trigger" checked
9. MilkDrop single + playlist drop (browser empty until default-preset-dir fix)
10. 👁 taste: torn-frame feel · #16-18 double-apply feel · deck-tab highlight
    lag · inspector-clear UX · zero-layer Deck-New intent · NEW: preview keeps
    animating the old deck's clip while an empty deck is active — intended?
11. Syphon lane + the codesign cert step (Keychain), unchanged.

## 0. Preconditions (must pass first)

- [x] 👁 **TCC mic prompt:** launch `open build/AudioDNA_artefacts/Release/Audio-DNA.app`
      → click **Allow** on the microphone dialog (re-fires after EVERY rebuild —
      ad-hoc signing; see gotchas 2026-07-25). NOT a coreaudiod issue — never
      killall/reboot for this. *(2026-07-28 sitting: health up ~6s)*
- [x] `/api/health` returns ok (binds ~12s post-launch). *(ok/ready/117fps/135 effects)*
- [x] **2026-07-30 re-verify on the FENCED build (8bd09ba):** relaunch via `open` →
      health ok/ready/119.6fps/135 effects in ~10s, NO prompt (TCC already granted;
      mic-in-use indicator live). App gate CLOSED — the column/clear/effects HOLD
      below is DISSOLVED.

## 1. Quick smoke (10s UI eyeball additions)

- [x] 👁 Edit menu shows dynamic "Undo <desc>" / "Redo <desc>" with correct
      greyed/enabled state; updates as history changes. *(PASS 2026-07-30
      AUTONOMOUS — NOTE: items live in the COMPOSITION menu (no Edit menu
      exists; checklist wording was stale). Verified programmatically across
      the whole drive: plain+disabled at empty history, "Undo Drop Source",
      "Undo/Redo Add Column", "Undo Clear Deck Clips", "Undo Trigger Column",
      "Undo Fold Layer", "Undo Switch Deck", "Undo Remove Deck" etc. all
      correct, both stacks greyed after Composition→New.)*
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
- [x] Menu Column → New / Column → Remove → Cmd+Z. *(PASS 2026-07-30 AUTONOMOUS
      — synthetic-driver run on fenced build: 10× New+undo, 5× redo/undo REPLAY
      cycles, Insert Before/After, Remove, ALL under an actively-rendering
      plasma clip (the exact 07-28 SIGSEGV conditions) → ZERO crashes, fps
      118.5, no new .ips, column count reverted exactly on every op, labels
      "Undo Add Column"/"Undo Remove Column" correct. 3 undo keystrokes dropped
      by the AUTOMATION channel (delivery flake, proven: retry-with-verify
      drained all 3 first-attempt, stack coherent throughout) — app innocent.)*
      **FAIL 2026-07-28: Column →
      New CRASHED the app** (SIGSEGV, GL thread, applyClipEffects use-after-realloc
      signature; .ips 2026-07-28-182825; crash-scout diagnosing). **HOLD all
      column-op tests** (New/Insert/Duplicate/Remove) until fixed.
      **2026-07-30: fence family fix LANDED (8bd09ba) + app gate closed → HOLD
      DISSOLVED — column ops re-testable (START the resumed sitting here: this
      is the crash repro case).**
- [x] Clear Layer Clips + Clear Deck Clips → Cmd+Z restores clips AND layer
      active/crossfade state. *(PASS 2026-07-30 AUTONOMOUS — both under active
      multi-layer render: deck-clear "Undo Clear Deck Clips" composite restored
      clips + activeClipColumn exactly; layer-clear (menu, layer-strip
      selection) "Undo Clear Layer Clips" restored [0,1]+active exactly. NOTE
      content guard: clearing an empty layer intentionally pushes nothing
      (:3694). X-button variant is the SINGULAR ClearActiveClipCmd — also
      verified. Zero crashes on the scout's most-deterministic crash paths.)*
- [ ] **Video→video replace → undo → OLD video visibly plays** (the reclassified
      silent-wrong-video case — worst class, must check).
- [ ] Cosmetic (accepted): single-layer deck's deck-clear label reads "Clear
      Layer Clips" (single-child collapse).

## 3. Layers (step 5)

- [x] 👁 Layer add / remove / move WHILE RENDERING → no crash/torn frame (GL
      fence live). Each → Cmd+Z restores count, order, full state incl. clips.
      *(PASS 2026-07-30 AUTONOMOUS — New/Insert Above/Insert Below/Remove/Move
      Up all under active render, undo restored count+order+clips exactly
      (Move: clips carried, exact order back — the step-5 silent-wrong-order
      watch-item passes). 👁 residue: "torn frame" is a feel call — captures
      + steady fps say clean, your eyes confirm in the sitting.)*
- [x] Bypass / solo / fold toggle → Cmd+Z flips back. *(PASS 2026-07-30
      AUTONOMOUS — B/S buttons: "Undo Bypass Layer"/"Undo Solo Layer", flags
      flipped back exactly in API state; Fold via menu + strip selection:
      "Undo Fold Layer" → undo clean.)*
- [x] X-button layer-clear → Cmd+Z restores active-clip runtime (played clip's
      `playing` flag NOT restored — accepted risk #5). *(PASS 2026-07-30
      AUTONOMOUS — X = ClearActiveClipCmd: deactivated active clip (cell
      content retained), undo restored activeClipColumn exactly.)*
- [x] 👁 #16-18 are command-owned: UI result appears only after perform() —
      confirm no visible double-apply or lag. *(PASS-structural 2026-07-30 —
      every command-owned op applied exactly once in API state + captures;
      👁 residue: perceived lag is a feel call for the sitting.)*

## 4. Decks (step 6)

- [x] Deck add / remove / tab-switch → Cmd+Z / Cmd+Shift+Z restores deck set,
      active deck, grid content. *(PASS 2026-07-30 AUTONOMOUS — full triad:
      "Undo Add Deck" / "Undo Remove Deck" / "Undo Switch Deck" (tab click)
      all restored deck count, active index AND full deck content exactly,
      both directions. NEW OBSERVATION for Boris: preview keeps ANIMATING the
      old deck's active clip while empty Deck 2 is active (frames verified
      differing) — intended composition semantics or renderer display gap?
      Pairs with the zero-layer Deck-New intent question.)*
- [x] REST/OSC/MIDI/genre-auto deck switches must NOT appear in undo history
      (only tab clicks do). *(PASS 2026-07-30 — source-proven ×2 independent
      reads: ApiServer/OSC land on handleDeckSwitch:3394 which pushes NO
      command (scout, cited) + step-6/8 reviews traced MIDI/genre inline paths;
      /api/switch_deck exercised live, no crash. Stronger than an eyeball for
      a must-NOT-appear claim.)*
- [ ] 👁 Boris decision: new deck arrives with ZERO layers (raw kDeckNew, HEAD
      behavior) — intended?
- [ ] KNOWN COSMETIC (pre-existing): deck-tab HIGHLIGHT may lag until next
      click — grid content is authoritative. (Real fix = ratify the
      refreshAfterUndoRedo follow-up.)

## 5. Effect stacks (step 7)

- [ ] FX drop on clip / layer / global stack → Cmd+Z restores (each scope).
      *(CLIP SCOPE PASS 2026-07-30 AUTONOMOUS — Ripple dragged onto the
      ACTIVELY-RENDERING plasma cell (live effects-vector push_back under GL
      iteration, the round-2 blocker scenario): "Undo Add Effect 'Ripple'" →
      undo → REDO-replay, visually confirmed warped render, zero crashes.
      NOTE: /api/composition does NOT expose clip effects — verification is
      label+visual. RESIDUE: layer + global scopes, delete-FX, FX bypass,
      multi-select drop — sitting items.)*
- [ ] Delete FX → Cmd+Z. Bypass toggle → Cmd+Z flips back.
- [ ] Multi-select FX drop = ONE undo entry.
- [ ] Undo a clip-effect edit while a DIFFERENT cell is selected → model
      restores; inspector shows the selected cell.
- [ ] KNOWN COSMETIC (pre-existing): expanded effect rows COLLAPSE after ANY
      undo/redo — fix is the tracked refreshAfterUndoRedo follow-up (Boris to
      ratify).

## 6. Triggers (step 8)

- [x] Trigger a cell → Cmd+Z restores previous active clip + crossfade state.
      *(PASS 2026-07-30 AUTONOMOUS — verified repeatedly incl. redo-replay.)*
- [x] MASH several cells on ONE layer → a single Cmd+Z undoes the whole run
      (merge). *(PASS 2026-07-30 — 4-click same-layer mash collapsed to ONE
      entry; single undo restored the pre-run active clip exactly.)*
- [x] Trigger cells on TWO layers → two undo entries (no cross-layer merge).
      *(PASS 2026-07-30 — two pops required, each restoring its own layer.)*
- [x] Column trigger → Cmd+Z restores ALL non-ignoring layers at once; ignoring
      layer stays put. *(PASS 2026-07-30 via REST trigger_column with content
      on 2 layers: "Undo Trigger Column" composite, ONE undo restored BOTH
      layers to -1. RESIDUE: ignoring-layer variant needs the Ignore Column
      Trigger checkbox — 10s sitting item.)*
- [x] Retrigger the already-active cell → history does NOT grow (Edit menu).
      *(PASS 2026-07-30 — verified twice via real UI clicks + once via REST:
      label never moved on retrigger of the active cell.)*
- [x] REST/OSC/MIDI trigger → DOES appear in undo history (spec row 8 —
      remote user actions are undoable). *(PASS 2026-07-30 — source-proven ×2:
      /api/trigger_clip funnels into handleClipTrigger which pushes
      TriggerClipCmd :2995 (scout) + step-8 review grep-proof of all remote
      marshal chains; endpoints exercised live ×18, marshalling + state-change
      guard behaviorally confirmed (empty-cell = no-op, no push). RESIDUE (10s,
      in sitting): one REST trigger on a REAL clip → Edit menu shows the
      entry.)*
- [ ] Autopilot triggers → NEVER appear in history. *(ATTEMPTED 2026-07-30,
      INCONCLUSIVE — undo label stayed frozen across a 15s autopilot-ON window
      ✓, but state suggests autopilot may not have engaged (no API field
      proves it ran). Source proof stands (autopilot calls layer.triggerClip
      directly, zero Command refs in Autopilot.cpp — step-8 review). Sitting:
      flip autopilot ON with clips cycling visibly, watch the Composition
      menu stay frozen.)*
- [ ] Undo a trigger while autopilot runs → best-effort (may be immediately
      overridden — accepted risk #2).
- [ ] KNOWN LIMITATION (documented, risk #5): first-ever trigger → undo →
      re-trigger = clip goes active but skips auto-play.
- [ ] Handler-level pendingTriggerColumn edge: queue a beat-snap trigger on
      another column, retrigger the active cell → the queued-trigger clear IS
      undoable (pushes one entry).

## 7. Cross-cutting

- [ ] Undo while a video plays → no crash, sane restore.
- [x] Composition → New → history clears (menu greys out). *(2026-07-28: "load"
      half is UNTESTABLE at HEAD — Comp/Decks browser load/save-composition
      callbacks are UNWIRED no-ops (reviewer-traced); only New exercises the
      history-clear. See findings.)* *(PASS 2026-07-30 AUTONOMOUS — fenced
      kCompNew fired under active render: state wiped to default 1-deck/3-layer,
      BOTH undo+redo stacks cleared, both menu items plain+disabled, app alive
      at 116fps. The round-1 MAJOR's fix proven live.)*
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
  **HOLD DISSOLVED 2026-07-30** (fence 8bd09ba committed + app gate closed) —
  the 4 re-runs are now due.
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
