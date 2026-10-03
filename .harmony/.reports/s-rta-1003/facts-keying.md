# FACTS: keying menu, Keying slider, compositing (blend) menu  (s-rta-1003)

Source read ONLY from RECON = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (commit a7491d4, lane/bf9b).
All paths below are relative to RECON/src unless absolute. Labels: VERIFIED (read the code, file:line) / INFERRED / UNKNOWN-NEEDS-A-RUN.
No build, no test, no run was done.

Boris (verbatim, boris-feedback-backlog.md:119): "Can you figure out how to get everything working in the keying menu? Does the keying slider actually do anything? Maybe we get rid of it. Some elements in the keying menu work and some don't. Should we use the transparency slider to do the work for keying elements? Also review the compositing menu, right slider in layer strip. These elements don't work: Creative, 3D,"

---------------------------------------------------------------------------------------------------
## 0. HEADLINE (read this first)

1. VERIFIED: The Keying slider ("K", LayerStrip) and the Inspector's Keying Threshold / Softness sliders do NOTHING visible in ANY keying mode. They feed `u_threshold` / `u_softness` (CompositorEngine.cpp:1332-1333) and NONE of the four key shaders declares or reads those two names (EmbeddedShaders.h:2482-2540).
2. VERIFIED: The 13 keying entries compile to only 4 distinct shader programs (Renderer.cpp:1895-1911). 8 of 13 entries have no implementation of their own: 6 render as plain Alpha (Inverted Luma Is Alpha, Edge Detection, Threshold Mask, Channel R/G/B), Inverted Luma Key and Saturation Key render as Luma Key. Luma Is Alpha is Max RGB under another label. (T1)
3. VERIFIED: Luma Key / Inverted Luma Key / Saturation Key shader reads `u_luma_threshold` / `u_luma_softness`; the C++ sets `u_threshold` / `u_softness`. The luma uniforms are never set (GL default 0). So even the "real" luma key has edge0 == edge1 == 0 (GLSL smoothstep undefined for edge0 >= edge1). Chroma Key reads `u_chroma_softness`, also never set (0).
4. VERIFIED: Keying only runs for layers of type Transparent (CompositorEngine.cpp:1157). Opaque layers (Layer 1 by default, Composition.h:210) ignore keyingMode AND blendMode entirely (clear accumulator + opacity draw, :1164-1198). There is NO UI anywhere that changes Layer::Type (grep of src/ui and MainComponent: none), so Layer 1's keying menu entry is a dead control.
5. VERIFIED: The strip's blend dropdown ("V" dropdown, under S|K|V) is ONE combo holding 13 keying entries (ids 101+) AND 55 MixMode entries (ids 1-55) (LayerStrip.cpp:1188-1213). It shows only blendMode on reload (LayerStrip.cpp:709); keyingMode is never reflected back. Picking a keying entry changes the combo's text but leaves blendMode alone, and vice versa.
6. VERIFIED: Layer blend (V dropdown -> `Layer::blendMode` -> `blendLayerOntoAccumulator`) implements ONLY 6 of 55 MixModes: Normal, Additive, Screen, Multiply, Darken, Lighten (CompositorEngine.cpp:1369-1394). The other 49 (incl. all of "Creative" and "3D") hit `default:` = plain Normal alpha blend (:1411).
7. VERIFIED: "Right slider in layer strip" = the F (fade) slider; its dropdown under it (`transitionDropdown_`, caret-only, LayerStrip.cpp:488-503) fills with the SAME 55-entry list (populateMixModes) but sets `Layer::transitionMode`, used ONLY by `getTransitionShaderName` (CompositorEngine.cpp:1457-1477). Only 15 shaders exist (dissolve + 14). All 9 "Creative" entries and 5 of 6 "3D" entries (Rotate X, Rotate Y, Spin, Cube, Fold) fall to `transition_dissolve`. Flip works (`transition_flip_h`).
8. VERIFIED: `Layer::transitionBlendMode` is saved/loaded (Layer.cpp:68,183) and read by NOTHING in render. Dead field.
9. VERIFIED: Opacity ("V" slider, `Layer::opacity`) multiplies alpha inside every key shader (`u_opacity`) for Transparent layers; so it already "does the keying amount" for alpha-honouring blends. For Multiply / Screen / Darken / Lighten the blend funcs ignore src alpha (CompositorEngine.cpp:1387-1394), so opacity has no effect on a Transparent layer in those four modes (INFERRED from GL funcs; confirm with a run, section 4 U4).

---------------------------------------------------------------------------------------------------
## 1. QUESTIONS ANSWERED

### Q1. Where is the keying menu built; every entry in menu order

There are TWO keying menus (both write `Layer::keyingMode`, a uint8 enum, Layer.h:255-261):

A. LayerStrip "V" dropdown `blendDropdown_` (the mixed Keying + Blend combo): built by `LayerStrip::populateBlendDropdown()` LayerStrip.cpp:1188-1213. Section heading "Keying" then these items, id = 101 + enum (kKeyingIdOffset = 101, LayerStrip.cpp:11):
B. LayerInspector (Layer tab) `keyingModeSelector_`, built by `populateKeyingModes()` LayerInspector.cpp:986-998, ids 1-13 = enum+1; visible ONLY when layer type is Transparent (LayerInspector.cpp:653-664, 590). The inspector also has Keying Threshold + Softness sliders (LayerInspector.cpp:337-347) in the same section.

