# lane render — s-rta-0926b (R1..R5 in src/render)

STATUS: PENDING (diagnosis committed before any source change; fixes follow)

Labels: VERIFIED = run/measured in this lane. INFERRED = reasoned from code, not run.
Metric: d(X,Y) = mean |X−Y| over RGB of the whole frame (or the named region), 0..255, decoded with PIL+numpy
from REST `render_frame` PNGs (756×878 preview).

Builds:
- MAIN = `/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app`
  (pre-change baseline, built from 6e8f120 per the packet; binary dated Sep 26 18:12; not rebuilt, read-only).
- TRACE = this worktree at bc69fd0 (code = 6e8f120) + `render-trace.diff` (the s-rta-0926 `xfade-trace.diff`,
  which applies cleanly, plus R1..R5 trace lines). Built in build-lane, copied to the scratchpad; launched with
  `open -g --env AUDIODNA_FBO_TRACE=1`. `parity-trace.diff` does NOT apply any more (`git apply --check` fails on
  CompositorEngine.cpp:1); `xfade-trace.diff` is its superset, so it was used alone.

Reproducer: `render-evidence/diag.py` + `run_diag.sh` (lock, `open -g`, graceful quit). Raw outputs:
`render-evidence/diag-MAIN-pre-change.txt` (MAIN, run dir `scratchpad/runs/diag.bgzGzg`) and
`render-evidence/diag-TRACE-pre-change.txt` (TRACE, run dir `diag.6ySiWO`, trace = its `err.log`).
Trace excerpts cited below: `render-evidence/trace-excerpts.txt`. Frames I looked at myself:
`render-evidence/red_r{1,2,3,4,5}_strip.png`.

## DIAGNOSIS (all five VERIFIED live on the pre-change code)

| Item | Verdict | Pixels (MAIN) | Trace (TRACE build) |
|---|---|---|---|
| R1 | VERIFIED | IN region d(f,IN-ref) = 0.34×d(A,B), OUT region 0.32×; control 0.00 | both chains `u_prev_frame` key 0 = tex14, both save to tex14 |
| R2 | VERIFIED | temporal: persistent layer = 33% deck-0 image (d=9.45); ring: persistent layer shows deck 0's frames (100%, d=29.39) | deck0 L0 and deck1 L0 read/write tex14; both push ring key 0 in one frame |
| R3 | VERIFIED | mid frames lie on OUT-unscaled→IN (resid 0.09–0.13), off OUT-ref→IN (resid 5.3–21.8) | transition samples raw image tex9; no clipTransform draw |
| R4 | VERIFIED | clip transform 28.26, layer fx 228.25, layer transform 30.87, feedback 15.02, clip opacity 9.62, transition = hard cut + frozen | no clipTransform/feedback/layerTransform draw; crossfade frozen at 0.031 |
| R5 | VERIFIED | creation frame fully transparent black (3/3 attempts) instead of 0.5×image | `fx:time_freeze drawFBO=0` sampling the new temporal tex11 as u_texture |

### R1 — outgoing clip's temporal state shares the incoming clip's key during a crossfade (VERIFIED)
Fixture (`diag.py r1`): one layer, OUT = image A + [Freeze 0.5], IN = image B + [Freeze 0.5], **Wipe Left** over
8 s, so each region shows ONE clip. Freeze = mix(current, prev, 0.5); with a still image it converges to its own
input, so the correct IN region equals IN-alone and the OUT region equals OUT-alone.
- MAIN, 8 mid frames t = 1.65…5.91 s: IN region d(f, IN-ref) / d(A,B) there = **0.34, 0.34, 0.34, 0.34, 0.34,
  0.34, 0.34, 0.34**; OUT region d(f, OUT-ref) / d(A,B) = **0.32 … 0.33** (e.g. t=4.07 s: IN 11.04 of 32.76, OUT
  10.06 of 30.96). Predicted by the shared-buffer fixed point: IN = (B + 0.5·A)/1.5 → 1/3 of A.
- Control `r1c` (OUT has no temporal effect, so it never saves): both regions **0.00** in all 8 frames.
- Trace f4490 (`trace-excerpts.txt`): incoming `fx:time_freeze … u_prev_frame@u1=tex14`, `temporalSave key 0 → tex14`,
  then the outgoing `fx:time_freeze … u_prev_frame@u1=tex14`, `temporalSave key 0 → tex14`. Each chain's "previous
  frame" is the other clip's output.
