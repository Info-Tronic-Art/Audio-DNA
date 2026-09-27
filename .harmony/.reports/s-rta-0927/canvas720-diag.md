## BUILDER REPORT — canvas720-diag (diagnosis only; nothing edited, built or committed)

STATUS: DONE
RESULT: NOT A BUG. The "four copies with (15,15,15) grey borders" in the 720p render_frame is the fixture image itself. The clip showing at that moment was B = `media/P16_02_Screen_Split_2x2.png`, a 756x878 screenshot of a Screen Split 2x2 that already contains four copies inside a #0F0F0F frame. That frame was compared against f0, which is fixture A. On main, every canvas size (1280x720, 1024x768, 1920x1080, 3840x2160), set either at runtime or from a fresh composition load, renders A as one full-bleed picture and B as B's own 2x2 grid. The preview panel and snapshots behave the same way. Nothing a performer would see is wrong.
FACTS: `/Users/boriskarpman/projects/RealTimeAudio/media/P16_02_Screen_Split_2x2.png` is 756x878. Viewed directly, it is four copies of the three-blob picture on a (15,15,15) frame. Lane evidence: d(`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/look/f720_render_frame.png`, B stretched to 1280x720) = 0.029, and d(same, `f0_render_frame.png` downscaled to 720p) = 29.42, which equals d(A, B) = 29.4 from the fixture doc. The lane's `fB_render_frame.png` at 1920x1080 already shows the grid: d(fB, B stretched) = 0.009, and all four quadrants are identical (quadrant d = 0.00). The probe script leaves B active: o_probe_tracks_change runs `trig(1)` (lane .harmony/probe-outputs.py line 317 @ e14027c, via git show), and o_probe_survives_resolution_change (`:328`) does not re-trigger col0 before its `f720 = cap(...)` (`:336`). Canvas path on main: `/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.cpp` lines 266-314 (debounced outputWidth/Height, then resolveCanvas, ensureCanvasFBO, fitCanvas present) and `/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.cpp` line 2071 (capture = whole canvas at canvasW_ x canvasH_). A `/Users/boriskarpman/projects/RealTimeAudio/src/render/Renderer.cpp` line 2144 comment says a user snapshot is the whole composition canvas. Those comments match the measured output. They are not the evidence.
METHOD: Two live launches of the MAIN app (`open -g … --args --test-mode`, under the lock as canvas720-diag). The probe's scene was rebuilt exactly (deck 0 L0 Opaque, col0 = A, col1 = B, 1920x1080). Steps: trigger col0 → f0; trigger col1 → fB; 8080 set_composition_params 1280x720; 0.4 s wait → f720 (this is the exact lane step). Then with A active at the same 720p canvas: render_frame, /api/snapshot, and a Quartz window-id capture. Then runtime sizes 1024x768 and 3840x2160 with both A and B. Then restore to 1080. Launch 2 loaded the composition at 1280x720 before any trigger. Every PNG was decoded with PIL/numpy: d vs A-stretched and d vs B-stretched, the 2x2 quadrant self-similarity, and the (15,15,15) run widths along the w/4 column and the h/4 row. Scripts: `scratchpad/canvas720/diag.py` and `run.sh`.
CONFIDENCE+VERIFY: HIGH. The two hypotheses predict different frames for A at a non-1080 canvas. A tiling render bug predicts four copies of A. The image-content hypothesis predicts one A. Measured: one A at every size (quadrant d = 46.26, no grey runs, d vs A ≤ 0.013). Re-check: `python3` decode of `…/scratchpad/canvas720/runtime.Fqyn07/f720_A_runtime.png` should give d(A stretched) ≈ 0.013 and no (15,15,15) runs. Also look at `…/runtime.Fqyn07/win_B_720.png`: the cB clip thumbnail in the grid is itself a 2x2 grid.
UNKNOWNS/NOT-DONE: Syphon and video recording at 720p were not run live. Both read the same canvas FBO per the Renderer.cpp:249-250 comment, and render_frame and snapshot came out identical (inferred, not measured). The Output window was not opened, per the rig rules.
NUANCE: The misread came from the reference frame, not from the app. f0 is A. f720 was captured after the tracks-change row left B triggered. The grey-border widths match B's own frame scaled to the canvas. At 1280x720 they are 21/42/21 px horizontal and 12/24/12 px vertical. That matches the observed "~23 L/R, ~13 T/B", and it is exactly 2/3 of the 1920x1080 widths (32/64/32, 18/36/18).
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### 1. Reproduction on MAIN (4b0c39a app, built 06:05; Test Server ON)