Menu order, enum value, label (strip label / inspector label if different):

| # | KeyingMode (Layer.h:255-259) | enum | Strip label | Inspector label | combo id strip / inspector |
|---|---|---|---|---|---|
| 1 | Alpha | 0 | Alpha | Alpha | 101 / 1 |
| 2 | LumaKey | 1 | Luma Key | Luma Key | 102 / 2 |
| 3 | InvertedLumaKey | 2 | Inverted Luma Key | Inverted Luma Key | 103 / 3 |
| 4 | LumaIsAlpha | 3 | Luma Is Alpha | Luma is Alpha | 104 / 4 |
| 5 | InvertedLumaIsAlpha | 4 | Inverted Luma Is Alpha | Inverted Luma is Alpha | 105 / 5 |
| 6 | ChromaKey | 5 | Chroma Key | Chroma Key | 106 / 6 |
| 7 | MaxRGB | 6 | Max RGB | Max RGB | 107 / 7 |
| 8 | SaturationKey | 7 | Saturation Key | Saturation Key | 108 / 8 |
| 9 | EdgeDetection | 8 | Edge Detection | Edge Detection | 109 / 9 |
| 10 | ThresholdMask | 9 | Threshold Mask | Threshold Mask | 110 / 10 |
| 11 | ChannelR | 10 | Channel Red | Channel Red | 111 / 11 |
| 12 | ChannelG | 11 | Channel Green | Channel Green | 112 / 12 |
| 13 | ChannelB | 12 | Channel Blue | Channel Blue | 113 / 13 |

VERIFIED (LayerStrip.cpp:1194-1207; LayerInspector.cpp:989-996; Layer.h:255-259). gotchas.md:48-50 ("KeyingMode enum != Layer::Type", 13 entries Alpha..ChannelB) matches.

Selection handlers: strip LayerStrip.cpp:460-475 (sel >= 101 -> keyingMode = sel-101; else blendMode = sel-1); inspector LayerInspector.cpp:330-334.
Strip does NOT reflect keyingMode on load: `setLayer` sets selected id to `blendMode + 1` only (LayerStrip.cpp:709). The inspector does reflect it (LayerInspector.cpp:923).
Persistence: Layer.cpp:41-47 (to), 148-154 (from) -- `keyingMode, keyThreshold, keySoftness, chromaKeyR/G/B, chromaKeyTolerance`.

### Q2. Per entry: model field, uniforms, shader branch

Render path (VERIFIED): `compositeShow` Transparent case -> `applyLayerKeying(layer, layer.keyingMode, clipTex, scratchFBO_ ...)` (CompositorEngine.cpp:1157-1162) -> switch maps enum to shader key (CompositorEngine.cpp:1303-1318) -> `ShaderManager::getProgram(key)` (falls back to "passthrough" if missing, :1320-1321). Uniforms set (every mode, same set, :1327-1335): `u_texture`=0, `u_opacity`=layer.eff(Opacity), `u_threshold`=keyThreshold, `u_softness`=keySoftness, `u_chroma_key_color`=(chromaKeyR,G,B), `u_chroma_tolerance`, `u_resolution`. A location of -1 (name not in the shader) is silently ignored by glUniform.
Shader key -> source program (Renderer.cpp:1895-1911, VERIFIED):

| shader key | compiled from | EmbeddedShaders.h |
|---|---|---|
| key_alpha, key_inv_luma_alpha, key_edge, key_threshold, key_channel_r/g/b (and key_vignette, unused) | transparencyAlpha | :2482-2492 |
| key_luma, key_inv_luma, key_saturation | transparencyLumaKey | :2495-2509 |
| key_chroma | transparencyChromaKey | :2512-2527 |
| key_max_rgb, key_luma_alpha | transparencyLight | :2530-2540 |

The four programs' bodies (VERIFIED, EmbeddedShaders.h):
- transparencyAlpha (:2482): `fragColor = vec4(col.rgb, col.a * u_opacity)`. Declares only u_texture, u_opacity.
- transparencyLumaKey (:2495): `luma = dot(rgb,(.299,.587,.114)); alpha = smoothstep(u_luma_threshold, u_luma_threshold + u_luma_softness, luma)`. Reads `u_luma_threshold`, `u_luma_softness`, `u_opacity`.
- transparencyChromaKey (:2512): `dist = distance(rgb, u_chroma_key_color); alpha = smoothstep(u_chroma_tolerance, u_chroma_tolerance + u_chroma_softness, dist)`. Reads `u_chroma_key_color`, `u_chroma_tolerance`, `u_chroma_softness`, `u_opacity`.
- transparencyLight (:2530): `alpha = max(r,g,b)`. Reads only u_opacity.

