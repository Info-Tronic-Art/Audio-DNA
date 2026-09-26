# Parity-lane diagnosis — s-rta-0926 (clip effect + layer effect → blank frame)

VERDICT: **Could not reproduce on the fixed build, so no fix was made (fixed=false).** Main's build/ app
(12:27:30, contains 4fca2c5) renders the bug fixture (V1: clip Hue Shift 0.5 + layer Saturation 0.8,
opacity 1) correctly every time: **0 blank out of 104 decoded V1 frames** across 12 fresh app launches (8 on main's
build, 46 frames; 4 on a same-source scratch build, 58 frames), plus 13 other variants with 0 blanks. The
V1 output is pixel-identical to the single-chain reference (clip Hue Shift then Saturation, no layer
effects): mean abs difference **0.0**. An instrumented build shows the **pre-fix** mechanism directly (the
Saturation pass samples tex3 while drawing into tex3, and its output is zero). The same build **with** the
4fca2c5 line draws into tex4 and gives a non-zero result. The session-close "post-fix RED" did not recur under
anything I tried; its cause is **UNEXPLAINED** (§4). Per the packet, I stopped instead of fixing.

Labels: VERIFIED = I ran/measured it in this lane. INFERRED = reasoning from evidence. ASSUMED = not checked.

## 1. Step 1 matrix — MAIN build, no rebuild (VERIFIED)

One app session. For each variant: load_composition → 1 s → trigger_clip(0,0) → 2 s → 8 × render_frame
0.3 s apart. Every PNG decoded with PIL+numpy. Blank = alpha>0 fraction < 0.5 or RGB std < 3.

| Variant | Config | frames | blank | alpha>0 | mean RGB | RGB std |
|---|---|---|---|---|---|---|
| V1 | 1 clip fx (Hue Shift) + 1 layer fx (Saturation) — THE BUG | 8 | 0 | 1.0 | (0.0, 45.8, 8.1) | 60.8 |
| V2 | 0 clip + 1 layer | 8 | 0 | 1.0 | (51.0, 0.0, 41.3) | 81.2 |
| V3 | 1 clip + 0 layer | 8 | 0 | 1.0 | (2.1, 34.0, 11.4) | 46.8 |
| V4 | 2 clip (+Invert) + 1 layer | 8 | 0 | 1.0 | (190.4, 159.8, 181.5) | 31.5 |
| V5 | 1 clip + 2 layer (Sat, Invert) | 8 | 0 | 1.0 | (178.0, 159.8, 174.8) | 24.2 |
| V6 | 3 clip (Hue, Invert, Brightness) + 1 layer | 8 | 0 | 1.0 | (234.3, 210.8, 229.8) | 26.4 |
| V7 | V1, layer opacity 0.99 | 8 | 0 | 1.0 | (0.0, 45.1, 8.0) | 59.8 |
| V8 | clip Invert + layer Brightness | 8 | 0 | 1.0 | (215.5, 228.3, 219.2) | 20.3 |
| V9 | V1, clip dryWet 0.99 | 8 | 0 | 1.0 | (0.0, 45.2, 8.1) | 60.0 |
| V10 | V1, blendMode 0 | 8 | 0 | 1.0 | (0.0, 45.8, 8.1) | 60.8 |
| V11 | V1, layer type Transparent | 8 | 0 | 1.0 | (0.0, 45.8, 8.1) | 60.8 |
| V0 | no effects (control) | 8 | 0 | 1.0 | (33.9, 2.0, 24.7) | 51.2 |
| REF | clip Hue Shift → Saturation, no layer fx | 8 | 0 | 1.0 | (0.0, 45.8, 8.1) | 60.8 |
| V1 again (last) | | 8 | 0 | 1.0 | (0.0, 45.8, 8.1) | 60.8 |

Pattern: no blanks (not alternating, not first-N, not random). V1 vs REF: mean abs pixel diff 0.0, max 0.
I looked at a V1 frame: three hue-shifted green/cyan blobs on black, which is real content.

## 2. More attempts to reproduce (all VERIFIED, all non-blank)

