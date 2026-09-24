# Session log — s-rta-0924 (2026-09-24 ~05:20 → ~10:30, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: 3 `.pending`
defect reports via coord_pending_write (report-fence mismatch, foreign builder report blocked by battery lock, G5 relay
result + reviewer Bash-bypass). Boris directives: "use workflows as much as you want"; "bluetooth test another time, not
that important"; "keep working till you hit 50% ctx then eos". Running log: .harmony/s-rta-0924-work.md.

| type | ref | msg |
|---|---|---|
| shipped | ef84a55 | composition position/anchor no longer render solid black (px/1920, px/1080 into comp_transform); probe COMP 12/12 |
| shipped | 12e607e | step-3 critic (BUILDABLE-WITH-FIXES, A1-A8 adopted; ManualWriter lane deleted) |
| shipped | a2c5b50 c737d3d 57ffe74 ae2eb17 | step 3 wave 1: /api/perf/* REST, RecorderHost+PerfStateCapture, probe-step3 gate assets |
| shipped | b5931e8 | step 3 wave 2: RecorderHost wired into MainComponent (onManual* hooks, dispatch, tick, REST callbacks) |
| shipped | 0f4ff08 | LayerTransport pad toggle keeps clip reverse (Harmony ruling; "resume" action) |
| shipped | 33295d2 | recorder false "tap self-stopped" at arm fixed (optimistic seed); diagnosis claim 2 disproven by test |
| shipped | 7fe16b5 → db72d56 | onset markers: duplicates removed, then lost pulses fixed via monotonic FeatureSnapshot::onsetCount |
| shipped | 546b66c c69d23a 7db6dc6 008dbc1 1e0f893 | R13: plan, spectral gating + bandValidMask, analysis resampler, rateChangedSinceArm, resampler LIVE + docs |
| shipped | 2b0168c 17f61f0 2ce3389 03c1b90 514976f | probe-step3 fixes: numeric compare, replay loops, opacity rows, asset delta, click noise floor + headroom, T2 grid pairing |
| gate | ctest | 357 at boot → 406/406 (Harmony-run after every merge) |
| gate | live | probe-lane3 12/0 (was 11/1); probe-step3 63 PASS / 0 FAIL on main (onset coverage 100.8%, drift -0.21 ms, R13 rows PASS) |
| finding | onset | aubio drops an onset whose confirming hop is true digital silence (fixture needs a noise floor; real audio fine) |
| finding | onset | render uniforms (CompositorEngine:1575, EffectChain:340, ProceduralSource:175) + GET /api/features still read one-hop onsetDetected → miss beats |
| finding | harmony | reviewers mutate src via Bash despite MINIMAL write BLOCK (tester-isolation.log rows 11:45/12:03); reported up |
| decision | reverse | LayerTransport play must not reset reverse (TopBar Play still does) |
| decision | r13 | resample to 48 kHz on the analysis thread; rateMismatch retired → rateChangedSinceArm |
| learning | receiver-verify | an agent reported the opacity rows fixed; lines unchanged — always re-read the file after a "done" |
| learning | skip-is-not-pass | probe in a worktree without .venv SKIPped T2 + window rows and still printed N/0 — compare row counts run to run |
| learning | inferred-root-cause | "1-sample impulse" onset cause was inferred, not tested, and was wrong — demand instrument-and-run for every root cause |
| slip | packet | self-stop diag agent attached lldb → Touch ID dialog → Escape hit another Harmony terminal; my packet didn't forbid it (gotcha filed) |
| shipped | f95c216 | retired RecorderHost rate shims, onset cap/wrap tests, probe FAILs (not SKIPs) without .venv — reviewer APPROVE |
| gate | final | main after all merges: build rc 0, ctest 408/408, probe-step3 63 PASS / 0 FAIL, no Audio-DNA process, screen looked at |
