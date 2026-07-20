# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). The 2026-07-19/20 session shipped **Undo v1 steps 1-3**
(3 local commits: 7c8d286 plumbing + GL fence VALIDATED, 7921572 SetClipCmd + all
single-cell sites + replace-undo media fix, daa9361 SwapClipsCmd). Tests: **130/130**
(`ctest --test-dir build`). Build: `cmake --build build --config Release -j`.
Lane ledger (step status, carry-forwards, known gaps, decisions):
`.harmony/undo-v1-ledger.md`. Behavioral gates: launch via
`open build/AudioDNA_artefacts/Release/Audio-DNA.app` (NEVER direct exec — hangs;
first open may stall in CoreAudio/TCC: sample → pkill -9 → re-open; gotchas.md),
:7070 binds ~7-12s.

START HERE — long task, begin at session start: **Undo v1 step 4 onward** per
`.harmony/specs/undo-v1-spec.md` §6 (steps 4-9: composites, layer ops, deck ops,
effect stacks, triggers, tests/e2e). It is the #1 Committed MUST. Read
`.harmony/undo-v1-ledger.md` FIRST — it carries: GL fence GREENLIT for steps 5-6
(empirically validated, 100/100 no-deadlock); step-4 composites must isEmpty()-guard
before perform (pushCommands helper already does); column-growth undo gap closes at
step 4; step-9 manual e2e checklist additions (video→video replace undo shows OLD
video; drag-move + far-column undo; Edit-menu dynamic Undo/Redo state). Established
patterns at HEAD: src/core/ClipCommands.h + MediaReconnect.h + UndoService,
makeSetClipCmd/pushCommands/refreshAfterUndoRedo in MainComponent.cpp — conform,
don't fork. Do NOT push to remote (standing rule). Quick Boris asks still pending:
Syphon.framework install + rebuild `-DAUDIODNA_BUILD_SYPHON=ON` (verify real
publish); 10s UI eyeball (menus / 3-tab Prefs / Sources rows / NEW: undo menu items
+ Cmd+Z after drop/drag).

## PRIMER

- HEAD at close: daa9361 on 11 unpushed local commits (8 triage-session + 3 Undo v1).
- Counts: 135 effects · 108 sources · 22/22 REST · 11/11 OSC (UDP 8000) · persistence
  COMPLETE · **130/130 tests** (114 baseline + 16 undo/media/swap).
- Undo v1 state: steps 1-3 DONE (each: independent Reviewer + Harmony behavioral gate,
  committed separately). Steps 4-9 queued — all M-sized except 3; deferred by this
  session at ~33% gauge per LONG-TASK DEFERRAL (a step + review cycle would cross the
  ~40% off-ramp).
- Verifier model: independent Reviewer on source + Harmony runs the behavioral gate
  (build + ctest re-run + `open` app + health probe). It EARNED ITS COST this session:
  review caught a silent wrong-video replace-undo bug the builder had misclassified as
  an accepted spec boundary (fixed pre-commit, MediaReconnect.h).
- Spec drift: undo-v1-spec lines are pre-Wave-0; symbols authoritative. Spec risk #7
  (kLayerClearClips) was FIXED by Wave 1-D — spec row #19 note is stale. Risk #9
  (GL fence) is RESOLVED — validated, see ledger.

## WHERE WE ARE IN THE BUILD
<!-- positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA VJ app — Wave-2 feature builds: Undo v1 in progress (3/9 steps shipped), Session Recorder + ISF specced and waiting.
SHIPPED (this session): real undo with Cmd+Z/menu for ALL clip-cell edits — every drop type (file/sequence/FX/MilkDrop single+playlist), replace content, lock, clear (multi-select = one undo), drag move/swap incl. column-count restore; dynamic "Undo <desc>"/"Redo <desc>" menu items; GL-fence threading question settled empirically; a silent replace-undo media bug caught by review and fixed pre-commit.
IN-FLIGHT: none — tree clean (hook-owned graphify churn only), all agents idle, every finished step committed.
NEXT: Undo v1 steps 4-9 (START HERE — composites, layer/deck ops, effect stacks, triggers, e2e); then Session Recorder; ISF anytime; Boris: Syphon install+verify, 10s UI eyeball (now incl. undo items), OSC port ratify, transport semantics ratify.
BLOCKERS: none.
YOU ARE HERE: undo exists and works for the whole clip grid — the remaining undo work is structural ops (layers/decks/columns/effects/triggers), then the manual e2e pass.