| Attempt | Build | Why | V1 frames | blank |
|---|---|---|---|---|
| Existing probe-effects-parity.sh ×3 | main | the gate itself | 9 | 0 |
| Timing: parity probe + probe-mastersignal once each | main | bound Harmony's 16:28 window | 3 (+22/0 ms) | 0 |
| First launch of a freshly linked binary, capture every 0.5 s from t=2 s to 26 s | build-lane (= main source) | "first launch after link" hypothesis | 40 | 0 |
| Harmony's exact 16:27:23Z sequence: pre-fix probe run → configure → build → ctest -j8 (539/539) → probe right away, ×3 (trace on) | build-lane + temporary trace | replay the RED conditions | 18 | 0 |
| Probe sequence ×3 with 10 CPU busy-loops (all cores; Harmony's run overlapped a background graphify rebuild) | main | CPU contention | 18 | 0 |
| Pre-fix control: same, with 4fca2c5's line put back to `writeFBO = 0` | build-lane | shows the probe/trace can see the bug | 12 | **12 (all-zero RGBA)** |

## 3. Trace evidence (VERIFIED; instrumentation saved as parity-trace.diff, removed from src)

`AUDIODNA_FBO_TRACE=1`: for 13 frames after a clip change, and on every frame with a capture pending, it logs
each compositor/present draw. Each line gives: draw FBO → colour attachment, FBO status, every sampler of the
bound program with its unit's texture, a FEEDBACK! flag, glGetError, and a centre 8×8 readback. It also logs
render_frame's readback (read/draw FBO, status, errors, whole-buffer mean, non-zero pixel count).
Texture IDs: accum=1 scratch=2 A=3 B=4 transition=5 feedback=6; image=8.

Pre-fix (writeFBO=0 put back), every traced frame:
```
applyClipEffects enter inputTex/writeFBO/texA/texB 3 0 3 4
fx:saturation   drawFBO=3->tex3 status=0x8cd5 prog=47 u_texture@u0=tex3 FEEDBACK! errDraw=0x0 errRead=0x0 centre=(0,0,0,0)
accumOpaque     drawFBO=1->tex1 ... u_texture@u0=tex3 ... centre=(0,0,0,0)
CAPTURE readFBO=0 drawFBO=0 status=0x8cd5 preErr=0x0 readErr=0x0 rect=(0,0 756x878) mean=(0.0,0.0,0.0,0.0) nonzeroPx=0/663768
```
The first zero draw is the layer-effect pass. It samples tex3 while tex3 is its own colour attachment (a GL
feedback loop). The driver writes (0,0,0,0) and raises no GL error. Every later stage just carries that
zero forward.

Post-fix (4fca2c5 line in place), same sequence, first launch after link:
```
applyClipEffects enter inputTex/writeFBO/texA/texB 3 1 3 4
fx:saturation   drawFBO=4->tex4 status=0x8cd5 prog=47 u_texture@u0=tex3 errDraw=0x0 errRead=0x0 centre=(0,0,0,255)
CAPTURE ... mean=(0.0,45.8,8.1,255.0) nonzeroPx=663768/663768
```
No FEEDBACK! line in any post-fix run (0 in 3 runs). render_frame reads the default framebuffer (FBO 0),
status complete, no GL errors, after the full per-frame render. A capture cannot land between passes: it runs
at the end of `renderOpenGL` on the GL thread.

## 4. Why session close recorded "post-fix RED"

- **What was recorded (VERIFIED from the session transcript):** at 16:28:33Z the probe printed
  "2 PASS / 3 FAIL". All three frames had alpha>0 fraction 0.0 and RGB std 0.0. This was right after the
  merge → configure → build → ctest -j8 (539/539). The handoff note "post 2/3 FAIL" is that line misread:
  3 of 3 frames failed, not 2 of 3. So the evidence never showed intermittency. It was one RED run, followed
  by a GREEN session: probe-mastersignal 22/0 ×3 in the same command.
- **Same binary is GREEN now (VERIFIED):** main's build/ app is the 12:27:30 binary Harmony used, and it
  passes 3/3 probe runs and 14/14 matrix variants.
- **Stale-PNG hypothesis (INFERRED, probably not the cause):** the probe ignores render_frame's response and
  never deletes old PNGs. The RED pre-fix run 80 s earlier had left blank parity_{a,b,c}.png in the same
  default output dir (/tmp/audiodna-effects-parity). So a failed capture would decode the previous run's blank
  frames. But a failed capture costs its 5 s timeout. Measured here: parity probe 9 s, mastersignal probe 16 s,
  configure+build+ctest about 8 s. That adds to about 65 s, against 69.9 s observed for the whole command.
  Three timeouts (+15 s) do not fit, so the captures most likely succeeded and really were blank. (The /tmp
  artefacts and logs are gone, so this cannot be checked directly.)
- **Not the cause, tested:** first launch after link; Harmony's exact build→ctest→probe sequence; full-core
  CPU contention (§2).
- **Honest status: UNEXPLAINED.** A one-off condition on that run produced a zero draw that I could not bring
  back. The trace diff is the tool to use if it ever recurs: re-apply it, launch with
  `--env AUDIODNA_FBO_TRACE=1`, and the first zero draw will be named.

## 5. Could it explain the 11:49 all-zero B1 frames (HANDOFF loose end 3)? — INFERRED, unproven

The two events share a shape. Each was the **first probe launch right after a merge + rebuild + ctest**.
Each had all-zero RGBA early in the session. The ms-white2 fix did not change the path B1 takes (B1 has no
layer effects). Both then went GREEN on every later launch of the same binary. So the two are probably one
transient condition, not two render defects. It is not the V1 aliasing mechanism: that is fixed and verified
in §3. If a blank recurs, keep the app running and re-apply parity-trace.diff before the next launch.

## 6. Minimal reproducer (of the fixed mechanism, for the record)

Pre-fix only: `.harmony/probe-effects-parity.json` (1 clip effect + 1 layer effect, opacity 1, Opaque, no
transform/feedback/crossfade) → the second `applyClipEffects` call gets inputTex == effectTex_A_ and draws into
effectFBO_A_ → all-zero. Fixed by 4fca2c5 (`writeFBO = (inputTex == effectTex_A_) ? 1 : 0`).

## 7. Separate bug seen while tracing (VERIFIED live; NOT fixed — out of scope)

**A crossfade between two clips that each have clip effects never shows the incoming clip.** The frame holds
the outgoing clip for the whole dissolve, then hard-cuts. The cause is the same shared A/B scratch as above,
but this is a clobber, not a feedback loop. compositeDeck runs the incoming clip's effects into effectTex_A_
(tex3). applyTransition then runs the *outgoing* clip's effects, also starting at effectFBO_A_, which
overwrites tex3. The transition draw then samples tex3 for both inputs:
```
fx:invert     drawFBO=3->tex3 u_texture@u0=tex10          (incoming clip -> tex3)
fx:hue_shift  drawFBO=3->tex3 u_texture@u0=tex8           (outgoing clip overwrites tex3)
transition    drawFBO=5->tex5 u_texture@u0=tex3 u_prevTexture@u1=tex3
```
Live check (main build, 6 s dissolve, clip A = Hue Shift, clip B = Invert): at t = 0.5 … 3.3 s into the dissolve,
|frame − A-only reference| = **0.00** and |frame − B-only reference| = 155.93. Expected: a blend moving toward
B. Script: scratchpad/parity/xfade.py. Same code shape for 1 or 2 effects on either clip. For the fix lane:
applyTransition needs the incoming texture kept out of the A/B range its own applyClipEffects call writes to.
One option is to start that call away from `newClipTex` the way 4fca2c5 does for `inputTex`.

## 8. Probe weaknesses (VERIFIED by reading .harmony/probe-effects-parity.sh; not changed — lane stopped at Step 1)

- It ignores the render_frame response and does not delete PNGs left from earlier runs, while reusing a
  default output dir. A failed capture decodes the previous run's file.
- 3 frames only. No parity-to-reference row (V1 vs REF equals 0.0 here, so a threshold near 1.0 is safe).
  No V5/V6 rows. No app-path override (only PARITY_BUILD_DIR, relative to ROOT).
- Worktree runs: PY=$ROOT/.venv/bin/python does not exist in a worktree, so every frame would decode as
  "BLANK". It needs a fallback to the main checkout's .venv or a REFUSE.
