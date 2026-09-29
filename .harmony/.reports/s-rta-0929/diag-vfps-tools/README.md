# diag-vfps tools (s-rta-0929) -- video playback frame rate on the M1 Pro

All paths are absolute to this session's scratchpad (`.../f33bd4ec-.../scratchpad/diag-vfps`) and the lane worktree
`/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps`; change `S=` / `W=` at the top of each file to
reuse them from another session (grep -l f33bd4ec *).

| file | what |
|---|---|
| `patch_juce.py <juce-src>` | TEMPORARY hooks in a PRIVATE JUCE copy (CRLF-preserving): display-link ticks (callback time, inNow, outputTime = the exact vsync grid), `swapBuffers` (flushBuffer wall time, JUCE's underrun/sleep hack), the render thread's `waitForWork` wait, the per-image display-link lambda. `juce::DiagVfpsHook::fn`, null unless installed. |
| `patch_app.py` | app instrumentation round 1: `src/diag/DiagVfps.{h,mm}` (lock-free record ring -> `$ADNA_VFPS_FILE` TSV, per-thread CPU / QoS / priority sampler every 250 ms, display + window state every 2 s, E/P-core calibration threads), per-frame records in `Renderer::renderOpenGL` (entry/exit, CPU number, phase split, uploads), GPU `GL_TIME_ELAPSED` raw values, `syncMedia` video time, `VideoPlayer` upload dispatcher + decode / sws / ring-wait records; arms `ADNA_VFPS_QOS / NOUPLOAD / BGRA / CLIENT / PBO`. |
| `patch_app1b.py` | round 1b: `ADNA_VFPS_UPCAP` (at most N video picks+uploads per render frame; a player over the cap holds) |
| `patch_app2.py` | round 2: `ADNA_VFPS_IOSURF` (c4a: IOSurface ring slots + `CGLTexImageIOSurface2D` rectangle textures + one GPU blit per new frame, slot released on the blit's fence) |
| `patch_app3.py` | round 3: `ADNA_VFPS_DECSTAGGER=ms` (decoder i sleeps ms x (i mod 4) after a ring-full wake), `ADNA_VFPS_QOS=3/4`, GL thread CPU time (kind 17 / kind 7) |
| `build.sh [cfg]` | configure + build build-lane (JUCE_DIR = the private copy unless overridden) |
| `fixtures.sh` | the probe-video fixtures (a1080_g250, a4k_g250, still4k_f100, pr1080) + still1080_f100 into `media/` |
| `run_arm.sh TAG SCEN N [LOADS]` | N clean launches (open -g, production mode) under `acquire_quiet_lock`; `ARM_ENV="VAR=v ..."`; taints + re-runs a launch that saw a compiler; counts Output-named and UserNotificationCenter windows after every launch |
| `drive.py OUT SCEN [loads]` | REST driver: `w1` (probe-video w1 scene, sequential trigger + wait_active per layer), `w1col` (one `/api/trigger_column`: all 4 clocks start on one render frame), `w2` (4 x 4K stills window + 4 x 4K video windows), `w2v`, `still1080`; windows stamped in CLOCK_UPTIME_RAW ns; `DRIVE_CAPTURE=1` = a render_frame PNG + code-band decode |
| `analyze.py RUN [--json]` | per window: frame fps, swap-to-swap histogram (vblanks), lost ticks, render / flush / wait costs, wake-up latency, phase vs the vsync grid, GL CPU placement (E-cores = CPUs 0,1 by calibration), uploads, GPU ms, decode, per-thread CPU ms/s + QoS |
| `timeline.py RUN WIN [N] [OFFSET_MS] [--dec]` | merged event timeline (DL ticks, frames, flushes, waits, decoder events) |
| `bunch.py TAGDIR` | the upload-bunch pattern per window ("4" = the 4 players' new frames land on ONE render frame) vs fps |
| `heavyphase.py`, `lostpos.py` | the heaviest frames' vblank phase and cost; where lost ticks fall after an upload frame |
| `q1table.py TAGDIR...`, `q2table.py TAGDIR...` | the per-arm tables the report cites (also write q1.json / q2.json into the arm dir) |
| `vtbench.sh` | ffmpeg decode pricing outside the app (software + sws vs VideoToolbox, 1 and 4 streams) under the live lock |
| `patch_app4.py` | round 4: `ADNA_VFPS_IOSURF=2` (bind the slot's IOSurface as GL_TEXTURE_2D, no blit) -- on this Mac `CGLTexImageIOSurface2D(GL_TEXTURE_2D)` returns CGLError 10008 and the texture is black: an INVALID arm, kept as the finding |
| `run_mix.sh PREFIX SCEN LOADS ROUNDS "arm=VAR=v,VAR2=v" ...` | INTERLEAVED arms (A B C A B C ...), one lock hold <= 11 min: the method the report's verdict tables use (sequential per-arm batches drifted ~5 fps over an hour) |
| `perlaunch.py TAGDIR...` | markdown, one row per launch: every load's fps, bunch pattern, swap-interval shares and lost ticks/s |
| `cleanup.sh` | save `instr.diff` (tracked diff + the new files) and `juce-instr.diff`, `git apply -R`, delete src/diag, rebuild build-lane against the main checkout's JUCE copy, strings check |
| `batch_q1.sh`, `batch_q2.sh`, `batch_all.sh`, `batch_smoke2.sh`, `chain2.sh`, `chain3.sh` | the batch sequences actually run (the sequential ones were superseded by `run_mix.sh`) |
| `instr.diff`, `juce-instr.diff` | the AUTHORITATIVE applied instrumentation (all rounds); the patch_*.py scripts reproduce it on a fresh tree |
| `agg/` | the aggregates the report cites (q1-table, q1-perlaunch, q1-bunch-sequential, q1-lostpos, q1-heavyphase, q2-table, q2b-table, q2-bunch, vtbench, main-reference) |

Analyzer conventions: E-cores = CPUs 0 and 1 on this M1 Pro (a BACKGROUND-QoS calibration thread ran only there in 40/40
launches; USER_INTERACTIVE only on 2-9). A "lost tick" = a display-link callback with no renderOpenGL start before the next
callback (JUCE's pendingRender is a flag, not a counter). The swap-to-swap histogram is bursty (flushBuffer returns in
pairs), so read lost ticks, not the histogram, for frame loss.
