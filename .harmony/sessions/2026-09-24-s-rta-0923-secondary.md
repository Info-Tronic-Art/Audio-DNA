# Session log — s-rta-0923 (2026-09-23 21:10 → 2026-09-24 ~01:30, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE (only idea-capture into
this repo's ledger; 2 learning rows to Harmony_Main's transient event log — a lane-B slip, see loose ends).
Boris directives: boot secondary, validate, check graphify, work autonomously to 55% ctx, use workflows;
mid-session: "after these tasks run eos, don't start new tasks".

| type | ref | msg |
|---|---|---|
| shipped | e3504be | inbox drain: codegraph-rta removed, .gitignore stamp visibility, ledger prose moved (507/507 lines), Ruling-28 specs |
| shipped | 8454e07 | Julia Set shader compiles again (duplicate diveRate since e3b0b35) — live app: 0 shader FAILED |
| shipped | 4898e41 | EnvelopeSignal default-mode sibling cases (addendum 5) |
| shipped | c923435 | Program::compile uses exact per-breakpoint stamps (review fix b) |
| shipped | 601ad4d | touch() no longer discards an open gesture (addendum 4a); fail-first re-proven by Harmony |
| shipped | 87b0ea2 | RecorderClock periodic anchors (fix c), Player backwards seek (addendum 2), Latch refuses loudly (4b) |
| shipped | 33bdc67 | Ruling 28: shared AudioStore, take format v3, WAV header flush every 10 s, fromV1Var TransportChange (fix a) |
| shipped | 9257537 | AudioTap OOB read on mono/short-channel devices fixed; test_bt_device_shapes ASan fail-first |
| shipped | dd646de | Record panel honest interim (disabled + dimmed + tooltips), 4-seat critic panel folded |
| shipped | 9da23b7..7b1071f | Lane 3 connection binding C0-C4: manualWrite funnel, renderer eff(), app tick + 11 writer sites, 21 inspector controls |
| gate | ctest | 306 at boot (clean rebuild) → 357/357 on clean forced rebuild after Lane 3 |
| gate | live | totalBarCount live 2→8 bars/10 s; Lane 3 oracles A+B PASS (layer opacity follows 1-beat LFO, pixels follow); CLIP PASS |
| gate | live | Lane 3 probe final 11 PASS / 1 FAIL: GRIP + window checks were probe bugs (fixed); COMP = pre-existing comp-position units bug (pixels into a -1..1 shader → solid black), next session #1 |
| finding | crash | 3 startup crashes with Bluetooth soundcore P31i (16 kHz); 0/9 once disconnected; NOT the AudioTap bug (tap idle at startup); open |
| finding | graph | graphify graph was 0 nodes since 09-07; root cause proven (wrapper delete-before-extract + hook incremental into void); rebuilt 7254N |
| decision | plans | Lane 3's ManualWrite is THE funnel (R8); step-3 plan's ManualWriter superseded; step 3 deferred to next session START HERE |
| learning | failfirst-probe | never run a revert/restore fail-first probe in the gate build dir — it manufactured a false deterministic FAIL (stale test_take.o, ODR layout) |
| learning | lookandfeel | setEnabled(false) does not dim buttons in this app — verify disabled states from a screenshot (repo gotcha filed) |
| learning | cmake-merge | multi-hunk 3-way conflicts in tests/CMakeLists.txt interleave targets — rebuild the file as HEAD + appended block instead |
| slip | log-event | lane-B session wrote learning rows to Harmony_Main's event log; skill says own notebook — copied to notebook.md |
| slip | commit | L5.* wildcard committed an 8044-line build log (removed next commit, stays in history) |
| slip | pgrep | my own wait loops embedded "MacOS/Audio-DNA" and self-matched the probe pgrep check (gotcha S22) — repo gotcha filed |
