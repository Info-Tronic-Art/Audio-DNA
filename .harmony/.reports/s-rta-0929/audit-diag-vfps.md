# Reviewer Verdict — audit-diag-vfps
STATUS: DONE
VERDICT: SOUND_WITH_GAPS

## Scope
Independent method audit (read-only) of `.harmony/.reports/s-rta-0929/diag-vfps.md`, its
`instr.diff` / `juce-instr.diff`, its `diag-vfps-tools/` scripts, and its raw evidence under
`scratchpad/diag-vfps/runs/` (222 diag.tsv launch dirs, `agg/*.txt`).

## What I independently re-derived (not recall — recomputed from the raw files)
- Instrumentation overhead / observer-effect risk: LOW. The per-frame recorder
  (`DiagVfps.mm`) is a lock-free SPSC ring drained by a background thread every 50ms;
  `dvf::rec()` on the GL/decode/render hot paths is an atomic fetch_add + struct write
  (no I/O, no lock). `fprintf` to disk happens only on the drain thread. The JUCE hooks
  (`juce-instr.diff`) add one function-pointer call + `mach_absolute_time()` per
  display-link tick / swapBuffers / condvar wake — negligible next to the ms-scale
  effects being measured. Critically, `dvfUpload()`'s no-arm (base) path is
  structurally identical to the pre-instrumentation `glTexSubImage2D` call (same branches
  execute in every arm), so the overhead is symmetric across base vs. counterfactual arms
  — a fair A/B, not a confound. GPU timing reuses the pre-existing double-buffered
  `GL_TIME_ELAPSED` query already in `Renderer.cpp` (no new pipeline stall introduced).
- Q1 arithmetic — VERIFIED by direct recomputation from `agg/q1-table.txt`,
  `agg/q1-bunch-sequential.txt`, `agg/q1-lostpos.txt`, `agg/q1-heavyphase-base_w1.txt`,
  and `runs/*/q1.json`: all cited medians/ranges/pattern splits reproduce exactly
  (e.g. pattern-4 n=20 range [103.8,115.3] vs spread n=9 range [117.6,119.5];
  poll-fps medians for cap1/cap2 recompute to 119.99/119.98, matching "120.0/120.0";
  uncapped pattern-4 windows sum to exactly 291/291 across 10 separate run directories;
  `video_late_frames` delta is 0 in every one of 222 `windows.json` entries with no
  exceptions).
- Q2 share table — VERIFIED: A+B+C sums to the base→120 gap in both sets
  (30.25/29.45 fps), each share's percentage recomputes correctly, and every cited
  fps/GPU/GL-CPU number reproduces exactly from `agg/q2-table.txt` / `q2b-table.txt`.
  VideoToolbox pricing (`agg/vtbench.txt`) recomputes to 14.4ms/0.87ms CPU per frame and
  333.3 fps aggregate for 4 concurrent streams — matches the cited 14.2/0.87/333 within
  rounding.
- RED-able gate is real: the unmodified main app (`agg/main-reference.txt`) reads
  peak_callback_ms 6.30–6.47ms (>4.0ms threshold) in 15/15 loads across two independent
  15-load sets, and poll-fps medians 112.7 / 115.8 (<118.5 threshold) — the proposed gate
  would indeed read RED on current main, and the cap1/cap2 prototypes clear both
  thresholds (2.08/3.48ms, ~120.0 poll fps).
- Cross-checked several "unattributed / correlation-only" claims (heavy-frame vblank
  phase vs. fps, E-core placement 40–67% slow / 6–21% fast windows, colbase occlusion
  r1–r4 wholly occluded / r5 partially) directly against the raw per-window JSON —
  all hold as stated and are correctly hedged as INFERRED/correlation, not causal.
- `CGLTexImageIOSurface2D` onto `GL_TEXTURE_2D` failing with CGLError 10008 (used to
  explain the invalid IOSURF=2 black-frame arm) is directly confirmed in
  `runs/q2b_iosurf2/r2/app-err.log`.

## Findings