## LOOSE-ENDS LEDGER

1. Syphon REAL publish unverified — framework absent, flag OFF. Install → rebuild
   `-DAUDIODNA_BUILD_SYPHON=ON` → verify in client → decide flag-default-ON.
2. Boris UI eyeball pending — menus/Prefs/Sources PLUS new undo surface (Edit menu
   dynamic items; Cmd+Z after drop and after drag-move; native-menu probe stays
   TCC-blocked headlessly, two sessions running).
3. OSC port 8000 hardcoded (Boris may prefer 7000) — one-line change.
4. TopBar transport semantics unratified (active-deck all-layers; Stop = pause+rewind).
5. MilkDrop playlist POSITION runtime-only (deliberate; trivial to serialize if
   Boris overrides).
6. Model thread-safety design DEFERRED (one family): Clip::playing plain bool,
   dual mapping-engine write-order, lock-free model reads. Undo v1 does not worsen
   it in kind (spec §1); structure-command GL fence exists for steps 5-6.
7. ~~GL-fence no-deadlock inference~~ RESOLVED 2026-07-19: validated empirically
   (100/100 blocking fences under live render, max 15.6ms; evidence path in ledger).
8. ISF v1 acceptance target unmeasured until built (corpus sampling = ISF step 7).
9. Hidden-surface decisions open: TimingWindow tabs, EffectsRackPanel+MappingEditor,
   AudioReadoutPanel+SpectrumDisplay.
10. No-push rule active — 11 commits local-only.
11. SR guard WARN-only; 48k hardcode still real for non-48k devices.
12. graphify-out churn is hook-owned, deliberately uncommitted.
13. NEW: Undo v1 known gaps until later steps (tracked in undo-v1-ledger.md):
    column growth from drops not undone until step-4 column ops; unselected-undo
    clears clip inspector (safety>UX — surface to Boris if it feels wrong).

## META-LEARNINGS

(2026-07-19/20 additions; prior session's six remain valid — see git history of this file)
- Independent review pays at the SILENT-failure class: the replace-undo bug produced
  no error, no test failure, wrong visual only — builder self-report classed it
  "accepted boundary", reviewer traced the id-keyed player lookup and proved it wrong.
  Route every "accepted risk" claim in a builder report through the reviewer explicitly.
- Existence of an id-keyed resource ≠ correct content of that resource (gotcha'd) —
  reconnect guards must compare content identity, not presence, when ids are
  content-stable.
- Warm-builder R3 loop (build→fix→fix) capped at 3 rounds then fresh spawn worked
  cleanly: fresh step-3 builder conformed to committed patterns with zero style drift
  when pointed at the files (not prose descriptions) as the contract.
- Pure-helper extraction (needsVideoReopen) turned an untestable renderer-coupled
  decision into a 4-case headless truth-table test — extract the decision, not the
  side effect.

## CHANNEL HARVEST

- Lane: FOREIGN-REPO secondary — zero harmony2 writes this session; all capture
  project-local (undo-v1-ledger.md, 2 new gotchas, this handoff).
- New gotchas: transient first-`open` CoreAudio/TCC stall (mimics direct-exec hang;
  sample→pkill→re-open); id-keyed renderer media resources have no file-match check.
- No Boris messages this session (autonomous drain execution of the ratified backlog);
  no idea-class capture owed.
