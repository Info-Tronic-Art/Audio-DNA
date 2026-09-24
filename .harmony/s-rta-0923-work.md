# s-rta-0923 work index (secondary, RealTimeAudio) — live state, externalized before delegation
Boot: HANDOFF s168 + s171 read; inbox 11 items (5 SENT at boot). Off-ramp: 55% ctx (Boris directive).
DONE: graph rebuilt 0 -> 7254N/11265E (graphify update ., code-only, 10s). Root cause of 0-node graph PROVEN:
  wrapper deleted graph.json (20:40 run, rc=1 semantic stage) -> post-commit hook at 22:20 (f5ae847, 1 doc file changed)
  did an incremental rebuild into the void -> "Rebuilt: 0 nodes" (~/.cache/graphify-rebuild.log). Hook kept.
DONE: codegraph-rta removed (.mcp.json, knowledge-tools.yml, disabled list); .gitignore stamp visibility; inbox statuses.
PENDING-CAPTURE (after triage lane finishes idea-ledger.md): idea-capture up-channel — graphify-refresh-launchd.sh
  delete-before-extract + semantic-stage rc=1 with no stderr => leaves void; hook then writes 0-node graph. Fix: extract to
  temp and swap, or fall back to code-only on semantic failure; capture stderr.
IN FLIGHT: wf_731e1a05-c0f design (ruling28 format spec, review-fixes plan, ledger triage, critic).
IN FLIGHT: clean rebuild + ctest (validation row 1) bsps976u3.
NEXT: validation rows 2-3 (ctest -R), row 4 live totalBarCount (production launch, NO output window).
NEXT: build workflow from the two specs (own -B dirs per lane), reviewer per lane, my gate.
ASK BORIS: re-enable clangd-rta/graphify-rta in .claude/settings.local.json? Resync-rewinds-counter question (addendum 1).
VALIDATION rows 1-3 PASS: clean forced rebuild rc=0, ctest 306/306; test_audio_tap_sync 7 cases/675 assertions --order rand;
  test_bpm_stabilization 24/88, test_oscillator_bar_fold 9/486, test_connection 27/116. NOTE rows 2-3 `ctest -R` regexes
  match NOTHING useful (ctest names are test-case names) — run the binaries; VALIDATION.md command text to be corrected.
FINDING (21:17-21:19): LIVE APP CRASHES AT STARTUP 3/4 launches (detached). Default in+out = Bluetooth soundcore P31i, 16 kHz
  (HFP). Signatures: SIGSEGV in JUCE AudioIODeviceCombiner::restartAsync (HAL device died/published); 2x SIGABRT malloc
  free-list checksum (heap corruption) on message thread. s168 AudioTap never ran live before tonight. -> wf_76da50a6-a56 (ASan).
