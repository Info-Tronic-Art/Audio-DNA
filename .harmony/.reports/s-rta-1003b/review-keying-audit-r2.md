# Review keying-audit r2 (independent method review, read-only)
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED: lane/keying-audit head e22ef2d (fix round 1b323c0 probe, e22ef2d results+report; base reviewed in r1: 85fb630); app source main 34179a2
MUST: 0   SHOULD: 1   NIT: 4

## Answer
Round 1 had 0 MUST; its 5 SHOULDs (S1-S5) are all fixed by a METHOD change committed BEFORE a live re-run (1b323c0 23:46:00, re-run 23:46:03-23:47:33, e22ef2d 23:57:20),
then `analyze` over all frames; no verdict was typed by hand. The fix introduced no new defect that changes a verdict. Keying + blend JSON sections are byte-for-byte unchanged
(VERIFIED: recursive diff 85fb630 vs e22ef2d: 0 differences under /keying and /blend; /noise N and FLOOR unchanged, only states 20->21 for the new black fixture).

## Round-1 findings: fixed by method + re-run? (VERIFIED = read on disk / executed against the committed JSON)
- S1 transition WORKS-by-position (VERIFIED). probe-keying-audit.py:986-1001: CANON = the key whose TRANS_SHADER is "dissolve" (24); WORKS only for e==CANON, every other dissolve-picture entry
  ALIAS-OF Dissolve (24), NOT-DETERMINED if 24 was not captured/fit. JSON: dissolve_reference 24; verdict counts 15 WORKS + 40 ALIAS = 55; the 15 WORKS = Dissolve + 14 own-shader entries (cut, 4 wipes, iris, 4 pushes, zoom in/out, flip_h, fade_black). 41 verdicts changed vs 85fb630,
  no distance changed except the 4 re-captured entries (0,3,11,24). The builder's correction of r1's arithmetic ("14 not 15") is right: count stays 15. The only entries that can be WORKS are those with a shader.
- S2 Multiply/Darken colour move (VERIFIED). capture_rt (py:428-433) is JUCE premultiply ((c*a+0x7f)>>8) then unpremultiply (min(255,(pm*255)/a)); PixelConvert.h:50-67 at 34179a2 (git show) does exactly p.premultiply(); p.unpremultiply() per pixel.
  JSON controls.blend_opacity rows 3 and 11, pair bg: capture_model_maxabs_at_0.25 = 0, _at_0.5 = 0 (exact, every pixel) with frame alpha 64/128/255. New family bfix (py:196-201, 346-355): third layer opaque black, type 1, blendMode 1 (Add);
  JSON blend_opacity_alpha_restored: frame_alpha [255,255] at all three opacities on both pairs, moved 0.0 for Multiply and Darken, GL formula identical at 0.25/0.5/1 (bg mean 0.247 / 0.0). This is an independent fixture, not a relabel, and it separates the app from the capture path. "REFUTED BY RUN" withdrawn at md fact-sheet row 9c. The canvas ALPHA finding (64/128/255) stays and is MEASURED.
- S3 undefined smoothstep caveat (VERIFIED in text): carried in section 1, fact-sheet row 3, U1/U2, cause hints, section 7; GPU/OS named.
- S4 chroma prose (VERIFIED): JSON controls.chroma_colour.pairs.kg.changed_fraction red 0.0, blue 0.0, white 0.0363, default 0.5195; kr red 0.069, blue 0.0448, white 0.0432; chroma_tolerance and chroma_subject_colour rows (subjred 8.72 %, d 200; subjblue 6.68 %, d 170; formula fit mean 0.0) all match md section 3 and section 4. Sheet-controls-other.png LOOKED at: kg tiles red/blue untouched plate, the kc2 row shows only the disc / only the block turned to checker. Agree.
- S5 inspector menus covered by enum identity (labelled INFERRED/read, no run claimed): ok.

