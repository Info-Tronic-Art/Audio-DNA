# lane render — s-rta-0926b (R1..R5 in src/render)

## BUILDER REPORT

STATUS: DONE_WITH_CONCERNS
RESULT: R1–R5 all VERIFIED live on the pre-change build; R2, R3, R4, R5 fixed in `src/render` (4 fix commits on `lane/render-0926b`), R1 delivered as a design fork; new probe `.harmony/probe-render-state.sh` + 2 new `probe-crossfade` rows, all RED on 6e8f120 and GREEN on the fix. This file is the report.
FACTS: the diagnosis commit dac0eb7 comes before every source commit (`.harmony/.reports/s-rta-0926b/render-evidence/diag-MAIN-pre-change.txt`, `.harmony/.reports/s-rta-0926b/render-evidence/trace-excerpts.txt`); fixes are in `src/render/CompositorEngine.cpp`, `src/render/CompositorEngine.h` and the new `src/render/LayerStateKey.h`; unit test `tests/test_layer_state_key.cpp`; probes `.harmony/probe-render-state.py`, `.harmony/probe-crossfade.json`; raw RED/GREEN outputs `.harmony/.reports/s-rta-0926b/render-evidence/probe-render-state-MAIN.txt`, `.harmony/.reports/s-rta-0926b/render-evidence/probe-render-state-FIX.txt`, `.harmony/.reports/s-rta-0926b/render-evidence/probe-crossfade-kl-MAIN.txt`, `.harmony/.reports/s-rta-0926b/render-evidence/probe-crossfade-FIX.txt`, `.harmony/.reports/s-rta-0926b/render-evidence/probe-effects-parity-FIX.txt`.
METHOD: FBO-trace build (`render-trace.diff` = xfade-trace.diff + R1..R5 trace lines, never in a src commit) plus REST fixtures decoded with PIL/numpy on the pre-change app. I wrote and committed the diagnosis, reverted the trace (`git apply -R`), made one commit per fix, ran the live probes RED on `build/` (6e8f120) and GREEN on `build-lane`, and ran ctest serially in build-lane.
CONFIDENCE+VERIFY: high for R2/R3/R4/R5 (every new pixel row goes from RED to GREEN, residuals at the 0.00–0.40 level). Gate: `RSTATE_APP=<build-lane app> .harmony/probe-render-state.sh` gives `PY 19 PASS / 0 FAIL`; `XFADE_APP=<build-lane app> .harmony/probe-crossfade.sh` gives `PY 35 PASS / 0 FAIL`; `PARITY_APP=<build-lane app> .harmony/probe-effects-parity.sh` gives `PY 46 PASS / 0 FAIL`; `ctest --test-dir build-lane` run serially gives `100% tests passed, 0 tests failed out of 583` (583/583: 580 existing + 3 new). RED: the same probes with the `build/` app give `PY 1 PASS / 18 FAIL` and, for rows k,l, `PY 4 PASS / 4 FAIL`.
UNKNOWNS/NOT-DONE: R1 is not fixed (it needs a ruling, open_forks R1). The R4 stages the docs do not settle are unchanged (open_forks R4-opaque, R4-router, R4-types). The R5 re-creation after a window resize was not run live (it needs UI input). VRAM growth from per-deck keys was not measured.
NUANCE: the R3 fix also changes the outgoing clip's OPACITY ORDER. It is now opacity-then-effects, exactly like the active clip (measured RED as row l). R2 keys ALL layer state by deck. As a result a non-persistent layer no longer inherits another deck's history on a deck switch; it resumes its own, possibly stale, history. Each (deck, layer) that uses Screen Split / Frame Stutter now gets its own 480-frame ring (249 MB at 1080p).
HANDOFF-NEEDS: none (open_forks need Boris; docs/claude/rendering.md + performance-controls.md each need a line — outside this lane's fence, see NEXT ACTION)

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
- FIX = `build-lane` at a832d96 (all four fixes; probe commit 0908476 touches only probes).

Reproducer: `render-evidence/diag.py` + `run_diag.sh` (lock, `open -g`, graceful quit). Raw outputs:
`render-evidence/diag-MAIN-pre-change.txt` (MAIN, run dir `scratchpad/runs/diag.bgzGzg`) and
`render-evidence/diag-TRACE-pre-change.txt` (TRACE, run dir `diag.6ySiWO`, trace = its `err.log`).
Trace excerpts cited below: `render-evidence/trace-excerpts.txt`. Frames I looked at myself:
`render-evidence/red_r{1,2,3,4,5}_strip.png`.

## DIAGNOSIS (all five VERIFIED live on the pre-change code; committed in dac0eb7 before any source change)

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

## FIXES (one commit each on lane/render-0926b)

| Item | Commit | Change (src/render only) |
|---|---|---|
| R5 | e657c26 | `applyClipEffects` gets the chain's temporal buffer ONCE, before its first pass binds anything (`TemporalBuffer* tempBuf`, CompositorEngine.cpp:281); the pass binds `tempBuf->tex` and the save uses the same pointer. No per-pass getOrCreate call is left. |
| R3 | f36bc71 | `applyTransition` runs the outgoing clip through `applyClipTransform` (transform + clip opacity) and then `applyClipEffects`, the same stages in the same order as the active clip. `applyClipTransform` gains `holdTex` so none of its passes writes the held incoming result. The separate effects-then-opacity pass into scratchFBO_ is gone. |
| R2 | 6c72dcc | New pure header `render/LayerStateKey.h`: a 64-bit key = deck id (high 32 bits) + layer id (+ the layer-chain bit), and `kGlobalEffects`. Temporal buffers, rings and feedback processors are keyed by it (maps become `uint64_t`). This replaces `kGlobalEffectsLayerId` / `kLayerChainStateBit`. The Layer Router output map is unchanged (fork R4-router). |
| R4 | a832d96 | New `renderLayerStages()` (clip transform + opacity, clip effects, transition, feedback, layer effects, layer transform) and `advanceCrossfade()`. Both are used by `compositeDeck` AND `compositePersistentLayers`, so a persistent layer runs the same code, keyed by its own deck. |

Default reading applied to R4 (packet): "a persistent layer renders exactly like the same layer would as a normal
layer". performance-controls.md says `persistent = true` "keeps a layer rendering even when its deck is not
active" and the P21.6 plan row says "keep playing when deck changes". I read the per-layer rendering stages as
covered by that and fixed them. I read the COMPOSITING step (Opaque replace vs blend over the active deck), Layer
Router publishing, and FX Only / Mask types as places where docs conflict or are silent. Those are open_forks and
unchanged.

## GREEN (VERIFIED on FIX = build-lane @ a832d96)

Diagnosis fixtures re-run on FIX (`render-evidence/diag-FIX.txt`, run `diag.SeI8ct`):
- R5: the creation frame is now 0.5·A (rgb 9.88, alpha 128) in 3/3 attempts (ids 77 b08, 78 b08, 79 b07). There is
  no blank frame.
- R3: mid frames fit OUT-ref→IN with residual **0.13–0.40** (it was 5.3–21.8); R3b residual **0.12–0.33** (it was
  4.2–16.3).
- R2: temporal and ring subject vs reference **0.00 / 0.00** (they were 9.45 / 29.39).
- R4: clip transform / layer fx / layer transform / feedback / clip opacity **0.00** each. For the transition, the
  persistent frames move p 0.09 → 0.21 → 0.32 → 0.44 (resid ≤ 0.12) while the deck is inactive, and back on the
  deck after 10.4 s the crossfade is done (d(f, IN) = 0.00).
- R1 unchanged, as expected (IN region still 0.34×). r4g unchanged (not fixed).
- Looked at: `green_r3_strip.png` (the mid frame shows the SCALED A dissolving into the grid), `green_r2_strip.png`,
  `green_r4_strip.png`, `green_r5_strip.png`, `r5_hold_main_vs_fix.png` (MAIN dark grey 13 | FIX black).

Probe runs (summary lines verbatim):
| Probe | App | Result |
|---|---|---|
| probe-render-state (all 10 rows) | MAIN (6e8f120) | `PY 1 PASS / 18 FAIL` (the 1 PASS is the burst's capture-count guard), `PROBE-RENDER-STATE RED` |
| probe-render-state (all 10 rows) | FIX | `PY 19 PASS / 0 FAIL`, `PROBE-RENDER-STATE GREEN` |
| probe-crossfade rows k,l | MAIN | `PY 4 PASS / 4 FAIL`, `PROBE-CROSSFADE RED` (k: 3 FAIL, l: on-line FAIL; resid 4.42–21.08) |
| probe-crossfade (all 12 cases) | FIX | `PY 35 PASS / 0 FAIL`, `PROBE-CROSSFADE GREEN` (k/l residual ≤ 1.0; a–j unchanged) |
| probe-effects-parity | FIX | `PY 46 PASS / 0 FAIL`, `PROBE-EFFECTS-PARITY GREEN` |

Unit test: `tests/test_layer_state_key.cpp` (3 cases, 184 assertions). It drives the REAL `LayerStateKey` against
decks built by the real `Deck::initDefault` (both decks number their layers 0,1,2, which is the collision
precondition). RED-first: the function does not exist on the base code, so teeth were shown on mutated COPIES of
the header (`render-evidence/teeth-layer-state-key.sh` / `.out`). Dropping the deck id fails 2/3 cases; dropping the
chain bit fails 1/3. The deliverable's sha256 was unchanged before and after (8c46a2a0…). R3/R4/R5 are GL-only (no
GL context in ctest), so the live probe rows are their RED-first reproducers.

ctest (serial, `ctest --test-dir build-lane`): `100% tests passed, 0 tests failed out of 583` = 583/583 passed (580 = the s-rta-0926 close count per the packet, not re-run on the base here, + the 3 new LayerStateKey cases, #30-#32). Raw output: `render-evidence/ctest-FIX.txt`.

## Probe changes
- NEW `.harmony/probe-render-state.{sh,py,json}` uses the probe-crossfade pattern: `open -g`, graceful quit, REFUSE if
  running, fresh `mktemp -d`, every render_frame response checked, every PNG decoded. Overrides: `RSTATE_APP`,
  `RSTATE_PY`, `RSTATE_ENV`. Rows: r2_temporal, r2_ring, r4_clip_transform, r4_clip_opacity, r4_layer_effects,
  r4_layer_transform, r4_feedback, r4_transition, r5_hold, r5_burst. Calibration is in the .py docstring.
  - r5_hold is the deterministic R5 row: [Screen Split, Freeze 1.0] on a fresh layer. Freeze 1.0 outputs its
    history, and a fresh history is cleared to transparent black. On 6e8f120 the Freeze pass's target kept only
    its glClear (Screen Split's grey, 13/255) and that became the held history forever; on FIX it is (0,0,0,0).
  - r5_burst is the visible symptom (the blank frame). It is RED only when a burst captures the creation frame (it
    did in every attempt on 6e8f120: 3/3 in the diagnosis, ≥3/5 in the probe run). It cannot false-FAIL on a fixed
    build.
- `probe-crossfade`: crossfades **k_outgoing_transform** and **l_outgoing_opacity_effect**. There is a per-case
  "on-line" check (`lineTol`, least-squares residual vs the OUT→IN line), used only by k/l because the old 10%
  "between" slack hides case l (excess 2.4–10.2 vs slack 23.6). Clip `extra` fields pass through. Rows a–j and
  their thresholds are unchanged.

## open_forks (details; the structured copy is in the lane's return)

### R1 — per-clip vs per-layer temporal history during a crossfade (DESIGN RULING NEEDED; not fixed)
- Verified mechanism: during a crossfade the incoming clip's chain and the outgoing clip's chain use the SAME key,
  so they share one temporal buffer (u_prev_frame) and one Screen Split / Frame Stutter ring. Each frame, each chain
  reads what the other saved. Measured at Freeze 0.5: each clip shows 1/3 of the other (IN region 0.34×, OUT region
  0.32–0.33× of d(A,B); control without a temporal effect on OUT 0.00).
- Keying today (HEAD a832d96): `renderLayerStages` computes `clipKey = LayerStateKey::clipChain(deckId, layer.id)`
  (CompositorEngine.cpp:821). It is used by the clip chain (:827) and passed to `applyTransition` (:832), whose
  outgoing chain uses it too (:1398). The layer chain has its own key (:846). At 6e8f120 it was `layer.id` at :901
  (clip chain) and :1343 (outgoing chain); the layer chain was `layer.id | kLayerChainStateBit` at :919-920.
- Sizes: a temporal buffer = one RGBA8 texture at render size. That is 8,294,400 B (7.9 MiB) at 1080p, 33,177,600 B
  (31.6 MiB) at 4K, and 2,654,976 B at the 756×878 preview. A frame ring = 480 RGBA8 textures + 480 FBOs at ¼ × ¼ of
  render size: 248,832,000 B (237 MiB) at 1080p, 995,328,000 B (949 MiB) at 4K, 79,470,720 B at the preview. (The
  header comment "~120MB at 480x270", CompositorEngine.h:223, is off by 2×.)
- Per layer today: at most 2 temporal buffers (clip chain + layer chain) and 2 rings, per (deck, layer) since R2
  (per layer id before), plus one of each for Global Effects.
- Lifetime today: created lazily on the first frame a chain with Echo / Posterize Time / Freeze (buffer) or Screen
  Split / Frame Stutter (ring) runs. Re-created on a render-size change. Freed only when the GL context closes
  (`releaseGL`). Never freed on clip change, layer delete or composition load.
- Options:
  - A. Per-clip history (key = deck + layer + clip) with a lifetime rule (free when the clip has not rendered for
    N s, or an LRU cap per layer).
    - Memory: +7.9 / +31.6 MiB (1080p / 4K) per clip with a temporal effect, +237 / +949 MiB per clip with Screen
      Split / Frame Stutter, until freed.
    - Frame-time risk: the first trigger of such a clip allocates on the render thread (a 480-texture ring = a
      hitch; not measured) unless the rings are pooled.
    - Performer sees: each clip keeps its own trails through a fade. Re-triggering a clip resumes its OLD trails
      unless they are also cleared on trigger.
  - B. Per-layer "outgoing" slot (handle swap). At transition start the layer's history (it belongs to the outgoing
    clip) moves to the layer's outgoing slot. The incoming clip starts on a cleared history (buffer: one glClear;
    ring: frameCount = 0, no clears). At transition end the slot returns to a per-layer spare.
    - Memory: at most +1 buffer and +1 ring per layer that crossfades between clips using those effects (+7.9 MiB
      / +237 MiB at 1080p), allocated once (a hitch on the first such fade, or pre-allocate).
    - Frame-time: none per frame; one clear per transition start.
    - Performer sees: the outgoing clip's trails continue unbroken through the fade. The incoming clip's trails
      build from nothing, hidden by its low weight at the start of the fade. Cuts are unchanged.
  - C. Bypass the outgoing clip's history effects during a fade; the incoming clip takes the layer's history,
    cleared at transition start.
    - Memory: 0. Frame-time: one clear.
    - Performer sees: the moment a fade starts, the outgoing clip's Echo / Freeze / Stutter vanishes (a visible
      pop).
  - D. Keep as is.
    - Memory: 0.
    - Performer sees: during any fade where both clips use history effects, each clip ghosts the other (1/3 at
      Freeze 0.5).
