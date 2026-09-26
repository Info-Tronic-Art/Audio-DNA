# xfade lane — s-rta-0926 (crossfade between two effected clips never showed the incoming clip)

VERDICT: **Fixed, and the bug class is closed in `src/render`.** A Dissolve between two clips that both
have clip effects now blends from the outgoing to the incoming clip. Measured: mid frames at d(f,OUT) =
p·d(OUT,IN) instead of 0.00. The cause was the one already diagnosed: the outgoing clip's effect chain
overwrote the incoming clip's result in the shared scratch texture. Instead of patching one more call site, I
replaced the three per-site patches with **one rule**: every pass that renders into the effect scratch pool
takes its target from `pickEffectTarget(readTex, holdTex)`, so it never writes the texture it samples or a
texture its caller still holds. The class sweep found **6 more live instances of the same shape** (3 visibly
wrong, 3 undefined GL feedback loops that happen to look right on this driver). All 6 are fixed and each has
a probe case. Three sites are still broken but not fixed; they are listed in section 5.

Labels: VERIFIED = I ran or measured it in this lane. INFERRED = reasoned from code or evidence, not run.
Builds: MAIN = `/Users/boriskarpman/projects/RealTimeAudio/build/…/Audio-DNA.app` (e1ed9cc, unmodified).
FIX = worktree `build-lane` at 5dc1272. PRE-TRACE = e1ed9cc + `parity-trace.diff` (+ a keying hook).
POST-TRACE = 5dc1272 + `xfade-trace.diff`.

## 1. RED on MAIN (VERIFIED — `.harmony/probe-crossfade.sh`, final version, run on MAIN)

Output dir `…/scratchpad/xfade/runs/xfade.hQFKwP` (full run). Earlier runs `xfade.sIdzaL` (a–i) and
`xfade.lp1hOL` (j) gave identical numbers. PRE-TRACE build: `xfade.eKoAwK`, same numbers again.

| Case | What | MAIN result | Numbers |
|---|---|---|---|
| a | both clips effected (THE BUG) | **FAIL** | d(OUT,IN)=155.93; every mid frame t=0.47…3.30 s: dA=**0.00**, dB=155.93 (matches parity-diagnosis §7) |
| b | only incoming effected | PASS | p 0.10→0.77, dA+dB=d(A,B) |
| c | only outgoing effected | PASS | p 0.10→0.78 |
| d | neither (control) | PASS | p 0.10→0.76 |
| e | two effects on each clip | **FAIL** | d(OUT,IN)=108.13; every mid frame dA=**0.00** |
| f | same procedural source type (solid_color red→blue), no effects | **FAIL** | d(OUT,IN)=170.00; every mid frame dA=**0.00** (held red, then cut to blue) |
| g | dry/wet 0.5 on a NON-first effect | **FAIL** | d(MIX, 0.5·DRY+0.5·WET) = **9.81** |
| h | Transparent layer + layer transform (scale 0.5) | PASS (UB) | d(subject, Opaque reference)=0.00; trace shows FEEDBACK! (section 2) |
| i | Layer Router routed to its own layer | PASS (UB) | d(router frame, held image)=0.00; trace shows FEEDBACK! |
| j | clip [Freeze .5] + layer [Invert 1, Freeze .5] | **FAIL** | d=**113.95** from layer [Invert 1] alone; frame is flat grey 128 (std 0.0) |

I looked at a decoded case-a frame myself (`…/scratchpad/xfade/runs/red_a_triptych.png`: OUT reference |
the t≈2.1 s mid frame | IN reference). The mid frame is pixel-for-pixel the outgoing hue-shifted blobs, with
no trace of the inverted grey incoming clip.

## 2. Mechanism (VERIFIED — FBO trace, PRE-TRACE vs POST-TRACE, same probe)

Texture ids: PRE accum=1 scratch=2 A=3 B=4 trans=5; POST accum=1 scratch=2 A=3 B=4 **C=5** trans=6.

