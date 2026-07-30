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
| 8 | Trigger cmds (#1-2) + merge | DONE — review CLEAN (1 MINOR → doc-comment fix round, pre-adjudicated; 1 NIT → step 9); code gates PASS ×2 (ctest 166/166 own runs); app gate Boris-assisted (TCC); committed 4ee2dac |
| 9 | Tests + manual e2e checklist | BUILD DONE — review CLEAN (1 MINOR doc fix folded, 1 NIT accepted); code gates PASS ×2 (ctest 170/170 own runs); all folds landed; committed 0a1c882. Checklist doc authored (.harmony/undo-v1-manual-e2e.md); manual RUN queued = Boris-assisted session (TCC Allow first) |

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
- 2026-07-25: Session resume (secondary, slim) — step 8 per START-HERE marker.
  Disk-verify at boot: HEAD b530525 (close chore atop d90e953), src/tests clean;
  pre-existing out-of-lane graphify-out/ cache churn in working tree (NOT lane
  files — step-8 commit must exclude). ENV: machine rebooted ~08:45 today
  (coreaudiod fresh 08:48) — wedge likely CLEARED; single-attempt app recheck
  at gate time will confirm and, if healthy, un-BLOCK the queued app-level
  manual checks. Step-8 packet: TriggerClipCmd/TriggerColumnCmd + §3 merge
  rules; wrap ONLY handleClipTrigger/handleColumnTrigger (spec §1 consequence 3
  LOAD-BEARING — autopilot triggers from GL thread, never create commands
  there); REST coverage via existing callAsync marshal, verify-don't-duplicate;
  retrigger-of-active-cell pushes NOTHING; per-lane rules: two-pattern
  decision stated with rationale, guarded jasserts, stale-coord tests with
  in-invariant values, merge-behavior tests in step 8; no GL fence
  (runtime-only field writes); lightweight refresh path per step-7 split;
  new headers into CMakeLists (step-7 precedent); builder does NOT commit.
- 2026-07-25: ENV ROOT CAUSE FOUND — the "coreaudiod wedge" was an unanswered
  TCC MICROPHONE PROMPT all along (screencapture during a stalled launch shows
  the live dialog; CoreAudioInternal::start blocks on the TCC response; no CLI
  probe can see it). App is AD-HOC signed (codesign verified) → every rebuild
  changes cdhash → TCC re-prompts per rebuild (explains recurrence + why
  reboot/pkill never helped). Synthetic Allow-click attempted once, denied
  (osascript lacks assistive access) — cap reached, stopped. DISPOSITION:
  stalled HEAD-binary app LEFT RUNNING with dialog on screen — Boris clicks
  Allow to instantly verify env; app-level gates become Boris-assisted (one
  Allow click after each rebuild) until/unless stable signing lands. Gotchas
  entry appended; 07-19/07-22 entries superseded with riders. Step-8 app gate
  plan: code gates + review as usual; app launch check runs when Boris is
  present to click the prompt.
- 2026-07-25: Step-8 builder (undo-s8) DONE (clean). Receiver disk-verify PASS
  (4 mod + 1 new == report, +394 lines, nothing staged; 166/166 in builder log).
  Headliners for review: NO separate TriggerColumnCmd class — column trigger =
  CompositeCommand of per-layer TriggerClipCmds via pushCommands (conform);
  mutate-then-push (idempotent value-assign undo/redo; ClearActiveClipCmd/
  SwitchDeckCmd shape); LayerRuntimeSnapshot REUSED + optional<bool> target
  `playing`; merge keeps original-before/adopts-latest-after with `playing`
  RETARGET to latest column (accepted imperfection under risk #5 — reviewer
  verdict requested); retrigger-no-push via handler state-change guard
  (pendingTriggerColumn-clear edge DOES push — genuinely undoable); GL-path
  proof: autopilot calls layer.triggerClip directly, never the handlers;
  REST/OSC/MIDI marshal through the SAME handlers → undoable BY SPEC (row 8) —
  doctrinal line: user-initiated local/remote = undoable, autonomous
  (autopilot) = never (contrast step-6 deck-switch non-user sites, per spec).
  Fresh reviewer undo-s8-review dispatched. Step-8 code gate (Harmony,
  independent): build exit 0, ctest 166/166 OWN RUN (/tmp/undo-s8-gate-ctest.log),
  residue CLEAN — GATE PASS. App gate: Boris-assisted only (TCC re-prompt on new
  cdhash) — step-8 manual items to checklist. Commit pending review verdict.
- 2026-07-25: Step-8 review CLEAN (10/10 areas independently re-traced; GL-path law
  grep-confirmed — TriggerClipCmd constructed ONLY at the 2 handler sites, Autopilot.cpp
  has zero Command refs; remote marshal chains traced end-to-end; merge algebra exact for
  runtime fields; retrigger guard + pendingTriggerColumn edge corroborated; no ctor
  invariant exists → lane test rule moot; hygiene clean). 1 MINOR: `hasBeenTriggered`
  set outside the snapshot window, never rolled back → first-trigger→undo→retrigger
  silently skips auto-play (Layer.h:247 branch) — same risk-#5 family, but previously
  UNDOCUMENTED; remedy = header-comment line (fix round with warm undo-s8,
  reviewer-prescribed, pre-adjudicated, no re-review — step-6 round-2 precedent) +
  step-9 manual-checklist entry. 1 NIT: pendingTriggerColumn-only-changed push edge
  untested → step-9 fold. Commit after comment lands + gate re-run.
- 2026-07-25: Step-8 fix round (doc comment) landed; gate re-run PASS (build 0,
  ctest 166/166 own run). Committed 4ee2dac (local, no push). Step 8 CLOSED.
  Side-fact confirmed: graphify-out/ churn is a post-commit hook rebuild — out
  of lane, never stage. Step 9 dispatched to WARM undo-s8 (2nd full packet —
  same posture as undo-s1@step2 / undo-s4@step5; retire after). Scope: spec §7
  remainder (merge-behavior + cap-eviction manager cases, seeded property test,
  coordinate-resolution coverage check) + folds (RemoveLayerCmd stale-coord
  test; pendingTriggerColumn push-edge at command level — handler level goes to
  manual; SyncScope::DeckStructure dead-code removal; EffectCommands.h:98 stale
  comment; dead 2-arg reinspect path disposition). Manual e2e checklist doc
  authored by Harmony (.harmony/undo-v1-manual-e2e.md) — consolidation of
  ledger items, memory-layer work. Reviewer: warm undo-s8-review (1 full packet
  load). Gauge 11.3% at dispatch.
- 2026-07-25: Step-9 builder (undo-s8, 2nd packet) DONE. Receiver disk-verify
  PASS: lane diff = exactly 4 claimed files; A1 already-covered claim VERIFIED
  on disk (test_composition.cpp:711 merge + :735 cap-eviction — spec §7 manager
  cases pre-existed; no duplication); DeckStructure grep = zero refs remain.
  Step-9 code gate (Harmony, independent): build exit 0, ctest 170/170 OWN RUN
  (/tmp/undo-s9-gate-ctest.log), residue CLEAN — GATE PASS. Commit pending
  review verdict.
  Dispositions: B6 REMOVED (arm was unreachable); B7 comment fixed; B8 LOUD
  COMMENT chosen over removal (removal orphans inspector_ → ripples into
  setCollaborators + MainComponent, out-of-lane) — full ReinspectTarget removal
  is a candidate FOLLOW-UP if reviewer prefers. New tests: seeded property test
  (mt19937 0xC0FFEE; excludes SetColumnCountCmd grow-only + trigger cmds by
  design — stated in-test), middle-layer coordinate-resolution gap, RemoveLayerCmd
  stale-coord (s5 NIT), pendingTriggerColumn-only edge (s8 NIT). Builder log
  170/170. Reviewer undo-s8-review dispatched (2nd packet, retires after);
  Harmony gate running. Builder undo-s8 RETIRED (2 packets + 1 fix — s1/s4
  precedent load).
- 2026-07-25: Step-9 review CLEAN (9/9 corroborated — B6 replacement mechanism
  independently verified real, not coincidental; B8 ripple claim + hazard-comment
  accuracy confirmed; B7 full call chain traced; A1 content-verified). 1 MINOR:
  UndoService.h:18-22 class doc still describes the removed renderer-re-point
  behavior — reviewer-prescribed one-line fix, dispatched to undo-s8 as trivial
  sub-packet (step-6 two-fix-round shape; retirement deferred one micro-edit).
  1 NIT accepted WITHOUT action: RuntimeOnly zero-caller annotation — rationale
  already tracked here (earmarked for the refreshAfterUndoRedo follow-up).
  Reviewer undo-s8-review RETIRED (2 full packets — s4-review precedent).
  Commit after doc line + gate re-run. B8 FOLLOW-UP candidate logged: full
  ReinspectTarget-path removal (inspector_ member + setCollaborators trim +
  MainComponent call site) — small, out-of-lane, optional.
- 2026-07-25: Step-9 doc line landed; gate re-run PASS (170/170 own run).
  Committed 0a1c882 (local, no push). Step 9 build portion CLOSED. **UNDO V1
  LANE BUILD-COMPLETE (steps 1-9)** — session total: steps 8-9 shipped
  (2 commits: 4ee2dac, 0a1c882), tests 158 → 170. Both agents retired
  (undo-s8: 2 packets + 2 trivial fix rounds; undo-s8-review: 2 full packets).
  REMAINING lane work is ALL Boris-gated: (a) manual e2e run per
  .harmony/undo-v1-manual-e2e.md (TCC Allow click is precondition #0);
  (b) decision queue below. Optional follow-ups (NOT started, need Boris nod):
  refreshAfterUndoRedo pointer/scope-aware skip (ratification pending);
  B8 full ReinspectTarget-path removal (small, out-of-lane).
- 2026-07-25 RATIFICATIONS (Boris, "go with recs then eos"): (a) TCC Allow
  CLICKED — health verified ok/ready/115fps/135 effects → env item CLOSED
  (root cause confirmed end-to-end); stale pre-step-8 instance then killed by
  Harmony (manual e2e must relaunch CURRENT build; expect ONE Allow click).
  (b) STABLE CODE-SIGNING IDENTITY = YES, ratified → next-session Committed
  follow-up: wire CMake codesign identity for dev builds (small Boris step:
  pick/create the identity in Keychain); kills the per-rebuild TCC re-prompt
  class. (c) Manual e2e sitting AGREED, deferred to next Boris-present RTA
  session — checklist .harmony/undo-v1-manual-e2e.md is the script.
- 2026-07-28: MANUAL E2E SITTING (partial) — 10 items PASS (preconditions, drop/
  drag-move/multi-video-edge/multi-SOURCE/multi-FX undos, swap-undo, prefs/menus
  eyeball); then **Column → New CRASHED the app** (SIGSEGV GL thread). Crash-scout
  diagnosis (disassembly-verified, binary-UUID-matched): PRE-EXISTING UAF — message-
  thread clips.resize under an unlocked GL renderOpenGL holding interior Clip*;
  spec's "status-quo risk profile" for column/cell writes FALSIFIED. Full detail:
  manual-e2e findings + notebook LAW entry (2026-07-28). Also found: mixed-drop
  image-discard bug (ClipCell external path — scouted, queued); Boris feature asks
  (Cmd+X cut-clear; MilkDrop presets default dir); "Audio-DNA" duplicate menu
  cosmetic. Sitting PAUSED with expanded HOLD list (columns, clears, clip-clear,
  + their undos); passed drop tests to RE-RUN post-fix (race-lucky).
- 2026-07-28 RATIFIED (Boris, AskUserQuestion): **FAMILY FENCE FIX + CERT lane**
  — fence ALL exposed clips-vector mutation paths (live handlers + command
  undo/redo replay via DeckFenceHook) per scout rec (a), + wire stable codesign
  identity into CMake (kills per-rebuild TCC class; Boris creating "Audio-DNA Dev"
  cert in Keychain). Full-tier verify. ASan pre-confirmation SKIPPED (evidence
  disassembly-grade). Builder fence-f1 dispatched; reviewer to follow; Harmony
  gates + one final Allow click at relaunch.
- 2026-07-28: fence-f1 builder DONE_WITH_CONCERNS. Receiver disk-verify PASS (6
  files == report; runFenced=24; 173/173 in builder log). Harmony code gate
  (independent): build 0, ctest 173/173 OWN RUN (/tmp/fence-f1-harmony-gate.log),
  residue CLEAN — PASS. Builder headliners: per-command fencing (AddLayerCmd shape,
  8 fenced commands); ONE fence per gesture at handler level, sequential-never-
  nested (verified by construction) + fenceActive_ jassert guard; found+fenced an
  EXTRA site beyond scout list (onMultiVideoDropped growth loop — the literal
  crash primitive) + ClearLayerClipsCmd replay; SetClipCmd/SwapClipsCmd left
  unfenced (replay ensureColumns provably no-op); CMake codesign wired, ad-hoc
  fallback verified end-to-end (cert not yet created). Deviations accepted:
  baseline-skip (corroborated by ledger+Harmony's own 170/170), MilkDrop
  ensureColumns reorder (reviewer-verified neutral).
- 2026-07-28: fence-f1 REVIEW = REQUEST_CHANGES (high-quality round: 12/12 sites
  + nesting + tests + CMake all corroborated CLEAN, but 2 NEW findings):
  (1) MAJOR (diff's own code): withDeckDetached not exception-safe — throw during
  mutation leaks fenceActive_=true AND leaves renderer deck-less; remedy RAII
  scope-guard. (2) MAJOR (pre-existing, in-family): kCompNew initDefault() clears/
  reallocs composition_.decks UNFENCED — Column-crash mechanism one level up;
  remedy fence identically (addDeck/removeDeck/fromVar = dead code, no concern).
  AREA-5 RULING (the carve-out question): effects-vector mutations are (a) REAL
  blocker-class same-family exposure — onEffectDropped push_back :789,
  EffectStackView push_back :459/erase :295 (+ performEdit/EffectCommands replay)
  realloc clip.effects under GL iteration (CompositorEngine :244); inferred-by-
  verified-mechanism, not yet reproduced. EFFECTS MANUAL TESTS STAY HELD → ROUND-2
  fence packet required after round 1. MINOR: ClipCommands.h exemption-invariant
  comment. Accepted: double-fence shapes, Release-assert posture, CMake NIT.
  FIX ROUND 1 dispatched to warm fence-f1 (RAII guard + kCompNew fence + comment).
- 2026-07-28: Fix round 1 DONE (RAII FenceResetGuard + ActiveDeckRestoreGuard;
  kCompNew fenced initDefault-only with clear()/UI-refresh sequential after;
  exemption comment with source-verified (a)/(b)/(c)). Receiver spot-verify PASS;
  builder 173/173 own run. Targeted re-review CONFIRMED-CLEAN — all 3 remedies
  source-verified incl. partial-mutation destructor safety (getActiveDeck bounds-
  check ⇒ throw-mid-realloc degrades to nullptr, no dangle). 1 NIT (comment
  doesn't name-check loadPreset/loadDeck as non-model paths) → folds into round 2.
  OPEN: addendum trace of CompDecksBrowser load path (model-replacing? fenced?
  clears history?) — answer gates the round-2 packet contents.
- 2026-07-28: Addendum verdict: CompDecksBrowser load/save-composition callbacks
  UNWIRED at HEAD (empty std::functions; only Save Deck + right-click delete
  live) → not exposed, INERT — but flagged as NAMED FUTURE-FENCE REQUIREMENT
  (source comment + notebook rider: wiring MUST use withDeckDetached +
  undoManager_.clear(), kCompNew precedent). §7 manual item narrowed to
  Composition→New only. NEW Boris decision-queue item: wire browser comp/deck
  load-save or defer to recorder/persistence wave. ROUND 2 dispatched to warm
  fence-f1 (2nd full packet, retires after): effects-vector fences (reviewer
  area-5 enumeration + builder completes the sweep incl. layer/global scopes),
  kClipReplaceContent ruling, NIT fold, future-fence comments, fence-count
  tests.
- 2026-07-28: ROUND 2 builder DONE_WITH_CONCERNS; receiver disk-verify PASS (16
  lane files == claim; fences + folds on disk; 174/174 builder log). Harmony R2
  code gate: build 0, ctest 174/174 OWN RUN (/tmp/fence-f1-r2-harmony-gate.log)
  — PASS. Headliners: clip scope fenced (proven GL read :242/252); layer scope
  EXPOSED via copy-assign read (CompositorEngine.cpp:752) — fenced; global scope
  zero render-side refs — fenced uniformly anyway (EffectStackCmd one class, 3
  scopes); kClipReplaceContent FENCED (whole-call, conservative); EffectFenceHook
  UI-side type (dependency direction); +1 fence test. Builder fence-f1 RETIRED
  (2 packets + fix round + folds). NEW CONCERN flagged by builder, review ruling
  requested: SetClipCmd::apply `cell = *state` on OCCUPIED cell reassigns inner
  vectors on live Clip during replay (SwapClipsCmd: two occupied cells) — same
  mechanism as area-5 blocker ruling, on the round-1-exempted line (exemption
  argued OUTER growth only). Round-3 fence vs not-exposed vs deferred — reviewer
  decides; gates lane commit.
- 2026-07-28: ROUND-2 REVIEW = FINDINGS (areas 1-7 clean: EffectFenceHook dup
  acceptable/byte-identical; uniform-global-fence accepted; replaceContent
  whole-call fence agreed; folds + tests corroborated). RULING on builder
  concern: (a) REAL in-family BLOCKER and WIDER than replay — occupied-cell
  `cell = *state` invokes Clip::operator= on the engaged optional whose address
  GL holds (Layer.h:151,160-165); undo/redo reassigns DIFFERENT-sized effects
  vectors ⇒ real realloc; reachable by drop-onto-active-cell + Cmd+Z (no
  occupied-gate in isInterestedInFileDrag). PLUS two live paths never fenced:
  handleFileDrop→applyFileDrop (~:3308-3322) and handleMultiFileDrop setClip
  (~:3360). ROUND 3 REQUIRED (prescribed): fence SetClipCmd/SwapClipsCmd
  replay + the 2 live paths; DELETE exemption comment (moot); fence-count
  tests; N-fence cost on composite undo accepted (batch-fence = optional
  future opt only if e2e shows stutter). Fresh builder fence-f3 dispatched;
  fence-f1-review does final targeted confirm then retires (2 full + 2
  targeted + addendum ≈ precedent boundary).
- 2026-07-28: ROUND-3 builder (fence-f3) DONE. Receiver disk-verify PASS (4
  round-3 files; typedef single-sourced in ClipCommands.h — judgment-call
  relocation, include-direction justified; exemption block deleted, 1 prose
  mention remains for reviewer confirm; 176/176 builder log). Harmony R3 gate:
  build 0, ctest 176/176 OWN RUN (/tmp/fence-f1-r3-harmony-gate.log) — PASS.
  Lane totals: 170→176 tests, 18 files. fence-f3 retired (1 packet). Final
  targeted confirm dispatched to fence-f1-review (last pass, retires after;
  incl. CompDecksBrowser stale cross-ref disposition). On CONFIRMED-CLEAN:
  single lane commit → cmake reconfigure (pick up cert if present) → rebuild →
  app gate w/ Boris Allow → sitting unblocks.
- 2026-07-29: ROUND-3 FINAL CONFIRM = CONFIRMED-CLEAN (typedef single-def +
  include-order verified; exemption grep-hit confirmed historical prose;
  replacement comments truth-checked; applyFileDrop exactly-2-callers proven
  no-nest; 4 riskiest nesting sites traced sequential incl. CompositeCommand
  child-loop; SwapClipsCmd test exercises the exact occupied-cell case). LANE
  MAY CLOSE. One pre-adjudicated comment-only fix (CompDecksBrowser.h stale
  cross-ref, reviewer-prescribed verbatim) → dispatched to fence-f3 as
  retirement-deferred micro-edit (step-9 precedent), no re-review. Reviewer
  fence-f1-review RETIRED (2 full + 3 targeted + addendum). Next: gate re-run →
  SINGLE LANE COMMIT (18 files, tests 170→176) → cmake reconfigure (cert
  pickup) → rebuild → app gate (Boris Allow) → HOLD list dissolves.
- 2026-07-29: **FENCE LANE COMMITTED — 8bd09ba** (16 files, +777/−246, tests
  170→176, local no push). Final gate re-run PASS post-micro-edit (build 0,
  176/176 own run, ad-hoc signed — cert still not created). APP GATE PARKED:
  fenced build relaunched 20:09, TCC mic prompt confirmed ON SCREEN via
  screencapture diagnostic (documented gotcha method); 10-min health watcher
  expired unanswered — Boris away. DISPOSITION: app LEFT RUNNING with dialog up
  (2026-07-25 precedent). RESUME POINT: Boris clicks Allow → health check →
  app gate closes → manual-test HOLD list DISSOLVES (columns, clears,
  clip-clear, effects + all undos testable; re-run race-lucky drop passes).
  Then: cert step (still pending, keychain empty) → one reconfigure+rebuild →
  final TCC prompt ever. All agents retired; lane fully closed on the code
  side.
- 2026-07-30: **APP GATE CLOSED** (session resume, secondary slim). Receiver-
  verify at boot: handoff claim "app running, prompt on screen" was STALE — app
  QUIT (pgrep empty, health down). Relaunched fenced binary via `open` 00:05
  (mtime 07-28 20:09 = final gate build; HEAD bb076c7 = chore atop 8bd09ba;
  post-build delta comment-only → behaviorally identical) → health
  ok/ready/119.6fps/135 effects in ~10s, NO TCC prompt: Allow evidently clicked
  off-session after the 07-29 close (mic-in-use menu-bar indicator live =
  GRANTED; TCC.db query FDA-denied; screenshot /tmp/audiodna-tcc-20260730.png).
  Zero Boris clicks spent. → Manual-test HOLD LIST DISSOLVED (columns / clears /
  clip-clear / effects + their undos testable); 4 race-lucky drop re-runs
  pending. Cert STILL absent (0 codesign identities) — next rebuild re-prompts
  ONCE; recommended order flip vs handoff: sitting FIRST on this live granted
  binary (zero clicks, zero latency), cert step after (no rebuild mid-sitting →
  nothing re-prompts). Tree sanity: non-graphify dirt = one untracked .audit/
  dir; src clean vs HEAD. ADDENDUM (morning, Boris directive "keep working the
  list autonomously"): (a) fenced build survived ~9h idle overnight at 119fps /
  DSP 1% — passive stability soak, zero new .ips (baseline = the 2 known 07-28
  files); (b) window-capture verification channel established (CGWindowList id
  1219 + `screencapture -l` — no focus steal); (c) audio pipeline VERIFIED live
  end-to-end from window capture: Mic Input + real waveform + analysis bands +
  tempo 128 locking w/ beat indicators (structural half of the "SignalBar moves"
  Boris check self-served); (d) osascript accessibility re-probed: still DENIED
  (menu/keystroke UI scripting unavailable; Boris unlock = System Settings →
  Accessibility → Ghostty); (e) api-surface-scout dispatched to map REST/OSC
  drivability for autonomous checklist execution. (f) SCOUT MAP LANDED (cited
  to file:line): undo/redo + ALL structural mutations (columns/clears/layer
  & deck structure/cell set-clear) are REMOTELY UNREACHABLE — UI/menu only;
  remotely drivable = trigger_clip/trigger_column (undo-recorded, callAsync-
  marshalled), switch_deck (NOT undo-recorded — bypasses SwitchDeckCmd,
  ApiServer→handleDeckSwitch:3394 direct), set_bpm, load_image/load_source
  (renderer-level, NOT grid cells), effect enable/params, OSC layer
  opacity/bypass/solo/mute (message-thread, no undo); /api/composition = full
  state readback probe. DO-NOT-CALL respected: snapshot/render_frame (disk),
  reset/set_effect_chain (silent chain wipe). Dormant TestServer (port 8080,
  --test-mode, compiled in) enumeration requested — may unlock autonomous
  undo/menu driving. (g) NEW LATENT FINDING (scout risk flag) → Boris queue
  item (4): ApiServer set_param clip branch (:399) + set_layer_opacity (:458)
  mutate the model ON THE HTTP THREAD unmarshalled (unlike the 4 callAsync
  endpoints) — field-write class, NOT the resize crash class, but
  unsynchronized concurrent writes; tiny marshal fix candidate; EXCLUDED from
  soak design (ambiguous evidence). (h) ENDPOINT EXERCISE PASS: 15 rapid
  trigger_clip + 3 trigger_column + switch_deck under live render → all ok,
  fps 119.58 steady, same PID, zero new .ips; post-state activeClipColumn -1
  everywhere ⇒ empty-cell triggers are state-no-ops ⇒ state-change guard
  pushed ZERO history entries (guard semantics behaviorally corroborated) —
  app state + undo history remain PRISTINE for the sitting. (i) Checklist
  items source-proven + exercised: §4 remote-deck-switch-not-in-history
  (scout :3394 + step-6 review, two independent source reads + behavioral
  run); §6 REST-trigger-pushes-history (scout :2995 + step-8 review
  grep-proof; menu-eyeball residue only).
- 2026-07-30 PM: **AUTONOMOUS UI DRIVE COMPLETE** (Boris Option A — Accessibility
  granted to Ghostty). Synthetic-event driver: CGEvent swift tool (click/drag) +
  System Events menus/keystrokes + mandatory verify-and-retry (state+label
  fingerprints; raw delivery flakes ~15%). Oracles: /api/composition,
  Composition-menu labels (dynamic undo descs — menu is COMPOSITION, not Edit),
  window captures by CGWindowList id. RESULT: **~130 mutations across EVERY
  crash-family path under active GL render — ZERO CRASHES** (fps 114-120, .ips
  count unchanged at 2). Headline: column ops ×33 incl. 5 undo/redo REPLAY
  cycles on the exact 07-28 SIGSEGV scenario — fence family PROVEN in-app.
  Also verified (checklist annotated per item): drop-source undo/redo; trigger
  undo/redo; retrigger-no-push (UI+REST); single-layer merge; cross-layer
  no-merge; column-trigger composite undo (REST, content on 2 layers); deck
  add/remove/tab-switch triad exact restores; layer add/insert/remove/move +
  fold + bypass/solo; X-button active-clear (singular cmd) vs layer-clear
  (plural) both restore-exact; clip-scope FX drop + undo + redo-replay
  (visually confirmed warp); Composition→New full wipe + BOTH stacks cleared +
  menus greyed. Scout adjudications ×2: all 5 "dead" menu items WIRED —
  selection preconditions (cell thumbnail=trigger vs name-bar=select
  ClipCell.cpp:180-194; strip-click selection LayerStrip.cpp:706-719; MoveUp
  `selLayer>0` guard; layer-clear content guard :3694); **NO Cut command
  exists at HEAD** (Cmd+X ask = net-new build). NEW findings → queue
  candidates: (5) menu enablement not gated on selection preconditions
  (silent-no-op class); (6) rebuildGrid leaks stale INVISIBLE selection
  (clearSelection only in drop handlers — misleads UI-state readers); (7)
  preview ANIMATES the old deck's clip while an empty deck is active
  (frames-differ verified — intent question, pairs with zero-layer Deck-New).
  Residue = 11-item Boris runsheet at checklist top (~15-20 min: Finder drops,
  video cases, name-bar gestures, FX scopes/rows, autopilot confirm, quantize
  edge, taste calls, Syphon, cert). App left healthy: 1 deck, 2 staged clips,
  history [Drop,Drop], 119fps.
- 2026-07-30 PM2: **NAME-BAR GEOMETRY CRACKED** (session resume, secondary slim;
  autonomous item from handoff). Boot receiver-verify: app HEALTHY (PID 68430,
  117fps, port 7070 — NB health/API port is 7070), composition intact from PM
  close; cert STILL ABSENT (0 identities) — order-flip stands (sitting first,
  cert after). Explore scout mapped the full drive surface, file:line-cited →
  `.harmony/scout-namebar-geometry.md`: name bar = BOTTOM 20px of 90x96 cell
  (safe point cell-local 45,86); cell x=250+col*90, y=displayRow*96 with
  displayRow = numLayers-1-layerIndex (INVERTED); drag = name-bar mouseDown +
  >5px continuous held drag (JUCE kills synthetic drags lacking either);
  drag pushes SwapClipsCmd, label "Move Clip"/"Swap Clips"; Clip>Clear = menu
  idx 5 first item, HANDLER-gated on selection (silent no-op unselected —
  same class as queue candidate 5), label "Clear Clip". NEW LATENT FINDING →
  queue candidate (8): fold-height MIRROR-INDEX bug — layoutGrid row-height
  loop uses deck->getLayer(displayRow) (DeckView.cpp:274) where every sibling
  loop mirrors the index; any folded layer skews ALL cell Y coords (product
  bug, not just driver hazard). Drive guard: all layers unfolded first.
  DRIVE STOOD DOWN: HID idle <1s = human live at the machine (state drifted
  in-session: cols 14→12, 7 clips staged, playback active) — no synthetic
  events into an occupied seat. OBSERVED ANOMALY (unverified, live-use noise
  possible) → queue candidate (9): BOTH layers' activeClipColumn point at
  empty/nonexistent cells (L0→col7 no clip object; L1→col8 empty stub) right
  after column count dropped 14→12 — stale-active-after-column-removal
  hypothesis; family of candidates (6)/(7). One sitting glance: does a playing
  highlight sit on an empty cell?
- 2026-07-30 PM3: **BORIS SITTING COMPLETE** (results annotated per-item in
  undo-v1-manual-e2e.md runsheet). Scoreboard: PASS ×6 (drops incl. all 3
  race-lucky re-runs + mixed-batch known-bug behavior; video-replace worst-class
  CLEAR; Clip>Clear; autopilot menu-frozen; ignore-column-trigger; drag-move*),
  *drag-move with CHECKLIST CORRECTION (drag can't create columns — drop target
  = existing cells only; column-count clause was a drop-path expectation; undo-
  restore confirmation pending as single-item follow-up). PARTIAL ×1: FX — layer
  scope via LAYER WINDOW works + undoes, but channel-strip drop NOT accepted,
  COMPOSITION (global) window drop NOT accepted; delete/bypass/multi-select
  untested. BLOCKED ×2: quantize edge (clicking a playing cell is IGNORED — no
  UI retrigger path; design question: Boris expects restart) · MilkDrop (empty
  browser) → **BORIS GREENLIGHT: default-preset-dir + crash-#2 mini-lane
  DISPATCHED** (builder, background). CERT RESOLVED: Boris created it correctly;
  failure was CSSMERR_TP_NOT_TRUSTED (self-signed root untrusted) — Harmony
  added user-domain trust (add-trusted-cert -p codeSign, no sudo needed) → 1
  VALID identity. Next: builder lands → Reviewer → cert RECONFIGURE + rebuild +
  relaunch + ONE final Allow → Boris retests item 9. NEW BUG FAMILY (sitting):
  (A) X-clear on playing clip → gone from strip, OUTPUT KEEPS PLAYING
  (model/renderer desync — invisible to /api/composition, explains why PM drive
  passed clears); (B) autopilot lands on a CLEARED empty cell; (C) = candidate
  (9) stale activeClipColumn — unified hypothesis: CLEAR paths don't purge
  runtime refs (renderer content, autopilot pool, active pointers). Triage
  scout dispatched (read-only) on: FX drop-target wiring/intent, retrigger-
  ignored guard, file-browser Desktop-open + list-button slowness (new perf
  issue), clear-path runtime-ref purge points. PROCESS RULE (Boris directive):
  taste/perceptual checks go to Boris ONE ITEM PER ASK — never batched (item 10
  batching rejected; remaining taste calls queued as singles).
- 2026-07-30 PM4: **TRIAGE SCOUT LANDED** — all 4 sitting findings root-caused,
  full cited report → `.harmony/scout-sitting-triage.md`. Verdicts: (Q1) FX
  drops — LayerStrip NOT-WIRED (no DragAndDropTarget); composition/global
  WIRED-BUT-~20px-target (empty stack collapses, EffectStackView.cpp:234);
  fix = panel-level target on CompositionInspector mirroring
  LayerInspector.cpp:977-1000. (Q2) retrigger-of-active EMERGENT no-op —
  branch runs (Layer.h:225-235) but writes model playhead only; renderer
  clobbers it every frame (Renderer.cpp:923); real restart needs player
  seekTo; queued-trigger clear DOES fire silently (Layer.h:223) — quantize
  runsheet item is behaviorally UNOBSERVABLE in UI, not broken. (Q3)
  file-browser slowness = sync FULL-RES image decode per file on message
  thread, zero cache (FilesBrowser.cpp:408-441,:491-502); List button re-runs
  whole decode pass and never draws thumbnails (:358-359 vs :270-288). (Q4)
  clear-path = TWO roots: (a) X-clear never purges renderer
  (activeSourceType_ stays live → shader keeps rendering; 4-line fix = lift
  MainComponent.cpp:2972-2979 purge into onLayerClearClip :678); (b)
  kClipClear writes Clip{} not nullopt (:3852) → blank cell has_value() →
  autopilot picks it (Autopilot.cpp:205+) + stale activeClipColumn = sitting
  bugs (b)+(c) explained; NB fixing (b) touches clear-command undo contract —
  lane-pattern care. FIX MENU to Boris (priority call his): A = clear-path
  renderer purge + blank-cell fix (his most-felt bugs) · B = CompositionInspector
  drop target (tiny) · C = FilesBrowser cache/async/list-skip (perf) · D =
  retrigger-restart (DESIGN ruling first) · LayerStrip-as-FX-target (product
  ruling). Disambiguator question sent (shader vs video kept playing).
- 2026-07-30 PM5: **MILKDROP MINI-LANE BUILD-COMPLETE** (commit 9229f87, local).
  Builder findings: (a) crash #2 — every path to the reported symbol was
  ALREADY guarded at HEAD; the one PROVEN null-deref was in selectPreset()
  (:825-835 unguarded getPreset after guarded setCurrentIndex) — fixed;
  getCuratedPresets/getPresetsForSection hardened to null-OR-empty guards
  (INFERRED-DEFENSIVE — exact .ips mechanism UNCONFIRMED; crashed binary UUID
  unmatchable, static analysis exhausted; if it recurs → ASan debug build).
  (b) preset-dir — pure BUILD-SYSTEM gap: MainComponent :1301-1318 wiring was
  always correct; nothing copied resources/projectm_presets (30 .milk +
  manifest) into the bundle, and the CWD fallback never fires on Finder/open
  launches. Fix = CMake POST_BUILD copy_directory registered before codesign.
  No new automated test (no GUI-capable test target exists; flagged for
  Reviewer weigh-in). Receiver-verified on disk: commit + 31 bundle items +
  still-adhoc signature (expected). IN FLIGHT: independent Reviewer on the
  commit + cert RECONFIGURE+rebuild (background) — then relaunch + Boris's
  ONE final Allow + behavioral gate (health, browser populated, SignalBar
  arrow probe) + Boris retests runsheet item 9.
- 2026-07-30 PM6: **MILKDROP LANE BEHAVIORAL GATE — PASS (Harmony-run); ZERO
  ALLOW CLICKS NEEDED.** Sequence + findings: (1) relaunch #1 overlapped the
  OLD instance CRASHING ON QUIT (NEW pre-existing bug → queue candidate (10):
  shutdown-path SIGBUS, destructor chain MainComponent→EffectsRackPanel→
  Label→Value teardown, .ips 2026-07-30-125725, EXC_ARM_DA_ALIGN — triggered
  by graceful osascript quit of the 07-28-era binary; any graceful quit runs
  this path); that overlap left instance #1 with "No audio device found"
  (CoreAudio enumerated mid-crash) + slow ApiServer accept. (2) SIGKILL'd
  instance #1 deliberately (skips the crashy destructors, no state to lose),
  clean relaunch → PID 57673 HEALTHY: 119fps, tempo LOCKED, full audio
  pipeline live, and — headline — **mic permission INHERITED by the
  cert-signed binary, NO TCC prompt** (cert promise exceeded: zero clicks, not
  one). (3) Gate results: browser POPULATED on open (Curated: Energetic 9 +
  Psychedelic 9 + more, manifest moods working) = fix (b) PASS; SignalBar
  arrow probe — the EXACT 07-28 crash gesture — app SURVIVED the full expand
  cascade at 116fps, no .ips = crash-family path behaviorally exercised.
  Tests 176/176 re-run by Harmony (not builder-claimed). Reviewer still in
  flight — lane close waits on verdict. RESIDUE (cosmetic, honest): SignalBar
  left in EXPANDED mode — 3 arrow-click attempts + View>Reset Layout all
  failed to cycle it back (Reset Layout does NOT govern SignalBar mode —
  itself a finding); one click from Boris (he knows the control). Boris still
  owns: item 9 retest (preset drag + playlist drop + Cmd+Z).
- 2026-07-30 PM7: **MILKDROP MINI-LANE CLOSED.** Reviewer APPROVE (0 blocking;
  independently re-traced all 26 presetManager_ call sites, POST_BUILD
  ordering, bundle path vs MainComponent:1304-1306). Verify-split complete:
  builder built (9229f87) · Reviewer read source · Harmony ran behavioral gate
  (PM6). 2 non-blocking nits queued as RIDERS for the next builder dispatch:
  (i) one-line CMake comment noting copy_directory doesn't delete stale
  bundle presets; (ii) note that copy-before-codesign ordering is
  position-enforced only. Reviewer suggestion parked: null-manager regression
  unit test IF a headless JUCE test target ever exists. Boris residue
  unchanged: item 9 retest + SignalBar collapse + 2 pending yes/nos + fix-menu
  priority call (A recommended).
- 2026-07-30 PM8: **FIX LANE A DISPATCHED** (Boris "continue working" = go on
  recommended order; D still parked on his design ruling). Scope: A1 X-clear
  renderer purge w/ explicit global-source OWNERSHIP rule (per-layer clear vs
  global activeSourceType_ edge in the packet); A2 kClipClear truly-empty
  cells + activeClipColumn reset + sibling clear-path audit + undo
  exact-restore proven by new tests; riders = 2 reviewer CMake nits. Serial
  lanes (shared build dir): B (CompositionInspector drop target) → C
  (FilesBrowser perf) after A closes. PARALLEL (read-only): shutdown-crash
  scout on .ips 2026-07-30-125725 (candidate 10) — diagnosis only, fix awaits
  Boris nod. GATE PLAN for A (machine-free required): stage clip via MilkDrop
  preset drag (browser now populated — internal JUCE drag, mouse tool) →
  thumbnail-trigger → verify output motion (2 captures diff) → strip-X click →
  motion MUST stop + /api/composition activeClipColumn=-1 → Clip>Clear on a
  staged cell → /api shows cell EMPTY (no blank stub) → Cmd+Z exact-restore →
  BONUS: drag-move staged clip + Cmd+Z closes runsheet item 3 without Boris.
- 2026-07-30 PM9: **LANE A BUILD-COMPLETE + SHUTDOWN CRASH ROOT-CAUSED.**
  (1) clearpath-builder a718572: A1 rescan-or-purge ownership rule (factored
  refreshPreviewFromActiveClip mirroring handleColumnTrigger tail — no new
  bookkeeping state, cannot blank another layer's visual); A2 Deck::clearCell
  → genuine nullopt (SetClipCmd already supported it), activeClipColumn reset
  via ClearActiveClipCmd child in SAME composite, sibling clear paths audited
  (already-correct, untouched); riders in. Tests 177/177 (builder + my rerun).
  Builder judgment call ACCEPTED: renderer refresh extended to kClipClear
  (avoids reopening root-a via its new clearActiveClip call). Known residual
  (pre-existing, flagged): rescan handles Image/Source only (Video/ImageSeq
  omitted — matches handleColumnTrigger's existing scope). Reviewer IN FLIGHT.
  BEHAVIORAL GATE BLOCKED: HID idle 34s = Boris at machine — relaunch+drive
  parked until machine free or his go. (2) shutdown-crash scout (candidate 10)
  → `.harmony/scout-shutdown-sigbus.md`: single-word corruption in a live
  Label discovered at teardown (odd shared_ptr ctrl pointer = ALIGN trap,
  byte-verified); writer unidentified; 2 candidate writers found (UNSYNCED
  cross-thread EffectChain init-vs-timer — 8bd09ba family; compacted-index
  bug EffectsRackPanel.cpp:472); smallest fixes: detach GL renderer at TOP of
  ~MainComponent (1 line) + fence initEffectChain + bounds-check :472; verify
  via ASan variant (repo has none) / malloc-guard env interim. CORRECTION to
  PM6: crash is state/timing-dependent, not every-quit. Fix lane = Boris
  decision (candidate 10 now fully scoped).
- 2026-07-30 PM10: **LANE A REVIEW: APPROVE-WITH-NOTES → FIX ROUND DISPATCHED
  (R3, warm builder).** HIGH finding: kLayerClearClips :3749-3781 +
  kDeckClearClips :3680-3710 miss the refreshPreviewFromActiveClip companion —
  A1 symptom reproducible via those menu paths; builder's "already correct"
  audit was true ONLY for the A2 angle. Reviewer VERIFIED the big risks:
  rescan method byte-identical to handleColumnTrigger tail; Video/ImageSeq
  rescan omission INERT (video is compositor-owned, fallback never renders
  it); undo composite fence-conformant (SetClipCmd fenced, ClearActiveClipCmd
  field-only unfenced per convention), transient mid-undo window harmless +
  pre-existing shape; tests non-tautological. Plan: builder patches 2 sites →
  reviewer delta-check (warm) → Harmony behavioral gate (still parked on
  machine-free) covers X-clear + Clip>Clear + Clear-Layer/Deck stale-render
  scenarios + item-3 drag-move closeout.
  FIX ROUND LANDED: 20fe75d, +11 additive-only (verified via show --stat),
  both sites patched (deck: once post-loop pre-rebuildGrid; layer: inside
  snapshot-has-content guard), 177/177 re-run by Harmony. No new test —
  accepted rationale: refreshPreviewFromActiveClip mutates previewPanel_/
  outputWindow_ (GL components), no headless target links MainComponent.
  Reviewer delta-check dispatched. Gate still parked (HID 43s, Boris active).
- 2026-07-30 PM11: **LANE A SOURCE-CLOSED** — delta-check APPROVE (both
  placements airtight: deck call post-loop sufficient — helper rescans all
  layers; layer call same-guard as clearActiveClip — no skip path; additive-
  only confirmed; no-test rationale verified against tests/CMakeLists.txt).
  Lane A = a718572 + 20fe75d, source fully approved; REMAINING: Harmony
  behavioral gate (machine-parked). **LANE B DISPATCHED** (build dir free —
  A is committed+built): CompositionInspector panel-level DragAndDropTarget
  mirroring LayerInspector.cpp:977-1000, forward-to-stack only (no second
  add-path), report-only sweep on multi-select-FX undo entry count (runsheet
  item 5 residue). LayerStrip-as-target still EXCLUDED (Boris product ruling
  pending, single-item ask queued).
- 2026-07-30 PM12: **LANE B BUILD-COMPLETE** (9c316e6, disk-verified 2 files
  +43/-1, 177/177 my rerun): CompositionInspector = panel-level FX drop
  target, LayerInspector mirror, forwards to existing stack path (no second
  add-path); undo already pushed by shared EffectStackView::itemDropped.
  SWEEP ANSWER (source-cited): multi-select FX drop = ONE undo entry ("Add N
  Effects") on ALL scopes — one runFenced loop + single onPerformEdit
  (EffectStackView.cpp:443-491) — closes runsheet item-5 sub-check at source
  level (behavioral confirm folds into the gate). Reviewer on B dispatched
  (warm, independent — built nothing). **LANE C DISPATCHED** (FilesBrowser
  perf: list-mode zero-decode, path+mtime thumbnail cache, async decode w/
  stale-job invalidation + teardown safety). Gate still parked (HID 69s).
- 2026-07-30 PM13: **LANE B SOURCE-CLOSED** — review APPROVE 0 issues.
  Notables verified: highlight-flag can never be true with null composition_
  (isInterested gates itemDragEnter); no double-handling (nested
  EffectStackView target wins deepest-first on direct hits — same shape as
  Layer/Clip panels); onPerformEdit/fence wiring pre-existing via
  InspectorPanel.cpp:145-156. A+B both await ONLY the Harmony behavioral gate
  (machine-parked). Lane C building.
- 2026-07-30 PM14: **CRASH #2 RESURFACED — LIVE, ON THE FIXED BINARY.** .ips
  2026-07-30-132512: SIGSEGV KERN_INVALID_ADDRESS, Message Thread,
  getCuratedPresets+128 ← paintGrouped ← paint, at 13:25:12 during Boris's
  live use (likely item-9 MilkDrop testing); app relaunched 2s later (PID
  66725). Binary 57673 INCLUDED 9229f87's guards ⇒ EMPTY-STATE THEORY DEAD —
  crash is past the guards reading preset data with a valid count. Prime
  suspect (per milkdrop-builder's flagged out-of-scope risk): raw
  presetManager_ pointer into GL-owned ProjectMSource — dangling/mutated
  while message thread paints; triggering a milkdrop clip may (re)create the
  source. Correlates: 07-28 original crash (same symbol, layout-path read) +
  shutdown-SIGBUS corruption theme = cross-thread writer family.
  milkdrop-race-scout DISPATCHED (both .ips + lifetime map + fix ranking).
  BORIS WARNED: hold off MilkDrop/item-9 testing until fixed; he's also on a
  STALE binary (has A1 only — missing 20fe75d fix-round + lane B; launched
  13:25:14, binary replaced 13:35+13:41).
- 2026-07-30 PM15: **LANE C BUILD-COMPLETE** (8c746ab, disk-verified 5 files
  +359/-14, 181/181 my rerun incl. 4 new ThumbnailCache unit tests).
  Design: message-thread-only LRU cache (path+mtime key, cap 500, standalone
  header — extracted per packet for headless testability); 2-thread
  ThreadPool decode with generation-counter stale-job no-op; static
  generateThumbnail (no this capture) + SafePointer-in-callAsync completion;
  view toggle = flip+repaint only. Builder-flagged trade-off accepted:
  fast-typing dup decode jobs (idempotent cache put, no correctness bug).
  Reviewer dispatched — directed HARDEST at the teardown reasoning (member-
  order "happens to" claim flagged as potentially fragile; this codebase just
  had a teardown-corruption crash). PENDING: C review · milkdrop-race scout ·
  combined A+B+C behavioral gate (machine still Boris-occupied, HID 53s).
- 2026-07-30 PM16: **CRASH #2 TRUE ROOT CAUSE — DETERMINISTIC UAF, SOLVED.**
  Scout report (disassembly-level, both .ips instruction-exact) →
  `.harmony/scout-milkdrop-uaf.md`. Mechanism: preview-panel hide (SignalBar
  EXPANDED, MainComponent.cpp:1727) or zero-size → SYNCHRONOUS GL detach on
  message thread → openGLContextClosing → activeSources_.clear() → ProjectM
  preset manager (by-value member) destroyed → browser's raw interior pointer
  dangles (wired once, ctor) → any later browser repaint/layout = UAF.
  Empty-state AND race theories DEAD (07-28 element ptr 0x1 = dead object,
  guards unreachable-proof). 07-28 sitting stack fits EXACTLY; 07-30 arming
  event INFERRED = Harmony's own 13:02 SignalBar probe (code path proven,
  instance unobservable) — detonated in Boris's session at 13:25. Also
  explains presets-vanish-after-expand/collapse (scan runs once). NEW queue
  candidate (11): activeSources_ unordered_map mutated lock-free from 3
  threads (message/GL/HTTP — TestServer.cpp:893) — separate hardening.
  uaf-fix-builder DISPATCHED: scout fix #1 (keep releaseGL loop, delete the
  clear(); 6 mandatory verifications incl. all-subclass dtor sweep +
  shutdown-lane interaction). Gate recipe now includes: expand/collapse
  SignalBar → browser repaint → presets INTACT (regression-proof for both
  the crash and the vanish).
- 2026-07-30 PM17: **LANE C REVIEW: APPROVE-WITH-NOTES → fix round (warm
  builder).** Teardown design CONFIRMED sound vs JUCE source (ThreadPool dtor
  = WaitableEvent poll not message-pump ⇒ queued callAsync can't fire
  mid-teardown; SafePointer cross-thread copy safe, deref message-thread-only).
  2 non-blocking: (i) pool-before-entries order is INCIDENTAL declaration
  order + default dtor — fix round makes it EXPLICIT (drain pool in dtor body
  + cross-referenced comments; per shutdown-sigbus doctrine); (ii) cache put
  re-queries mtime at put-time — stale-content false-hit window; fix = capture
  mtime at read, thread through. Reviewer also surfaced: view-toggle no longer
  resets scroll (assessed UX IMPROVEMENT, kept). Boris re-warned: REAL crash
  trigger = SignalBar expand / preview collapse (not MilkDrop per se).
- 2026-07-30 PM18: **UAF FIX BUILT + GATE ATTEMPT ABORTED (Boris returned).**
  76594fd disk-verified (+10/-2 Renderer.cpp only), 181/181 my rerun, all 6
  builder verifications reported; reviewer dispatched. Gate: snapshotted
  Boris's composition (1.1KB, near-empty → .harmony/boris-session-snapshot-
  1430.json), SIGKILL stale 66725, relaunched → PID 91888 HEALTHY 120fps on
  the FULLY-FIXED binary (A+B+C-initial+UAF). Then ABORT: first drive click
  landed in Boris's Firefox (he returned silently; frontmost was his browser;
  click hit a popup illustration, visibly no action). Root failure: stale HID
  check + NO frontmost-app check → gotchas rule (11) SYNTHETIC-CLICK PREFLIGHT
  added (same-command fresh HID + frontmost=Audio-DNA, else abort). Gate
  scenarios (UAF arrow replay, X-clear stop, Clip>Clear empty+undo, comp-panel
  FX drop, FilesBrowser timing, item-3 drag-move) all PENDING next window.
  NOTE: Boris's next app use is already on the fixed binary — SignalBar
  warning DOWNGRADED (source-fixed, behaviorally unproven).
- 2026-07-30 PM19: **UAF FIX SOURCE-CLOSED** — review APPROVE 0 issues
  (exhaustive subclass sweep confirmed 2-type enumeration; presets-survive
  mechanism traced end-to-end incl. by-value member address stability;
  regression angles cleared: activeSources_ growth bounded by compile-time
  ~90-id registry, outputTex_ has no accessor bypassing the !glInitialized_
  gate). Crash #2 = FIXED at source (76594fd), behavioral replay pending gate.
  DISAMBIGUATOR ASK DROPPED (shader-vs-video): moot — UAF diagnosis explains
  the crashes; lane-A rescan-or-purge handles both media classes (video
  compositor-owned). Boris's open items reduced to ONE: a machine window for
  the combined gate (or his 30s self-test of the arrow replay).
- 2026-07-30 PM20: **LANE C FIX ROUND LANDED** (9b74c7d, disk-verified 4
  files +108/-28, 182/182 my rerun incl. new stale-mtime false-hit test).
  Explicit dtor drain (named 5000ms constant) + cross-ref member comments;
  mtime captured on pool thread at read-time, explicit-mtime put overload on
  the async path. Delta-check dispatched (final). Running instance 91888
  predates this commit — the eventual gate relaunch picks it up (teardown-
  internal + cache-key only; behaviorally invisible otherwise). Builder noted
  pre-existing unrelated warning (FileListContent::hitTest hides base) — not
  touched, logged here for visibility.
- 2026-07-30 PM21 (new session, ~15:05): **BORIS GATE FEEDBACK — 7/8 PASS, THE
  GATE IS BEHAVIORALLY CLOSED.** Boris ran the 8-item list on PID 91888 (fixed
  binary, 56min uptime, ZERO new .ips — disk-verified pre-ingest): (a) crash-#2
  UAF arrow replay PASS → **crash #2 CLOSED end-to-end** (source 76594fd +
  review + behavioral); (b) X-clear output-stop PASS + (c) 2-layer isolation
  PASS → A1 closed; (d) Clip>Clear empty+autopilot-skip+undo PASS → A2 closed;
  (e) comp-panel FX drop + one-undo-entry PASS → lane B closed; (f) FilesBrowser
  instant+async thumbs PASS → lane C closed; (g) name-bar drag-move undo PASS →
  **item-3 self-closed YES**; (h) item-9 PARTIAL: single preset drop + Cmd+Z
  PASS; dragging group header "Energetic (9)" dropped ONE preset — Boris: "not
  sure what a playlist is" (discoverability gap too). NEW FINDING: "autopilot
  does not work for sources." Two Explore scouts dispatched (playlist-drop
  wiring at HEAD · autopilot-vs-sources root cause) — receiver-verify before
  any fix dispatch. Checklist rows dated in undo-v1-manual-e2e.md.
- 2026-07-30 PM22: **PLAYLIST-DROP DIAGNOSED — MODEL EXISTS, HEADER-DRAG NEVER
  WIRED.** Scout verdict (dossier: `.harmony/scout-playlist-drop.md`, HEAD
  972e8dd): full playlist stack is LIVE at HEAD — Clip model (Clip.h:122-150),
  "milkdrop_playlist:" drag flavor, ClipCell/MainComponent drop handlers with
  undoable edit, beat-synced playback (Renderer.cpp:259-351) — but a GROUP
  HEADER drag has no code path to it: mouseDrag does no hit-testing, falls to
  last-clicked SINGLE preset (exactly what Boris saw). Working gesture today:
  Playlist mode button → Cmd-click multi-select → drag ("MilkDrop Playlist
  (N)" cell). Dead decl `startDrag()` (MilkDropBrowser.h:155) = natural fix
  home; small scoped fix surface, drop side needs ZERO changes. SIDE FINDING:
  the 3 Playlist-mode controls (cycle/timing/blend) are DECORATIVE — never
  read; RandomBag+8-beats hardcoded (MainComponent.cpp:1124-1126). → BORIS
  DECISION: wire header-drag→playlist (+optionally un-decorate controls)?
  Scout idled silently once before delivering — SendMessage-nudge recovered it
  (standing contract rule re-confirmed). Autopilot-sources scout still out.
- 2026-07-30 PM23: **ITEM 8 CLOSED → GATE 8/8.** Boris ran the working playlist
  gesture (Playlist mode → Cmd-click multi-select → drag to cell) — PASS. The
  full 8-item behavioral gate is now CLOSED: every fix lane from the landmark
  sitting (crash-#2 UAF, A1, A2, B, C, item-3, item-9) is source-closed +
  reviewed + Boris-verified live. Still UNPUSHED (lane rule). OPEN from this
  thread: header-drag→playlist wiring + un-decorating the 3 playlist controls
  (Boris decision, recommended YES as one lane) · autopilot-vs-sources scout
  still out. Next-lane fork (Session Recorder default / ISF / visual-design)
  goes live once autopilot verdict lands.
- 2026-07-30 PM24: **AUTOPILOT-VS-SOURCES DIAGNOSED — "ADVANCE-GATE STALL"**
  (dossier: `.harmony/scout-autopilot-sources.md`, HEAD 972e8dd). NOT a skip,
  NOT a trigger no-op: autopilot has zero type filtering and sources render
  fine when triggered — but BOTH advance-away gates are unreachable for
  MediaType::Source. (a) End-of-Video mode: nothing ever writes a source's
  playhead (only writers are video-only, Renderer.cpp:931/:1001) → 0.0 vs 0.99
  threshold, frozen forever, 100% reproducible. (b) On-Beat mode: gate on
  clip->playing (Autopilot.cpp:68); sources born playing=false, the 4
  source-creation sites never set it (video paths do), and the
  hasBeenTriggered latch is NEVER reset → a source once-clicked-then-
  paused/cleared is permanently dead to autopilot (videos rescued by
  Renderer.cpp:934 per-frame resync). Fix surface small, 3 files, keyed on
  isPlayable() (also repairs Image+Camera in EoV mode); NO contact with
  clear-path/GL-fence hardening. Side notes logged separately: source clips
  all share id=0 (inert, latent) · autopilot bypasses handleClipTrigger →
  stale inspector after advance (cosmetic) · tests/ has NO autopilot unit
  tests at all. Scout idled silently pre-delivery AGAIN (2/2 despite explicit
  delivery clause) — nudge recovered; contract wording needs a fix at EOS.
  → BORIS: approve autopilot-fix lane (+ pending header-drag→playlist lane).
- 2026-07-30 PM25: **BORIS APPROVED BOTH LANES** (autopilot-sources bugfix +
  header-drag→playlist wiring incl. un-decorating the 3 playlist controls),
  SEQUENTIAL (both touch MainComponent.cpp). PLAN: Lane AP (autopilot) builder
  dispatched now per scout fix surface + first-ever test_autopilot.cpp → indep
  review → Lane PL (playlist) builder per scout-playlist-drop.md fix surface →
  indep review → ONE combined behavioral gate (Harmony) → Boris 60s replay.
  Commit per lane, NO push. Out of scope (logged, not bundled): id=0 latent ·
  stale-inspector-after-advance cosmetic · retrigger design ruling.
- 2026-07-30 PM26: **BORIS: "WORK ON ALL OF THEM NOW" — FULL QUEUE APPROVED
  (13 items).** Wave plan (build-lane cap 3 · ONE writer per file at a time ·
  commit per fix, NO batched commits, NO push · indep review per lane · ONE
  combined behavioral gate at the end + Boris replay list):
  W1 (now): AP autopilot fix (running; MainComponent-writer) · GRID lane =
  (8) fold-height mirror-index DeckView.cpp:274 + (6) rebuildGrid stale
  invisible selection (DeckView) · API lane = #4 marshal HTTP-thread model
  writes (ApiServer.cpp:399/:458).
  W2 (as slots free): PL playlist wiring (after AP; MainComponent-writer #2) ·
  ASAN infra lane (CMake sanitizer variant, build-system only) · then
  SHUTDOWN+MUTEX lane = shutdown bundle (detach-GL-first + EffectChain fence +
  :472 bounds-check) + (11) activeSources_ no-mutex UB, verified under ASan
  (Renderer/GL-writer).
  W3: MISC lane = id=0 rider + (5) menu enablement + (7) preview-animates-old-
  deck + B8 ReinspectTarget removal (MainComponent-writer #3, after PL) ·
  SYPHON lane (deps + publish module; Renderer-writer after SHUTDOWN+MUTEX).
  W4: FEAT lane = (D) retrigger-restart (built to Boris's RECORDED expectation
  — checklist line ~38 "Boris expects a restart") + mixed-drop image-discard +
  Cmd+X cut-to-clear + LayerStrip FX-drop (mirror 9c316e6 pattern)
  (MainComponent-writer #4, after MISC).
  Design defaults chosen by Harmony (flagged for Boris replay, not blocking):
  retrigger=restart-from-inPoint · LayerStrip drop→that layer's FX stack ·
  Cmd+X = copy+clear as ONE undo entry · mixed-drop keeps images. Syphon
  behavioral verify limited without a Syphon client — Boris-check item.
  Context guard: ~9% at plan time; wrap-with-handoff if 40% nears.
- 2026-07-30 PM27: **LANE AP LANDED** (201654a, disk-verified at HEAD; 4 files
  +47/-3-ish, 186/186 incl. 4 new TEST_CASEs/27 assertions in the repo's
  FIRST test_autopilot.cpp; full app target also compiled clean). Fix matches
  scout surface exactly: EoV frame-loop skips non-playable; beat-loop skip now
  `autopilotEndOfVideo && isPlayable()` (fall-through to beat advancement);
  playing=true at the 4 source-creation sites (fences untouched). Builder
  concerns (honest): tests-bite proven by manual trace not mutation; no
  behavioral run (mine). reviewer-autopilot-fix DISPATCHED (directed at bite
  check, fall-through beat accounting, hasBeenTriggered/clear-path
  interaction). LANE PL DISPATCHED (MainComponent slot freed): 2 commits —
  header-drag→playlist + un-decorate 3 controls (backward-compatible payload
  if extended). Build lanes at cap 3: GRID · API · PL.
- 2026-07-30 PM28: **LANE GRID LANDED** (a05d64d fold-height mirror-index +7/-2 ·
  37a1e12 rebuildGrid stale-selection +14/-0; both DeckView.cpp only,
  disk-verified, 186/186, app target builds+codesigns clean). Notable builder
  deviation (sound): stale selection fixed by DROP-invalid + resync visuals,
  NOT remap (no stable layer IDs across rebuild — remap = guessing). Coverage
  manual-gate-only (DeckView never in any test binary — pre-existing gap).
  Builder flagged in-flight ApiServer uncommitted changes in shared tree
  (expected — API lane, callAsync marshal visible). reviewer-grid-lane
  DISPATCHED (directed at mirror-convention proof, wrong-target-selection
  residual for Clip>Clear precondition, validation ordering).
  ASAN INFRA LANE DISPATCHED into freed slot: ADNA_SANITIZE option
  (address/undefined/thread variants), proof = full suite under ASan+UBSan,
  pre-existing findings LISTED not fixed. Build lanes at cap 3: API · PL ·
  ASAN. Reviews in flight: AP · GRID.
- 2026-07-30 PM29: **LANE API LANDED + EXTENDED** (f6b208f, +51/-48 ApiServer.cpp
  only, disk-verified, 186/186). Both flagged sites callAsync-marshalled per
  the 4-sibling house pattern. FLAGGED semantics change (consistent w/
  siblings): endpoints now return ok:true unconditionally — old sync error
  strings gone (grep: no in-repo consumers). Builder SWEEP found 6 MORE
  unmarshalled HTTP-thread writes → follow-up packet sent to the SAME warm
  builder (R3): effectChain_ field-write sites ONLY (:421, :705/:718,
  :841/:853/:865, reset's setEnabled loop). EXCLUDED by design: renderer_
  calls (:640 load_image w/ GL-sleep gotcha, :671 setActiveSource, :800-801
  clears) — logged as NEW QUEUE ITEM "renderer_-via-HTTP thread-safety design
  look". reviewer-api-marshal DISPATCHED — directed HARD at the builder's
  WRONG-AS-STATED `this`-lifetime claim in callAsync (teardown-UAF family
  history; assess real exposure vs pre-existing sibling pattern) + lost-
  validation semantics + repo-wide old-error-string consumers. GATE PLAN
  ADDITION (builder rec): live /api/set_param + /api/set_layer_opacity against
  rendering app + /api/composition readback.
- 2026-07-30 PM30: **LANE AP SOURCE-CLOSED** — review APPROVE-WITH-NOTES, 0
  blocking (201654a). Reviewer independently re-traced both loops old-vs-new:
  EoV freeze genuinely fixed; Video/ImageSeq EoV behavior verified
  byte-identical; beat accounting clean (beatsPlayed reset per trigger, frame/
  beat loops never share a clip's counter); 4 MainComponent sites = plain
  field writes, fence topology unchanged, clear-path unaffected (Clip::clear
  unconditionally sets playing=false). HONEST TEST GRADE: only TEST_CASE 1
  Section B truly discriminates pre/post-fix; TC3 CANNOT cover the
  MainComponent parity half (MainComponent linked into no test target —
  architecturally impossible headlessly, disclosed in-test); TC2 intentional
  non-regression; TC1-A/TC4 incidental coverage. → BEHAVIORAL GATE MUST COVER:
  (i) dropped source shows playing:true via /api deck-state (reviewer NOTE:
  ApiServer.cpp:288 now reports playing:true for never-triggered sources —
  visibility change, pre-existing pattern for Image/Video), (ii) live
  autopilot advancing OFF a source cell. NOTES logged: "repairs Camera" claim
  vacuous (no Camera creation path exists in src/ — aspirational). No fix
  round needed.
- 2026-07-30 PM31: **GRID REVIEW: a05d64d APPROVE (mirror convention proven
  across all 4 loops, boundaries clean, no un-mirrored reads left) · 37a1e12
  REJECT — BLOCKING wrong-target selection.** Layer reorder (Move Up/Down →
  Deck::moveLayer adjacent swap) shifts indices with size unchanged → drop
  predicate passes → selection silently names a DIFFERENT layer, highlight
  stays on the same screen row (mirror math = zero visual cue), feeds
  destructive consumers (kClipClear :3938, kClipReplaceContent :3985,
  source-drop-to-selected :1253). Mis-map SURVIVES undo/redo (UndoService:25
  → rebuildGrid). Add/remove can't shift at HEAD (last-only) — reorder is the
  one live vector. Reviewer also corrected builder claim: Layer HAS stable
  uint32_t id (Layer.h:31); CellPos lacks the plumbing. FIX ROUND dispatched
  to warm builder: minimal clearSelection on ALL reorder-induced rebuild paths
  incl. undo/redo direction (mirrors existing selectLayer re-point idiom).
  DEFERRED to queue: ID-based selection remap (selection survives reorder) —
  plumb Layer.id through CellPos. Directed-review posture validated: the
  nightmare case named in the dispatch is exactly what was found.
- 2026-07-30 PM32: **API FOLLOW-UP LANDED** (8077af7, +104/-62 ApiServer.cpp
  only, disk-verified, 186/186). All 4 in-scope effectChain_ sites marshalled
  (setParam global branch, setEffect, setEffectChain both loops, reset's
  disable loop); renderer_ calls + FeatureBus untouched per boundary; builder
  extracted request JSON to plain values BEFORE lambdas (no juce::var across
  threads — reviewer to verify per capture). Same ok:true semantics trade-off
  (no in-repo consumers of old error strings). reviewer-api-marshal scope
  EXTENDED to both commits — added checks: combined 6-site `this` exposure ·
  per-capture verification · sync-400 vs async-write coherence · reset's
  renderer-cleared-but-effects-enabled window. GATE ADDITIONS (builder rec):
  live set_effect / set_effect_chain / reset / set_param-global + readback.
  API lane build-complete; awaiting combined review.
- 2026-07-30 PM33: **LANE PL LANDED** (c491cfb header-drag→playlist, +85/-11
  MilkDropBrowser.* · 903b453 controls-made-real, +38/-2 incl. MainComponent
  drop-handler region; disk-verified, 186/186 re-verified at HEAD after
  interleaved lanes). Design notes: paths CACHED at mouseDown (no dangling
  PresetInfo*/wrong-Y re-derive); payload format UNCHANGED (controls queried
  at drop time — respects lane boundaries); dead startDrag() decl REMOVED;
  BUILDER DESIGN CALL flagged: header-drag works in ANY play mode (matches
  Boris's gesture; multi-select still requires Playlist mode) → Boris replay
  note. No new tests — juce_gui_basics headless boundary is a documented
  project convention (tests/CMakeLists.txt:261); behavioral gate covers
  gestures. reviewer-playlist-lane DISPATCHED — directed at gesture-state
  lifecycle (stale pressedSectionPresetPaths_ → later single drag emits old
  playlist = nightmare), toggle-then-drag seam, delimiter injection ('|' in
  paths), combo-id→enum mapping. PL builder later no-op'd a replayed task
  assignment correctly (disk-verify first — good pattern). Tree note: GRID
  fix-round WIP visible in MainComponent (clearSelection @ move handlers +
  refreshAfterUndoRedo signature) — expected, in flight.
- 2026-07-30 PM34: **API REVIEW (f6b208f half): APPROVE, 0 blocking.** The
  `this`-lifetime concern RESOLVED-SAFE by reviewer's end-to-end teardown
  trace through VENDORED JUCE source (not memory): MessageBase::post()
  refuses+destroys queued lambdas once quitMessagePosted flips; ~MainComponent
  runs only after dispatch loop exit; ApiServer::stop() joins all httplib
  workers (no handler mid-flight at destruction). Same net already protecting
  the 4 sibling endpoints; load-bearing assumption = ApiServer lifetime is
  1:1 with app (verified single start/stop sites). Bonus finding: lambdas
  re-validate deck/layer/clip/effect at EXECUTION time → structural-mutation
  races correctly no-op. Repo-WIDE grep (incl. design/docs/.harmony): zero
  consumers of removed error strings; visual test scripts hit TestServer:8080
  not :7070. ONE suggestion: builder's safety COMMENT misdescribes the
  mechanism → tiny comment-fix commit tasked to warm API builder. Reviewer
  proceeding to 8077af7 half per scope extension. [UPDATE: comment fix landed
  — 2d1744d, comment-only +15/-0, shared explanation at first callAsync site +
  pointers at the other 5; 186/186.]
- 2026-07-30 PM35: **GRID FIX-ROUND LANDED** (f924470, MainComponent.cpp/.h
  +42/-8, disk-verified, 186/186). Forward path: clearSelection at both move
  handlers. Undo/redo path: refreshAfterUndoRedo gained processedDescription;
  4 call sites capture undo/redoDescription BEFORE the call; clears iff
  "Move Layer Up"/"Move Layer Down" (builder traced: description read before
  index moves · MoveLayerCmd never merges · single-child push keeps string
  verbatim · strings constructed nowhere else). 6 paths covered; grep confirms
  undo/redo called from only those 4 sites. RE-REVIEW dispatched to warm GRID
  reviewer — directed at: uncovered reorder entry points (REST/OSC/MIDI?),
  the STRINGLY-TYPED description match robustness (grade: acceptable vs
  another round), capture-direction correctness, no regression to a05d64d.
  ALSO: ba0ae70 (ADNA_SANITIZE wiring) confirmed COMMITTED by ASan builder
  pre-idle — report still owed (nudged); SYPHON unblocks on that report.
  MISC LANE DISPATCHED (MainComponent freed): 4 commits — source-clip ids ·
  (5) menu enablement · (7) preview-old-deck · B8 ReinspectTarget removal.
  Lanes: ASAN(report-owed) · MISC · reviews GRID-fix + API-8077af7 + PL.
- 2026-07-30 PM36: **LANE PL SOURCE-CLOSED** — review APPROVE both (c491cfb ·
  903b453), 0 blocking; reviewer corroborated 186/186 with own rebuild+ctest.
  Verified safe: gesture lifecycle (pressedSectionPresetPaths_ cleared at top
  of EVERY mouseDown, single-instance, no stale bleed) · sectionAtY pre-toggle
  read provably order-independent (header y depends only on preceding
  sections) · combo-id→enum mapping exact (incl. blend 1.5s default parity) ·
  browser accessor lifetime safe · += refactor logic-neutral. NON-BLOCKING
  NOTES logged: (i) '|' delimiter injection PRE-EXISTING, surface widened by
  whole-section drops; (ii) sectionAtY/handleSectionHeaderClick duplicated
  walk = desync risk; (iii) toggle-fires-on-mouseDown = visual flicker seam
  during header-drag (cosmetic). BORIS REPLAY DECISION queued: mode-gate
  inconsistency — header-drag makes playlists from ANY mode, row multi-select
  still requires Playlist mode (gate header-drag too, or ungate multi-select).
  Source-closed lanes now: AP · GRID-c1 · PL · API-f6b208f-half.
- 2026-07-30 PM37: **GRID FIX-ROUND: APPROVE-WITH-NOTES → LANE GRID FULLY
  SOURCE-CLOSED** (a05d64d + 37a1e12 + f924470). Re-reviewer verified
  structurally: exactly 4 undo/redo call sites repo-wide (no bypass), zero
  MIDI/OSC/REST layer-move entry points, capture-before-call index math traced
  in UndoManager.cpp (undoDescription reads the exact slot undo() acts on),
  no regression to prior commits, and a beneficial ordering side effect (clip
  inspector now reads the cleared selection, not a wrong-target one).
  STRING-MATCH fragility graded acceptable-with-note: fails closed to the
  known bug (not novel breakage), no live trigger today. FOLLOW-UPS QUEUED
  (post-MISC, MainComponent busy): (a) Command::affectsLayerOrder() structural
  hardening (reviewer-designed, ~same diff size, rename+composite immune) —
  warm GRID builder; (b) ID-based selection remap (survives reorder); (c)
  marker: dead DeckView.h:48 onLayerReorder callback = uncovered 7th path IF
  ever wired (drag-reorder feature seam).
- 2026-07-30 PM38: **LANE ASAN DONE+ACCEPTED** (ba0ae70 Sanitizers.cmake +
  app/15-test-target wiring; 66d6b97 gitignore rider). Proof: 15/15 targets
  compiled under ASan+UBSan (4m32s), full suite 186/186 CLEAN — grep sweep 0
  sanitizer diagnostics; TSan configured + seqlock stress smoke clean; invalid
  combo guard fires. STRATEGIC FINDING: zero findings = current suite NEVER
  walks the teardown paths → shutdown lane packet requires NEW
  teardown-driving coverage (folded in). build-asan/ + build-tsan/ left ready.
  **SHUTDOWN+MUTEX LANE DISPATCHED** with explicit STALENESS TRIAGE
  requirement (scout plan predates 76594fd's teardown-semantics change — each
  piece re-validated before applying), 4-part verification bar (normal suite ·
  ASan suite · new teardown test or honest infeasibility · TSan for the map),
  house-pattern preference: confine+marshal over hot-path mutex.
- 2026-07-30 PM39: **LANE API FULLY SOURCE-CLOSED** — combined review: f6b208f
  APPROVE · 8077af7 APPROVE-w/notes, 0 blocking (10 callAsync `this` sites all
  under the same JUCE quitMessagePosted net; per-capture claim verified
  line-by-line — plain values only; 400-vs-async coherence clean; reset
  ordering artifact = few-frame cosmetic flicker, unobservable-by-construction
  in the worse direction). SUBSTANTIVE FINDING routed: Effect::enabled_ +
  EffectParam::value read EVERY FRAME on GL thread w/ zero sync vs all writers
  (pre-existing) → (i) advisory sent to shutdown builder (EffectChain-fence
  triage must cohere; report don't silently expand), (ii) queued
  renderer-thread-safety design pass UPGRADED to include effectChain_ (was:
  renderer_-via-HTTP only). Riders: 2d1744d comment fix landed; final rider
  sent — extend load-bearing single-lifecycle invariant note to all 10 sites.
  [UPDATE: bacda0d landed — pointers at all 4 pre-existing sites + invariant
  named; builder disk-verified the single-lifecycle claim by grep before
  writing (one ctor :1453, one start :1479, one stop :1601). API lane CLOSED:
  f6b208f · 8077af7 · 2d1744d · bacda0d.]
- 2026-07-30 PM40: **CROSS-LANE HANDOFF (clean escalation).** Shutdown builder
  confirmed piece 1 (detach-GL-first) STILL APPLICABLE at HEAD (previewPanel_
  :196 declared before effectsRackPanel_ :200 → destructs after → GL live
  during FX teardown) but the fix lives in ~MainComponent() — MISC lane's
  file. Builder correctly REFUSED the boundary cross and escalated with
  options while continuing pieces 2/3 + Task B. RULING: option (a) —
  one-liner relayed to warm MISC builder as its FIX 5 (own commit, shutdown
  bundle credited); exception NOT granted (two writers mid-edit in one file =
  live collision risk, already evidenced by API builder's transient compile
  error against MISC's WIP). Shutdown lane's final ASan verification must
  confirm the detach commit present at HEAD (report DONE_WITH_CONCERNS naming
  the gap if not). One-writer-per-file rule held under pressure.
- 2026-07-30 PM41: **10b68cb (shutdown lane, EffectChain mutex) BREAKS FULL
  BUILD + FALSE-GREEN TRAP.** MISC builder caught it (out-of-lane, disclosed
  not touched — correct): std::mutex member implicitly deletes EffectChain
  copy/move → test_mapping_engine.cpp:32 return-by-value fails to compile;
  `cmake --build .` FAILS at HEAD while ctest reports 186/186 off a STALE
  July-17 pre-mutex binary. Disk-verified. DIRECTIVES to shutdown builder:
  (1) fix the test helper NOW (do NOT add copy/move to a mutex-holding class);
  (2) PERMANENT: full clean build of all targets before ANY ctest claim —
  doubly for build-asan/ (its binaries predate all lane commits — rebuild or
  the ASan verification is meaningless); (3) design accountability OWED per
  packet: piece mapping · mutex-vs-confinement justification · GL hot-path
  cost per frame · why the advisory expansion wasn't reported pre-commit.
  Learning logged (log-event: stale-binary-false-green). Also visible at
  HEAD: 1f5442e = bundle piece 3 (EffectsRackPanel bounds-check).
- 2026-07-30 PM42: **MISC LANE 4/4 BUILD-COMPLETE** (ca1fc5c source ids —
  incl. necessary s_nextClipId decl relocation, grep-verified no id==0
  consumers · db9e8bd menu gating via hasClipSelection callback mirroring
  existing pattern, reads selection live per menu open · d4f5d86 preview
  deck-switch reconcile — root cause: handleDeckSwitch skipped the documented
  refreshPreviewFromActiveClip ownership rule; single shared entry point
  covers tab/REST/OSC/MIDI/genre paths · ca068e4 B8 removal, net -38 lines,
  verified-dead-first, incl. setCollaborators signature trim + 43 mechanical
  test-call updates). Per-commit isolated verification (revert-build-test-
  reapply, no stash). FIX 5 (relayed detach one-liner) crossed with the DONE
  report — confirmation requested, still owed. MISC review will cover all 5
  commits together once fix 5 lands.
- 2026-07-30 PM43: **MISC LANE 5/5 COMPLETE** (cc5c0c3 detach-first landed —
  exact members matched at HEAD, idempotent per JUCE detach semantics, +12/-0).
  reviewer-misc-lane DISPATCHED over all 5 commits — highest care on cc5c0c3
  (teardown ordering × 76594fd lazy-re-init interaction) + independent
  dead-code re-verification for B8 + id-serialization collision check for
  ca1fc5c. **FEAT LANE DISPATCHED** (MainComponent freed): LayerStrip FX-drop
  (mirror 9c316e6) · mixed-drop keeps images · Cmd+X cut-to-clear
  (clipboard-world investigation first; smallest honest thing) · retrigger-
  restart (Boris's recorded expectation; BLOCKED-COLLISION rule if Renderer
  needed while shutdown lane owns it). Shutdown lane task board: triage +
  Task B done, Task A pieces in progress, teardown tests + 4-part verification
  pending. Remaining queue after FEAT: SYPHON (Renderer-gated) ·
  affectsLayerOrder hardening (MainComponent-gated, after FEAT) · ID-remap ·
  renderer-thread-safety design pass (incl. effectChain_).
- 2026-07-30 PM44: **MISC REVIEW: 4× APPROVE + d4f5d86 APPROVE-W/NOTES with
  ONE BLOCKING scope gap** — genre auto-switch (setOnGenreChanged,
  MainComponent.cpp:592-607) bypasses handleDeckSwitch → preview bug still
  LIVE on that path; "applies uniformly" claim false. Fix relayed to FEAT
  builder (owns MainComponent) as appended item 5: route callback through
  handleDeckSwitch. Verified clean elsewhere: ca1fc5c id-serialization hazard
  is PRE-EXISTING (video path identical; no load-time reconciliation exists —
  noted, not new) · db9e8bd enablement byte-identical to all 3 handler guards,
  menu built fresh per open · ca068e4 dead-code claim independently re-proven
  at parent commit · cc5c0c3 verified against VENDORED JUCE SOURCE end-to-end
  (detach no-op-when-detached; synchronous GL-thread removal + context-current
  closing before return; 76594fd lazy-reinit unreachable post-detach; stray
  execute() fails safe). NEW QUEUE ITEM from review: OutputWindow's SECOND GL
  thread shares EffectChain by reference (OutputWindow.h:7,14-16,45) —
  verified not a live teardown gap today (reset at :1685 precedes, dtor
  detaches synchronously) but the output-window path has never had
  crash-family scrutiny → separate scout item queued.
- 2026-07-30 PM45: **PRE-EXISTING DEBUG-ONLY COMPILE BUG surfaced by sanitizer
  variants** (shutdown builder, boundary held): OutputWindow.cpp:273 unqualified
  addAndMakeVisible fails under ANY Debug build (JUCE_DEBUG-only ResizableWindow
  overload name-hides Component's) — invisible until ba0ae70 made Debug real.
  One-liner (Component:: qualify) routed to warm ASan-infra builder w/ verify:
  AudioDNA under build-asan (completes app-level sanitizer bonus) + Release
  intact. Shutdown lane meanwhile verified its OWN work honestly: 188/188
  test-target-only under ASan clean (2 NEW tests — teardown coverage growing),
  app-level ASan gate pending the one-liner. FEAT lane progress: LayerStrip
  FX-drop + mixed-drop DONE, Cmd+X in progress, retrigger + genre-relay
  pending.
- 2026-07-30 PM46: **FEAT LANE 4/4 BUILD-COMPLETE** (8f41bd9 LayerStrip
  FX-drop — mirrored 9c316e6's CONCEPT not letter (LayerStrip has no embedded
  EffectStackView; used ClipCell's forward-a-callback shape into the same
  EffectStackCmd/EffectScope::layer machinery, GL fence honored) · 4ba9748
  mixed-drop — new onMixedFilesDrop combined callback so image+videos land as
  ONE composite undo entry · 814f633 Cmd+X — investigation: NO clipboard at
  HEAD but kClipCut/Copy/Paste enum values RESERVED-unwired in MenuBarModel.h:
  76-80; smallest honest thing = Cmd+X as second shortcut on existing
  Clip>Clear · b391b64 retrigger-restart — root cause Layer.h:225-235 resets
  model playhead but never seeks the PLAYER; fixed in handleClipTrigger as
  sibling to beat-snap seek block, ZERO Renderer edits, no undo entry per
  trigger-path doctrine). test_undo_commands 355/355 after each. ITEM 5
  (genre relay) crossed with report — task board shows it in_progress now.
  BORIS REPLAY MINI-RULINGS queued: column-trigger retrigger parity? ·
  future Cut/Copy/Paste (enums already reserved)? ALSO VISIBLE at HEAD:
  shutdown lane's f0916d1 "restore move semantics after mutex addition" —
  possibly the approach my directive cautioned against (vs fixing the test
  helper); judgment held for its report + reviewer scrutiny; 22fcedc Task B
  = CONFINEMENT (preferred design) chosen.
- 2026-07-30 PM47: **SHUTDOWN+MUTEX LANE BUILD-COMPLETE, DONE_WITH_CONCERNS**
  (10b68cb EffectChain mutex+idempotency — BOTH required: mutex for
  first-population race vs 10Hz timer, guard for silent effect-duplication on
  context recreation, a LIVE path post-76594fd · 1f5442e :472 bounds-check
  mirroring siblings · f0916d1 move-semantics restore (directive said fix
  test helper instead — justification NOT provided; reviewer to adjudicate on
  merits) · 22fcedc activeSources_ CONFINEMENT: GL-thread-owned,
  getCurrentContext dispatch, non-GL callers block via executeOnGLThread,
  zero MainComponent/TestServer edits). Verification: 188/188 normal ·
  188/188 ASan test-scope 0 diag · teardown test honestly infeasible headless
  (app-level gate recipe supplied: browse→preset→playlist→graceful-quit→no
  .ips, "OpenGL Renderer" absent from traces) · TSan 187/188. NEW PRE-EXISTING
  FINDS routed: FeatureBus buffer-slot-aliasing race (test_feature_bus.cpp:
  143 vs :161, TSan-confirmed, real not harness) → queue triage item ·
  OutputWindow Debug break (already in flight w/ infra builder).
  **REVIEWER DISPATCHED with a LOAD-BEARING CONTRADICTION to resolve:** MISC
  reviewer read JUCE execute() as no-op-when-detached; shutdown builder says
  executeOnGLThread HANGS without attached context — likely DIFFERENT
  mechanisms (JUCE's vs Renderer's custom queue+WaitableEvent). If the custom
  marshal hangs post-detach, an HTTP handler calling getOrCreateSource during
  app quit = DESTRUCTOR HANG — would defeat the lane's own crash-family goal.
  Reviewer must resolve ground truth + teardown-order window at HEAD.
- 2026-07-30 PM48: **FEAT LANE 5/5 COMPLETE** (6b9c831 genre→handleDeckSwitch:
  documented-intent gap per the handler's own comment; strict-superset claim +
  non-undoable guarantee source-traced; callback already callAsync-marshalled).
  reviewer-feat-lane DISPATCHED over all 5. GRID-HARDENING follow-up packet
  sent to warm GRID builder (affectsLayerOrder structural check replacing the
  string match). **SYPHON DEFERRAL DECISION (drain doctrine):** context ~26%,
  remaining pipeline = 2 reviews + possible fix rounds + hardening + infra
  report + COMBINED BEHAVIORAL GATE + replay + EOS ≈ lands near the 40%
  off-ramp. Syphon = the one remaining net-new feature, medium lane, AND its
  behavioral proof needs Boris at the machine with a Syphon client regardless
  → DEFERRED to next session as #1 Committed MUST (START HERE marker at EOS)
  unless Boris overrides. 12 of 13 mandate items land verified this session.
- 2026-07-30 PM49: **GRID HARDENING LANDED** (43ff194, 7 files +69/-19):
  Command::affectsLayerOrder() default-false · MoveLayerCmd true ·
  CompositeCommand aggregates · 4 call sites swap string capture for bool
  capture, same capture-before-mutate shape · refreshAfterUndoRedo takes the
  flag. DISCLOSED BOUNDARY EXTENSION (accepted pending review): UndoManager
  .h/.cpp gained undoAffectsLayerOrder()/redoAffectsLayerOrder() mirrored on
  the existing description-peek pattern — only non-invasive way to expose the
  flag (alternative = shadow history in MainComponent, rejected as two-sources-
  of-truth). 188/188; accessors compile in headless test build too. Sent to
  the designing reviewer for verify + boundary adjudication.
  [UPDATE: APPROVE — aggregation walks real children_ w/ recursive virtual
  dispatch; 17 Command subclasses swept, only the 2 needed overrides exist;
  accessor index math re-proven against unmodified undo()/redo(); boundary
  extension ADJUDICATED CORRECT (mirrors the class's own peek pattern; the
  avoided alternative = the same two-sources-of-truth shape this chain exists
  to kill). GRID CHAIN FULLY CLOSED: a05d64d · 37a1e12 · f924470 · 43ff194
  (+cc5c0c3 relay).]
- 2026-07-30 PM50: **SHUTDOWN LANE SOURCE-CLOSED — 0 blocking at HEAD.**
  CONTRADICTION RESOLVED w/ vendored-source proof: executeOnGLThread IS
  execute() (juce_OpenGLContext.h:401); no hang path exists — CachedImage::
  stop() DRAINS the work queue before pause(); pendingDestruction/null-cached
  → immediate nullptr no-op. **cc5c0c3 introduces NO quit-hang** (dtor order
  traced at HEAD; TestServer:893 null-checks → graceful 500; httplib stop()
  joins all workers). Verdicts: 10b68cb APPROVE (lock released before GL
  draw loop; idempotency correctly Renderer-scoped vs context-scoped) ·
  1f5442e APPROVE · 22fcedc APPROVE (thread_local dispatch proven on all 3
  caller classes; inline helper structurally unreachable off-GL) · f0916d1
  MOOT — builder SELF-CORRECTED pre-review (05114eb revert + out-param helper
  across 13 call sites per original directive + cbeb287 honest deferred-
  boundary comment); reviewer confirmed directive right on merits (latent
  double-ownership of prevFrame GL handles in the move impl). FeatureBus race
  confirmed pre-existing (45ae7e8-era). RIDER sent: comment-truth fix in
  test_renderer_source_confinement.cpp (false hang claim committed as
  rationale; test = honest pattern-simulacrum, not Renderer proof). GATE
  ADDITION: live HTTP set_preset against running app (closes the unit test's
  admitted gap).
- 2026-07-30 PM52: **FEAT LANE SOURCE-CLOSED — 5× APPROVE, 0 blocking.**
  Depth highlights: Cmd+X focus safety traced through vendored JUCE dispatch
  (focused TextEditor consumes Cmd+X before MainComponent sees it; handler's
  own selection guard covers the empty case independent of menu graying) ·
  one-undo-entry claims VERIFIED from command construction (single before/
  after snapshot pair; composite undo iterates in REVERSE so columns shrink
  after cells clear) · retrigger no-undo proven from LayerRuntimeSnapshot
  field set · strip targeting correct under folds (mirrored model index) ·
  genre superset claim diffed body-vs-body (only new effect = the reconcile).
  FLAGS: (i) NOTE latent read-only-TextEditor Cmd+X leak — unreachable today
  (zero setReadOnly in src/), forward guard only; (ii) PRODUCT CALL →
  **Boris replay decision list**: Source/MilkDrop clips still NO-OP on
  retrigger (their time base is app-init-scoped; commit honestly Video/
  ImageSeq only) — does Boris want restart semantics for sources too?
  **BORIS REPLAY DECISION LIST (accumulated):** 1. sources-retrigger-restart
  scope · 2. column-trigger retrigger parity · 3. Cut/Copy/Paste suite
  (enums reserved) · 4. playlist mode-gate consistency (header-drag any-mode
  vs multi-select Playlist-only) · 5. Syphon deferral confirm.
- 2026-07-30 PM53: **REVIEWER SELF-CORRECTION — THE HANG WINDOW IS REAL,
  BLOCKING on cc5c0c3's ordering.** Builder pushback → reviewer re-derived
  from scratch: CachedImage::stop() does a ONE-TIME workQueue empty-check; an
  add() landing between that check and RenderThread::remove()'s
  setSafe(false) is stranded forever (renderFrame bails via isListChanging
  without draining) — permanent WaitableEvent hang, JUCE-inherent, made
  REACHABLE by cc5c0c3 (pre-cc5c0c3 the servers stopped long before any
  detach). PIPELINE WAS AHEAD OF THE VERDICT: both endorsed fixes already
  dispatched pre-correction — structural reorder (servers-stop-before-detach,
  MISC lane, in flight) + isAttached() guard (in-lane, green-lit). Reorder
  verified NOT to reintroduce the SIGBUS class (server stop = I/O join, not
  UI teardown; GL still detaches before all UI teardown). Comment-rider
  wording updated to adjudicated truth. PROCESS NOTE for EOS learnings:
  adversarial pushback against an APPROVE verdict, argued from source,
  produced ground truth BOTH initial readings missed — and the cheap
  both-mitigations ruling made the flip cost zero schedule.
- 2026-07-30 PM54: **HANG-WINDOW FIXES LANDED + PROCESS INCIDENT (no loss).**
  84092b0 comment-truth · ff19094 isAttached() guard AND (accidentally swept
  via shared git index) MISC's servers-stop-before-detach reorder — the
  structural hang fix IS at HEAD, byte-for-byte as its author staged it, but
  under ff19094's misleading banner (shutdown builder disclosed immediately;
  NO history rewrite in shared local history — THIS LEDGER ENTRY is the
  authorship record: ff19094 = shutdown guard + test comment + MISC-authored
  reorder w/ its own "Shutdown bundle piece 2" comment block). Author-confirm
  requested from MISC builder. Learning logged (shared-index-commit-sweep:
  plain `git commit` commits the whole index — use --only). Suite 188/188
  incl. the reorder. Remaining before gate: MISC author-confirm · infra
  ASan-app report (OutputWindow one-liner uncommitted `M` visible in tree —
  still building/verifying). BORIS mid-turn directive: run EOS when the task
  finishes — gate → replay list → eos-secondary.
- 2026-07-30 PM55: **COMBINED BEHAVIORAL GATE: PASS (API+lifecycle scope).**
  Fresh Release binary at HEAD (deleted-then-rebuilt, codesigned 16:38).
  Launch clean, health ready 118-120fps/135 effects. LIVE API BATTERY under
  active render, all PASS w/ readback: set_layer_opacity (0.42 landed →
  restored 1.0) · set_effect (Perspective Tilt enabled:true) · reset (ALL
  effects disabled; NOTE: reset takes ~5.0s pre-existing GL-wait — first curl
  hit its own 5s limit, NOT a hang; retried and verified) · switch_deck ·
  fps stable throughout. **CAPSTONE PASS: graceful quit UNDER CONCURRENT API
  LOAD (12-call burst poking the fixed shutdown race) → clean process exit
  ~6s, ZERO new .ips vs baseline** — crash family AND hang window
  behaviorally clean. Synthetic-click scenarios NOT run (Boris at machine —
  preflight doctrine); gesture items → Boris replay list. Fresh instance
  PID 35688 left running at 120fps for Boris. Gate scope honestly split:
  API/lifecycle = mine, gestures = Boris replay (list in PM52+PM55-adjacent
  report).
- 2026-07-28: SITTING crash #2 (separate subsystem): SignalBar arrow → layout
  cascade → MilkDropBrowser::getCuratedPresets null-deref (empty preset state;
  .ips 2026-07-28-190701). Queued as post-lane mini-lane candidate bundled with
  Boris's default-preset-dir ask. Boris nod pending.
- BORIS DECISION QUEUE (still open): (1) ratify the refreshAfterUndoRedo
  pointer/scope-aware skip follow-up (fixes expanded-row collapse + deck-tab
  highlight class); (2) is zero-layer raw Deck-New intended?; (3) undo-with-no-
  cell-selected clears clip inspector (carried from s. 2026-07-19, UX check —
  fold into the manual e2e sitting); (4) NEW 2026-07-30: marshal the two
  HTTP-thread model writes (ApiServer.cpp:399 set_param clip branch, :458
  set_layer_opacity) onto the message thread via callAsync like the other four
  endpoints — unsynchronized-concurrent-write latent bug, field-write class,
  tiny fix, post-lane candidate (scout-flagged, cited).

## Queued non-lane items (from handoff, deferred while build lane occupies build dir)
- FIRST (env, Boris-level — ROOT CAUSE KNOWN 2026-07-25): click **Allow** on the
  Audio-DNA microphone TCC prompt (dialog is on screen now; app left running).
  NOT a coreaudiod wedge — do not killall/reboot. NOTE: ad-hoc signing re-fires
  the prompt after EVERY rebuild, so each first-launch-after-rebuild needs one
  Allow click until a stable signing identity is adopted (see gotchas.md
  2026-07-25 entry). Required before ANY app-level manual check below can run.
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
- Step-8 manual e2e ADDITIONS (all Boris-assisted — TCC Allow click needed first): trigger
  a cell → Cmd+Z restores previous active clip + crossfade state; MASH several cells on
  ONE layer → a single Cmd+Z undoes the whole run (merge); trigger cells on TWO layers →
  two undo entries (no cross-layer merge); column trigger → Cmd+Z restores ALL
  non-ignoring layers at once, ignoring layer stays put; retrigger the already-active
  cell → history does NOT grow (check Edit menu); REST/OSC/MIDI trigger → DOES appear in
  undo history (spec row 8 — user-initiated remote); autopilot triggers → NEVER appear in
  history; KNOWN LIMITATION (documented, risk-#5 family): first-ever trigger → undo →
  re-trigger = clip goes active but skips auto-play.
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