- Recommendation: **B**. Memory is bounded (one spare per layer that needs it). There is no pop on either side, the
  outgoing clip looks exactly as it did before the fade, and it reuses today's per-layer buffers. Choose A only if
  Boris wants a re-triggered clip to resume its own old trails.

### R4-opaque — how a persistent Opaque layer composites over the active deck (not fixed)
- Facts: on the active deck an Opaque layer clears the accumulator and draws with `opacity_blend` (layer opacity
  applied; `compositeDeck`, Opaque branch). The persistent path calls `blendLayerOntoAccumulator` (the layer's
  blend mode; for Normal that is a plain alpha blend with passthrough). So a persistent Opaque layer's layer
  opacity is ignored.
- Docs conflict: Layer.h says Opaque "replaces everything below"; performance-controls.md says persistent layers are
  "composited ... after the active deck's layers".
- Options:
  - (1) Blend over the active deck (today) but honour layer opacity.
  - (2) True Opaque: hide the whole active deck.
  - (3) Keep as is.
- Recommendation: (1).

### R4-router — Layer Router output from persistent layers (not fixed)
- Facts: `saveLayerOutput(layer.id)` runs only on the active deck, and `layerOutputTextures_` is keyed by layer id
  alone. A persistent layer from another deck publishing under its id would overwrite the active deck's same-id
  layer (the R2 class).
