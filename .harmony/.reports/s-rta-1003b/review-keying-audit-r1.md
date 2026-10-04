# Review keying-audit r1 (independent method review, read-only)
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED: lane/keying-audit head 85fb630 (51ba916 probe+prereg, ca8e695 results, 85fb630 report); app source 34179a2
MUST: 0   SHOULD: 5   NIT: 7

## Answer
No MUST. Every table cell I recomputed from the committed JSON matches; the menus are complete and in menu order; every
NO-OP / ALIAS verdict is independently confirmed by the source (not an artefact of the fixtures); the probe quits only
its own pid and opens no Output window. Five SHOULD items are about labels/overclaims, not about a wrong number.

## Checks (VERIFIED = executed/read on disk)
1. Completeness (VERIFIED): LayerStrip.cpp:10-100 populateMixModes = 55 entries in 13 groups (5/6/4/2/3/4/2/6/4/2/6/2/9),
   populateBlendDropdown :1099-1121 = 13 keying entries; Layer.h:217-260 enum order == report order (Normal=0 is menu "Alpha",
   first of Compositing). 13 + 55 + 55 = 123 rows in JSON and in the md tables. Report prose counts (49 blend no-ops, 14 own-picture
   transitions, 41 dissolves, 4 distinct keying pictures, 6 do-nothing) all re-add correctly.
2. Method (VERIFIED): diff 51ba916..85fb630 of .py/.sh changes only capture merging, the tbm family, an extra NOT-DETERMINED guard
   and a logged alpha range; formulas, TOL (3 / 8), FLOOR rule and verdict code are byte-identical to the pre-registration. No tolerance widened.
   Layer type 1 = Transparent (Layer.h:193-196); Layer.cpp:137-161 reads type/keyThreshold/keySoftness/chroma*/transitionSpeed unconditionally.
   Inputs back bit-identical (JSON inputs.*: 0), so no sRGB conversion between capture and formula. vflip chosen by the transition fit is False
   for all 14 own-shader entries (no hidden up/down swap). Transition models match the shaders (iris, flip, cut, fade_black read at Emb.h; wipes/pushes fit mean<=0.08).
   Source independently confirms every alias/no-op: Renderer.cpp:1895-1911 (key_inv_luma / key_saturation = LumaKey; key_luma_alpha / key_max_rgb = Light;
   6 entries = Alpha "fallback"); CompositorEngine.cpp:1386-1411 (6 blend cases + default = Normal); :1459-1474 (14 shaders + default dissolve);
   :1332-1333 sets u_threshold/u_softness, EmbeddedShaders.h:2501-2506 reads u_luma_*; no stale-frame artefact (entry 4 equals Alpha, not entry 3, etc.).
3. Numbers (VERIFIED, script): all 13 keying rows, all 55 blend rows (verdict, d_ref, by-name worst, GL worst), all 55 transition rows
   (verdict, p gap, dissolve worst, min d_cut) and the controls rows (opacity ratios, chroma fits, blend-opacity moved/alpha, fade slider,
   tbm, freeze) match keying-audit.json exactly. 0 mismatches (only cosmetic "(1)" alias suffix).
4. Sheets (VERIFIED, looked at kg, kr, ka, blend ba/bb, transition tb, controls-other): agree for Chroma Key (green removed), Luma Key (only black block / thin
   left line), Multiply/Darken black on ba. One disagreement: see S4.
5. NOT-DETERMINED: none appear in the tables; section 7 items are honestly listed. Cheaper path for "K slider writes keyThreshold": LayerStrip.cpp:423-425 (read, true).
6. Controls: model fields do arrive from the file (chroma tolerance/colour steer the picture through the same fromVar), cause stands (no key shader declares u_threshold).
7. Scripts (VERIFIED): probe.sh gates on lock owner, refuses if any Audio-DNA runs, records OURPID, quits via probe-quit-ours.sh (kills only OURPID;
   foreign pid => untouched). probe.py calls only health, load_composition, ui_text, trigger_column/clip, set_layer_opacity, render_frame: no Output, no input, no capture.

## Findings
S1 SHOULD (VERIFIED) transition table row 1 "Alpha: WORKS" and row 25 "Dissolve: ALIAS-OF Alpha (0)" (probe.py:896-899, report :161,:185,:242-245):
   Alpha has no shader (default dissolve); the pre-registered WORKS means "fits the entry's own shader". The code manufactures WORKS for the first dissolver.
   Report discloses only the Dissolve half (concern 4). Fix: use Dissolve (24) as the canonical reference, mark Alpha "ALIAS-OF Dissolve (24)", so WORKS = 14 not 15.