| Case | PRE (e1ed9cc) | POST (5dc1272) |
|---|---|---|
| a | `fx:invert 3←tex8` (incoming → A), `fx:hue_shift 3←tex10` (outgoing **overwrites A**), `transition u_texture=tex3 u_prevTexture=tex3` | `fx:invert 3←tex9`, `applyClipEffects holdTex=3`, `fx:hue_shift 4←tex11`, `transition u_texture=tex3 u_prevTexture=tex4` |
| e | incoming invert→3, saturation→4; outgoing hue→3, brightness→**4**; `transition tex4/tex4` | outgoing hue→3, brightness→**5 (C)**; `transition u_texture=tex4 u_prevTexture=tex5` |
| f | `transition u_texture=tex7 u_prevTexture=tex7 centre=(255,0,0)` (one shared source output texture) | `xfade:keepIncoming 3←tex8`; `transition u_texture=tex3 u_prevTexture=tex8 centre=(228,0,27)` |
| g | `fx:dry_wet drawFBO=3→tex3 u_texture=tex4 u_original=tex3` **FEEDBACK!** ×15 | `fx:dry_wet(blend) drawFBO=4→tex4 u_texture=tex3` (no FEEDBACK) |
| h | `keying drawFBO=2→tex2 u_texture=tex2` **FEEDBACK!** ×14 (layer transform returned scratchTex_) | `layerTransform 3←tex11`, `keying 2←tex3` |
| i | `saveLayerOutput drawFBO=7→tex9 u_texture=tex9` **FEEDBACK!** ×16 | skipped (no draw, no FEEDBACK) |
| all | FEEDBACK draws: dry_wet 15, keying 14, saveLayerOutput 16 | **FEEDBACK draws: none** |

Case j needed a second look. With only the layer-chain key split (first attempt), j still failed (d=118.05,
alpha 128). The trace showed why: the layer chain's `fx:time_freeze` sampled its own buffer, but **no
temporal save followed**. `applyClipEffects` saved only `if (currentInput != inputTex)`. A two-pass chain that
starts on pool texture A ends on A again (A→B→A), so that check was false and the save never ran. Both
causes are fixed (section 3). j = 0.21 now.

Trace summaries: `…/scratchpad/xfade/trace_pre_summary.txt` and `trace_post_summary.txt`. Raw logs are the
`err.log` files in `runs/xfade.eKoAwK` (pre) and `runs/xfade.Tu5ih8` (post).

## 3. The fix rule

**Rule (render/ScratchPool.h + `CompositorEngine::pickEffectTarget`):** every pass that renders into the
effect scratch pool (effectFBO_A_/B_/C_) gets its target from `pickEffectTarget(readTex, holdTex)`. That
target is never the texture the pass samples and never a texture the caller still holds (`holdTex`, 0 = none).
The pool has **three** members, so a pass that reads one pool texture while its caller holds another still has
a free target. With nothing held, the result is exactly the old A/B ping-pong (external→A→B→A…), so every
existing chain renders the same. Evidence: probe-effects-parity V1/V5/V6 = 0.00 against their single-chain
references on both builds.

What goes through it, and where the fix lives:
- `applyClipEffects(…, layerId, holdTex)`: each effect pass and the Screen Split grid. This replaces
  4fca2c5's `writeFBO = (inputTex == A) ? 1 : 0` and the Screen Split "next write goes to B" special case.
- `applyTransition`: the outgoing clip's chain runs with `holdTex = newClipTex`. That is the crossfade fix.
- `applyTransition`: if both clips are the **same procedural source type**, one `ProceduralSource` output
  FBO is shared between them (Renderer caches sources per type). In that case only, and only while the
  incoming result is still that shared texture, the incoming result is copied into the pool before the
  outgoing clip renders. This is one full-frame copy, and it happens only during such a crossfade.