### MUST
1. **Overclaimed correctness verification for the Q2 IOSurface fix ("byte-identical", "max |diff| 0").**
   `diag-vfps.md` §2 states: *"Correctness of c4a: the canvas captured through `render_frame`
   is byte-identical to the shipped path at the same frame (codes 211 and 212: max |diff| 0),
   3 captures. VERIFIED."* — and the top-line RESULT paragraph repeats this as *"gives a
   byte-identical picture."* This is not what the tooling measured. `tools/drive.py`'s
   `DRIVE_CAPTURE` path (lines ~152-166) never computes a pixel/byte diff between two PNGs;
   it computes ONE coarse 10-bit "barcode" per capture (whether the mean luma of 10
   macro-regions is >= 128) plus a mean-RGB-to-1-decimal print. I grepped every log/script in
   the scratchpad and confirmed no byte-diff computation exists anywhere in the session
   (`grep -rn "byte-identical\|max_diff\|maxdiff" .` → zero hits outside the report itself).
   The evidence that actually exists (`runs/q2b_base/r*/drive.out`, `runs/q2b_iosurf1/r*/drive.out`)
   is: base and iosurf1 alternate between codes 211/212 and matching mean-RGB triples
   (5 base captures + 6 iosurf1 captures = 11 capture events, not "3"), which is consistent
   with — but far weaker evidence for — pixel-identity than the stated "max |diff| 0". A
   10-region luma threshold cannot detect localized artifacts (e.g. the blit's edge/rounding
   behavior, or the fact that the smoke2 format arms bgra1/client/pbo produced yet other codes,
   213/214, that were never reconciled against base).
   **Fix:** either (a) actually run a real per-pixel diff (numpy array subtraction, not the
   luma-code proxy) between a base-arm and c4a-arm PNG at a matched frame before calling this
   VERIFIED, or (b) downgrade the claim's language and confidence to match what was measured
   ("mean RGB and a coarse region code matched across N captures; a true pixel diff was not
   run") in both §2 and the top-line RESULT. This matters because a builder acting on option 2
   ("Q2 -- IOSurface-backed ring slots + one GPU blit") would reasonably treat "byte-identical,
   VERIFIED" as license to skip a real correctness check before shipping.

### SHOULD
2. **Launch/sample bookkeeping in the FACTS section doesn't reconcile.** The report states
   "218 instrumented launches... (2,806 samples)" for the display-state-identical claim. Direct
   count of `runs/*/*/diag.tsv` gives 222 launch directories and 3,081 total `D` lines; even after
   excluding the 6 smoke/smoke2 dirs (46 D-lines) the totals are 216 dirs / 3,035 D-lines, and after
   further excluding the 3 known tainted-extra files (~14 lines each) it's still ~213/~2,993 — none
   of the exclusion combinations I tried lands on exactly 218/2,806. This is very likely a
   categorization question (which launches count toward "218": main-app launches have zero D-lines
   and are correctly excluded, but I could not reconstruct the exact rule). It does not affect the
   substance — I independently re-verified the qualitative claim (max 120fps / 8.333ms refresh /
   appActive=0 / key=0 in every single D-sample across the FULL 3,081-line corpus, no exceptions) —
   but the specific tally in a VERIFIED-tagged fact should be corrected or its derivation shown.

### NIT
3. **Colbase occlusion comparison off by ~0.5fps.** §4 "What remains unattributed" states occluded
   colbase r1-r5 median "roughly equals" visible r6-r10 median, "~117.5 vs ~117.2" — recomputing from
   `runs/colbase/q1.json` gives r1-r5 median 117.5 (matches) and r6-r10 median 116.7, not 117.2. Already
   hedged with "roughly equals" and "INFERRED", and the qualitative conclusion (occlusion is not the
   fps explanation) is unaffected, so this is cosmetic.
4. **"3 captures" undercounts the actual capture evidence** for the c4a correctness claim (see MUST #1)
   — 11 capture events exist across the two relevant arms (base=5, iosurf1=6), not 3. Rolled into the
   fix for MUST #1.

## Verdict rationale
The causal diagnosis itself (Q1's per-load clock-phase bunching mechanism, the counterfactual
upload-cap tables, the Q2 attribution-by-difference table, the VideoToolbox pricing, and every
correlation explicitly labeled INFERRED/unattributed) is unusually well-verified: every number I
spot-checked against the raw per-launch JSON and TSV reproduced exactly, arithmetic (shares summing
to the gap, uncapped-pattern-4 windows summing to 291/291 across 10 separate directories) checks out
to the integer, the instrumentation is lightweight and symmetric across arms (no observer-effect
confound), and the proposed RED-able gate genuinely reads RED on unmodified main. The one real gap is
a specific, VERIFIED-tagged correctness claim (byte-identical / max diff 0) for the Q2 fix option that
the session's own tooling never actually computed — a genuine spec/claim-fidelity miss that should be
fixed before anyone treats the IOSurface-blit path as picture-correctness-proven. That gap, plus a
non-reconciling bookkeeping total, is why this is SOUND_WITH_GAPS rather than SOUND: the diagnosis's
conclusions (what causes the fps loss, and by how much) stand; one of its ancillary "the fix's output
is provably identical" claims does not.