S2 SHOULD (INFERRED) report :299 (9c) "REFUTED BY RUN": the 3-level colour move under Multiply/Darken at frame alpha 64/128 equals the 255/(2*alpha) quantisation of an 8-bit
   premultiplied path (report :54-55 already shows that path at alpha 4); GL Multiply RGB = src*dst does not depend on alpha. The color-moves-by-3 evidence cannot separate app from capture.
   Fix: relabel NOT-DETERMINED / "capture-path quantisation likely"; keep the alpha-channel (64/128/255) finding, which is solid.
S3 SHOULD (VERIFIED in source, dropped in report) Luma/Inverted/Saturation Key "step(luma>0)" and Chroma "hard edge" rest on smoothstep(e,e,x) which GLSL leaves undefined for edge0>=edge1
   (facts-keying.md headline 3 says so; the audit does not carry it). MEASURED on this GPU only. Fix: add the caveat to :14-23 and :288-306.
S4 NIT/SHOULD (VERIFIED, cropped sheet-controls-other.png) report :276 "chroma colour green / red / blue / white each removes that colour": on pair kg red and blue key remove nothing
   (disc 220,30,30 is 0.216 from pure red > tol 0.2; same for blue); only kr shows red/blue removal. The numeric row (:224) is right (formula fits); fix the prose.
S5 SHOULD (VERIFIED) coverage: report :349-350 lists "the inspector's 25-entry Blend Mode list" as NOT TESTED. LayerInspector.cpp:990-1003 is enums 0-24 in enum order and onChange :166-170 writes blendMode=sel-1, so it is covered by rows 1-25 of the blend table
   (INFERRED by identity). Same for the inspector 13-entry keying list (:1007-1018) and its transition list (:303-307, writes transitionMode). Say "covered by enum identity".
N1 NIT (VERIFIED) row :238 "Transition blend method (transitionBlendMode)" and fact-sheet 8 "CONFIRMED BY RUN": run only shows no effect on a Dissolve at 3 values; "read by nothing" is the source grep (git grep: only Layer.cpp/Layer.h),
   and no widget writes this field (the inspector's transitionBlendSelector_ writes transitionMode, LayerInspector.cpp:307). Relabel.
N2 NIT (VERIFIED) F slider 0 s: LayerClock.h:21 turns speed<=0 into 0.5 s; the D=0 sample (captured 1.0 s later) cannot tell instant from 0.5 s. :236 "works" holds for 2/4/8 s.
N3 NIT headline :14 "K slider ... MEASURED: no": swept via file field; widget->field link is code-read (true, LayerStrip.cpp:423-425). Mark INFERRED for the widget half.
N4 NIT "Cut WORKS" (:186): fits the shader, which holds the old clip until p=0.5 (EmbeddedShaders.h:4128-4140); disclosed in sections 4 and 6, but the table cell says WORKS. Add "(delayed to 0.5)".
N5 NIT frames (97 MB) are git-ignored; JSON stores d_ref but no alias distances. A reader cannot re-derive ALIAS/NO-OP between non-reference entries. Commit a per-frame sha256 manifest.
N6 NIT pre-registration timing: 51ba916 is 22:52:20 and runs are stated as starting 22:52; nothing committed proves the first frame is later. JSON "head":"34179a2" is a literal (probe.py:584), not read from the binary
   (binary 18:37 follows merge 17574d1 18:35 and src is unchanged to 34179a2: consistent, INFERRED).
N7 NIT probe-quit-ours.sh:37-47 "ours" = the only Audio-DNA after launch; if Boris launched his app between the pre-launch check and `open`, `open` would not start a second instance and the probe would adopt his pid. Very narrow window; INFERRED.

## Concern 1 of the report (transitions given verdicts instead of NOT-DETERMINED)
Supported: LayerClock.h:17-24 advances crossfadeProgress by dt regardless of the time override (VERIFIED); p fitted within +-0.08 of the wall clock agrees to <=0.0063;
dissolve residual max 0.94 < FLOOR 1.5; own-shader means <=0.68; entries differing from dissolve differ by >=104. A one-free-number fit cannot turn Zoom/Push/Flip into a dissolve. Harmony may still rule it a packet deviation.
