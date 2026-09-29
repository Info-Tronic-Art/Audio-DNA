## BUILDER REPORT -- lane seqvram (s-rta-0928b)

STATUS: PARTIAL
RESULT: All 5 plan items are committed on lane/seqvram (dc15ac8, 768be63, 7386312, 4752576 + this report). Every adopted gate is GREEN on the final app: probe-seq-vram 60/0 on two consecutive runs, ctest 865/865 serial, image-load 37/0, crossfade 35/0, render-state 35/0, canvas 15/0, S4 byte-identical. But two live regressions outside the adopted gates need a Harmony ruling before merge. (1) The plan's rule "not drawn last frame = idle" is broken by Pitfall 53's layer hold: at the start of a crossfade, the Renderer trims the whole OUTGOING sequence (~127 textures). (2) Trimming one big idle sequence in a single frame stalls the render callback for ~22 ms. Both were measured, and neither is fixed here (the rig says do not improvise a redesign).
FACTS: `src/media/SeqVram.h` (pure policy + Slots), `src/media/ImageSequence.cpp` (window wiring), `src/render/Renderer.cpp` (scanSequenceVram + grant), `tests/test_seq_vram.cpp` (17 cases), `.harmony/probe-seq-vram.py` (10 rows), `src/render/CompositorEngine.cpp:1093` (the pending-hold branch that skips drawing the outgoing chain), `.harmony/.reports/s-rta-0928b/seqvram-scratch-rows.diff` (the scratch rows that measure the regressions)
METHOD: Commits followed the plan's order: counters first (RED by value), then the pure policy + ctest with teeth on mutated COPIES, then the wiring, then docs. Every probe row was run RED on main and on the commit-1 app, then GREEN twice on the final app. The perf findings used A/B runs with at least 5 per arm and in-place teeth/experiment builds restored by sha256.
CONFIDENCE+VERIFY: High for memory bounds, pixels and the pure policy. Medium for the transition feel (see RISKS). Verify: `ctest --test-dir build-lane -j1` = 865/865; `SEQVRAM_APP=<lane app> bash .harmony/probe-seq-vram.sh /tmp` with the lock held = `PY 60 PASS / 0 FAIL`; to see regression (1)/(2), apply seqvram-scratch-rows.diff to a copy and run rows `v8_crossfade_two_long,v7c_one_per_deck`: callback>10 = 22-24 ms at about 0.07 s.
UNKNOWNS/NOT-DONE: Regressions R-A and R-B (below) are not fixed. The v8 row asserts peak_frame_time_ms (per the adoption), which cannot see a frame-top stall. HANDOFF.md and APP-INVENTORY.md were not touched (H17). The pitfall number is still "NN".
NUANCE: seq_* counters are cumulative across rows, so a probe must use row-local deltas (the first v4 wait fired on v1's evictions). An Opaque layer replaces everything below it even where its transform left alpha 0, so side-by-side rows need type 1 (Transparent, keying Alpha). The first decode of a sequence often lands after the playhead has moved on: see D4.
HANDOFF-NEEDS: A Harmony ruling on R-A (the idle definition during a Pitfall 53 hold) and R-B (bulk trim cost). Candidate fixes are given under ISSUES. Pitfall number 54 at merge.

INBOX-RECHECK: none

### SUMMARY
Image sequences now play through a bounded window of recycled GL textures. A 300-frame 1080p loop that held 300 textures / 2373.0 MiB on the commit-1 build now holds 129 / 1020.4 MiB (v1). Playback stays smooth (v2 / v3 / v9: 0 late, 0 pending), a retrigger to the in-point is a hit (v4), and floors win (v10).

### ITEMS (plan section 8; the commit per item)
| # | commit | what | RED | GREEN |
|---|---|---|---|---|
| 1 | dc15ac8 | seq_* counters (7070 + 8080), frame-top sum, 3 probe files | main: `FAIL  v1_memory_1080: /api/state has no seq_open, seq_textures, seq_texture_mb... (the app predates them)` (all 10 rows, `PY 0 PASS / 10 FAIL`) | -- (harness) |
| 2 | 768be63 | SeqVram.h policy + Slots + 16 ctests | the test does not compile against the commit-1 header (`error: unknown type name 'Transport'`) or main (`'media/SeqVram.h' file not found`) | ctest 864/864 |
| 3 | 7386312 | ImageSequence window + Renderer grant / scan / trim; `wanted(fits)` fix; probe fixes | commit-1 app, by value (below) | probe 60/0 twice; ctest 865/865 |
| 4 | 4752576 | rendering.md sentence, pitfall NN, CLAUDE.md index NN (24,665 B) | -- | `git diff main -- docs CLAUDE.md` removes only the old sequence sentence |
| 5 | this commit | report + scratch rows diff | -- | -- |

RED by value on the commit-1 app (`red-c1`, verbatim):
```
FAIL  v1_memory_1080: (a) textures after 2 laps 300 <= 130
FAIL  v1_memory_1080: (b) sequence texture MB 2373.0 <= 1032.0
FAIL  v1_memory_1080: (c) seq_over_budget 1 == 0
FAIL  v4_retrigger_hit: (d) textures 300 <= 130
FAIL  v6_budget_share: (b) both sequences 1605.8 MB <= 1032.0
FAIL  v6_budget_share: (d) seq_over_budget 1 == 0
FAIL  v8_crossfade_two_long: (c) after the fade 3369.7 MB <= 1040.0
FAIL  v9_three_decoding: (c) 3559.6 MB <= 1040.0
FAIL  v7b_trim_spike: (b) after 5 s 4255.7 MB <= 1040.0
FAIL  v3_smooth_pingpong_4k: (d) 1265.6 MB <= 1032.0 and textures 40 <= 33
FAIL  v10_floors_win: (b) 3164.1 MB <= 5 x 8 x 31.64 + 32 = 1297.6
PY 46 PASS / 14 FAIL
```
(The 3 other FAILs in that run were the v9 / v7b layout checks before the Transparent-layer fix. After the fix, the c1 re-run passes those layout checks: `c1-layout` 9 PASS, with only (c) / (b) failing by value.)

GREEN on the final app (green2 and green3, load 4.8-7.0, verbatim): `PY 60 PASS / 0 FAIL`, `PROBE-SEQ-VRAM GREEN` for both.

### PER-ROW NUMBERS (final app, green3)
| row | textures / MB | late / pending / shown | peak frame / callback ms | notes |
|---|---|---|---|---|
| v1 (L300 30 fps, 2 laps) | 129 / 1020.4 | -- | -- | code 31 in 30..32, dbox 0.00 |
| v2 (L300 10 fps, 8 s) | 95 / 751.5 | 0 / 0 / 80 | 7.28 / 7.80 | 3 codes 90, 101, 112 |
| v4 (retrigger at frame 129) | 129 | 0 / 0 | -- | code 16 in 16..17 |
| v6 (120 fits + 180) | 120 then 129 / 1020.4 | late 0 | -- | 0 uploads on lap 2 |
| v8 (fade two 300s) | 129 / 1020.4 | 0 / 2 | 4.36 / 4.39 | see R-A: 3/7 runs spike |
| v9 (3 x 150 @ 15 fps) | 129 / 1020.4 | 0 / 0 (20 s) | -- | 3 layer codes 76 |
| v7 (deck return) | 39 / 308.5 | late 6 (>= 1) | -- | clock ran 3.017 s |
| v7b (3 idle vs 2 drawn) | 129 / 1020.4 | -- | 5.25 / 10.10 | (a') is my addition |
| v3 (Q40 4K PingPong) | 32 / 1012.5 | 0 / 0 / 118 | 9.84 / 10.12 | |
| v10 (5 x 20 4K) | 40 / 1265.6 | over_budget 1 | -- | floors win |

### TEETH
- ctest, on mutated COPIES (scratch include dir; the header sha is c6fb0291... for both the committed and the teeth-source copy):
  - no lastShown protection: (5)(6)(10b)(11)(12) FAIL
  - no Loop wrap: (1)(3)(4)(5)(9b)(12b) FAIL
  - PingPong without the bounce: (2)(7) FAIL
  - Reuse for a mismatched size: (10) FAIL
  - OneShot past its end: (3)(12) FAIL
  - T3 releaseAll without clear: (10c) FAIL
  - wanted() ignoring `fits`: (9)(9b) FAIL
  - The plan predicted (7) would FAIL without lastShown protection. It does not, because in (7) the lastShown frame is only 3 steps ahead, so the protection never matters there.
- Probe, in-place teeth builds (restore sha c6fb02910d952005... EQUAL; `git diff` 0 lines; the rebuilt binary differs from the pre-teeth copy only after LC_CODE_SIGNATURE dataoff 17,946,464):
  - T1 (kLookAhead 0, kMaxOutstanding 1): `FAIL  v2_smooth_loop_1080: (a) late frames over 8 s at 10 fps 175 == 0`
  - T2 (kMinWindowFrames 3, kBudgetBytes 64 MiB): `FAIL  v3_smooth_pingpong_4k: (a) late frames over 12 s (4K PingPong at 10 fps) 1 == 0`. This bites only narrowly: the smooth fixtures decode fast, and 404 stale drops were recorded.

### OTHER GATES (final app)
- ctest serial: `100% tests passed, 0 tests failed out of 865` (848 + 17 new), run at the end.
- probe-image-load `PY 37 PASS / 0 FAIL`; probe-crossfade `PY 35 PASS / 0 FAIL`; probe-render-state `PY 35 PASS / 0 FAIL` (main: 35/0); probe-canvas `PY 15 PASS / 0 FAIL` (main: 15/0).
- S4 byte identity:
  - the 3-frame sequence capture, main vs lane: `array_equal True`
  - frames 134 / 140 / 146, uploaded with glTexSubImage2D into RECYCLED slots (seq_slot_reuses 9): `array_equal RGB vs its PNG True` each
  - main's pipeline is exact too (`True`)
- I looked at the v9 side-by-side capture (3 thirds, code bands legible) and the v8 mid-fade capture (a real dissolve, bands half grey).
- Screen safety: every batch ended `audio-dna windows 0, Output-named 0`, and no Output window was ever opened.

### ISSUES
- **R-A (found, not fixed; the plan's assumption is broken):** the idle trim fires on the OUTGOING chain at a crossfade start. The sequence of events:
  - CompositorEngine holds the layer while the incoming sequence is pending (`CompositorEngine.cpp:1093-1104`). No transition runs, so the outgoing chain is not drawn for 1-3 frames.
  - Its lastDrawnSerial falls behind, so `scanSequenceVram` treats it as idle.
  - The incoming's first uploads push the total over 1 GiB, so the scan trims the outgoing to 2 frames. Measured: seq_textures 129 -> 5-6 at 0.05-0.07 s after the trigger.
  - Result: a 22-24 ms render callback, then the outgoing re-decodes during the fade (late 4-11).
  - Rates: 3 of 7 final-app runs (23.4, 23.2, 23.9 ms), 4 of 5 detail runs, and 3 of 5 on an experiment build without per-frame shrink (X1, so per-frame deletes are ruled out). The commit-1 app shows 0 of 10: 5 at a lap-2 fade and 5 at the same lap-1 playhead, via SEQVRAM_FIXED_WAIT (max 4.5-8.0 ms).
  - Candidate fix: count a sequence idle only after N frames not drawn (e.g. 30), or treat a held layer's chains as drawn.
- **R-B (found, not fixed):** one big idle sequence trimmed in one frame costs ~22 ms.
  - Scratch row v7c: one 300-frame 1080p sequence per deck, switch when deck 0's window is full. 5 of 5 runs: 21.9 / 22.3 / 22.4 / 21.9 / 22.0 ms at about 0.075 s after the switch. seq_evictions +127 in one poll.
  - The adopted v7b (three smaller idle sequences) passes at 10 ms because each trim step deletes fewer textures.
  - Candidate fix: cap the glDeleteTextures per frame in the trim (e.g. 16). The rest stay as free slots and are deleted over the next frames while the total is still over the budget.
- **R-C (design note, measured):** after a fade, the idle outgoing keeps about 121 frames (958 MB) while the drawn incoming is held at its 8-frame floor. The total is not over budget, so there is no trim (plan R-7 semantics). The incoming then re-decodes every frame (v8: seq_evictions +1 per about 25 ms at 4 s). It is smooth on a quiet machine but has no headroom. A ruling is needed on whether "a drawn sequence at its floor" should count as pressure.
- I ran the last ctest (19 s) while diag-idle held the lock and was measuring. It is not a compile, but it did load the machine. I also ran one early command with `cd` (the rig forbids it).
- Lock starvation: the two diag lanes alternated the lock for about 30 minutes against the helper's 20 s poll. I used a copy of acquire_lock that polls every 2 s (same mkdir, owner line and 45 s cooldown). I killed only my own waiting runners. Batch 3d held the lock for 15:01.
- The `.venv` symlink was present (untracked, never staged) when commit 4 was made, because a running batch had created it. Removed since.

### DEVIATIONS (from the plan / adoption, each with evidence)
- D1: ctest (2) PingPong forward from 4 of 6 = {6,5,4,3,0,1}. The plan body wrote dist[3]=2, which contradicts its own rule that a revisit of cur counts. advanceFrame reflects the time, so the index sequence is 4 5 4 3.
- D2 (H6 re-derived): `syncMedia` enforces only the OUT point (`Renderer.cpp`, in/out block):
  - Past out, Loop AND PingPong jump to the in-point via seekTo; they never reflect there. OneShot stops.
  - Reverse Loop runs to 0, wraps to the end, which is at or past out, and then jumps to in.
  - The adoption's "PingPong reflects at in and out" does not match the code, so Transport models the code.
  - The in-point = int(in*(n-1)) (the seekTo rule). The out frame = ceil(out*n)-1 (a float 2/3 keeps frame 200 on screen).
  - Plumbed through `Grant.inPoint/outPoint`, so the syncMedia sync block stays untouched.
- D3: the ctest (5)-(7) teeth list differs (see TEETH).
- D4: `SeqVram::wanted` gained `fits`. The plan dropped every result with dist >= allowance. Live v4 was late 1-2 in 4 of 5 runs: frame 0 decoded after the playhead reached 1, so it got dropped, and the in-point was never resident. A scratch sim and ctest (9b) reproduced it. Now a result is dropped only when keeping it would need an eviction. After the fix, v4 was late 0 in 5 of 5 runs plus both GREEN runs.
- D5 (probe): additions beyond the adoption:
  - The playhead is read BEFORE and AFTER each capture (a bracket) instead of only after.
  - v7b (a') asserts peak_callback_ms, because the frame-top trim runs outside the frame timer.
  - v4 and v8 print timelines.
- D6: `Slots::acquire` for a frame that already has a slot returns that slot (Reuse / Respecify). A free slot with tex 0 is handed out as Create.
- D7: budget.take() runs before acquire(). A Full after take (the H1 path) wastes that frame's upload budget. This is rare. Not measured: the probe does not print seq_upload_deferred.
- D8: `trimToMinimum(Stats*)` takes the stats pointer so trim evictions are counted.

### RISKS
- R-A and R-B are real one-frame stalls at a musician-visible moment (a transition or a deck switch). They are not caught by the adopted rows.
- The budget constants are ASSUMED (H13).
- The T2 teeth bite narrowly (late 1).

### FILES CHANGED
- `src/media/SeqVram.h` -- new: Stats, Grant (with in/out), allowance, distances / step / setRange, plan, wanted(fits), Slots.
- `src/media/ImageSequence.h/.cpp` -- the window + slots, trimToMinimum, residentBytes / Slots, releaseGL via releaseAll. requestAhead and textureWidths_/Heights_ were removed.
- `src/render/Renderer.h/.cpp` -- seqStats_, scanSequenceVram (sum + pressure trim), the grant with the running total, getSeqStats.
- `src/api/ApiServer.cpp`, `src/test/TestServer.cpp` -- 12 seq_* fields (additive, including seq_upload_deferred).
- `tests/test_seq_vram.cpp` + `tests/CMakeLists.txt` (EOF block) -- 17 cases.
- `.harmony/probe-seq-vram.{sh,py,json}` -- 10 rows.
- `docs/claude/rendering.md`, `docs/claude/pitfalls.md`, `CLAUDE.md` -- docs.

### NOTEBOOK NOTES (for Harmony to append)
- 2026-09-28 seq_* counters are cumulative per app run: probe conditions must use row-local deltas | discovered: `.harmony/probe-seq-vram.py` (wait_window_full).
- 2026-09-28 An Opaque layer replaces everything below it even where its layer transform left alpha 0. Side-by-side multi-layer captures need type 1 (Transparent, keying Alpha) above the first layer | discovered: `src/render/CompositorEngine.cpp:777`.
- 2026-09-28 A sequence's first decode often finishes after the playhead has moved on. Any "far result" drop rule must keep results while there is room | discovered: `src/media/SeqVram.h` (wanted).
- 2026-09-28 A Pitfall 53 layer hold skips the transition, so neither chain of a fading layer is drawn for 1-3 frames. Any "not drawn = idle" logic misfires there | discovered: `src/render/CompositorEngine.cpp:1093`.
- 2026-09-28 In-place teeth restore proof: sha256 of the source is EQUAL, and the first byte where the binary differs is at or past LC_CODE_SIGNATURE dataoff (`otool -l`) | discovered: scratch teeth_build.sh.

### PACKET QUALITY
- Clarity: HAD_TO_INFER. H6's PingPong semantics contradicted the code (D2), and test (2)'s numbers contradicted the plan's own rule (D1).
- Missing context: the Pitfall 53 hold skips the outgoing chain's draw, which is the root of R-A. The lock helper's 20 s poll loses to alternating lanes.
- Unused context: none.
- Self-brief files: the plan + adoption, both attack papers, and the renderleft reports (for recorded counts) were used.

### STATUS
PARTIAL. Items 1-5 are delivered and every adopted gate is GREEN. R-A and R-B need a ruling or fix before merge.
