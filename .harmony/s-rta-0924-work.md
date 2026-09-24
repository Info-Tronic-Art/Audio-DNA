# s-rta-0924 work log (secondary)
- boot: handoff s-rta-0923 read; screen-safety law read. Tree: .harmony-version modified (not ours), AGENTS.md untracked (not ours) — leave.
- #1 comp-position units bug: builder dispatched (TDD, scratch dir fail-first). Gate = probe-lane3.sh COMP check (expect 12/12 after).
- #2 step 3: architect critic dispatched -> .harmony/.reports/s-rta-0924/step3-critic.md (S3-M superseded by Lane 3 ManualWrite seams).
- rig note: MINIMAL-profile read-only REPORT_FILE must be under <repo>/.harmony/.reports/ (dispatch gate blocks memory/.reports/ in a foreign repo).
- Boris 2026-09-24: Bluetooth headset crash re-test DEFERRED ('another time, not that important') — stays on loose-ends as low priority, not owed this session.
- #1 builder DONE: ScalarMath::posPxToCompUniformX/Y (px/1920, px/1080), Renderer applyCompTransform uses them for position+anchor; new test_comp_transform_units (5 cases). ctest 362/362 (builder-reported).
- #1 HARMONY LIVE GATE: probe-lane3.sh 12 PASS / 0 FAIL (was 11/1). COMP frames no longer black: comp_a mean 77 / black band right 8.1% cols; comp_b black band left 2.9% -> real horizontal shift. Looked at comp_a.png. App gone, 0 Output windows. Reviewer pending -> then commit.
- #2 critic DONE: BUILDABLE-WITH-FIXES, 5 blocking, A1-A8 adopted; banner added to plan. Report .harmony/.reports/s-rta-0924/step3-critic.md.
- step3 orchestration: W1 workflow = lanes S3-A (recording), S3-C (REST), S3-G (gate assets) in worktrees, each builder->reviewer->1 fix round. Harmony merges, builds, ctest. W2 = S3-B (MainComponent) + S3-D docs. Then Harmony runs probe-step3.sh.
- W1 workflow launched: run wf_4ad45345-ed0 (task wnyofy2y4). Script under ~/.claude/projects/.../workflows/scripts/rta-step3-wave1-*.js. On completion: merge lane branches to main in order C, A, G; one clean build; ctest; then W2 (S3-B + S3-D).
- W1 DONE: S3-C a2c5b50, S3-A c737d3d, S3-G 57ffe74+ae2eb17 (1 fix round) all reviewer-APPROVE; merged bacc6c3/615473c/fa67f0a; build rc0, ctest 377/377 (Harmony-run). Pushed. W2 = S3-B MainComponent next; then Harmony live probe-step3.sh; then S3-D docs.
- W2 launched: run wf_3879ebde-fa2 (task w3ufsvhn2) — S3-B builder + 2-lens review (spec, threads) + <=2 fix rounds. Then merge, build, ctest, Harmony runs .harmony/probe-step3.sh live, then S3-D docs.
- Boris 2026-09-24: 'keep working till you hit 50% ctx then eos' — off-ramp = 50% [CTX] gauge; then eos-secondary. Parallel while W2 runs: R13 (non-48k device) plan by architect (read-only).
- W2 DONE: S3-B b5931e8, both reviewers APPROVE r1; merged f6198da; build rc0; ctest 377/377 (Harmony). Deviation to rule on: LayerTransport play now resets clip->reverse (applyClipPlaying consolidation).
- LIVE GATE probe-step3.sh run 1: FAIL on probe string-compare "48000.0"!="48000" (probe defect) -> Harmony added normnum() to .harmony/probe-step3.sh.
- LIVE GATE run 2: 51 PASS / 5 FAIL. PASS: arm, provisional take, 4 triggers, set_bpm, periodic save, stop, store asset+sidecar, take v3, seg rate 48000, fp1, tempo 12800, load Resolved, wallClock replay reaches 3, overdub safety (1 point, same asset, 1 asset), screen law. FAIL analysis (take ~/Documents/Audio-DNA/Takes/step3gate1.adna-take):
  (a) opacity 3 gestures x 2 breakpoints: CORRECT product behaviour (3 REST writes 1 s apart, decaying hold 250 ms each) -> probe expectation wrong.
  (b) replay order_ok=0: first sample (col 2) is leftover pre-replay state; recipe ends with trigger_column 2 so true seq is 0,1,2,3,2 -> probe expectation wrong, BUT checkpoint0 (col -1, opacity 1.0) apparently NOT restored at replay start -> suspect.
  (c) replay opacity never >0.2 -> continuous replay not landing? suspect real.
  (d) markers=0 -> T2 alignment NA; onset markers not recorded? suspect.
  (e) checkpoint0.bpm=0.0 at arm (no tempo yet) -> note.
  -> diagnosis agent dispatched (live app, no src edits).
- RULING (Harmony): LayerTransport play must NOT reset clip->reverse (a pad pause/play on a reversed clip flipping it forward is a live-performance regression). TopBar Play keeps resetting reverse. Fold into the post-diagnosis fix round (applyClipPlaying gets a 'resume' variant that leaves reverse alone).
- R13 plan DONE (.harmony/.reports/s-rta-0924/r13-plan.md): resample to 48k on analysis thread, bypass at 48k, band gating + bandValidMask, rateMismatch retires -> rateChangedSinceArm. Now: workflow critic -> lane B -> lane A (same worktree). Lanes C (RecorderHost) + D (MainComponent/probe) WAIT for step-3 fix-round merge.