Per-entry result (see T1 for the table). Notes that matter:
- The comment at Renderer.cpp:1900-1901 says "(the shader handles threshold direction, so inverted luma = just flip threshold)". NOTHING flips a threshold: no code anywhere sets an inverted threshold, so Inverted Luma Key renders exactly as Luma Key. VERIFIED (no such code in CompositorEngine.cpp:1289-1340).
- Renderer.cpp:1905 "reuse with sat" for Saturation Key: the shader has no saturation math. Saturation Key == Luma Key. VERIFIED.
- Luma Is Alpha is mapped to transparencyLight = max(R,G,B), not luminance (comment at Renderer.cpp:1903 says "luminance -> alpha"). VERIFIED. Inverted Luma Is Alpha falls to the Alpha program ("fallback" comment, :1904).
- Chroma key has model fields chromaKeyR/G/B and chromaKeyTolerance but NO UI sets them (grep of "chromaKey" outside Layer.h/.cpp and CompositorEngine.cpp: no hits). Key color is fixed at default green (0,1,0), tolerance 0.2, softness (unset uniform) 0. VERIFIED.
- No keying entry is "overwritten elsewhere" after applyLayerKeying; `blendLayerOntoAccumulator(layer, scratchTex_...)` takes the keyed texture next (:1162). The per-clip / per-layer opacity (RGB scale, `clip_opacity_blend`, :742-781) happens BEFORE keying, so keying sees dimmed RGB. VERIFIED.

### Q3. The Keying slider (K) on the layer strip

- UI: `keyingSlider_` (LayerStrip.cpp:415-426), ResettableSlider 0..1 step 0.01, default 0.1, drawn by `sKeyingLAF` with label "K" (LAF class sKeyingLAF; "K = keying threshold slider (no dropdown)" comment :415). onValueChange -> `layer_->keyThreshold` (plain float, not Relaxed, not a LayerScalar; no connection / no routine / no REST path to it). Loaded in `setLayer` (:708). It sits in the strip as S | K | V | thumbnail | F (LayerStrip.cpp:671-681).
- Model: `Layer::keyThreshold` (default 0.1, Layer.h:262). Inspector's keyThresholdSlider_ writes the same field (LayerInspector.cpp:337-341); `keySoftness` (default 0.1, :263) only from the inspector.
- Uniform: `u_threshold` (CompositorEngine.cpp:1332) and `u_softness` (:1333).
- Shader use: NONE. grep over the entire src tree for `u_threshold` / `u_softness` in key shaders: the four key programs (EmbeddedShaders.h:2482-2540) do not declare either name. The luma program's names are `u_luma_threshold` / `u_luma_softness` (never set from C++).
- Per keying entry (all 13): value of K / Softness is IGNORED in every mode. VERIFIED. The slider is a dead control. (The only effect is the value being saved in the composition file.)
- UNKNOWN-NEEDS-A-RUN: what Luma Key actually draws with both luma uniforms 0 (GLSL says smoothstep is undefined when edge0 >= edge1; on this GPU likely a step at luma > 0, i.e. nearly everything opaque; INFERRED). Cheapest test: U1 below.

### Q4. The opacity ("transparency") slider path; how keying and opacity combine

- UI: `opacitySlider_` ("V", LayerStrip.cpp:429-452), component id "layerOpacity", 0..1 default 1.0; onValueChange -> `layer_->opacity` (RelaxedFloat, Layer.h:~187); drag start/end grip a Held connection on `LayerScalar::Opacity` (LayerStrip.cpp:446-451). Follows the model at 30 Hz (LayerStrip.cpp:820-836) so routines/REST/MIDI/OSC writes show.
- Other writers: REST `POST /api/set_layer_opacity` (ApiServer.cpp:196, 7070); Inspector opacity control; connections (`eff(LayerScalar::Opacity)`, Layer.cpp:8).
- GL read: always through `layer.eff(LayerScalar::Opacity)`:
  - Transparent layer: as `u_opacity` in the keying shader (all four programs multiply `alpha * u_opacity`), CompositorEngine.cpp:1331. So final alpha = keyed alpha x layer opacity. Then `blendLayerOntoAccumulator` blends scratch with SRC_ALPHA for Normal (:1372) and Additive (:1369).
  - Opaque layer: `opacity_blend` shader (alpha * u_opacity) with GL blending enabled only if opacity < 0.999 (CompositorEngine.cpp:1166-1198), over a freshly black-cleared accumulator (the layer REPLACES everything below).
  - FX Only layer: `combinedOpacity(layer, clip)` dilution (CompositorEngine.cpp:862-909).
  - Clip opacity is a separate, earlier stage (RGB scale `clip_opacity_blend`, :742-781), not the strip's V slider.
- Combination: keying alpha (from shader) x opacity, in one multiply, in the key shader. Keying does not read opacity separately; opacity does not read keying. With Alpha / Edge / Threshold / Channel R,G,B / Inverted Luma Is Alpha (all = transparencyAlpha), the keying stage reduces to "col.a * opacity" = pure opacity.
- Blend modes that ignore src alpha (so V has no visible effect on a Transparent layer): Screen (GL_ONE, GL_ONE_MINUS_SRC_COLOR, :1387), Multiply (GL_DST_COLOR, GL_ZERO, :1390), Darken / Lighten (GL_MIN / GL_MAX with GL_ONE,GL_ONE, :1393-1405). INFERRED from the GL funcs (read, not run). Note this matches the comment at CompositorEngine.cpp:750-766 about alpha-only reduction being a no-op under those modes.

