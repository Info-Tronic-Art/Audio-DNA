# Undo v1 — Build Lane Ledger

Session: 2026-07-19 (secondary, slim boot). Spec: `.harmony/specs/undo-v1-spec.md`
(Boris-ratified Wave-2 MUST #1). Build order §6 steps 1-9. Baseline at lane start:
HEAD b4ee380, source tree clean, tests 114/114 (handoff claim — re-verify at first gate).

## Standing constraints
- NO push to remote (standing rule). Commit per step after gates pass — no batching.
- App launch ONLY via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`; :7070 binds ~12s.
- Verification split: Builder implements → independent Reviewer reads source → Harmony
  runs behavioral gate (build + ctest + app-level checks). Builder never claims verified.
- Spec lines are pre-Wave-0; SYMBOLS authoritative. Known drift: Wave 0 G3 touched menu
  code; Wave 1-D fixed the Clear-Clips bug (spec risk #7 / row #19 note stale); Wave 1-C
  landed toVar fidelity (no dependency either way — v1 uses value copies).

## Step status
| Step | Work | Status |
|---|---|---|
| 1 | Plumbing + GL-fence empirical validation (risk #9) FIRST | DONE — Reviewer CLEAN + behavioral gate PASS; committed 7c8d286 |
| 2 | SetClipCmd + single-cell sites (#4-6,8,10-12) | DONE — review FINDINGS→fix→CLEAN (open()-failure edge verified safe); gate PASS ×2; committed 7921572 |
| 3 | SwapClipsCmd (#3) | DONE — review CLEAN (1 cosmetic nit fixed); gate PASS (ctest 130/130, [swap] 21/21, app health); committed daa9361 |
| 4 | Composites (#7,9,4-multi,25-26,19,23) | DONE — review CLEAN + targeted re-review (1 NIT jassert fixed); gate PASS ×2 (ctest 138/138, app health); committed 6d2def4 |
| 5 | Layer ops (#13-18,20) + GL fence | DONE — review CLEAN (10/10 areas; 1 trivial NIT → step 9); code gates PASS (ctest 145/145 ×2); app gate BLOCKED-ENV (change-independent); committed 7f87094 |
| 6 | Deck ops (#21,22,24 — #23 done in step 4) | DONE — review CLEAN (10/10) + 2 targeted re-reviews; code gates PASS (ctest 153/153 ×3); app gate BLOCKED-ENV; committed 316a2bf |
| 7 | Effect stacks (#27-29) performEdit + scope descriptor | DONE — review FINDINGS (1 MAJOR: false comment guarantee) → remedy (b) fix round → CONFIRMED-CLEAN + UNBLOCK; code gates PASS (ctest 158/158 ×2); app gate BLOCKED-ENV; committed d90e953 |
| 8 | Trigger cmds (#1-2) + merge | queued — NEXT SESSION START HERE (deferred at ~38% gauge per LONG-TASK DEFERRAL) |
| 9 | Tests + manual e2e checklist | queued (folds in: RemoveLayerCmd stale-coord test, SyncScope::DeckStructure dead code, EffectCommands.h:98 stale inline comment, dead 2-arg reinspect path note) |

## Decision log
- 2026-07-19: GL-fence validation outcome gates `withDeckDetached` usage in steps 5-6.
  If fence deadlocks or is unreliable → fallback per spec: plain unsynchronized writes
  (status-quo risk profile) for structure commands + escalate as spec deviation.
- 2026-07-19: **GL-fence VALIDATED — PASS.** 100/100 blocking fences from message thread
  under live ~110fps rendering; roundtrip max 15.6ms / mean 2.8ms; no deadlock; app
  responsive after. Spec risk #9 resolved → `withDeckDetached` GREENLIT for steps 5-6.
  Evidence: /tmp/undo-fence-validation.log (+ -final copy).
- 2026-07-19: Step-1 behavioral gate (Harmony, independent): diff scope == report (9 mod
  + 3 new, src/tests/CMake only); harness residue grep CLEAN; ctest re-run 118/118
  (114 baseline + 4 [undo]); app launch via `open` → /api/health ready ~10s. PASS.
- 2026-07-19: Builder deviations accepted pending review: (1) guarded jassert (headless
  Catch2 aborts on bare form — identical enforcement in app); (2) +juce_events on test
  target; (3) syncAfterModelChange as two overloads (nested-struct default-arg compile
  error); (4) withDeckDetached restores activeDeck_ by RE-RESOLVE not saved pointer
  (survives decks-vector reallocation — strictly safer than spec sketch).
- 2026-07-19: New gotcha captured: transient first-`open` CoreAudio/TCC stall
  (gotchas.md 2026-07-19 entry).
- 2026-07-19: Step-1 review CLEAN (all 8 focus areas; deviations corroborated).
  Committed 7c8d286 (local, no push). Reviewer carry-forwards folded into lane:
  (a) step-4 composites MUST check CompositeCommand::isEmpty() before perform;
  (b) UndoService resolve* unit tests — added to step-2 packet.
- 2026-07-19: Step 2 dispatched to warm builder undo-s1 (R3 round 2; respawn
  fresh if round 3 needed or context bloats). Packet adds: operator== field-
  completeness statement required (post-Wave-1-C Clip fields), risk-#4 reconnect
  guard in SetClipCmd apply path, single-cell #4 with optional early composite.
- 2026-07-20: Step-2 review cycle: FINDINGS (1 real bug — video replace-undo
  silent no-op) → fix (MediaReconnect file-compare) → targeted re-review CLEAN
  (open()-failure edge safe: failed reopen dies on discarded temporary; locking
  consistent; immunity claims for images/sequences source-verified). Committed
  7921572. Carry-forwards: (a) VideoPlayer::getFile() unlocked — message-thread
  only, comment rider in step-3 packet; (b) SwapClipsCmd numColumns snapshot
  closes the column-growth gap for the MOVE op (drop/FX column growth still
  waits for step 4).
- 2026-07-20: Step 3 → FRESH builder undo-s3 (undo-s1 retired: 3 R3 rounds,
  context carrying 2 packets + fix loop). Budget watch: ~25% at dispatch;
  drain off-ramp ~40% — reassess after step 3 lands; steps 4+ likely defer to
  next session per LONG-TASK DEFERRAL.

- 2026-07-22: Session resume (secondary, slim). Step 4 packet folds in carry-forwards:
  (a) CompositeCommand::isEmpty() guard before perform (use pushCommands helper);
  (b) column-growth undo gap closes here — drop/FX-driven numColumns growth must be
  undone (SwapClipsCmd already covers the MOVE op); (c) spec row #19 note is STALE —
  Wave 1-D fixed the Clear-Clips bug; builder verifies actual HEAD behavior and wraps
  it faithfully; (d) no GL fence needed in step 4 (fence GREENLIT but reserved for
  steps 5-6 structure ops; column/cell writes keep status-quo risk profile per spec).
  Builder runs baseline ctest FIRST (expect 130/130) — if red at baseline, BLOCKED.
- 2026-07-22: Step-4 builder (undo-s4) DONE_WITH_CONCERNS (concerns = reviewer-focus
  notes, not failures). Receiver disk-verify PASS (diff scope == report; 138/138 in
  builder log). Notable: SetColumnCountCmd unifies #25 + drop-growth restore (one
  before/after-count cmd, conform-don't-fork); #23 = composite of ClearLayerClipsCmd;
  #19 confirmed per-layer at HEAD (Wave 1-D fix; spec note stale); onSourceDropped (#9)
  was previously pushing NO command at all — now wrapped. Builder's #1 flagged risk:
  RemoveColumnCmd.execute() idempotency depends on last-column-only removal (v1-safe,
  future middle-column removal needs a different execute) — handed to reviewer.
- 2026-07-22: Step-4 behavioral gate (Harmony, independent): build exit 0 / 0 warnings;
  ctest re-run 138/138 (own run, /tmp/undo-s4-gate-ctest.log); residue grep CLEAN;
  app launch via `open` → first-open CoreAudio/TCC stall reproduced (sample showed
  CoreAudioInternal::start hang — gotcha confirmed twice now) → pkill -9 → re-open →
  /api/health ready 19s, ok:true, 135 effects. GATE PASS. Commit pending review verdict.
- 2026-07-22: Step-4 review CLEAN (all 9 focus areas corroborated; diff scope
  confirmed). 1 NIT → fix round with warm builder: RemoveColumnCmd last-column-only
  invariant enforced only by caller discipline + comment — add guarded jassert in
  ctor (reviewer's strongest residual risk: future middle-column call site would
  silently double-erase). Reviewer also established: composite ordering NOT strictly
  load-bearing (SetClipCmd::apply self-heals via ensureColumns) but conforms to
  SwapClipsCmd convention — keep; DeckView uses deck->numColumns (not clips.size())
  for grid width, so grow-only undo semantics render correctly.
- 2026-07-22: Step-4 fix round DONE (guarded jassert, UNDO_ASSERT_MESSAGE_THREAD form;
  reviewer verified it can never arm in the Catch2 binary — no MessageManager is ever
  created there). Targeted re-review CONFIRMED-CLEAN; gate re-run PASS (build 0,
  ctest 138/138 own run). Committed 6d2def4 (local, no push). Step 4 CLOSED.
- 2026-07-22: Step 5 (layer ops #13-18,20 + GL fence) dispatched to warm builder
  undo-s4 (1 packet + 1 tiny fix in context — same posture as undo-s1 at step-2
  dispatch; respawn fresh if round 3 needed or context bloats). Gauge ~11% at
  dispatch; off-ramp ~40% — step 6 (S-M) likely also fits this session.
- 2026-07-22: Step-5 builder DONE_WITH_CONCERNS; receiver disk-verify PASS (6 source
  files == report; 145/145 in builder log). Notable deviations: COMMAND-OWNS-THE-
  MUTATION for #16-18 (non-idempotent layer ops — deliberate departure from
  mutate-then-push; handed to reviewer as focus #1); AddLayerCmd captures appended
  layer for deterministic-id redo; LayerRuntimeSnapshot factored out of step-4
  LayerClipsSnapshot; LayerInspector re-point by coordinate added to
  refreshAfterUndoRedo (risk #3). Pre-existing dead wiring found: LayerStrip
  onFoldToggle unwired at HEAD (left as-is, out of scope).
- 2026-07-22: Step-5 behavioral gate (Harmony): build 0 / ctest 145/145 own run /
  residue CLEAN. APP-LEVEL CHECK BLOCKED-ENVIRONMENTAL — CoreAudio start-mutex
  stall on 4 launch attempts (sampled ×2, identical signature to 2026-07-19 gotcha;
  step-4 gate hit the same stall BEFORE step-5 code; step-5 diff has 0 audio-path
  refs → change-independent, VERIFIED). 90s coreaudiod settle did not clear; needs
  `sudo killall coreaudiod` or reboot (Boris-level). New gotchas.md entry appended.
  App-launch verification PREPENDED to manual checklist below.
- 2026-07-22: Step-5 review CLEAN (10/10 areas corroborated; MoveLayerCmd inverse
  verified algebraically; fence-absence for toggles enforced by TYPE SIGNATURE;
  bypass/solo callbacks were never consumed before — nothing detached). 1 trivial
  NIT accepted WITHOUT fix round (RemoveLayerCmd own stale-coord test — structurally
  identical to 2 tested siblings; reviewer rated it pedantry) → folded into step-9
  test scope. Reviewer's residual watch-item: MoveLayerCmd undo correctness relies
  on layer reordering having exactly ONE code path — if a future step adds another
  reorder path, wrong ORDER (not crash) is the silent failure mode. Committed
  7f87094 (local, no push). Step 5 CLOSED. PATTERN NOW ESTABLISHED: non-idempotent
  structural ops = command-owns-the-mutation; idempotent field writes =
  mutate-then-push.
- 2026-07-22: Step 6 (#21 AddDeckCmd, #22 RemoveDeckCmd, #24 SwitchDeckCmd; #23
  landed in step 4) → FRESH builder undo-s6. undo-s4 retired (2 packets + fix loop
  ≈ undo-s1 retirement load). Gauge ~17% at dispatch. App gate still BLOCKED-ENV —
  same posture: code gates + review; renderer-pointer manual checks to checklist.
- 2026-07-22: Step-6 builder (undo-s6) DONE_WITH_CONCERNS. Receiver disk-verify PASS
  (4 files == report; 153/153). Headliners for review: (1) DELIBERATE latent-bug fix
  — HEAD kDeckNew/kDeckRemove never re-pointed renderer after decks-vector mutation
  (stale/dangling activeDeck_); fenced commands now re-resolve — spec §6 row 6
  mandates it, NOT smuggled scope; (2) kDeckNew at HEAD creates RAW zero-layer deck
  (skips initDefault) — wrapped faithfully, surface to Boris as UX observation;
  (3) handleDeckSwitch has 4 NON-USER call sites (REST/OSC/genre-auto/MIDI) — wrap
  lives ONLY at tab-click entry point; (4) SwitchDeckCmd mutate-then-push (idempotent
  SET per pattern rule); (5) preserved-not-fixed latent risk: remove-BEFORE-active
  deck would misalign activeDeckIndex (unreachable at HEAD — only active-deck removal
  exists). Fresh reviewer undo-s6-review dispatched (undo-s4-review retired at 2 full
  packets + 1 targeted). Step-6 gate (Harmony): build 0, ctest 153/153 own run,
  residue CLEAN; app single-attempt recheck still wedged → BLOCKED-ENV stands.
- 2026-07-22: Step-6 review CLEAN (10/10 corroborated; latent renderer-re-point bug
  CONFIRMED real at HEAD, fix is purely the existing fence re-resolve, spec-mandated).
  1 MINOR → fix round with warm undo-s6: RemoveDeckCmd guarded jassert on the
  active-deck-only invariant (step-4 parity; silent-mis-clamp class). Review also
  SURFACED (pre-existing at HEAD, NOT fixed, NOT step-6 regressions): (a) deck-tab
  HIGHLIGHT never updates via rebuildGrid — undo/redo of deck ops shows correct grid
  but stale tab highlight until next refresh()-triggering interaction (reviewer's
  strongest residual: will LOOK like an undo bug to a tester — checklist item added);
  (b) raw kDeckNew decks all get id=0 + size-based names → id collision across
  successive adds (HEAD weirdness, faithfully preserved); (c) UndoService
  SyncScope::DeckStructure is dead code (defined, never passed) — step-9/cleanup
  candidate. Genre auto-switch INLINES the switch logic (doesn't call
  handleDeckSwitch) — packet naming imprecision only, invariant holds (no push).
- 2026-07-22: Step-6 fix round 1 (jassert) CONFIRMED-CLEAN by targeted re-review;
  gate re-run PASS (153/153 own run). Re-review established the REAL standing
  precedent: "no test ever constructs in violation of a command's own asserted ctor
  invariant" (step-4's stale test deliberately used other commands) → reviewer chose
  FIX-NOW over step-9 deferral for the one violating construction. Fix round 2
  (reviewer-specified verbatim, one arg value 0→9 + comment truthfulness): dispatched
  to undo-s6; no third review pass needed (edit pre-adjudicated by reviewer; Harmony
  gate re-run covers). LANE RULE captured: tests must satisfy the ctor invariants of
  the commands they construct — stale-coordinate coverage uses in-invariant values or
  a different command.
- 2026-07-22: Step-7 builder (undo-s7) DONE_WITH_CONCERNS. Receiver disk-verify PASS
  (13 mod + 2 new == report; 158/158). Headliners: inspectors are POINTER-based and
  not coordinate-aware → scope THREADED from MainComponent at every inspection site
  (the wiring risk); refresh SPLIT is the load-bearing UI design (lightweight
  refresh() for bypass keeps expanded rows; row-count changes rebuilt via
  refreshAfterUndoRedo + new rebuildCompositionEffects for global); Clip/Layer
  inspectors forward DnD into the ONE wrapped itemDropped (no double-wrap); old
  onEffect* callbacks left intact (still wired nowhere). No GL fence (in-place
  vector writes — correct per risk profile). CONVENTION GAP flagged by Harmony:
  2 new headers not added to CMakeLists (step-4 precedent added DeckCommands.h) —
  handed to reviewer as area 9. Step-7 gate (Harmony): build 0, ctest 158/158 own
  run, residue CLEAN, app single recheck still wedged (BLOCKED-ENV). NOTE: step 7's
  refresh-split is exactly what the blocked app gate would prove — manual checklist
  items are LOAD-BEARING for this step; review area-1 trace is the compensating
  control pre-commit.
- 2026-07-22: Step-7 review FINDINGS — 1 MAJOR + 2 NIT. MAJOR: "expanded rows survive
  bypass-undo" documented guarantee is FALSE — refreshAfterUndoRedo (pre-existing,
  unconditional at HEAD before step 7) rebuilds all 3 effect hosts on ANY undo/redo →
  rows_ rebuilt with expanded=false. DATA round-trip correct (proven); UI-state-loss
  only; same pre-existing-gap family as deck-tab highlight. RIDER fired → commit HELD.
  DISPOSITION (Harmony, reviewer-sanctioned remedy (b)): comment-truth fix + 2-line
  CMake NIT now (NIT-effort, no behavior change) → targeted re-review → commit if
  confirmed. Remedy (a) — pointer/scope-aware refresh skip in refreshAfterUndoRedo —
  is a SHARED-PATH behavior change: tracked FOLLOW-UP requiring Boris ratification
  (would fix the whole collapse-on-undo class incl. keeping rows expanded; reviewer
  note: correct fix distinguishes pointer/scope-unchanged, not command type). NIT-2
  (latent): dead UndoService 2-arg reinspect path would be scope-None (silently
  un-undoable) if ever revived — step-9/cleanup note. Side-obs: no-op commands still
  push an inert history entry (currently unreachable) — noted, no action.
- 2026-07-22: Step-7 fix round (comments + 2 CMake lines) → targeted re-review
  CONFIRMED-CLEAN + UNBLOCK (reviewer's independent call, explicitly invited to
  overrule Harmony's framing). Reviewer's 2 on-record unblock conditions verified
  already satisfied in this ledger (collapse-cosmetic checklist line; tracked
  follow-up item). Precedent cited by reviewer: step-6 deck-tab-highlight handling.
  Gate re-run PASS (158/158). Committed d90e953. Step 7 CLOSED. Session total:
  steps 4-7 shipped (4 commits: 6d2def4, 7f87094, 316a2bf, d90e953), tests
  130 → 158.
- BORIS DECISION QUEUE (from this session): (1) ratify the refreshAfterUndoRedo
  pointer/scope-aware skip follow-up (fixes expanded-row collapse + deck-tab
  highlight class); (2) is zero-layer raw Deck-New intended?; (3) env: clear
  coreaudiod wedge (sudo killall coreaudiod / reboot) to unblock ALL app-level
  manual checks; (4) undo-with-no-cell-selected clears clip inspector (carried
  from s. 2026-07-19, still open).

## Queued non-lane items (from handoff, deferred while build lane occupies build dir)
- FIRST (env, Boris-level): clear the coreaudiod wedge (`sudo killall coreaudiod` or
  reboot), then verify Audio-DNA launches + /api/health ready — required before ANY
  app-level manual check below can run (see gotchas.md 2026-07-22 entry).
- Step-5 manual e2e ADDITIONS: layer add / remove / move while rendering → no
  crash/torn frame (fence wiring in-app); each → Cmd+Z restores layer count, order,
  full state incl. clips; bypass/solo/fold toggle → Cmd+Z; X-button layer-clear →
  Cmd+Z restores active-clip runtime (played clip's `playing` flag NOT restored —
  spec risk #5, accepted); NOTE #16-18 now command-owned: UI result appears only
  after perform() — confirm no visible double-apply or lag.
- Step-6 manual e2e ADDITIONS: deck add / remove / tab-switch → Cmd+Z/Cmd+Shift+Z
  restores deck set, active deck, grid content; KNOWN COSMETIC (pre-existing HEAD
  bug, NOT undo's fault): deck-tab HIGHLIGHT may lag until the next click — grid
  content is authoritative; REST/OSC/MIDI/genre deck switches must NOT appear in
  undo history (only tab clicks do); new deck arrives with ZERO layers (HEAD
  behavior, pre-existing) — Boris: confirm that's intended for Deck-New.
- Step-7 manual e2e ADDITIONS: FX drop on clip/layer/global stack → Cmd+Z restores
  (each scope); delete FX → Cmd+Z; bypass toggle → Cmd+Z flips back; multi-select FX
  drop = ONE undo entry; undo a clip-effect edit while a DIFFERENT cell is selected
  (model restores; inspector shows selected cell); KNOWN COSMETIC (pre-existing
  refresh path, NOT step-7's fault): expanded effect rows COLLAPSE after ANY
  undo/redo — real fix is the tracked refreshAfterUndoRedo follow-up (Boris to
  ratify).
- Syphon.framework install + rebuild -DAUDIODNA_BUILD_SYPHON=ON (verify real publish)
- 10s UI eyeball (menus / 3-tab Prefs / new Sources rows) — ADD: Edit menu shows
  dynamic "Undo <desc>"/"Redo <desc>" with correct greyed/enabled state (native-menu
  probe was TCC-blocked headlessly); quick Cmd+Z after a drop = restores cell;
  drag-move a clip (incl. to a FAR empty column) → Cmd+Z → both cells + column
  count restore (step-3 gesture path, not unit-testable headlessly).
- Step-9 manual e2e ADDITIONS from step 4 (gesture paths, not headless-testable):
  multi-video + multi-source drop near RIGHT EDGE → Cmd+Z restores cells AND column
  count; multi-FX drop onto empty far cells → Cmd+Z restores columns; menu Add
  Column / Remove Column → Cmd+Z; Clear Layer Clips + Clear Deck Clips → Cmd+Z
  restores clips AND layer active/crossfade state; confirm multi-SOURCE drop is
  undoable at all (#9 pushed no command before step 4); single-layer-deck cosmetic:
  deck clear label reads "Clear Layer Clips" (pushCommands single-child collapse).

## Known gaps until later steps (accepted, tracked)
- numColumns growth from multi-FX/edge drops NOT undone until step-4 column ops land
  (extra empty columns persist after undo — builder risk (a), step-2 report).
- ~~Same-id replaceContent-of-video undo: player keeps new file~~ RECLASSIFIED
  2026-07-19 by reviewer: NOT a §8.4 boundary — silent wrong-video on undo (worst
  class: quiet failure). Fix in flight (file-compare in media hook + Renderer
  loaded-file query). Step-9 manual e2e MUST include: video→video replace → undo →
  OLD video visibly plays.
- Undo with no cell selected clears clip inspector (safety over UX) — surface to Boris
  at wrap if UX feels off.