Run 1 output dir: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/canvas720/runtime.Fqyn07/`

| frame | size | d vs A stretched | d vs B stretched | max quadrant d (0 = 2x2 tiled) | (15,15,15) runs, row h/4 / col w/4 |
|---|---|---|---|---|---|
| f0_A_1080 | 1920x1080 | 0.008 | 29.431 | 46.26 | none |
| fB_B_1080 | 1920x1080 | 29.431 | 0.009 | **0.00** | 32/64/32 ; 18/36/18 |
| **f720_B_runtime** (exact probe step) | 1280x720 | 29.425 | **0.029** | **0.00** | 21/42/21 ; 12/24/12 |
| snap_B_720 (/api/snapshot) | 1280x720 | 29.425 | 0.029 | 0.00 | 21/42/21 ; 12/24/12 |
| **f720_A_runtime** | 1280x720 | **0.013** | 29.431 | 46.26 | none |
| snap_A_720 (/api/snapshot) | 1280x720 | 0.013 | 29.431 | 46.26 | none |
| f1024x768_A_runtime | 1024x768 | 0.012 | 29.428 | 46.26 | none |
| f1024x768_B_runtime | 1024x768 | 29.427 | 0.027 | 0.00 | 17/34/17 ; 13/26/13 |
| f3840x2160_A_runtime | 3840x2160 | 0.007 | 29.430 | 46.26 | none |
| f3840x2160_B_runtime | 3840x2160 | 29.429 | 0.009 | 0.00 | 64/128/64 ; 36/72/36 |
| fback_A_1080 | 1920x1080 | 0.008 | 29.431 | 46.26 | none |

The app's stderr confirms each capture was taken at the requested canvas size (`[Eyes] Processing capture: canvas 1280x720` and so on).

(a) Fresh composition at 1280x720, loaded before any trigger (run 2, `…/canvas720/fresh720.G0a2oU/`):
- fresh720_A: d vs A = 0.013, quadrant d = 46.26, no grey. This is the same as the runtime change.
- fresh720_B: d vs B = 0.029, the same grid.
- The runtime change and the fresh load produce identical frames.

(b) 1024x768 and 3840x2160 set at runtime: see the table. A is a single picture at every size, and B shows its own grid scaled to the canvas.

(c) Quartz window-only captures of the main window, window id 4444, 1 layer-0 window, no Output-named window:
- `win_A_720.png`: the lower-left Preview panel shows one A picture, letterboxed 16:9 inside the panel.
- `win_B_720.png`: the panel shows the 2x2 grid. The cB clip thumbnail in the clip grid shows the same 2x2 grid, because that is what the file looks like.
- The performer sees the right thing in both cases.

(d) Snapshot at 1280x720: /api/snapshot wrote `~/Documents/Audio-DNA/Snapshots/snapshot_20260927_1112{28,29}.png`, both 1280x720 and identical to render_frame. I copied them into the out dir and deleted the app-side copies I had created.

### 2. Mechanism

There is no rendering mechanism behind this. The picture is the input.
- `media/P16_02_Screen_Split_2x2.png` is an effect-reference screenshot (the name says so): four copies on a #0F0F0F frame. The deck path stretches the clip to the canvas. d(frame, B bilinear-stretched) is between 0.009 and 0.029 at every size.
- In the lane probe, row order is matches_canvas (A), then portrait, then tracks_change (`trig(1)` = B, lane `probe-outputs.py:317`), then survives_resolution_change (`:328-336`). That last row resizes and captures without re-triggering, so f720 is B at 720p.
- Harmony's "at 1920x1080 the same scene is one full-bleed picture" was f0, which is A. The lane's own fB at 1920x1080 shows the same four copies.
- The resolution-change code (`Renderer.cpp:266-314`: debounce, resolveCanvas, ensureCanvasFBO, clear to black, fitCanvas present) works as designed.
- Confirming run: the discriminator in section 1 (A at 720 is one picture) is that run.

### 3. probe-canvas coverage

On main, three `.harmony/probe-canvas.py` rows cover content identity at non-1080 sizes, and all use A or a ramp:
- c_4k_shape (`:237`): 3840x2160, downsized and compared with the default A frame, tolerance 6.
- c_custom_4x3 (`:264`): 1024x768, d vs A resized, tolerance 6 (`:275`).
- c_runtime_change_keeps_history (`:305`): 1080 → 2560x1440 at runtime, ramp image, d(f1 resized, f0) ≤ 6.

A render that tiled the canvas 2x2 would give d ≈ 46 on these rows (the measured quadrant d of A), which is far above 6. So they have teeth for this class of bug. They passed because nothing is tiling. B appears only in the perf and F2 rows, which never check shape.

### Boris-visible symptom

None. The preview panel, render_frame and snapshot show the true composition at 1280x720, 1024x768 and 3840x2160, whether the size is set at runtime or at load. Syphon and recording read the same canvas (inferred; not run live).

### Fix options (probe-side only; there is no app defect)

1. Lane `.harmony/probe-outputs.py` o_probe_survives_resolution_change (`:328`): add `trig(0); time.sleep(settle)` before the resize, so f720 is A. Then add a content assert: d(f720, A stretched to 1280x720) ≤ 6. This also pins the canvas content at non-1080 on the frame path.
2. Or keep B and check f720 against the matching reference: d(f720, fB resized to 1280x720) ≤ 6. Name the reference in the output line so a human reviewer compares like with like.
3. Hygiene: `P16_02_Screen_Split_2x2.png` looks like a render artefact when viewed out of context. Add a line to the probe-outputs.json and probe-canvas.json `_doc` noting that fixture B is itself a 2x2 grid on a #0F0F0F frame.

### FILES CHANGED
None in the repo. The only file written in the repo is this report. Scratch files are under `scratchpad/canvas720/` (diag.py, run.sh, runtime.*, fresh720.*).

### TESTS
Two live runs: 11 frames in run 1 plus 2 snapshots, 2 in run 2, and 4 window captures. All render_frame/snapshot calls returned ok and the PNGs decoded.

### RIG
- Lock acquired after 4 polls: routines-timing held it at first.
- AUDIODNA_LOCK_OWNER=canvas720-diag.
- Both apps quit via osascript within 30 s; no pkill was needed.
- After release, routines-timing re-took the lock (owner routines-timing 40161) and launched its own app (pid 40422, started 11:14:13). That app is not mine and was left alone.
- I released my lock: `/tmp/audiodna-live.lock` was absent right after release.
- No Audio-DNA process that I launched is still running.
- No Output window: every window census showed 1 layer-0 window.
- No full-screen capture, lldb or synthetic input.

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: the packet did not say which clip was active when f720 was taken. The row order in the lane probe answered that, and it was the key fact.
- Unused context: the (a)-(d) fan-out went unused as a mechanism probe once A at 720 settled the question. It was still run, as evidence that no surface is affected.
- Self-brief files: CLAUDE.md (Pitfall 37 canvas) was useful. No department config.

### KNOWLEDGE CONTEXT
- Tools used: grep plus live decode.
- Impact authority: none, and no change was proposed to app code.
- Risk level: NORMAL.