### Q5. "The compositing menu, right slider in layer strip"

IDENTIFICATION:
- Strip layout (LayerStrip.cpp:671-693): sliders S (speed) | K (keying) | V (opacity) | thumbnail | F (fade speed). Rightmost slider = F. Its dropdown is `transitionDropdown_` (caret only, LayerStrip.cpp:488-503, bounds :693 at x = fX). That is almost certainly what Boris means by "right slider" (INFERRED: he did not say F). Its menu = `populateTransitionDropdown()` (:1215-1220) = `populateMixModes(transitionDropdown_, 1)`; first heading is literally "Compositing" (LayerStrip.cpp:19). It writes `Layer::transitionMode` (LayerStrip.cpp:498-502).
- The V dropdown (`blendDropdown_`, three slider-widths wide at S..V, LayerStrip.cpp:692) holds the SAME 55-item list after the 13 keying items (LayerStrip.cpp:1210); it writes `Layer::blendMode`. If Boris means "the compositing menu" = the V dropdown, the groups are the same, but the behaviour differs (see below). Report BOTH.
- LayerInspector has two more copies: Video "Blend Mode" `blendModeSelector_` (only 25 entries, enum 0-24, LayerInspector.cpp:971-984) and Transition `transitionBlendSelector_` (all groups, LayerInspector.cpp:~220-300; writes transitionMode, :302-308). Both ids = enum+1.

GROUPS in menu order (populateMixModes, LayerStrip.cpp:17-95), 55 entries, enum 0-54 (Layer.h:159-186):
Compositing(5): Alpha(Normal 0), Add(Additive 1), Screen(2), Multiply(3), Overlay(4)
Light(6): Soft Light 5, Hard Light 6, Vivid Light 7, Linear Light 8, Pin Light 9, Hard Mix 10
Compare(4): Darken 11, Lighten 12, Darker Color 13, Lighter Color 14
Dodge / Burn(2): Color Dodge 15, Color Burn 16
Inversion(3): Difference 17, Exclusion 18, Subtract 19
Component(4): Hue 20, Saturation 21, Color 22, Luminosity 23
Special(2): Dissolve 24, Cut 25
Wipe(6): Wipe Left 26, Wipe Right 27, Wipe Up 28, Wipe Down 29, Wipe Ellipse 30, Wipe Diagonal 31
Push(4): Push Left 32, Push Right 33, Push Up 34, Push Down 35
Zoom(2): Zoom In 36, Zoom Out 37
3D(6): Rotate X 38, Rotate Y 39, Spin 40, Cube 41, Flip 42, Fold 43
Color Fade(2): To Black 44, To White 45
Creative(9): Pixelate 46, Blur 47, Noise 48, RGB Split 49, Glitch Blocks 50, Strobe 51, Slide 52, Stretch 53, Displace 54
(Total 55 = 25 blend-style + 30 transition-style, per the enum comments.) VERIFIED against Layer.h:159-186 and LayerStrip.cpp:17-95. test_show_model.cpp:1435 uses MixMode 46 (Pixelate) as a sample value.

IMPLEMENTATION, two consumers of the one list (VERIFIED):
(a) As LAYER BLEND (V dropdown; `blendLayerOntoAccumulator`, CompositorEngine.cpp:1342-1427; no shader, fixed-function GL blend using the "passthrough" program): implemented are Normal (:1372, SRC_ALPHA/ONE_MINUS_SRC_ALPHA separate), Additive (:1369, SRC_ALPHA,ONE), Screen (:1387, ONE,ONE_MINUS_SRC_COLOR), Multiply (:1390, DST_COLOR,ZERO), Darken (:1393, GL_MIN), Lighten (:1394, GL_MAX). Everything else = `default:` at :1411 = the Normal alpha blend ("Fallback: standard alpha blend for unimplemented modes"). There is no shader that reads the accumulator, so Overlay / Soft Light / ... / Hue..Luminosity (which need dst color in a shader) have no implementation and cannot be done with fixed-function blend funcs alone (INFERRED).
(b) As CLIP TRANSITION (F dropdown; `getTransitionShaderName`, CompositorEngine.cpp:1457-1477, program compile Renderer.cpp:2110-2124): 14 mapped + Dissolve default:
  WipeLeft/Right/Up/Down -> transition_wipe_*; PushLeft/Right/Up/Down -> transition_push_*; ZoomIn/Out -> transition_zoom_in/out; WipeEllipse -> transition_iris; Flip -> transition_flip_h; Cut -> transition_cut; ToBlack -> transition_fade_black; Dissolve and `default:` -> transition_dissolve. All 15 shaders exist (EmbeddedShaders.h:3886-4160). Every unmapped MixMode renders as DISSOLVE.
  Transition inputs: `u_texture` (new clip, unit 0), `u_prevTexture` (old clip, unit 1), `u_crossfadeProgress` (CompositorEngine.cpp:1533-1551). That is all a transition shader gets: no depth, no 3D camera, no time, no resolution.