FINDING: source_julia_set shader FAILS to compile at startup (every other shader OK). -> same workflow, diagnosis lane.
FINDING: R13 is LIVE today on Boris's rig — device reports 16000 Hz (Bluetooth headset mic), analysis assumes 48000.
IN FLIGHT: wf_b6c30536-17a build (amend -> lanes R28,E,L2,L3,L6,J,H1 in worktrees, patches in /private/tmp/rta-patches, reviewers). L5 RecordPanel DEFERRED (needs visual critic gate). Then: merge patches serially, clean build, ctest, commit per lane.
COMMITTED: 8454e07 J, 4898e41 L6, c923435 L2, 601ad4d L3 (ctest 309/309 on merged tree; L3 fail-first re-proven by Harmony; julia OK live). H1 FOUND: AudioTap::writeFrames padding path reads past silenceStorage_ when numSamples>maxBlock_ && chans<channels_ (ASan READ overflow; CombinedCallback derives tap channels from OUTPUT channels). NOT proven to be the live heap-corruption cause (that needs a WRITE). Fix lane after R28 merges (AudioTap owned by R28).
NEAR-MISS (filed the turn it arose): lane E looked green (ctest 315/315, test_take 480 assertions) — my fail-first spot check (revert RecorderClock, rebuild, restore identical, rebuild) then exposed a DETERMINISTIC failure in the long-take case (periodicCount 286201 <= 170, 572562 assertions). The first green was on a build state I cannot explain; had I committed on it, a broken tempo-anchor fix would have shipped. Diagnosis dispatched (builder, build-efix dir). Lesson: a green after merging a patch is not proof until the target has been rebuilt from a known state; the revert/restore probe doubles as that rebuild.
CORRECTION to the NEAR-MISS above: lane E was CORRECT. The deterministic failure was created BY my probe — after the revert/restore cp cycle in ./build, test_take.cpp.o stayed compiled against the reverted RecorderClock.h (silent class-layout/ODR mismatch, stack object too small, lastAnchorBeat_ write lands outside it -> 286201 anchors). E2 builder reproduced it bit-for-bit on demand and proved a clean build passes (480 assertions, periodicCount 160) twice. Real lesson: never run a revert/restore probe in the gate build dir — use a scratch -B dir, or force a clean rebuild after restoring. Report: /private/tmp/rta-patches/E2.report.md (copied to .harmony/.reports/s-rta-0923/).
STARTUP CRASH STATUS (No Unexplained Residue — filed as ESTABLISHED / RULED OUT / NEXT TEST):
  ESTABLISHED: 3 startup crashes 21:17-21:19 with Bluetooth soundcore P31i default in+out at 16 kHz (HFP):
    1x SIGSEGV in JUCE AudioIODeviceCombiner::restartAsync (zero app frames), 2x malloc free-list corruption on the msg thread.
    9/9 clean launches (baseline pre-s168, ASan HEAD, Release HEAD) once the earbuds were disconnected (built-in 48 kHz).
    Real AudioTap OOB READ found headless (fixed lane H1FIX) — but push() only touches buffers while a take is RECORDING.
  RULED OUT: the AudioTap read as the cause of the startup crashes (nothing records at startup; and a read cannot corrupt malloc metadata).
  NEXT (cheapest discriminating test, needs Boris's earbuds connected + selected): launch the pre-s168 baseline and the ASan HEAD
    build 3x each with the P31i as default in+out. Baseline crashes too -> JUCE/CoreAudio (H2), consider JUCE bump past 8.0.4;
    ASan report -> our code, with stacks.
PLANS LANDED: .harmony/specs/s-rta-0923-lane3-plan.md (752 lines, lanes C0..C5) + s-rta-0923-step3-plan.md (825 lines, S3-A/B/C/M/D).
RECONCILIATION (Harmony ruling, R8): both plans define the manual-write funnel (lane3 C1 src/connect/ManualWrite.*; step3 S3-M
  src/connect/ManualWriter.*). R8 = the CONNECTION lane owns it -> lane3 C1 is THE funnel; step3 S3-M is SUPERSEDED — the step-3
  builder must hook lane3's onManualWrite/onManualRelease seam, never create ManualWriter. Both plans claim src/MainComponent.cpp
  (C3 vs S3-B) and ApiServer (C3 vs S3-C) -> serialize: lane 3 first, step 3 rebases on it.
BUDGET (gauge 31.4%, off-ramp 55%): Lane 3 fits (builders' context is their own); step 3 does NOT also fit -> DEFER step 3 to next
  session's START HERE (long task, begin at session start) with this reconciliation as its first instruction.
BORIS DIRECTIVE (mid-session): finish the in-flight Lane 3 workflow (wf_f21c0ef9-872), start NO new tasks, then run EOS. Step 3 deferred to next session START HERE.
SLIP (filed): wrote 2 learning rows to ~/Harmony_Main event log via log-event.sh from a FOREIGN-repo lane; eos-secondary Step 3 says lane B records learnings in its own .harmony/notebook.md. Transient telemetry, not a system file -> no escalation. Kernel text ('secondary -> .pending or log-event append') and the skill disagree for lane B — worth an up-channel note.
LANE 3 LIVE GATE FINAL: 11 PASS / 1 FAIL. GRIP fail = probe sampled by index not time (product holds 250 ms + 120 ms glide, correct). Window check = probe counted hidden helper windows (fixed to Output-window filter, 0 found).
COMP FAIL = REAL PRE-EXISTING PRODUCT BUG: Renderer.cpp ~2078 feeds composition PosX/PosY in PIXELS (+-1920/1080) into u_comp_position, which comp_transform (EmbeddedShaders.h ~97/121) treats as normalized -1..1 -> any non-zero comp position renders SOLID BLACK. Anchor X/Y same. Predates lane 3. NEXT SESSION #1 fix (normalize by half canvas), with a render test.
MY SLIP: my own `timeout ... until ! pgrep -f "MacOS/Audio-DNA"` wait loops self-match pgrep -f (gotcha S22) and false-positived the probe's refuse check twice. Never embed the app path literally in a watcher; use pgrep -x Audio-DNA or a pid file.