- Options:
  - (1) Don't publish (today).
  - (2) Deck-scoped key plus a deck selector on the Layer Router.
  - (3) Publish only when the active deck has no layer with that id.
- Recommendation: (1) until the router can address a deck.

### R4-types — FX Only / Mask (and media-less effect clips) as persistent layers (not fixed)
- Facts: `compositePersistentLayers` skips every type but Opaque/Transparent ("for now"). It also skips an
  Opaque/Transparent clip with no media, which the active-deck path applies as FX Only.
- Options:
  - (1) Support them: FX Only persistent = a look over whatever deck is active; Mask persistent = masks the active
    deck.
  - (2) Keep them skipped and disable the Persistent toggle for those types.
- Recommendation: (1) for FX Only, (2) for Mask until asked.

## found_not_fixed
1. **Persistent layers vanish when the active deck has no active clip** (VERIFIED, `r4g`: d(f, B) = 19.46, frame
   rgb 0.00; triggering any clip on the active deck brings it back, d = 0.00). `compositeDeck` returns 0 when no
   active-deck layer is active. `Renderer::renderOpenGL` (Renderer.cpp:500-570) then presents the image/source/black
   fallback and discards the accumulator the persistent layers were drawn into. It is in src/render but is not R2-R5.
2. CompositorEngine.h:223 comment "~120MB at 480x270" — the real 1080p ring is 248.8 MB.
3. Non-persistent layers on inactive decks keep a frozen crossfade progress (a fade started on deck B resumes when B
   is active again). INFERRED from code; the persistent variant was VERIFIED and fixed.