"CREATIVE" (9 entries) and "3D" (6 entries), per entry, why no effect:

| entry | as BLEND (V) | as TRANSITION (F) | what it would need (INFERRED, no design) |
|---|---|---|---|
| Pixelate 46 | default -> Normal | default -> Dissolve | a transition shader with a block-size from u_crossfadeProgress |
| Blur 47 | default -> Normal | default -> Dissolve | a multi-tap transition shader (neither input is blurred) |
| Noise 48 | default -> Normal | default -> Dissolve | noise hash + (time or seed) uniform; none is passed |
| RGB Split 49 | default -> Normal | default -> Dissolve | per-channel offset transition shader |
| Glitch Blocks 50 | default -> Normal | default -> Dissolve | block hash + time uniform; none passed |
| Strobe 51 | default -> Normal | default -> Dissolve | time or beat uniform; none passed |
| Slide 52 | default -> Normal | default -> Dissolve | an offset-pair transition shader (similar to Push; not present) |
| Stretch 53 | default -> Normal | default -> Dissolve | UV-scaling transition shader |
| Displace 54 | default -> Normal | default -> Dissolve | a displacement source (a third texture or noise); none passed |
| Rotate X 38 | default -> Normal | default -> Dissolve | a 3D/perspective UV transform shader; no shader exists |
| Rotate Y 39 | default -> Normal | default -> Dissolve | same |
| Spin 40 | default -> Normal | default -> Dissolve | 2D rotation transition shader |
| Cube 41 | default -> Normal | default -> Dissolve | perspective cube-face shader |
| Flip 42 | default -> Normal | WORKS: transition_flip_h (EmbeddedShaders.h:4104) | n/a |
| Fold 43 | default -> Normal | default -> Dissolve | perspective fold shader |

No entry in either group needs a depth value, a second texture beyond prev/new, or a uniform that nobody sets in the SHADER; the real reason is simpler: there is no shader (transition) or no case (blend) at all for them. (VERIFIED by grep: the only `transition_*` compiles are the 15 at Renderer.cpp:2110-2124; the only blend cases are the six at CompositorEngine.cpp:1369-1394.) The "ThreeD" LAYER TYPE (Layer::Type::ThreeD) is a different thing: `case ThreeD: // 3D: not implemented yet` (CompositorEngine.cpp:~1225), and no UI sets it; do not confuse it with the "3D" menu group (transitions).

### Q6. Do transitions use the same blend table?