- `applyClipTransform` (transform + clip opacity) and `applyLayerTransform` pick pool targets. The layer
  transform used to return `scratchTex_`, which a Transparent layer's keying then sampled while drawing into
  scratchFBO_.
- Dry/wet < 1 is now done **in place with fixed-function blending**. The target already holds the effected
  result (dst); the pre-effect input is drawn over it with `GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA`,
  constant alpha = w. That gives src·(1−w) + dst·w, which is `effect_dry_wet`'s `mix(orig, eff, w)`. It needs no
  third texture and samples nothing it writes. Measured: MIX vs 0.5·DRY+0.5·WET = 0.05.
- `saveLayerOutput` skips the draw when srcTex is the layer's own output texture (self-routed Layer Router).
- Temporal state: the layer chain keys its temporal buffer and Screen Split/Frame Stutter ring with
  `layer.id | kLayerChainStateBit`, so it is never shared with the clip chain. The temporal save now runs
  when any pass rendered (`renderedAny`); it no longer relies on `currentInput != inputTex`.

Costs (INFERRED from code, not profiled):
- Memory: one extra RGBA8 full-frame texture (effectTex_C_). That is 2.65 MB at the 756×878 preview size and
  8.3 MB at 1080p. It is allocated only in initGL/resize.
- Per frame: no new passes in steady state. The layer transform and dry/wet each still take one pass.
- The one extra copy runs only during a same-source-type crossfade.
- A layer that uses temporal or ring effects in **both** its clip chain and its layer chain now gets two
  temporal textures and, if both chains use Screen Split/Frame Stutter, two rings. That memory is the price of
  correct state.
- No mutex, no per-frame allocation.

Unit test: `tests/test_scratch_pool.cpp` (4 cases) tests the **real** picker. It is a pure function, not a
mirror of the compositor. It checks exhaustively that the pick is never the sampled or held texture, that the
historic ping-pong is kept when nothing is held, that C is used only while a pool texture is held, and that a
2-member pool cannot honour a hold (why C exists). Teeth (VERIFIED): compiled against a mutated **copy** of
ScratchPool.h with the hold check removed, it fails 3/4 cases (`…/scratchpad/xfade/teeth.sh`). The deliverable
was not touched (sha256 fa03416d… unchanged). This test does not claim GL coverage; the live probe covers the
GL call sites.

## 4. Class sweep — every site where a produced texture is held while a later pass may write

