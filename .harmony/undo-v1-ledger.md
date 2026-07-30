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