- Looked at: `red_r1_strip.png` (OUT ref | mid frame | IN ref): the mid frame shows the IN grid with a ghost of the
  large OUT ellipses across it.
- Not fixed (design ruling) — see open_forks R1 at the end.

### R2 — persistent layers' state keys collide across decks (VERIFIED)
Fixture (`r2`, `r2b`): deck 0 (active) layer id 0 = A + fx; deck 1 layer id 0, persistent, Opaque/Normal = B + fx
(it covers the frame). Reference = the same composition with deck 0's layer WITHOUT the effect.
- Temporal (`r2`, both Freeze 0.5): subject vs reference **d = 9.45** (3 frames, identical); least-squares fit of
  the frame on the B→A line: **0.33 of A** (predicted 1/3). Reference = B exactly.
- Ring (`r2b`, both Frame Stutter depth 0 / stutter 0 = "1 frame ago"): subject vs reference **d = 29.39** =
  d(A-only, B) 29.41; fit **1.00 of A** — the persistent layer shows deck 0's image instead of its own.
- Trace f7468: deck0 layer 0 and deck1 layer 0 both `u_prev_frame@u1=tex14`, both `temporalSave key 0 → tex14`.
  f8614: `ringPush key 0 writeIndex 35` (deck 0) then `key 0 writeIndex 36` (deck 1) in the same frame.
- Feedback processors are keyed the same way (`feedbackProcessors_[layer.id]`, CompositorEngine.cpp:911) but the
  persistent path never runs feedback today (R4), so that collision is latent: INFERRED, it goes live the moment R4
  adds feedback to persistent layers — so R2 must land with R4.
- Looked at: `red_r2_strip.png` (reference | subject with A ghost | ring subject = A | A-only).

### R3 — applyTransition ignores the outgoing clip's transform (VERIFIED)
Fixture (`r3`): OUT = A with clip scale 0.5, IN = B, Dissolve 4 s. References rendered live: OUT-ref (A scaled),
IN-ref, and OUT-unscaled (A with no transform = what applyTransition feeds the dissolve).
- MAIN, 7 mid frames t = 0.38…3.19 s: fit on OUT-unscaled→IN residual **0.09–0.13**; fit on OUT-ref→IN residual
  **21.78, 19.01, 16.20, 13.60, 10.76, 7.99, 5.28**. The dissolve blends the UNSCALED A.
- R3b, same class, outgoing clip opacity: OUT = A, clip opacity 0.5 + [Invert 1.0]. Active path bakes opacity
  BEFORE effects (applyClipTransform → applyClipEffects) = invert(0.5·A); applyTransition applies effects THEN
  opacity = 0.5·invert(A) (d between the two = 127.45). Mid frames: residual on 0.5·invert(A)→IN **0.10–0.29**, on
  OUT-ref→IN **4.19–16.25**. So an outgoing clip with opacity < 1 and a non-linear effect also changes look (a snap)
  at transition start. Same function, same fix (route the outgoing clip through the same applyClipTransform).
- Trace f2830 (outgoing clip scale 0.5): `transition … u_prevTexture@u1=tex9` (the raw image) and no
  `clipTransform` draw that frame; f2164 (the same clip while active): `clipTransform drawFBO=3 u_texture=tex9`.
- Looked at: `red_r3_strip.png` (OUT ref scaled | mid frame showing the full-size A ellipses | IN ref | OUT-unscaled).

### R4 — compositePersistentLayers skips stages a normal layer gets (VERIFIED)
Fixture (`r4a`–`r4e`): base = deck 0 layer id 0 Opaque = A. Subject layer id 5 (no id shared with the base, so no
R2 interaction), Transparent, Normal blend, Alpha key, clip = B + one stage. Reference = the subject as a NORMAL
layer on deck 0 above the base; subject = the same layer PERSISTENT on deck 1; "stage-absent" = the persistent layer
without the stage.

