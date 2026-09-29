# Independent Method Audit, Round 2 — diag-media (s-rta-0928b)

VERDICT: SOUND_WITH_GAPS

Scope: read-only re-derivation from raw evidence at
`SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media`
against `.harmony/.reports/s-rta-0928b/diag-media.md` and source at main HEAD.
No app run performed. Every headline number below was recomputed directly
from `runs/*/app-err.log`, `runs/agg-{v,k,iq}.txt`, `runs/pool-{v,k,iq}.txt`,
`runs/seekstats.txt`, and `instr.diff`, independent of the report's own
prose.

## 1. Per-cause verification (counterfactual, not just inference)

All eight claimed causes are PROVEN, not merely inferred — each has either
a same-launch counterfactual arm or a direct source-line match, and I
independently reproduced the numbers from raw data:

- **S1b (re-seek loop).** `runs/seekstats.txt` `retrig_g250`: 47-48 calls,
  call_ms_med 66.7-68.5, dur_ms 3254-3314 — matches report exactly.
  `retrig_g250_4k`: 14 calls, call_ms_med 263.6-268.9, dur_ms 3469-3559 —
  matches exactly. `back_g30` (GOP-30 control) shows 1 call, no loop —
  confirms the GOP dependency, not just correlation. Source: I traced
  `VideoPlayer::decodeFrameAtTime` (`VideoPlayer.cpp:441-469`, `diff > 2.0`
  triggers `seekToTimestamp`, 30-decode cap) and confirmed the mechanism
  independently: the loop duration (3.25-3.56s) equals the gap between the
  retrigger's `inPoint`-derived target and the *next* keyframe (5.0→8.33s =
  3.33s), because `currentTime_` keeps advancing via the transport clock
  while decode falls >2s behind and re-seeks every call — this is why the
  loop self-terminates exactly at the next keyframe, not at the original
  target. The report's prose ("until the clock reaches the next keyframe")
  is precisely this mechanism, correctly described.
