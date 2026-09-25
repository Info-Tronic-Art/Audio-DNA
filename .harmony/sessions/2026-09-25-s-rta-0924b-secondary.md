# Session log — s-rta-0924b (2026-09-24 12:05 → 2026-09-25 ~16:20, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: NONE.
Boris directives: "Use workflows"; "we will never use bluetooth audio for any reason. it is slow and bad. never use it
again. keep going and make a note of this"; "we will only use hard wired sound input or the onboard mic"; "continue".
Running log: .harmony/s-rta-0924-work.md (s-rta-0924b section). Reports: .harmony/.reports/s-rta-0924b/.
One ~19 h usage-limit/network pause mid-session (workflow agents died ENOTFOUND; resumed from run cache).

| type | ref | msg |
|---|---|---|
| shipped | 4532779 51d614d | onset render path: OnsetPulse acts on onsetCount deltas (Renderer reads bus once/frame; OutputWindow + AudioReadoutPanel own pulses); /api/features onsetCount, /api/status renderOnsetPulses; test-mode mirror under TestServer lock |
| shipped | bffa2d9 | probe-step3 opt-in STEP3_LONG=1 (+STEP3_LONG_MINUTES) long take, D10.3 window + slope drift rows |
| shipped | 28477dc 5eb2b47 b0e3249 0205ca7 | recorder spec step 4: Record panel (RecordPanelModel pure fn + view + one perf* funnel); stopPlayback during overdub disarms + stops transport |
| shipped | 45667cb 1b7caaf | step 4 critic fixes F1-F8: recorder clock restarts per take (was app-launch origin → wall-clock replay +12 s, durations inflated), real playback length, plain-word notices, "Saved" notice, placeholder, contrast; probe pin RED 2 vs 14 s → GREEN 2 vs 2 |
| shipped | 7e9ca8b 6ec7344 | AudioTap stop: close the gate after the in-flight wait, second wait, then drain (was: 512-frame block lost ~1/20 stops); finalize errors in /api/perf/status; probe rows + probe-finalize-loop.sh; probe-step3 clears its `open` logs |
| shipped | 8bf2558 | cpp-httplib 0.18.3 → v0.57.1 (zstd OFF): bodyless POST no longer stalls 5 s (upstream fix 337fbb07, v0.28.0) |
| gate | ctest | 408 at boot → 437 → 438 → 441 → 445/445 (Harmony-run after every merge) |
| gate | live | probe-step3 63/0 → 64/0 (pin) → 69/0 (trunc rows); onset render 13/0 (60 clicks = 60 onsets = 60 pulses); STEP3_LONG 10 min 75/0, 20 min 82/0 (drift +0.28 ms ±0.46); finalize loop 40/0 (pre-fix stderr 2/40); bodyless POST 0.0005 s (was 5.006) |
| gate | visual | 5-seat critic panel on live screenshots (1 FAIL, 4 PASS-WITH-FIXES) → fix plan → fixes → window-only re-shoot verified all 10 states |
| finding | bt-crash | Bluetooth startup crash ROOT CAUSE: JUCE 8.0.4 reopen() sets bufferSize=requested(512) after allocating temp buffers for device size(320) → audioCallback overflow (ASan juce_CoreAudio_mac.cpp:795); upstream fix f6df3e3 in 8.0.8 |
| decision | boris | NO Bluetooth audio ever; only hard-wired input or onboard mic (binding-decisions.md) |
| decision | juce | JUCE 8.0.8 bump DEFERRED WITH TRIGGER: before any wired audio interface is used; plan in bt-crash-fix-plan.md |
| decision | harmony | onset critic MAJOR → one locked inject site; step4 critic MAJOR → stop-play during overdub stops transport; "Saved" notice dropped then re-added as disclosed critic fix |
| residue-closed | 5s-stop | "5 s stop lag" = bodyless curl POST + httplib 5 s read timeout, not the recorder (proven 5.006 vs 0.0007 s) |
| residue-closed | drift | 10-min -1.44 ms was noise: 20-min run +0.28 ms; linear prediction -2.9 ms refuted (10-min run predates F1) |
| slip | screenshots | full-screen screencapture while Boris worked grabbed his private documents; deleted, never committed; gotcha: window-only `-l <id>` + `open -g` |
| slip | workflow | fix-round trigger regex on prose ("FAIL" in "fail-first") ran 2 no-op fix rounds; use a structured verdict field |
| learning | open-appends | `open --stderr` APPENDS; stale lines false-FAILed a new probe row; clear logs before launch, prove by planting a line |
| learning | asan-wait | ASan report was cut by a fixed-time pkill; let the process exit by itself (log_path) |
| learning | critic-vs-builder | critic claimed a fail-first test would not go RED; builder AND reviewer independently reproduced RED in the real target — measure, don't adjudicate prose |