4. docs/claude/rendering.md ("`layerTemporalBuffers_` map keyed by layer ID") and performance-controls.md (persistent
   layers) no longer match the code. docs/ is outside this lane's fence.

## RISKS
- VRAM (medium). Keys are now per (deck, layer), so a composition with several decks that use Screen Split / Frame
  Stutter on the same layer index now holds one 480-frame ring per deck (237 MiB each at 1080p) instead of one per
  layer index. Nothing frees them (pre-existing, see R1 facts). Mitigation: the R1 ruling's lifetime rule should
  cover rings. The cheaper alternative, per-deck keys for persistent layers only, is also correct for the collision.
- Behaviour change at a deck switch (low, INFERRED, not run). A non-persistent layer used to start with the other
  deck's same-index history (Echo/Freeze/Stutter briefly showed the other deck). It now resumes its own last history,
  which may be stale by the time spent on the other deck.
- R3 opacity order (low, intended, measured). An outgoing clip with opacity < 1 AND effects now renders as it did
  while active (opacity before effects). Before, it switched to effects-before-opacity at the start of the fade.

## Rig / screen-safety log
- Every live run held `/tmp/audiodna-live.lock` with owner `render <pid> <epoch>`, required `pgrep` empty, launched
  with `open -g [--env AUDIODNA_FBO_TRACE=1] --stdout/--stderr`, quit via osascript (pkill only as the 30 s fallback;
  every run ended with the app terminated), and released the lock. There was no lldb/debugger, no screencapture, no synthetic input, no Output window, and no system dialog
  seen.