## Checks of the pinned brief, re-run on the fixed state
1. Completeness/menu order: unchanged from r1 (13 keying, 55 mix, 55 transition); the enum list at 34179a2 not touched by this lane (git diff 85fb630 e22ef2d touches only .harmony/). 123 rows in JSON.
2. Method artefact hunt: Normal = entry 0, layer 1 type 1 (Transparent), layer 0 type 0; no tolerance changed (TOL 3/8, FLOOR 1.5 N=0 unchanged); Cut: tg_i2 shows A, tb_i2 shows B at p_wall 0.4981/0.499 (sheets) = switch at ~0.5, consistent with EmbeddedShaders.h transitionCut (p < 0.5).
3. Numbers: parsed 54 of 55 md transition rows and compared label, verdict, p gap (max over 6 frames), Dissolve-model worst max-abs, min d_cut and "ends on B" with the JSON: 0 mismatches (the Cut row has a differently shaped cell and was read by hand: dissolve worst 104.49, d_cut 0, ends on B: matches). Keying and blend tables identical to r1 (diff of md sections = none). Controls rows bfix / kc2 / capture_model / changed_fraction match JSON (above). Cosmetic only: "max 0" cell for Dissolve own fit is "%.0f" of 0.5; 51.9 % is 0.5195 truncated.
4. Sheets LOOKED at (sheet-transition-tb, -tg, sheet-controls-other): agree - Dissolve tiles are cross-dissolves for all 40 aliases; Wipe Ellipse opens an ellipse from the centre; To Black / Flip show black at the middle instant; Cut tile jumps A to B; Multiply/Darken restored tiles are three identical pictures per row. Disagreement: none found.
5. NOT-DETERMINED: no entry reads it in the tables; section 7 items unchanged and honest.
6. Controls sweeps: K/Softness "no effect" rests on the file field; model field arrives (chroma tolerance/colour from the same fromVar do steer) - unchanged from r1.
7. Scripts (VERIFIED): `git diff 85fb630 e22ef2d -- .harmony/probe-keying-audit.sh` is comment/usage lines only; quit path (probe-quit-ours.sh: refuse_foreign_start, record_ourpid, quit_ours kills only OURPID) unchanged; the new py families call only the existing routes (load_composition, ui_text, trigger_column/clip, set_layer_opacity, render_frame); no Output window, no input, no screen capture.

## Findings
S6 SHOULD (VERIFIED in JSON, explanation INFERRED) md section 3 controls rows "canvas alpha brought back to 255" (md :256-257) and JSON blend_opacity_alpha_restored rows[*].pairs.ba.d_unrestored_at_1 = 128.0 (unrestored alpha 0..255):
  the text calls the third layer "neutral at opacity 1" but on pair ba the restored frame differs from the two-layer frame by 128, explained only by "its alpha 0-255". Neutrality is proven by a number on bg (0) only. The explanation is plausible
  (low-alpha pixels lose most under premultiply/unpremultiply) but is not tested although capture_rt exists and the frames are on disk: add to analyze `maxabs(capture_rt(restored_rgb, unrestored_alpha), unrestored)` (no new run). Does not change any verdict (the identical-at-three-opacities result is inside the restored frames).
N8 NIT (VERIFIED) md :93 "Over all 330 frames the fitted p is within 0.0046 of the wall-clock p" (and :439): true for the 324 non-Cut frames (max 0.0046); the 6 Cut frames have gap up to 0.08 (JSON entries[25].fits, own.p 0.4181 vs p_wall 0.4981) because p is unidentifiable for a cut (table says "n/a"). Say 324 / "excluding Cut".
N9 NIT (VERIFIED) keying alias rule left un-amended: Max RGB reads ALIAS-OF Luma Is Alpha while Luma Is Alpha is BROKEN (md concern 4, :447). Disclosed; inconsistent with the S1 logic applied to transitions. Harmony to rule.
N10 NIT md section 1 "they ignore the opacity slider" for Screen/Multiply/Darken/Lighten: for Multiply/Darken the canvas ALPHA follows the opacity (64/128/255, MEASURED); only the colour is identical. The parenthesis says "the colour"; add "the canvas alpha does follow it; what that looks like on screen is not measured (section 7)".
N11 NIT transitions 3 and 11 were re-captured in the fix round only because the shared enum list filters bfix and trans alike (probe usage); harmless (static frames bit-identical across launches, p fitted), but the 4 re-captured entries come from a second app launch while 51 stay from the first; say so in the hand-over.

## Risk
Residual: everything is MEASURED on one GPU (Apple M1 Pro, macOS 15.7.9); the unfixed S6 means the "third layer neutral" claim on pair ba is INFERRED, not shown. Frames are not committed (r1 N5): ALIAS/NO-OP between non-reference entries cannot be re-derived from the committed JSON alone.