- They share the ENUM and the MENU LIST (`populateMixModes`, used by both the V and F combos; the Inspector's Transition row lists the full set), but NOT an implementation table: blends -> `blendLayerOntoAccumulator` switch; transitions -> `getTransitionShaderName` switch. VERIFIED.
- The same entries are broken in both, in different ways: Creative (9) and 3D (all but Flip) are broken as blend (-> Normal) and as transition (-> Dissolve). Additionally as TRANSITION these are broken too: every blend-style entry (Normal..Dissolve except the Dissolve default, i.e. Alpha, Add, Screen, Multiply, Overlay, the 6 Light, 4 Compare, 2 Dodge/Burn, 3 Inversion, 4 Component), WipeDiagonal, ToWhite. As BLEND, every transition-style entry (Cut, Wipes, Pushes, Zooms, 3D, Color Fade, Creative) renders as Normal.
- `Layer::transitionBlendMode` ("Transition blend method", Layer.h:283) is serialized only; no reader in render. VERIFIED (grep: Layer.h:283, Layer.cpp:68,183 only).
- Wiring check: `LayerStrip::onBlendModeChanged` is declared and invoked (LayerStrip.cpp:472-473) but nobody assigns it (grep: LayerStrip.h:126 only). Harmless, but the callback does nothing.

---------------------------------------------------------------------------------------------------
## 2. TABLES

### T1. Keying entries
WORKS-BY-CODE = reaches a program that implements what the label says. Slider-used? = does K / Softness change the picture.

| entry | enum | implemented where | slider used? | status |
|---|---|---|---|---|
| Alpha | 0 | key_alpha -> transparencyAlpha, EmbeddedShaders.h:2482 (col.a * u_opacity) | no | WORKS-BY-CODE (it is "use the picture's own alpha x opacity"; with opaque images it is a no-op) |
| Luma Key | 1 | key_luma -> transparencyLumaKey :2495 | no: C++ sets u_threshold/u_softness; shader reads u_luma_threshold/u_luma_softness (unset = 0) | SUSPECT: has a branch, but both edge uniforms are 0 so the key is degenerate (smoothstep with edge0 == edge1, undefined in GLSL); threshold cannot be moved by any UI |
| Inverted Luma Key | 2 | key_inv_luma -> SAME program as Luma Key (Renderer.cpp:1902); nothing inverts | no | NO-BRANCH (alias of Luma Key; renders identical to entry 1) |
| Luma Is Alpha | 3 | key_luma_alpha -> transparencyLight :2530 = alpha = max(R,G,B), NOT luminance | no | SUSPECT (works, but is Max RGB under a luma label) |
| Inverted Luma Is Alpha | 4 | key_inv_luma_alpha -> transparencyAlpha (Renderer.cpp:1904 "fallback") | no | NO-BRANCH (renders as Alpha) |
| Chroma Key | 5 | key_chroma -> transparencyChromaKey :2512 | no (K ignored); u_chroma_key_color + u_chroma_tolerance ARE set (fixed green / 0.2, no UI); u_chroma_softness unset = 0 | SUSPECT: branch exists, keys fixed green; hard edge; no way to pick colour in the UI |
| Max RGB | 6 | key_max_rgb -> transparencyLight :2530 | no | WORKS-BY-CODE |
| Saturation Key | 7 | key_saturation -> transparencyLumaKey (Renderer.cpp:1905 "reuse with sat"); no saturation maths exists | no | NO-BRANCH (renders as Luma Key = degenerate) |
| Edge Detection | 8 | key_edge -> transparencyAlpha (fallback, :1906) | no | NO-BRANCH (renders as Alpha) |
| Threshold Mask | 9 | key_threshold -> transparencyAlpha (fallback, :1907) | no | NO-BRANCH (renders as Alpha) |
| Channel Red | 10 | key_channel_r -> transparencyAlpha (fallback, :1908) | no | NO-BRANCH (renders as Alpha) |
| Channel Green | 11 | key_channel_g -> transparencyAlpha (fallback, :1909) | no | NO-BRANCH |
| Channel Blue | 12 | key_channel_b -> transparencyAlpha (fallback, :1910) | no | NO-BRANCH |

Count (VERIFIED by the table above): 2 WORKS-BY-CODE (Alpha, Max RGB), 3 SUSPECT (Luma Key, Luma Is Alpha, Chroma Key), 8 NO-BRANCH (Inverted Luma Key, Inverted Luma Is Alpha, Saturation Key, Edge, Threshold Mask, Ch R/G/B). So "some work, some don't" = 2 + partial.
Gate common to all rows: keying runs only on Transparent layers (CompositorEngine.cpp:1157); on an Opaque layer (Layer 1 by default) all 13 are inert. No entry is overwritten elsewhere after `applyLayerKeying`.
Keying-slider verdict common to all rows: K / Softness unused (Q3).

### T2. Blend entries by group
"as V blend" = `blendLayerOntoAccumulator` branch. "as F transition" = `getTransitionShaderName`. Layer enum in () .

| group | entry (enum) | as V blend | as F transition | status |
|---|---|---|---|---|
| Compositing | Alpha=Normal (0) | case Normal, CompositorEngine.cpp:1372 | default -> dissolve | blend WORKS; transition = dissolve (equal to the default) |
| Compositing | Add (1) | case Additive :1369 | default -> dissolve | blend WORKS |
| Compositing | Screen (2) | case Screen :1387 (alpha ignored) | default -> dissolve | blend WORKS (no opacity) |
| Compositing | Multiply (3) | case Multiply :1390 (alpha ignored) | default -> dissolve | blend WORKS (no opacity) |
| Compositing | Overlay (4) | default :1411 -> Normal | default -> dissolve | NO-BRANCH |
| Light | Soft Light (5), Hard Light (6), Vivid Light (7), Linear Light (8), Pin Light (9), Hard Mix (10) | default -> Normal | default -> dissolve | NO-BRANCH (6 entries) |
| Compare | Darken (11) | case Darken :1393 (GL_MIN, alpha ignored) | default -> dissolve | blend WORKS |
| Compare | Lighten (12) | case Lighten :1394 (GL_MAX) | default -> dissolve | blend WORKS |
| Compare | Darker Color (13), Lighter Color (14) | default -> Normal | default -> dissolve | NO-BRANCH |
| Dodge / Burn | Color Dodge (15), Color Burn (16) | default -> Normal | default -> dissolve | NO-BRANCH |
| Inversion | Difference (17), Exclusion (18), Subtract (19) | default -> Normal | default -> dissolve | NO-BRANCH |
| Component | Hue (20), Saturation (21), Color (22), Luminosity (23) | default -> Normal | default -> dissolve | NO-BRANCH |
| Special | Dissolve (24) | default -> Normal | transition_dissolve (default case, :1473) | transition WORKS; blend = Normal |
| Special | Cut (25) | default -> Normal | transition_cut | transition WORKS; blend = Normal |
| Wipe | Wipe Left/Right/Up/Down (26-29) | default -> Normal | transition_wipe_* | transition WORKS; blend = Normal |
| Wipe | Wipe Ellipse (30) | default -> Normal | transition_iris | transition WORKS |
| Wipe | Wipe Diagonal (31) | default -> Normal | default -> dissolve | NO-BRANCH |
| Push | Push Left/Right/Up/Down (32-35) | default -> Normal | transition_push_* | transition WORKS |
| Zoom | Zoom In (36), Zoom Out (37) | default -> Normal | transition_zoom_in / _out | transition WORKS |
| 3D | Rotate X (38), Rotate Y (39), Spin (40), Cube (41), Fold (43) | default -> Normal | default -> dissolve | NO-BRANCH (5 entries; Boris: "doesn't work") |
| 3D | Flip (42) | default -> Normal | transition_flip_h | transition WORKS; blend = Normal |
| Color Fade | To Black (44) | default -> Normal | transition_fade_black | transition WORKS |
| Color Fade | To White (45) | default -> Normal | default -> dissolve | NO-BRANCH |
| Creative | Pixelate (46), Blur (47), Noise (48), RGB Split (49), Glitch Blocks (50), Strobe (51), Slide (52), Stretch (53), Displace (54) | default -> Normal | default -> dissolve | NO-BRANCH (9 entries; Boris: "doesn't work") |

Counts (VERIFIED from the cases): as V blend 6 implemented / 49 fall to Normal. As F transition 14 implemented non-dissolve + Dissolve = 15 distinct behaviours / 40 fall to dissolve (counting Dissolve's own entry as working).
All transitions listed WORKS-by-code are from reading only (shader existence + name mapping VERIFIED; the picture they produce is not run).

---------------------------------------------------------------------------------------------------
## 3. WHAT EXISTS TODAY vs WHAT BORIS ASKED (gaps only, no design)

- Boris: "get everything working in the keying menu". Today: 13 entries, 2 do what they say, 3 partly, 8 are aliases of Alpha or Luma Key (T1). Keying applies only to Transparent layers; the Opaque layer's menu entry is inert.
- Boris: "Does the keying slider actually do anything? Maybe we get rid of it." Today: no. K and Inspector Threshold + Softness set uniforms no shader reads (Q3). Removing them changes no picture.
- Boris: "Should we use the transparency slider to do the work for keying elements?" Today: opacity (V) is already multiplied into the keyed alpha inside every key shader (Q4), so V is the only slider that changes keyed output; there is no per-mode amount/threshold control wired to any shader. V is ignored under Multiply / Screen / Darken / Lighten blends (INFERRED).
- Boris: "review the compositing menu, right slider in layer strip. These elements don't work: Creative, 3D," (the quote ends with a trailing comma; the list may continue). Today: the right-hand F dropdown lists 55 modes, only 15 transitions exist; Creative 0/9, 3D 1/6 (Flip) work as transitions; all Creative + 3D fall back silently to Dissolve with no indication in the UI. The same entries also fall to Normal in the V blend dropdown.
- Other broken groups Boris did not name but the same fall-through hits: blend modes Overlay through Luminosity (23 of the 25 blend-style names; only Normal, Add, Screen, Multiply, Darken, Lighten work), Wipe Diagonal, To White, and all blend-style names used as transitions.
- UI/model inconsistencies a builder would meet: (1) strip V combo mixes keying + blend and never reshows keyingMode; (2) Inspector's Video Blend Mode lists only enum 0-24 while the strip can set 0-54 (a value > 24 shows blank in the Inspector, LayerInspector.cpp:914 with 25 items); (3) no UI for chroma key colour / tolerance / softness; (4) no UI to change Layer::Type, so Opaque cannot be made Transparent in the app (only via a composition file) and vice versa; (5) `transitionBlendMode` dead; (6) `onBlendModeChanged` never assigned.

---------------------------------------------------------------------------------------------------
## 4. UNKNOWN-NEEDS-A-RUN

U1. What Luma Key / Inv. Luma Key / Saturation Key actually draw with `u_luma_threshold = u_luma_softness = 0` (smoothstep edge0 == edge1). Cheapest test: load a composition (below) with a Transparent layer over a bright opaque layer, keyingMode 1, a gradient source; POST /api/render_frame; read the PNG; compare against keyingMode 0 (Alpha). If pixel-identical, the key is a no-op in practice.
U2. Chroma Key with the default green (0,1,0) and softness 0: confirm hard-edge. Same method with a green-containing image/source vs keyingMode 5.
U3. That K / Softness slider values change nothing: render the same layer with keyThreshold 0.0 and 1.0 in each of 13 modes; expect identical decoded pixels (VERIFIED by code, confirm by diff).
U4. Opacity under Screen / Multiply / Darken / Lighten on a Transparent layer: render at opacity 1.0 vs 0.3; expect identical pixels (INFERRED from GL funcs).
U5. That every unmapped blend mode equals Normal: render blendMode 4 (Overlay) vs 0 (Normal) with the same inputs; expect identical. And Additive vs Normal differ (control).
U6. A Creative / 3D transition: trigger a clip with crossfade at a fixed `time` and transitionMode 46 vs 24 (Dissolve) mid-fade; expect identical. (Needs a crossfade frozen mid-way; probe-crossfade.py does this, see section 6.)
U7. Whether the F dropdown (rather than V) is what Boris called "the compositing menu, right slider": ask Boris or accept both.

---------------------------------------------------------------------------------------------------
## 5. FILES A BUILDER WOULD TOUCH (paths only)

/Users/boriskarpman/projects/RealTimeAudio/src/ui/LayerStrip.cpp  (populateBlendDropdown :1188, populateMixModes :17, K slider :415, V dropdown :455-475, F dropdown :488-503, setLayer :698-725)
/Users/boriskarpman/projects/RealTimeAudio/src/ui/LayerStrip.h  (keyingSlider_ :187, blendDropdown_ :191, onBlendModeChanged :126)
/Users/boriskarpman/projects/RealTimeAudio/src/ui/LayerInspector.cpp  (keying :330-347, :581-583, :653-664, :921-925, :986-998; blend :166-171, :971-984; transition list ~:220-308)
/Users/boriskarpman/projects/RealTimeAudio/src/ui/LayerInspector.h
/Users/boriskarpman/projects/RealTimeAudio/src/render/CompositorEngine.cpp  (applyLayerKeying :1289-1340, blendLayerOntoAccumulator :1342-1427, getTransitionShaderName :1457-1477, applyTransition :1479-1553, compositeShow Transparent/Opaque :1157-1198)
/Users/boriskarpman/projects/RealTimeAudio/src/render/CompositorEngine.h  (:461-464, accumulator/scratch FBO decls)
/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.cpp  (key_* compiles :1895-1911; transition_* compiles :2110-2124; passthrough :1790; opacity_blend :1792)
/Users/boriskarpman/projects/RealTimeAudio/src/render/EmbeddedShaders.h  (key shaders :2482-2540; transitions :3886-4160; opacityBlend :69; clipOpacityBlend :92)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Layer.h  (MixMode :159-186, KeyingMode :255-265, transitionBlendMode :283)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Layer.cpp  (keying/blend/transition serialisation :40-68, :147-183)
/Users/boriskarpman/projects/RealTimeAudio/src/model/Composition.h  (default layer types :210, makeLayer :470)
/Users/boriskarpman/projects/RealTimeAudio/src/api/ApiServer.cpp  (layer JSON exposes blendMode only, :439, :480; NO keying fields; only set route is /api/set_layer_opacity :196)
/Users/boriskarpman/projects/RealTimeAudio/src/test/TestServer.cpp  (only if a test-only route is added)
/Users/boriskarpman/projects/RealTimeAudio/docs/claude/effects.md (:55 pipeline line), /Users/boriskarpman/projects/RealTimeAudio/.harmony/APP-INVENTORY.md (:59, :73 "13 keying + ~55 mix modes"), /Users/boriskarpman/projects/RealTimeAudio/tests/test_compositor.cpp (:394-413 only round-trips keyingMode)
(Paths above are written under the main repo root for the builder; the facts were read from the RECON checkout at the same relative paths.)

---------------------------------------------------------------------------------------------------
## 6. EXISTING TESTS / PROBES; how to drive this area without synthetic input

Existing tests touching this area (VERIFIED by grep of RECON/tests and .harmony):
- tests/test_compositor.cpp:394-413 (Transparent layer, blendMode default Additive, keyingMode ChromaKey JSON round-trip; NO render test of any keying or blend result).
- tests/test_composition.cpp:52-54, :177 (Transparent type only); tests/test_show_model.cpp:1127-1452 (blendMode kept through migration, e.g. MixMode 46); tests/test_undo_commands.cpp (mentions blend/key in grep).
- tests/test_compositor_opacity_alias.cpp, tests/test_compositor_effects_parity.cpp (opacity / effects; not keying).
- tests/test_layer_strip_follows_model.cpp (strip widgets follow the model; V).
- Probes that load compositions with a layer blendMode: .harmony/probe-lane3-layer.json (blendMode 1), probe-composition.json, probe-crossfade.py/.sh (+probe-crossfade.json), probe-effects-parity.py, probe-render-state.py, probe-capture.py.
- No existing test or probe renders a keying mode or compares blend modes. (Absence: INFERRED from grep for keyingMode / key_ / applyLayerKeying across tests/ and .harmony/: only the files above matched, none a keying render.)

How to drive WITHOUT synthetic input (production REST 7070 and test server 8080; docs/claude/testing-eyes.md, integration.md):
- `POST /api/load_composition {"path": "<file>"}` (ApiServer.cpp:251; used by probe-crossfade.py:102): write a bf9b-format composition JSON with a top-level "layers" array (Layer::toVar fields: type, opacity, blendMode, keyingMode, keyThreshold, keySoftness, chromaKeyR/G/B/Tolerance, transitionMode) plus "decks" with rows (Deck.h:117-146; Composition.h:989-1001). A pre-bf9b composition with per-deck layers also loads via ShowMigration (first deck's layer settings win). INFERRED format; copy .harmony/probe-lane3-layer.json (old format) or a saved show from `POST /api/debug/save_composition`.
- `POST /api/trigger_clip` / `trigger_column` (ApiServer.cpp:178,182) to start clips; `POST /api/set_layer_opacity` (:196) for V; `POST /api/render_frame {"output_path": ...}` (7070 and 8080) for a deterministic capture, compare by DECODED PIXELS (testing-eyes.md:74).
- NO REST route sets `keyingMode`, `keyThreshold`, `keySoftness`, `blendMode`, `transitionMode` or `Layer::type` (grep of ApiServer.cpp / TestServer.cpp routes: only /api/set_layer_opacity touches a layer); the only ways are a composition file (above) or the UI. `/api/composition` (GET) and `/api/state` expose `blendMode` per layer (ApiServer.cpp:439, 480) but not keyingMode (VERIFIED).
- For a source with known alpha, use `POST /api/load_source` / a procedural clip in the composition ("mediaType": 4, "sourceType": ...), as probe-lane3-layer.json does.