- `.venv` symlink not used: the probes fall back to the main checkout's `.venv` through the git common dir.

### SUMMARY
All five suspected render defects were reproduced live on the pre-change build. Four were fixed in src/render, each
in its own commit with a RED-first live probe row: R5 (temporal buffer created mid-pass), R3 (the outgoing clip lost
its transform, and its opacity order, during a crossfade), R2 (per-deck state keys) and R4 (persistent layers run the
same per-layer stages as active-deck layers). R1 needs a design ruling and is delivered as open fork R1 with sizes
and options.

### FILES CHANGED
- `src/render/CompositorEngine.cpp`
  - R5: the temporal buffer is created before the chain's passes.
  - R3: applyTransition runs the outgoing clip through applyClipTransform, then its effects.
  - R2: every state map lookup takes a LayerStateKey.
  - R4: new advanceCrossfade + renderLayerStages, shared by compositeDeck and compositePersistentLayers.
- `src/render/CompositorEngine.h`: 64-bit state keys; the old key constants are removed; `applyClipTransform(...,
  holdTex)`, `applyTransition(layer, stateKey, ...)`, `applyFXOnlyLayer(..., stateKey, ...)`, and new
  `advanceCrossfade` / `renderLayerStages` declarations with comments.
- `src/render/LayerStateKey.h` (new, pure): deck + layer (+ chain) key and the Global Effects key.
- `tests/test_layer_state_key.cpp` (new, 3 cases) + `tests/CMakeLists.txt` registration.
- `.harmony/probe-render-state.{sh,py,json}` (new): R2 / R4 / R5 rows.
- `.harmony/probe-crossfade.{sh,py,json}`: cases k / l (R3), per-case `lineTol` check, clip `extra` pass-through.
  Existing rows unchanged.
- `.harmony/.reports/s-rta-0926b/render.md` (this report), `render-trace.diff`, `render-evidence/` (fixtures, raw
  outputs, strips, teeth).

