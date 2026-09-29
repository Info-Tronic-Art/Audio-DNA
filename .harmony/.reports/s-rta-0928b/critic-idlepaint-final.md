# Critic Verdict — idlepaint (final, lane/idlepaint 5c5f21d..34a8cf8)
STATUS: PARTIAL
VERDICT: FAIL (one MUST — LOGIC hat: addendum 3 L2 "v4 becomes INFO" is not implemented in code)

## VISUAL hat
PASS. Reproduced independently in $TMPDIR/critic scripts against the disk evidence (not taken on trust):
- Raw full-frame diff of production idle captures main/s1.png vs lane/s0.png (fix2/p1) = 18,982 px differing
  (max delta 211) BEFORE masking — all of it inside the SignalBar/waveform/TopBar-beat/fps rects (live audio content,
  real mic input, values differ legitimately: Sub 0.31/0.25, Air 0.17/0.92, Tempo 135/"---" — confirmed by eye,
  cropped comparison). AFTER applying the v3p masks (signalbar_rect [8,134,3440,168]px, waveform_rect
  [8,1970,756,120]px, topbar_rect fps + beat/tempo sub-rect, all derived from the real GET /api/debug/ui_paint
  geometry + title=28pt, scale=2 — matches the plan's "ASSUMED 28pt" exactly) the diff is 0 px outside masks, 0 max
  delta — matching the report's v3p r1 exactly (fd456e5/8d8bc40 rows, F3.log). r2 of the same pair type reads 33 px
  at max delta 1 (sub-JND, matches the reported noise floor).
- Eyeballed the Files-grid folder-icon crop (main vs lane, both idle, production) side by side: indistinguishable.
- Eyeballed fix2/look-v3p-k1.png (the WITHDRAWN K1 arm vs main): visibly different in the folder-icon crispness
  once K1 is applied — corroborates why K1 was correctly withdrawn (it would make the lane diverge from main).
- Conclusion: main idle == lane idle at <=1/255 outside declared live-content masks, and main-after-a-pass ==
  lane-after-a-pass (13,507px/66 class present identically in both, per the report's fix-round-2 production
  measurement, table "K1 -- premise check"). The stated measurement (main idle == lane idle 0px; main-after-pass ==
  lane-after-pass 0px) holds under my own re-derivation.

## UX hat
PASS. F3.log (fix round 2, this exact HEAD's code unless noted): `v2b_fallback_frames (I1, under g4's routine):
10 fallbacks over 5 panel+menu cycles, a layer showed over an overlay in 0 vblank(s) (== 0)`; `(I4): the layer is
back <= 2 vblanks after the overlay closed (max 1)`. A real parented PopupMenu over the SignalBar also fell back and
returned (0 covered). Animation rates (g1, F1.log): waveform/signalbar layer draws 29.1-29.2/s (in [24,36]), TopBar
paints 14.9/s (in [12,26]), modes native [0,0], 0 fallbacks — rates preserved at 30/30/15 Hz as specified.

## LOGIC hat
Overlay types checked against OverlayWatch.{h,cpp} (src/ui/OverlayWatch.cpp, read in full): explicit
(BindingOverlay, MidiLearnOverlay) + `isRootOverlay` (any `juce::TooltipWindow`, or a root child added after the
ctor baseline — covers the ClipCell drag image and a recreated TooltipWindow) + `isWindowOverlay` (any top-level
child that isn't the content, a ResizableCornerComponent or a ResizableBorderComponent — covers the 7
`withParentComponent` menu sites). This matches plan 2.3 verbatim; NativeLayerCache's Native->Fallback transition
is synchronous (`setFallback` calls `sink_.layerShown(false)` + `sink_.peerNeedsDisplay(...)` in the same call,
matching Harmony ruling I1) — read in full, matches the adopted design.

BLOCKING FINDING (verified on disk, not inferred): addendum 3 ruling L2 says "v4_full_pass_identity becomes an
INFO row (it documents the first-pass class; it is not a lane-vs-main identity)." Commit 34a8cf8 — the ONLY commit
after fix round 2, whose message is literally "v4_full_pass_identity is INFO (addendum 3 L2)" — changes exactly
one line in `.harmony/probe-idle-paint.py`: a comment (`git show 34a8cf8 --stat` = "1 file changed, 1 insertion(+)").
`row_v4()` (probe-idle-paint.py:1062-1088) is unchanged: it still ends with
`(ok if all(r[2] == 0 for r in res) else no)(f"v4_full_pass_identity (K3): ...")` — i.e. it still calls `no()`
(FAIL, increments the global FAIL counter that the script's exit code depends on: `sys.exit(1 if FAIL else ...)`)
on a violation, exactly like a gating row. `v4_full_pass_identity` is also still in `DEFAULT_ROWS` (only `x1` and
`v1b` are excluded from the default row set), so it runs by default. Since the row is DESIGNED to always find the
13,507px/66-delta class on any build (main included — that is the whole point of the addendum-3 finding), the
default `probe-idle-paint.sh` run will ALWAYS end with FAIL>0 and exit code 1, contradicting the ruling that v4
should be advisory-only. Confirmed empirically, not just by reading the source: `S/fix2/runs/F3.log` (this
worktree, fix round 2, same row body) reads `FAIL v4_full_pass_identity (K3): ...` and the run's summary line reads
"16 PASS / 2 FAIL / 0 SKIP" — v4 is one of the 2 FAILs. Nothing in 34a8cf8 changes that code path.
This is exactly the toothless-gate pattern review dimension 7 asks for: the commit message and the addendum both
CLAIM v4 is now informational, but the executable behavior still gates on it. A merge/CI step that checks this
probe's exit code will fail today even though Harmony explicitly ruled the opposite.
Fix (mechanical, not re-litigating the ruling): in `row_v4()`, replace the final `(ok if ... else no)(...)` with an
unconditional `info(...)` call (mirroring `x1_within_build_off`'s pattern), and update the module docstring's
"Exit 0 iff no FAIL" row description so v4 is listed as INFO like g3/g5/x1. No pixel-identity or gate-threshold
code needs to change — only the verdict plumbing for this one row.

## SLIM
No excess found in this lane's diff (`git diff --stat 5c5f21d..34a8cf8 -- src tests` = 20 files, all additions map
to the plan's named files; nothing orphaned).

## Other dimensions (spot-checked, no blocking issues)
- Readability/Patterns/DRY: NativeLayerCache/OverlayWatch/NativeLayerHost read clean, comments cite the exact JUCE
  behavior + line numbers they rely on (matches the plan's own VERIFIED citations).
- Docs fidelity: `docs/claude/pitfalls.md` NN and `CLAUDE.md`'s "Periodic repaints" line match the shipped
  mechanism and the measured numbers (21.8->4.5ms / 347->135ms/s etc.) — no overclaiming found there.
- g4 CPU (163 ms/s > 150) is correctly reported as INFO per ruling J2, and IS wired that way in code
  (`gate(row, runs, ..., cpu_gate=False)` at probe-idle-paint.py's g4 dispatch) — this is the contrast case that
  shows the v4 wiring is the outlier, not the pattern.

SUMMARY: 1 file (`.harmony/probe-idle-paint.py`) reviewed for the addendum-3 delta; lane diff (20 files, 5c5f21d..34a8cf8)
spot-checked against the plan. 1 blocking issue (v4 verdict wiring contradicts addendum 3 L2, verified by direct
code read + F3.log execution evidence), 0 suggestions. VISUAL and UX hats: PASS, independently reproduced from disk
evidence. LOGIC hat: FAIL on the one item above; overlay-rule coverage itself is correct.