| # | Site (file:line at 5dc1272) | Status | Evidence |
|---|---|---|---|
| 1 | Incoming clip fx → transition's outgoing clip fx (`applyTransition`, CompositorEngine.cpp:1343) | **FIXED** | live a, e RED→GREEN; trace table §2 |
| 2 | Same procedural source type on both clips (`getClipTexture(prev)` re-renders the shared output, :1317-1336) | **FIXED** | live f RED (dA=0.00)→GREEN; trace §2 |
| 3 | All 15 transition modes (one path: `getTransitionShaderName` → same draw, :1264-1393) | FIXED by #1/#2 — INFERRED | only Dissolve was run live; the fix is in the input textures, not the shader |
| 4 | Clip fx → layer fx (the 4fca2c5 site, :901→:918) | FIXED (subsumed by the picker) | probe-effects-parity V1/V5/V6 parity 0.00 on MAIN and FIX ×2 |
| 5 | Clip transform → clip opacity → clip fx (the 52cd76c site, :496, :549) | SAFE (now via the picker; forceCopy kept) | probe-effects-parity + crossfade GREEN; forceCopy is now belt-and-braces (section 6) |
| 6 | Dry/wet < 1 on a non-first effect (`applyClipEffects`, :417-443) | **FIXED** | live g 9.81→0.05; trace FEEDBACK ×15 → 0 |
| 7 | Layer transform → Transparent keying (`applyLayerTransform` :633, keying :932) | **FIXED** (was UB) | live h 0.00 both builds; trace FEEDBACK ×14 → 0 |
| 8 | Self-routed Layer Router → `saveLayerOutput` (:171) | **FIXED** (was UB) | live i 0.00 both builds; trace FEEDBACK ×16 → 0 |
| 9 | Temporal `u_prev_frame`: clip chain and layer chain shared one buffer (:918-920) | **FIXED** | live j 113.95→0.21 |
| 10 | Temporal save skipped on an A→B→A chain (:448-458) | **FIXED** | found by trace (no save after the layer Freeze); j GREEN needs it |
| 11 | Screen Split target (always wrote effectFBO_A_ whatever the caller held) (:297) | FIXED — INFERRED | now picked away from input+hold; no live case with Screen Split during a crossfade |
| 12 | Temporal/ring state of the transition's OUTGOING chain: same key (`layer.id`) as the incoming clip chain (:1343) | **BROKEN-UNFIXED — INFERRED** | code read; not run live. The fix is a design choice (section 5) |
| 13 | Frame ring push (`pushFrameToRing`, :1515) | SAFE | writes ring[writeIndex] and samples the chain input (pool/external), never a ring texture |
| 14 | Feedback (`FeedbackProcessor::process`, FeedbackProcessor.cpp:88-126) | SAFE | its own 2-texture ping-pong; samples input + own read texture, writes own write texture |
| 15 | `u_feedbackTex` / feedbackTex_ (`updateFeedbackBuffer`, :1398) | SAFE | written once per frame after compositing, from the accumulator; only sampled during compositing |
| 16 | transitionFBO_ (:1366) | SAFE | written only by the transition draw; its inputs are pool/scratch/external, never transitionTex_ |
| 17 | scratchFBO_ writers: FX-only copy :685, mask copy :738, keying :932/:1076, outgoing clip opacity :1354 | SAFE (after #7) | none of them samples scratchTex_ now (trace: `keying 2←tex3`) |
| 18 | FX-only layer (`applyFXOnlyLayer`, :661) | SAFE | chain reads the accumulator and writes the pool; the copies are distinct |
| 19 | Mask layer (`applyMaskLayer`, :729) | SAFE | scratch ← accumulator, accumulator ← mask(clipTex, scratch) |
| 20 | Persistent layers (`compositePersistentLayers`, :1012) | SAFE for pool/scratch; **SUSPECT — INFERRED** for state | layer ids are per deck (Deck.h:34 `layer.id = i`), so a persistent layer from another deck can share temporal/ring/feedback state with the active deck's layer of the same id. Not run live |
| 21 | Global effects (`applyGlobalEffects`, :1086) | SAFE | input is the accumulator, output is the pool, key is `kGlobalEffectsLayerId` |
| 22 | Layer Router, cross-layer (`layerOutputTextures_`) | SAFE | separate per-layer storage; the self case is #8 |
| 23 | Comp transform (Renderer.cpp ~2040-2090) | SAFE | blit default→compTransformFBO_, then draw compTransformTexture_ into the default FBO |
| 24 | Cross-deck transition (Renderer.cpp:586-625, 646-658) | SAFE | reuses compTransformFBO_ only after comp transform consumed it; draws into the default FBO |
| 25 | Master opacity (Renderer.cpp ~697-720) | SAFE | blends dst only into the default FBO |
| 26 | Syphon (Renderer.cpp:2147), capture (:1884), VideoRecorder | SAFE | read the default FBO after all passes, into their own targets |
| 27 | `EffectChain` mid-chain dry/wet (src/effects/EffectChain.cpp:188-201) | **BROKEN-UNFIXED — INFERRED, out of fence** | composites into `1 - writeFBO`, which is the FBO holding `preEffectTexture` (sampled as u_original) for any non-first effect. Same shape as #6. Not run live |

## 5. Not fixed (list only)

- **#12 Outgoing-chain temporal state during a crossfade** (INFERRED). The outgoing clip's chain saves into the
  same temporal buffer and ring as the incoming clip's chain, so during a transition the incoming clip's Echo
  or Freeze reads the outgoing clip's frames. The correct fix is per-clip temporal history instead of per-layer.
  That is a design decision: per-clip 480-frame rings are large and would need a lifetime rule. Needs a ruling.
- **#20 Persistent-layer state keys collide across decks** (INFERRED).
- **#27 EffectChain dry/wet feedback loop** (INFERRED, src/effects is outside my fence).
- Unrelated, found while reading (INFERRED):
  - `applyTransition` applies only the outgoing clip's opacity, not its transform, so an outgoing clip with
    Position/Scale snaps to identity during the crossfade (:1289-1393).
  - `compositePersistentLayers` skips transform/transition/feedback/layer effects/layer transform.
  - `getOrCreateTemporalBuffer` is called **inside** an effect pass. On the frame it creates the buffer,
    `createFBO` rebinds GL_TEXTURE_2D on unit 0 and leaves framebuffer 0 bound, so that one pass draws into the
    default framebuffer. This is a one-frame glitch.

## 6. GREEN (VERIFIED on FIX = build-lane @ 5dc1272)

- **probe-crossfade ×3: 27 PASS / 0 FAIL each** (`runs/xfade.J8kP7u`, `xfade.xDWkwb`, `xfade.DUVzGU`).
  - a: every mid frame strictly between the references, e.g. t=2.14 s dA=80.40 dB=75.53 p=0.52; final
    d(final,IN)=0.00.
  - e: t=2.14 s p=0.51. f: t=2.28 s dA=95.33 dB=74.67 p=0.56. g: 0.05. h: 0.00. i: 0.00. j: 0.21.
  - I looked at a decoded case-a mid frame and a case-f mid frame (`…/scratchpad/xfade/runs/green_a_strip.png`).
    Both inputs are visible, and f is red+blue = purple.
- **probe-effects-parity (hardened) ×2 on FIX: 39/0 each**. V1/V5/V6 parity to their single-chain reference is
  0.00. **×1 on MAIN: 39/0** (`runs/parity.mG4XcN`), so it stays GREEN on main as required.
- **ctest (serial, build-lane): 543/543** (539 existing + 4 new `test_scratch_pool`).
- Build: only pre-existing warnings (`totalCells` unused in applyScreenSplit, JUCE double-promotion).
- Trace instrumentation is removed from src. It is saved as `.harmony/.reports/s-rta-0926/xfade-trace.diff`,
  which applies to 5dc1272 and compiles (checked). Launch with
  `XFADE_ENV=AUDIODNA_FBO_TRACE=1 XFADE_APP=<traced app> .harmony/probe-crossfade.sh`.

## 7. Probe changes

- NEW `.harmony/probe-crossfade.sh` / `.py` / `.json`: 10 cases (6 crossfades a–f, 4 single-state g–j). It is
  screen-safe (open -g, graceful quit, REFUSE if running), uses a fresh `mktemp -d` dir per run, checks every
  render_frame response and decodes every PNG. Overrides: `XFADE_APP`, `XFADE_PY` (falls back to the main
  checkout's .venv), `XFADE_ENV`. Thresholds and calibration are in the .py docstring: noise 0.00 on every
  case, FLOOR 1.0.
- HARDENED `.harmony/probe-effects-parity.sh` (+ new `.py`), per parity-diagnosis §8:
  - checks every render_frame response, 6 frames per variant, fresh `mktemp -d` dir;
  - adds V5 and V6 rows, plus a PARITY-TO-REFERENCE row per variant (tol 1.0, measured 0.00);
  - `PARITY_APP` override (keeps `PARITY_BUILD_DIR`), python fallback to the main .venv, REFUSE without
    PIL/numpy/requests.