- **S1a (steady playback).** Recomputed from `runs/agg-v.txt`: v0
  1080p×4 `cb_med=6.823 cb_p90=9.134 fps=93.2`; v1 (NODECODE)
  `cb_med=6.325 fps=117.2`; v2 (UPLOAD_ONLY_NEW) `cb_med=2.657 fps=104.1`.
  4K×4: v0 `12.358/81.746/36.0`, v1 `4.43/4.815/95.9`, v2 `9.945/40.119/46.0`.
  All match the report's table to 2-3 significant figures. The
  `frameReady_` claim (`VideoPlayer.cpp:340`) is confirmed by grep — the
  flag is set true at lines 180/239/251 and never cleared after
  `uploadToTexture`'s read at 340; `v1080x1` pool shows `upl_calls=603
  dec_calls=151` (exactly 4:1, i.e. 120Hz vs 30fps), matching "603 uploads
  against 151 decodes" verbatim.
- **S2 (video open+thumbnail).** `runs/pool-v.txt`: 1080p
  `omd.videoClip 25.659/26.947/37.736 (n=80)`; 4K `99.039/101.194/103.012
  (n=80)`. `mc.loadComposition` 16×1080p `423.346/434.923`, 16×4K
  `1582.537/1616.590` — matches "0.42 s"/"1.58 s" and the ranked-table
  m3 "reads today 1577-1617ms" exactly.
- **S2-drop.** `runs/agg-iq.txt` `drop_v4k nodeck_all=14`, `mc.applyFileDrop
  med=106.601`; `drop_v1080 nodeck_all=5`, `applyFileDrop med=28.915`;
  `drop_img4k nodeck_all=2`. All match the table row and gate m4's "reads
  today 13-14" verbatim. Source: `UndoService.cpp:66`
  (`renderer_->setActiveDeck(nullptr)`) and `:89`
  (`executeOnGLThread(...,true)`, the blocking fence) — line numbers exact,
  and this genuinely explains why the GL thread renders deck-less
  (`Renderer::renderOpenGL` reads `deckActive` from the same pointer).
- **S3 (sequence open).** `runs/pool-iq.txt` `load_seq mc.loadComposition
  128.437/134.360`; `seq.open.frame0Decode` and `.statLoop` sub-splits
  match the 98.5/26.0ms per-clip breakdown. `drop_seq300
  mc.applyMultiFileDrop 27.069` matches "27.1ms".
- **S4/S5.** `img4k_x4` (i0 vs i1/NO_EXISTS): `cb_med 0.475→0.464` —
  matches "inside the noise" verbatim. `loaded_mixed_idle` (24 cells,
  mixed grid) gives `stat_us_med=5.1 p90=8.2 max=42.7 stat_ms/s=4.277,
  paint_ms/s=59.436` (base) vs `img4k_x4`'s NO_PAINTSTAT arm
  `paint_ms/s=55.334` — matches "5.1/8.2/42.7µs, 4.3ms/s" and the
  "-4.2ms/s" delta (59.436-55.334≈4.1, within rounding).
- **X1 / found_not_fixed #5 (data race).** `VideoPlayer::getThumbnail`
  (`VideoPlayer.cpp:389-402`) reads `frameBuffer_` with no lock; the GL
  thread's `convertFrameToRGBA` writes the same member with no lock
  either. Correctly labeled INFERRED (a plausible-but-unobserved race,
  not a reproduced corruption) — the report does not overclaim this one.
- **found_not_fixed #1 (every fence black-frames).** Confirmed at the
  source level (see S2-drop above) and empirically for every drop/load
  phase in the evidence (`nodeck_all` is nonzero on every load and drop
  phase, 1-14 depending on scope of mutation).

All eight are PROVEN by the standard the packet asks for (≥5 clean
launches, counterfactual arm or an exact source match). I found no case
where the report inflated an inferred claim into a "VERIFIED" one — S4/S5
are correctly downgraded to "Medium" confidence in the report's own
CONFIDENCE section, and the two q1_5 stalls and the camera/MilkDrop paths
are correctly left as UNKNOWNS/INFERRED rather than folded into the ranked
causes.

## 2. Round 1's MUST 2 — instrumentation's own per-frame cost

Confirmed present as described: `Renderer.cpp`'s `DgFrame` destructor
fires an unconditional `std::fprintf(stderr, "[GF] ...")` on every
`renderOpenGL()` return path (`instr.diff:425-440`), and `DgVid`'s
destructor fires `[GV]` on every `syncMedia` video call regardless of the
`decode` flag (`instr.diff:516-533` — the struct is constructed
unconditionally inside the `Video` branch, before the `if (decode)`
check, so even non-decoding/off-screen-deck calls print a line).

**Measured line rate** (directly counted from `runs/v0_1/app-err.log`,
a 64s run): 6423 `[GF]` + 7064 `[GV]` + 17264 `[CP]` = 30751 lines / 64s
≈ 480 lines/s, average 62 bytes/line (2,067,718 bytes / 33,393 total
lines) ≈ 30KB/s total I/O. `stderr` is unbuffered on both glibc and
Apple libc, so each `fprintf` call is one `write(2)` syscall plus
double-formatting overhead.

**Estimated cost**: at ~480 writes/s with a local-disk `write()` typically
1-10µs plus ~1-3µs of `snprintf`-style double formatting, total
instrumentation overhead is on the order of 5-10ms/s, i.e. **~50-80µs
average per GL frame at 100-120fps** — roughly 0.6-1% of the reported
1080p×4 median (6.82ms) and under 0.1% of the 4K×4 median (12.36ms).
This is comfortably below the effect sizes being measured and does **not**
change any ranked ordering, because:
- the same instrumentation density is present in every arm (v0, v1, v2),
  so the counterfactual *deltas* (which is what every "attribution" in the
  report rests on) cancel this cost out;
- the absolute fps figures used for user-facing framing ("93 fps", "36
  fps") could be biased downward by this fixed low-single-digit-percent
  tax, but not enough to cross any of the report's qualitative thresholds
  (8.33ms/16.67ms VSync, the ranked order, or gate PASS/FAIL bounds — e.g.
  gate m1's "≥110 fps" bar sits >13 fps above the reads-today 36.0fps 4K×4
  case, and >15fps above the reads-today 93.2fps 1080p×4 case).

**GAP (SHOULD, not blocking):** the report itself never states or bounds
this cost anywhere in METHOD or CONFIDENCE — it is left for the auditor to
derive. Round 1 flagged this as a MUST; my own arithmetic above resolves
it (the cost is real but negligible and self-cancels in every
counterfactual), but the report should say so explicitly (one line: "GF/GV/CP
add ≈480 stderr writes/s, ≈60-80µs/frame amortized, below the noise floor
of every reported delta") so a future reader doesn't have to redo this
derivation from raw logs to trust the absolute fps numbers.

## 3. Could the instrumentation create or hide an effect?

- **Heartbeat perturbation**: `Heartbeat::start()` (`instr.diff` DiagTrace.h)
  posts one `callAsync` ping every 2ms only when the previous one has been
  served (`served.load()` gate), so it self-throttles to at most ~500
  extra message-thread queue items/s. It measures message-thread stall via
  ping *latency*, and the report is careful to use it only as a
  cross-check ("heartbeat max next to it for comparison"), not as the
  primary metric — actual site costs come from the `DIAG_SCOPE_MIN` scopes
  timed directly at the call site (`mc.loadComposition`, `us.fence.wait`,
  etc.), which the heartbeat cannot perturb (it doesn't wrap them). This
  is architecturally sound: the heartbeat is a canary, not a stopwatch on
  the measured code.
- **The TEMPORARY `/api/diag/media` hook**: confirmed in `MainComponent.cpp`
  (`instr.diff:14-31`) — `op == "drop"` calls `self->handleFileDrop(...)`
  or `self->handleMultiFileDrop(...)`, the exact same message-thread
  entry points a real Finder drop reaches (`DeckView::onFileDropped` /
  `onMultiFileDropped` funnel into these). No synthetic/short-circuited
  path — claim verified, not just asserted.
- **Phase-mark ordering via `callAsync`**: `tools/drive.py:138-139`
  explicitly documents and implements "every phase mark is posted BEFORE
  its action (both run on the message thread in order)" — I confirmed this
  is structurally guaranteed because both `mark()` and the action (`trig`/
  `hook`) go through sequential, `Connection: close` HTTP requests that
  each resolve to one `callAsync` enqueue; single-threaded FIFO ordering on
  the message queue makes the mark's callback always precede the action's.
  This is the exact gotcha the report's own Notebook section documents
  (i.e., they hit it, fixed it, and the fix is visible in the driver) —
  correctly self-reported, not swept under the rug.
- No other hidden-effect vector found (no `DIAG_MEDIA_HOOK`-only code path
  that changes production behavior when the flag is off; the hook handler
  itself is unreachable in a normal build since it requires
  `DIAG_MEDIA_HOOK=1` AND a REST hit).

## 4. Arithmetic

Recomputed every ranked-table score independently:

| site | report score | recomputed | match |
|---|---|---|---|
| S1b 1080p | 9,900 | 3300×1×3=9,900 | exact |
| S1b 4K | 10,500 | 3500×1×3=10,500 | exact |
| S1a moderate | 3,240 | (2×1.51)×3600×0.3=3,261.6 | within rounding of C (report's C=1.51 is itself a rounded per-layer figure) |
| S1a heavy 1080p×4 | 90,144 | 6.26×7200×2=90,144 | exact |
| S1a heavy 4K×4 | 254,880 | 11.8×7200×3=254,880 | exact |
| S2 16×4K | 79 | 1583×0.05×1=79.15 | matches |
| S2 16×1080p | 21 | 423×0.05×1=21.15 | matches |
| S5 | 64 | 0.0051×42,000×0.3=64.26 | matches |
| S2-drop 4K | 35 | 117×0.1×3=35.1 | matches |
| S2-drop 1080p | 12.5 | 42×0.1×3=12.6 | matches |
| S4 (moderate, 3/frame) | 27 | 0.0042×21,600×0.3=27.2 | matches |
| S4 (4-layer, 5/frame) | 45 | 0.0042×(5/3×21,600)×0.3=45.4 | matches once the 5-stats-per-4-layer fact (S4 FACTS: "4 image layers → 5 stats") is used instead of naively scaling 3→4 |
| S3 drop | 12.5 | 42×0.1×3=12.6 | matches |
| S3 load | 6.4 | 128×0.05×1=6.4 | exact |

No arithmetic errors found. The one place a naive recheck looks off (S4
"4 layers 45") resolves correctly once the FACTS section's own stat count
(5, not a linear 4/3 scaling of 3) is applied — the report is internally
consistent, just terse about which F it used for that one cell.

## 5. Are the RED-able gates m1-m5 real?

Yes, all five gates' "reads today" figures are independently reproducible
from evidence, and each targets a mechanism I traced in source that a
PASS could only satisfy via the stated fix (not a metric-gaming shortcut):

- **m1** (`gl_video_decode_calls==0` at 4K×4, fps≥110, p90≤8.3ms): today's
  36.0fps/81.7ms p90 is reproduced exactly from `agg-v.txt` row `v4kx4`
  (v0). A `gl_video_decode_calls==0` condition can only be satisfied by
  moving `avcodec_send/receive` off the GL thread (fix a) — there's no way
  to zero this counter while still decoding on the GL thread, since the
  counter would need to be wired directly to the `avcodec_receive_frame`
  call site.
- **m2** (retrigger mid-GOP, every callback ≤16.7ms AND
  `gl_video_max_decodes_per_call≤2`): today's "30 decodes per call,
  67-69ms callbacks for 3.3s" matches `seekstats.txt` `retrig_g250` (47-48
  calls × ~30 decodes ≈ matches the "up to 30 decode attempts" cap in
  `decodeFrameAtTime`). Only fix (b2) — bounding/removing the re-seek
  loop — can satisfy both the decode-count cap and the callback-time bound
  simultaneously (upping cap without fixing decode work wouldn't fix
  callback time; only removing the redundant work would).
- **m3** (`peak_message_stall_ms≤50` over 16×4K load): today's 1577-1617ms
  reproduced exactly. Only async-open (S2 fix a) plausibly gets a 1.5s
  message-thread hold under 50ms; the "cheap step" (b, thumbnail-only) at
  best gets to ~1108ms (measured NO_THUMB counterfactual), still far over
  50ms — so the gate correctly forces the full asynchronous-open fix, not
  a token thumbnail optimization.
- **m4** (`frames_without_deck` delta==0 over a 4K drop): today's 13-14
  reproduced exactly from `nodeck_all=14`. Only moving the open outside
  `withDeckDetached` (fix c) removes the fence-held mutation; the fence's
  own bare drain (`executeOnGLThread` with an empty lambda) still black-
  frames at least once per the found_not_fixed #1 finding, so a truly
  strict `==0` is arguably unreachable even with fix (c) applied — the
  gate's PASS bar may be one frame stricter than fix (c) alone delivers.
  **NIT**: the report doesn't reconcile m4's `==0` bar against its own
  found_not_fixed #1 ("every fence renders ≥1 black frame"); a correct fix
  (c) would likely still read `delta==1` (the fence-only black frame), not
  `0`. Worth a one-line caveat in the gate spec.
- **m5** (`msg_image_decodes==0` over the seq load + drop): today's "4 per
  load, 2 per drop" matches the sequence math (PNG open+thumb decode +
  JPEG open+thumb decode = 4; JPEG-sequence-drop open+thumb = 2) traced
  through `pool-iq.txt`'s `seq.open.frame0Decode` / thumbnail splits.
  Reachable only by moving both decodes (dims-from-first-async-result +
  `ClipThumbnails`-sourced thumbnail) off the message thread, matching the
  stated fix.

## 6. Attributed-but-unevidenced / evidence contradicting the report

None found. Every "VERIFIED" claim I checked traces to a source-line match
plus a directly-reproduced number; every "INFERRED" claim is correctly
labeled as such in the report's own CONFIDENCE/UNKNOWNS sections (S4/S5
Medium confidence; the slow-volume risk; the 4K GOP-30 deck-return hitch;
camera/MilkDrop paths; the q1_5 237/253ms frames, which I independently
found in `runs/q1_5/app-err.log` at t=45366.866 (252.647ms, v/s fields all
zero) and t=50437.996 (236.931ms) — matching the report's citation and
confirming "no media work in them" from the zeroed `v`/`s` accumulator
fields in the `[GF]` line itself).

## Findings

**MUST**: none. The eight causes are proven, not inferred; the gates are
real and RED on current main by the stated numbers; no arithmetic errors;
no hidden instrumentation effect found.

**SHOULD 1** — Quantify the instrumentation's own per-frame cost in the
report (§2 above). Add one line to METHOD or CONFIDENCE: "~480 stderr
writes/s (measured from `runs/v0_1/app-err.log` line counts), ≈60-80µs/
frame amortized — below the noise floor of every reported delta; absolute
fps figures may be biased low by <1%." This is cheap to add and closes
the exact gap Round 1's MUST 2 was probing for.

**SHOULD 2** — Reconcile gate m4's `frames_without_deck delta==0` bar
against found_not_fixed #1 ("every `withDeckDetached` fence renders ≥1
black frame"). As written, m4 may be unreachable even after the
recommended S2(c) fix (open-before-fence) because the fence's own
`executeOnGLThread` drain still forces one deck-less frame. Either loosen
m4 to `≤1` or note that reaching `0` requires also addressing
found_not_fixed #1 as a prerequisite.

**NIT** — The ranked-table's S4 "4 layers 45" score is only reproducible
once the reader applies the FACTS section's "5 stats per 4 layers" detail
rather than naively scaling the moderate-model's 3-per-frame figure by
4/3; a one-word annotation ("F=5, not 4×3/3") in that table cell would
save a reader the cross-reference.

## Evidence index consulted
- `.harmony/.reports/s-rta-0928b/diag-media.md` (full text, all sections)
- `SP/runs/seekstats.txt`, `SP/runs/agg-{v,k,iq}.txt`, `SP/runs/pool-{v,k,iq}.txt`
- `SP/runs/v0_1/app-err.log` (line-rate count for §2), `SP/runs/q1_5/app-err.log` (§6 GF-line spot check)
- `SP/instr.diff` (DiagTrace.h, all instrumentation sites), `SP/tools/drive.py` (phase-mark ordering)
- Source at main HEAD: `src/media/VideoPlayer.cpp` (:340-345, :389-402, :441-469), `src/render/Renderer.cpp`
  (:1577-1642), `src/core/UndoService.cpp` (:31-90), `src/model/Clip.cpp` (:215)

## Confidence label
VERIFIED — every headline number in this audit was independently
recomputed from raw evidence files or matched against source line numbers
by direct `Read`/`grep`, not recalled or trusted from the report's prose.