### TESTS
- LayerStateKey (3 cases, #30-#32): PASS — the same layer id in two decks gets two keys; all keys unique and never
  the Global Effects key; stable keys. Teeth on mutated copies: 2/3 and 1/3 FAIL.
- Full ctest serial: `100% tests passed, 0 tests failed out of 583`.
- Live probes (FIX): probe-render-state `PY 19 PASS / 0 FAIL`; probe-crossfade `PY 35 PASS / 0 FAIL`;
  probe-effects-parity `PY 46 PASS / 0 FAIL`.
- Live probes (MAIN, RED-first): probe-render-state `PY 1 PASS / 18 FAIL`; probe-crossfade k,l `PY 4 PASS / 4 FAIL`.

### SLIM CHECK
nothing to cut. advanceCrossfade / renderLayerStages each have two callers. The `holdTex` param is exercised by
applyTransition. The pre-existing `if (processedTex == 0)` guard in compositePersistentLayers is now unreachable but
predates this lane (left, not mine to remove). The trace instrumentation is not in any src commit
(`grep -rn FboTrace src/` is empty).

### ISSUES
- `parity-trace.diff` no longer applies to the code (CompositorEngine.cpp:1). `xfade-trace.diff` applies and is a
  superset, so it was used (+ R1..R5 lines, saved as `render-trace.diff`).
- `build-lane/` shows as untracked in `git status` (not covered by `.gitignore`). It is kept on purpose per the
  packet (Harmony deletes it at merge). Everything else is clean.
- The Write tool refused a report-shaped scratch file once, so the report was written directly here (the packet's
  required path).

### SKILL_PROPOSALS
- "gl-state-reproducer": when a GL defect is one frame long, look for a deterministic amplifier that makes it
  persistent before relying on frame-aligned captures. Example: Freeze 1.0 holds whatever the creation frame saved,
  and a preceding Screen Split sets a non-black clear colour. Trigger: a probe row that must be RED for a one-frame
  glitch. Existing probe habits cover pixel decoding, not this.

### METRICS
- Self-check: build-lane full build rc=0 (only the pre-existing `totalCells` unused warning in
  CompositorEngine.cpp); ctest 583/583 serial; live probes as above; `git diff` read per commit.
- Tool calls: ~150. Files read: ~25.

### KNOWLEDGE CONTEXT
- Tools used: grep (no KNOWLEDGE_TOOLS block in the packet). Impact authority: grep (not authoritative). I took a
  conservative posture: every caller of the changed CompositorEngine functions was grepped across src/ and tests/
  (only CompositorEngine.cpp and Renderer.cpp call in). God nodes: n/a. Risk level: NORMAL.

### PACKET QUALITY
- Clarity: CLEAR. The R4 rule ("default reading ... if the docs are silent or conflict on a stage, do NOT fix that
  stage") reads two ways. I applied the default reading to per-layer rendering stages (performance-controls.md
  "keeps a layer rendering" supports it) and forked the compositing / router / types questions.
- Missing context: parity-trace.diff no longer applies (xfade-trace.diff does); JUCE's default framebuffer is 0 in
  this setup (so the R5 stray draw lands in the real output FBO); render_frame capture cadence (every frame vs every
  other frame) differs between the trace build and the plain build.
- Unused context: none.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: xfade-report.md §5, parity-diagnosis.md §7-8, review-xfade-r1.md, notebook "wave 2 — xfade +
  fader" and "ms-white2" (all useful, current); docs/claude rendering / performance-controls / testing-eyes / pitfalls
  / effects (useful).

### STATUS
DONE_WITH_CONCERNS — every packet success criterion is met. The concerns are the VRAM growth from per-deck ring
keys and the deck-switch history behaviour change (see RISKS), plus R1 and three R4 sub-questions waiting on Boris.

### NEXT ACTION
- Boris: rule on open_forks R1 (recommend B), R4-opaque (recommend 1), R4-router (recommend 1), R4-types (recommend
  1 for FX Only).
- Harmony (outside this lane's fence):
  - docs/claude/rendering.md: temporal buffers / rings / feedback are keyed by LayerStateKey (deck + layer + chain),
    not layer ID.
  - docs/claude/performance-controls.md: persistent layers get every per-layer stage and keep crossfading while
    their deck is inactive.
  - Notebook lesson: a one-frame GL glitch can be made deterministic (Freeze 1.0 + a non-black glClear) for a RED
    row.
  - Consider found_not_fixed #1 (persistent layers vanish when the active deck has no active clip) for a follow-up
    lane.

INBOX-RECHECK: none (no addenda received during this run)