| Stage | d(persistent, normal reference) | d(persistent, same layer without the stage) |
|---|---|---|
| r4a clip transform (scale 0.5) | **28.26** | 0.00 |
| r4b layer effects ([Invert 1.0]) | **228.25** | 0.00 |
| r4c layer transform (layerScale 0.5) | **30.87** | 0.00 |
| r4d feedback (amount 0.6, scale 0.97) | **15.02** | 0.00 |
| r4e clip opacity 0.5 | **9.62** | 0.00 |

- r4f transition (persistent layer OUT=A, IN=B, Dissolve 8 s; trigger IN on deck 1, leave the deck 0.3 s later):
  every persistent frame t = 0.78…3.63 s has **d(f, IN-ref) = 0.00** (a hard cut, TRACE build run) and, back on
  deck 1 at t = 10.39 s (> 8 s), the frame sits at **p = 0.07** (fit resid 0.09): the crossfade progress does not
  advance while the deck is inactive. Trace: `R4TRACE persistent transition: crossfade x1000 … 31 0 1` in all 17
  traced persistent frames; f13919 shows keying+blend of the IN texture only.
- Trace f10161 (r4a): persistent frame draws only `keying` + `blendLayer` of tex13 (the image); no `clipTransform`.
- Looked at: `red_r4_strip.png` (pairs reference | persistent for r4a, r4b, r4c).
- Stages the docs do not settle (open_forks, not fixed): a persistent **Opaque** layer (normal path clears the
  accumulator = hides the active deck, and applies layer opacity; persistent path alpha-blends and ignores layer
  opacity), **Layer Router output** for persistent layers (`saveLayerOutput` keyed by layer id — a cross-deck id
  collision), **FX Only / Mask** persistent layers (skipped: "for now" comment, CompositorEngine.cpp:1042).
- Found while building r4f (VERIFIED, `r4g`, not an R4 stage): a persistent layer is **invisible when the active deck
  has no active clip** — d(f, B) = 19.46, frame rgb 0.00; after triggering a base clip on the active deck d = 0.00.
  compositeDeck returns 0 (hasActiveLayers_ false) and Renderer then presents the fallback, discarding the
  accumulator the persistent layers were drawn into (Renderer.cpp:500-570). found_not_fixed.

### R5 — getOrCreateTemporalBuffer creates the buffer mid-pass (VERIFIED)
Fixture (`r5`): one layer with a FRESH layer id (77/78/79, never used this run), col 0 = A, col 1 = A + [Freeze 0.5];
burst-capture frames back to back (decode-free) across the 0→1 trigger.
- MAIN: in all 3 attempts the first Freeze frame is **fully transparent black** (rgb 0.00, alpha 0.00, 0/663768
  non-zero pixels) followed by 0.75·A (alpha 191), 0.94·A (239)…: ids 77 (b07), 78 (b08), 79 (b08).
- TRACE (every frame captured): f629 = creation frame: `R5TRACE CREATE temporal buffer key=77 (drawFBO before=3)`;
  after the call `drawFBO 0, targetFBO 3, unit0 tex 11, currentInput 9`; `fx:time_freeze drawFBO=0->tex0 …
  u_texture@u0=tex11 u_prev_frame@u1=tex11` — the pass draws into framebuffer 0 and samples the new (black)
  temporal texture instead of the clip; the pool target tex3 keeps only its glClear, is saved as history and
  presented: `CAPTURE … mean=(0.0,0.0,0.0,0.0) nonzeroPx=0/663768`. f630: `drawFBO=3 u_texture=tex9`, capture
  mean alpha 128 = the 0.5·A frame the creation frame should have been.
- The stray draw into FBO 0 is the real output framebuffer here (`frame start: defaultFBO 0`) but the present pass
  covers the whole 756×878 viewport afterwards, so that draw itself is not visible in this configuration; the
  visible symptom is the one blank layer frame. INFERRED (not run: needs a window resize): a render-size change
  re-creates every temporal buffer on its next use, so every temporal layer blanks for one frame after a resize.
- Only this call site creates GL objects inside a bound pass: ring buffers (Frame Stutter :314, Screen Split
  :1576), layer-output FBOs (:165) and FeedbackProcessor::ensureSize are all created before their pass binds.
- Looked at: `red_r5_strip.png` (frame before | blank creation frame | next frame).

## FIXES
(pending)
