# ruling-bf10 -- ARCHITECT RULING on the blind council's attacks, lane bf10 (s-rta-1002b)

Architect, 2026-10-02.
- Plan: `.harmony/.reports/s-rta-1002b/plan-bf10.md` (base main fa9604d).
- Seat papers: `.harmony/.reports/s-rta-1002b/attack-bf10-papers.md`. They were inline in the dispatch, not on disk, and
  were written there verbatim before this ruling. There are two seats, both AMEND:
  - attack-bf10-live, A1-A10, cited below as "live-An";
  - bf10-gates, A1-A10, cited below as "gates-An".
- No app was launched. The new evidence comes from:
  - three offline rigs that link the STOCK libprojectM 4.1.1 in a drawable-less CGL 4.1 core context on this Mac (Apple
    M1 Pro);
  - static disassembly of the installed dylib;
  - code reads.
- The evidence index is at the end. VERIFIED = run or read here. INFERRED and ASSUMED are labelled where they are used.

**VERDICT: plan-bf10 is SOUND WITH AMENDMENTS. It is ready to build as amended below.**

The design stands:
- backport `projectm_opengl_render_frame_fbo`;
- draw straight into the canvas-sized `outputFBO_`;
- delete the framebuffer-0 blit.

One attack changes production code, by three lines:
- **live-A5 (GL state).** The ruling rig shows that libprojectM leaves GL sampler objects bound after every frame:
  - texture units 1-4 in steady state;
  - units 0-11 during a soft cut, several of them NEAREST / REPEAT.
- The app never binds a sampler, and `saveGLState` does not cover them. So today every layer drawn after a MilkDrop frame
  samples with projectM's wrap mode. `restoreGLState` now binds sampler 0 on units 0-15 (amendment 8).

Everything else is tests, gate validity, build hygiene and docs.
- **Count.** All 20 attacks are ruled: 19 accepted in whole or in part, and 1 rejected with evidence (live-A10).
- **Two seat premises are corrected by evidence:**
  - live-A1: no deck-switch or clip-trigger path loads a preset.
  - live-A6: projectM draws its final picture with blending OFF.
- **The relayed request** ("decks are boxes of clips; switching decks never changes what plays") belongs to lane bf9b.
  For MilkDrop, bf10:
  - adds a deck round-trip row (m9) that is valid under both today's deck semantics and bf9b's;
  - hands bf9b its 20-deck acceptance row and three constraints (H1-H4). One of them is a real interplay neither seat
    named: the MilkDrop preset-playlist advance only walks the active deck's layers.

---

## 1. RULINGS, attack by attack

### live seat (10)

- **live-A1 [MUST] No row switches decks while MilkDrop plays.**
  -> **ACCEPT, scoped (amendments 3 and 4). REJECT one premise.**
  - **Premise rejected (VERIFIED by grep).** No deck-switch or clip-trigger path calls `ProjectMSource::loadPreset`. Its
    only callers are:
    - the MilkDrop browser selection (`src/MainComponent.cpp:1780`);
    - the 8080 test route (`src/test/TestServer.cpp:1746`);
    - the BEAT-driven playlist advance (`src/render/Renderer.cpp:655`, `:669`, `:680`);
    - the autopilot callback (`src/sources/ProjectMSource.cpp:30-32`).
  - The canvas is the only input that resizes MilkDrop, and it is composition-level (`Renderer.cpp:454-455`). A deck
    switch therefore never resizes MilkDrop or reloads its preset.
  - **Accepted: m9_deck_roundtrip joins G2.** Its gate predicate compares the picture before leaving deck 0 with the
    picture after returning to it. That holds under today's deck semantics AND under bf9b's.
  - The captures taken while another deck is shown are INFO only, because that is exactly what bf9b changes.
  - **bf9b owns "switch between 20 decks and the playing is not affected".**
    - H1 gives bf9b the row.
    - H2 is the constraint the seat asked for: a deck switch never calls loadPreset, resize or releaseGL.
  - **Interplay the seat did not name (H3).** The playlist advance iterates only the active deck's layers inside
    `if (deckActive)` (`Renderer.cpp:594-683`). Under "decks are boxes", a playing MilkDrop clip's playlist would freeze
    while another deck's grid is shown. That changes "how they are playing".

- **live-A2 [MUST] Opaque black first frame after a canvas change.**
  -> **ACCEPT measurement and disclosure (amendment 17). REJECT "hold the last picture" and "alpha 0".**
  - **The black frame is pre-existing, not new in bf10.** diag-bf10 captured it ON MAIN: `lock_new_640x360`, mean RGB 0,
    alpha 255. bf10 does not touch `resize()` or `projectm_set_window_size` (plan I3: "No change to initGL, resize").
  - **Alpha 0 is pixel-identical to opaque black on the bottom layer.** The canvas is cleared to opaque black every frame
    (`Renderer.cpp:507-511`).
  - **Neither proposal reaches the visible part.** The visible part is the multi-frame re-warm, while the trails rebuild
    from black.
  - **Instead:**
    - measure the re-warm in both arms on a real preset, together with the peak frame time;
    - put the numbers into Boris question Q2.
  - If Boris answers "not fine", F4 is filed (keep the picture through a size change).

- **live-A3 [MUST] The perf gate cannot fail at 4K, and arm A is unrepresentative.**
  -> **ACCEPT the teeth fix (amendment 5) and a two-layer INFO row. REJECT the two-layer absolute floor and A' as the
  baseline.**
  - **Teeth.** The 4K exemption and the "INFO when the bar is ≤ 2 x spread" demotion are removed. The noise rule now:
    - uses per-round paired deltas;
    - escalates to 10 rounds;
    - ends in UNPROVEN, which is never PASS.
  - **No floor on two MilkDrop layers.** Two layers render ONE shared projectM instance twice per frame (plan E5 / E15b;
    `Renderer.cpp:1314-1325`). That is R8's pre-existing defect, and a floor on it would fail bf10 for R8's reason. It is
    measured as INFO (`perf_md_1080_2l`) and feeds F1.
  - **A stays the baseline.**
    - Arm A does strictly LESS final-pass work than B: its last draw is clipped to the panel. So A is the conservative
      arm: any bar B passes against A, it also passes against A'.
    - Using A' would hide R3 (the rebuilt library), which Boris receives in the same merge.
    - A' stays the discriminator.
  - **Expected cost (INFERRED from physics).** The fix adds one full-canvas copy: about 8.3 MP at 4K. On this GPU that is
    of the order of 0.1-0.4 ms, well inside the 2.0 ms bar.

- **live-A4 [SHOULD] After the merge every other configure is FATAL; the build needs the network.**
  -> **ACCEPT sequencing, an offline source path and a stop rule (amendments 6 and 7). REJECT the opt-out flag.**
  - VERIFIED: `~/.local/opt` does not exist yet.
  - **Why other worktrees are not broken.** The prefix is machine-level (`$HOME/.local/opt/...`). The amended
    FindProjectM searches it first and drops a cached `projectM4_DIR` that lies outside it (plan I2). Once S1 has run on
    this Mac, every worktree here configures. So the real precondition is "the prefix is installed and verified on this
    machine before S2 merges", and amendment 7 makes that the merge precondition.
  - **Why no opt-out.** An `AUDIODNA_ALLOW_STOCK_PROJECTM` opt-out needs one of two things:
    - a second code path (the old blit), which is F3(b), rejected because it silently ships the bug;
    - a MilkDrop-less build, which is F3(c), rejected because Boris silently loses MilkDrop.
  - CI never finds projectM (plan E18), so CI is unaffected.
  - **Accepted: `PROJECTM_SRC=<dir|tarball>` builds without network.** It is also the missing vehicle for R1's fallback:
    a fetched tarball had no way into the script. The script also keeps a source archive for offline re-runs.

- **live-A5 [SHOULD] GL state leakage is untested.**
  -> **ACCEPT (amendments 8, 9 and 12). One premise corrected.**
  - **Premise corrected.** The blit never "reset" state. `restoreGLState` rebinds `GL_FRAMEBUFFER`, so both READ and
    DRAW, to the saved binding (`ProjectMSource.cpp:300`), with or without the blit.
  - **What libprojectM changes across `projectm_opengl_render_frame` (rigs R-E3 / R-E4, VERIFIED):**

    | state | restored by saveGLState? |
    |---|---|
    | READ framebuffer | yes |
    | active texture unit (up to unit 12) | yes |
    | blend enable and blend funcs | yes |
    | texture bindings on units other than the caller's active unit | no (F3) |
    | sampler-object bindings | **no** |

  - **The sampler bindings left behind:**
    - steady state: units 1-4;
    - during a soft cut: units 0-11;
    - units 0 and 1 get LINEAR / CLAMP_TO_EDGE;
    - units 3, 5, 7, 9 and 11 get NEAREST / REPEAT.
  - **Colour masks are never changed.**
    - Rig: the diff shows no change.
    - Static: `glColorMaski` is called only in `Framebuffer::MaskDrawBuffer` (0x37894), which has no caller.
  - **Why it matters to the app.**
    - The app and JUCE never bind a sampler: grep of `src/` and `tests/`; JUCE only loads the function pointer.
    - The render code samples units 0 and 1 (`GL_TEXTURE0` x44, `GL_TEXTURE1` x8).
    - So today, every layer drawn after a MilkDrop frame samples unit 1 with projectM's wrap mode, and unit 0 too during
      soft cuts.
  - **Fix (amendment 8):** `restoreGLState` binds sampler 0 on units 0-15.
  - **The fix is safe for projectM's own rendering (VERIFIED).** The rigs unbind every unit before EVERY call, and
    libprojectM re-binds everything it needs inside each call.
  - **Tests:**
    - T5 asserts every state saveGLState restores, plus the sampler bindings and the colour masks.
    - m10 adds a live canary: an opaque image layer over MilkDrop.

- **live-A6 [SHOULD] Alpha and blend; outputFBO_ is never cleared.**
  -> **ACCEPT the T1 strengthening (amendment 10). REJECT the unconditional `glDisable(GL_BLEND)`.**
  - **Rig R-E2 (VERIFIED).** The rig enters the call with blend ON and ONE/ONE. Blend is OFF after the call:
    - for a comp-shader preset;
    - for a no-comp-shader preset;
    - during a soft cut.
  - **Static:**
    - `FinalComposite::Draw` disables blend (0xff54);
    - `Filters::Draw` ends with a disable (0xf6bc);
    - `CopyTexture::Draw` (0x34040-0x34134) never touches blend;
    - `PresetTransition::Draw` never touches blend.
  - **So projectM's final draw is unblended, whatever state the caller enters with.** A disable at entry cannot reach it.
  - **The real guard is a test.** T1 re-clears the sentinel to alpha 0 before every one of 30 frames. Two fixes are
    pre-registered, each tied to the symptom it cures:
    - alpha < 255 with no sentinel leak: F6(a)'s alpha-only clear AFTER the call;
    - sentinel leaking into the result: a coverage fix or an opaque clear BEFORE the call.

- **live-A7 [SHOULD] Paths the plan forgot.**
  -> **ACCEPT T6b and the scope statement (amendments 11 and 19). REJECT the recorder-across-change row and the routine
  row.**
  - **T6b** puts a soft cut in flight across a canvas change, in the unit rig. Live REST has only a hard-cut route
    (`TestServer.cpp:1746`, smooth=false).
  - **Recorder (no row).**
    - No REST route starts it; only a menu command does (`MainComponent.cpp:7097-7116`).
    - It reads `canvasFBO_` by `glReadPixels` (`Renderer.cpp:1035-1038`). That is the same texture every render_frame
      capture reads.
    - bf10 touches no recorder code.
  - **Routines (no row).** A routine fires a clip through the same trigger and the same compositor `sourceRenderFn_` call
    that m4 exercises (plan E4).

- **live-A8 [SHOULD] Peak frame time at a canvas change is never gated.**
  -> **ACCEPT the seat's alternative: report the number to Boris in Q2 (amendment 17). REJECT the gate.**
  - bf10 does not change the allocation path: `resize()` and `projectm_set_window_size` are untouched.
  - A single-event peak on a shared Mac has no stable bar.
  - A canvas change is a setup action. Under bf9b the canvas stays composition-level, and Load Deck never changes it.

- **live-A9 [NIT] T1 "RED on main" is only inferred.**
  -> **ACCEPT. Now VERIFIED (amendment 1).**
  - Rig R-E1 runs main's exact `render()` shape. In it:
    - framebuffer 0 is `GL_FRAMEBUFFER_UNDEFINED` (0x8219);
    - both `projectm_opengl_render_frame` and the blit raise `GL_INVALID_FRAMEBUFFER_OPERATION` (0x506), on every
      frame;
    - 100.00 % of the magenta, alpha-0 sentinel survives, at 1920x1080 and at 640x360, across 7 sampled frames each.
  - T5 is RED on main on steady frames too, through the 0x506 error and the sampler bindings.
  - The builder still records the RED of the committed test before the fix.

- **live-A10 [NIT] Pixel diff of presets, stock vs patched, with a time override.**
  -> **REJECT. Amendment 16 replaces it.**
  - **No time can be pinned.**
    - libprojectM 4.1.1 has no frame-time API (header grep: no frame_time / set_time / timestamp).
    - It times itself with `std::chrono` (it imports `steady_clock::now` and `system_clock::now`).
    - `ProjectMSource::render` ignores the app's time argument (`float /*time*/`, `ProjectMSource.cpp:121`).
  - Two runs of a live preset therefore never match pixel for pixel.
  - **"Same engine" is checked deterministically instead (G0.6):**
    - the GLSL string set is identical (stock: 129 strings, set hash prefix bffe0439eff25046);
    - the exported symbols are identical plus exactly the one new function (stock: 52 exports, hash prefix
      e2eae5a7bac65856);
    - both embed the same VCS SHA.

### gates seat (10)

- **gates-A1 [MUST] T1/T2/T3/T6 are RED on main for a rig artefact.**
  -> **ACCEPT the relabel (amendment 1). REJECT both stand-in rigs.**
  - **"A test-owned window-sized FBO as the default target" cannot work.** libprojectM binds the literal name 0
    (`glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)` at 0x89518, plan E8). No FBO the test owns can receive the stock
    library's final draw.
  - **"A drawable-backed context" needs a window inside ctest.**
    - No GL test here does that; they all use a private CGL context (`tests/test_source_defaults_gl.cpp:99-183`).
    - diag-bf10 found no core-profile pbuffer path.
  - **What the unit RED really is.** It is RED-by-absence, and it is the ROOT CAUSE: projectM's picture never reaches
    `outputFBO_`. That is exactly the invariant the fix establishes.
  - The crop SHAPE is witnessed only live: m1 / m6 / m7 with the pre-registered box fingerprint (amendment 2).

- **gates-A2 [MUST] The live RED values are not pre-registered.**
  -> **ACCEPT (amendment 2).**
  - **Fingerprint.** On the pre-lane app the alpha-255 region is ONE bottom-left box, the same size at every RED canvas.
    That size is the Preview panel's physical size (756x840 in the diag).
  - **Coverage** = box / canvas, within ±1 point:

    | canvas | expected coverage |
    |---|---|
    | 1280x720 | 59.06 % |
    | 1920x1080 | 30.63 % |
    | 2560x1440 | 17.23 % |
    | 3840x2160 | 7.66 % |
    | 1080x1920 | 30.63 % |

  - These match plan E6 recomputed: 756x840 / (1920x1080) = 30.625 %.
  - **Stop rule.** A RED row that passes on the pre-lane app, or fails without the fingerprint, voids the row and stops
    the lane.

- **gates-A3 [MUST] G5 has no bars.**
  -> **ACCEPT the bars, pinned presets, critic outputs and FAIL-on-MUST (amendment 15). REJECT the pixel-diff
  regression row** (see live-A10; replaced by G0.6).
  - The three pinned presets cover both composite code paths:
    - the comp-shader path (`FinalComposite::Draw`);
    - the no-comp path (`VideoEcho` + `Filters`).
  - The two paths are distinct: the disassembly branches at 0xff34.

- **gates-A4 [SHOULD] The teeth rule can erase the perf gate.**
  -> **ACCEPT (amendment 5).**

- **gates-A5 [SHOULD] No row checks a real Output or a recorded frame.**
  -> **ACCEPT the seat's own fallback: the gap is stated in the gate list, plus an erratum (amendment 18).**
  - A recorder row needs a new REST route. That is new test surface, beyond this lane.
  - The recorder and Syphon read `canvasFBO_` directly (`Renderer.cpp:1035-1038`, `:1045`). Those are the pixels m1-m5
    and m8-m10 capture.
  - m7 covers Output windows (`publishToOutputs`, `:1117-1122`).
  - W1 covers the projector.

- **gates-A6 [SHOULD] T5 has gaps.**
  -> **ACCEPT (amendment 12).**
  - **New variants:** a pending hard load and a pending soft load on the measured frame. Both now run inside the
    save/restore window, because saveGLState moves to the first statement.
  - **RED reasons, now labelled:**
    - binding 0 on init / resize frames: VERIFIED by code (`ProceduralSource.cpp:286` and `ProjectMSource.cpp:71`, both
      before `:164-165`);
    - the GL error: VERIFIED by rig (0x506);
    - the sampler clause: VERIFIED by rig.
  - **The error clause keeps its teeth.** S1's lib-probe records every error code inside the fbo call with a complete FBO
    bound. If it records any, T5 tolerates exactly those codes and nothing else.

- **gates-A7 [SHOULD] The fixtures are unverified.**
  -> **ACCEPT (amendment 13).**
  - Each fixture is validated in the lib-probe, on the patched library, at 320x180 and 180x320. It is then frozen
    (sha256) before T1-T6b run.
  - Fixtures may be iterated only there. After the freeze, a T-case failure is a code failure and is never retuned.

- **gates-A8 [SHOULD] An INVALID m6 has no consequence.**
  -> **ACCEPT (amendment 14).** INVALID after one rerun = G2 FAIL.

- **gates-A9 [NIT] The BF9 relation has no gate.**
  -> **ACCEPT, scoped (amendments 3 and 4)**, as for live-A1.

- **gates-A10 [NIT] S1's network assumptions.**
  -> **ACCEPT (amendment 6).**
  - Unless `PROJECTM_SRC` is set, S0 first runs `git ls-remote` with a 20 s timeout.
  - If that fails, S1 is BLOCKED (exit 3) and S2 never starts.

---

## 2. NEW EVIDENCE (this ruling)

- **R-E1 (rig, VERIFIED): main's render() shape in the unit-test context.**
  - The rig (`ruling10/rig.cpp`) uses the stock lib and reproduces `ProjectMSource.cpp:170-179` exactly:
    1. a magenta, alpha-0 sentinel in outputFBO_, re-cleared every frame;
    2. bind framebuffer 0 and set the viewport;
    3. call `projectm_opengl_render_frame`;
    4. blit READ 0 into outputFBO_.
  - Results:
    - framebuffer 0 status = 0x8219 (GL_FRAMEBUFFER_UNDEFINED);
    - the render call error = 0x506 and the blit error = 0x506, on every sampled frame;
    - the sentinel survives 100.00 %.
  - Cases: preset 319 then a soft cut to Superstrings at 1920x1080; Organic Circuits then a soft cut to Confetti at
    640x360; frames 0, 1, 59, 60, 61, 90 and 119.
- **R-E2 (rig, VERIFIED): blend is OFF after every call.**
  - Entry state: blend ON, ONE/ONE. Blend is OFF after the call in all 14 sampled frames: comp-shader, no-comp and
    soft-cut cases.
  - `CopyTexture::Draw` touches no blend state (disassembly 0x34040-0x34134). So the final copy is unblended.
- **R-E3 (rig, VERIFIED): sampler objects left bound** after `projectm_opengl_render_frame` (`ruling10/samplers*.cpp`):

  | case | units left with a sampler |
  |---|---|
  | preset 319, steady | 1-3 (LINEAR / CLAMP_TO_EDGE), 4 (LINEAR / REPEAT) |
  | Geiss Confetti, steady | 1-3 (LINEAR / CLAMP_TO_EDGE) |
  | EVET Superstrings, steady | none |
  | soft cut | 0-1 LINEAR / CLAMP_TO_EDGE; 2, 4, 6, 8, 10 LINEAR / REPEAT; 3, 5, 7, 9, 11 NEAREST / REPEAT |

  - The rigs unbind all 16 units before EVERY call, and the bindings reappear after each call. So libprojectM sets them
    inside each call.
  - GL_MAX_TEXTURE_IMAGE_UNITS = 16 on this GPU.
- **R-E4 (rig, VERIFIED): the other changes across the call**, beyond samplers:
  - READ framebuffer: 0 -> 8 / 9 / 11;
  - active texture unit: 2 -> 0 or 12;
  - texture bindings on units 1-3;
  - blend enable and blend funcs (770 / 771 after a comp preset).
  - Unchanged: viewport, program, VAO, array buffer, pack / unpack alignment, unpack row length, depth / stencil / cull /
    scissor enables, and colour masks of draw buffers 0-3.
- **R-E5 (static, VERIFIED): where libprojectM touches state.**
  - `glColorMaski` appears only in `Framebuffer::MaskDrawBuffer` (0x37894), which has no call site.
  - `glPixelStorei` appears only in SOIL's image loaders, in save/restore pairs.
  - `glViewport` appears in `MilkdropPreset::RenderFrame` (0x11cd8) and in `BlurTexture::Update`'s set/restore pair.
  - libprojectM imports no `glScissor` and no `glBlendEquation`.
  - `FinalComposite::Draw`:
    - with a comp shader: `glDisable(GL_BLEND)` at 0xff54;
    - without one: `VideoEcho::Draw` then `Filters::Draw`, which ends with `glDisable(GL_BLEND)` (0xf6bc).
- **R-E6 (grep, VERIFIED): who loads presets, and what the deck switch touches.**
  - The callers of `loadPreset` are the four listed under live-A1.
  - The canvas is composition-level (`Renderer.cpp:454-455`).
  - The MilkDrop playlist advance walks only the active deck's layers (`Renderer.cpp:594-683`, inside
    `if (deckActive)`).
- **R-E7 (VERIFIED): there is no way to pin MilkDrop's time.**
  - The 4.1.1 headers have no frame-time API.
  - The dylib imports `std::chrono::steady_clock::now` and `system_clock::now`.
  - `ProjectMSource::render` ignores `time` (`:121`).
- **R-E8 (VERIFIED): the stock dylib's identity.**
  - `nm -gU`: 52 exports; sorted list sha256 prefix e2eae5a7bac65856.
  - `strings -n 8 | grep -E 'void main|#version|uniform ' | sort -u`: 129 strings; sha256 prefix bffe0439eff25046.
  - It embeds `03aa8a7ffdf81165136ee64643c6a781f5c6a391`.
- **R-E9 (code, VERIFIED): what outputs read.**
  - The recorder reads `canvasFBO_` (`Renderer.cpp:1035-1038`).
  - Syphon publishes `canvasFBO_` (`:1045`).
  - The output tap feeds Output windows (`:1117-1122`).
  - Recording starts only from the menu command (`MainComponent.cpp:7097-7116`); there is no REST route.
- **R-E10 (grep, VERIFIED): nobody else binds samplers.**
  - There is no `glBindSampler` in `src/` or `tests/`.
  - JUCE's `juce_gl.cpp/.h` only declare and load the pointer.
  - The render code uses texture units 0 and 1 only (src/render, src/effects, src/output).
- **R-E11 (VERIFIED): the S1 prefix is absent.** `~/.local/opt` does not exist (`ls`).
- **R-E12 (plan defect): T6 as written cannot pass.**
  - T6 takes "40 paced frames" at about 16 ms, about 0.64 s of wall time.
  - The soft cut lasts 1.0 s on projectM's own clock (INFERRED from R-E7).
  - So "the last frames equal B" cannot hold. Amendment 11 restates T6 in wall time, so it holds whichever clock projectM
    uses.
- **R-E13 (code, VERIFIED): the legacy fallback and the deck transition.**
  - A deck with no active clip falls back to the legacy single source (`Renderer.cpp:816-823`).
  - `load_milkdrop_preset` makes MilkDrop that source (`TestServer.cpp:1730`).
  - So a probe that shows an empty deck after loading a preset sees MilkDrop through the fallback, not through any deck
    semantics. m9 and H1 de-confound this with 8080 `load_source {"source_type":"plasma"}`.
  - The default deck transition lasts 0.3 s (`Composition.h:114`).

---

## ARCHITECT RULING (s-rta-1002b)

The numbered amendments below OVERRIDE the plan body wherever they conflict with it. Unamended plan text stands.

### Amendments

**1. Unit RED labels (gates-A1, live-A9).** This replaces plan I3 "All six are RED on main", the T1 and T5 RED notes,
R12, and G1's RED clause.
- **T1, T2, T3, T4, T6 and T6b are RED on main BY ABSENCE.**
  - In the drawable-less test context, framebuffer 0 is GL_FRAMEBUFFER_UNDEFINED.
  - Main's render call and its blit both raise 0x506 every frame.
  - The returned texture keeps the sentinel (R-E1).
  - This is the root cause: projectM's final picture never reaches `outputFBO_`. It is not the live crop shape. The crop
    shape is witnessed only live (amendment 2).
- **T5 is RED on main on every frame kind:**
  - steady frames: the 0x506 error (R-E1) and the sampler bindings (R-E3);
  - init and resize frames: the caller's binding is replaced by 0. `createFBO` binds 0 (`ProceduralSource.cpp:286`) and
    so does `initGL` (`ProjectMSource.cpp:71`), both before `saveGLState` (`:164-165`).
- **R12 is closed.** It is VERIFIED that framebuffer 0 has no storage in the test context.

**2. Pre-registered live RED values and the void/stop rule (gates-A2).** This replaces plan I4 "Calibration".
- **The box fingerprint (pre-lane app).** The alpha-255 pixels form ONE solid axis-aligned rectangle:
  - anchored at the picture's bottom-left;
  - with ≥ 99.9 % alpha 255 inside it and none outside it;
  - the same width and height (±2 px) at every RED canvas of the run. That size is the panel's physical size; the diag
    measured 756x840.
- **Expected coverage** = rectangle / canvas, ±1 point:
  - with 756x840 these are the values in the table under gates-A2;
  - if the measured rectangle differs from 756x840, recompute the fractions from the measured rectangle and record the
    window bounds.
- **Order.** Calibration runs on the pre-lane app FIRST, all rows, and only then on the lane app. Both are quoted in the
  lane report.
- **RED rows.** On the pre-lane app each must FAIL its GREEN bar AND show the fingerprint:
  - m1 at 1280x720, 1920x1080, 3840x2160 and 1080x1920;
  - m2, m3 and m4 at all their canvases;
  - m5, m8, and m9's C0 capture;
  - m6: at every canvas, including 640x360, at least one bar side's 12-px strip has a mean more than 6 from the control's
    strip;
  - m7: at least 60 % of `output_probe` pixels lie outside ±6 of the fixture colour (expected 69.4 %).
- **GUARD rows** must PASS on the pre-lane app: m1 at 640x360, and m10.
- **VOID / STOP.** If a RED row passes on the pre-lane app, or fails without the fingerprint:
  - that row is VOID;
  - the lane STOPS before any lane-app run;
  - the builder reports the numbers and never retunes.
- **A GUARD row failing on the pre-lane app** is a pre-existing defect: record it and file it. It must still PASS on
  the lane app.

**3. New G2 row: m9_deck_roundtrip (live-A1, gates-A9).**
- **Setup.**
  - 7070 `load_composition` of a JSON with:
    - canvas 1920x1080;
    - `globalTransitionSpeed` 0.3;
    - `activeDeckIndex` 0;
    - unique deck ids and clip ids (Pitfall 36).
  - Deck contents:
    - deck 0: L0 col0 = `{mediaType 4, sourceType "projectm_visualizer"}`;
    - deck 1: L0 col0 = an UNtriggered image clip (a probe-made solid blue 1920x1080 PNG);
    - deck 2: no clips.
- **Solid phase.**
  1. 7070 `trigger_clip {layer 0, column 0}`.
  2. 8080 `load_milkdrop_preset(bf10_solid)`.
  3. 8080 `load_source {"source_type":"plasma"}`. This makes the legacy fallback show plasma, not MilkDrop (R-E13).
  4. Wait 1.5 s, then capture C0.
  5. 7070 `switch_deck 1`, wait 1.0 s, capture C1 (INFO).
  6. `switch_deck 2`, wait 1.0 s, capture C2 (INFO).
  7. `switch_deck 0`, wait 1.5 s, capture C3.
- **Live phase.**
  1. `load_milkdrop_preset(P1)`, then `load_source plasma` again.
  2. Wait 3 s.
  3. Repeat the deck walk 0 -> 1 -> 2 -> 0.
  4. After the return, take two captures 0.5 s apart.
- **GREEN:**
  - C0 and C3 each meet "uniform";
  - per channel, |median(C3) - median(C0)| ≤ 6 (nothing reloaded, nothing stopped);
  - the live-phase pair meets m2's bar.
- **RED (pre-lane app):** C0 shows the box fingerprint.
- **INFO:** what C1 and C2 show. Today that is deck 1 / deck 2 content or plasma. After bf9b it must be the MilkDrop
  picture, and that is bf9b's gate, H1.

**4. Hand-off to bf9b (live-A1, gates-A9).** Harmony relays H1-H4 (section "HANDOFF") to the bf9b planner. bf10 ships no
bf9b code and does not wait for bf9b. Either lane may merge first.

**5. G3 perf decision rule (live-A3, gates-A4).** This replaces the plan's G3 table rows, the teeth rule and the 4K
exemption. The driver command is unchanged.
- **Rows:**
  - `perf_md_1080` (GATE);
  - `perf_md_4k` (GATE);
  - `perf_md_1080_2l` (INFO): the m4 composition with L0 and L1 both MilkDrop clips, triggered, the heavy preset, at
    1920x1080.
- **Paired deltas.** For each round r (interleaved: A, then B), compute d_r = B_r - A_r for gpu_ms and for fps, using
  the launch means.
- **Bars** (medians over rounds):
  - **1080:**
    - (a) median fps_B ≥ 58 when median fps_A ≥ 58; otherwise median d_fps ≥ -2;
    - (b) median d_gpu ≤ 1.0 ms.
  - **4K:**
    - (a) median d_gpu ≤ 2.0 ms;
    - (b) median d_fps ≥ -3.
- **Decidability.** This replaces the teeth rule.
  - A delta bar is decidable iff spread(d) = max(d) - min(d) ≤ 2 x the bar's size:
    - d_gpu: 2.0 ms at 1080, 4.0 ms at 4K;
    - d_fps: 4 at 1080 (the -2 variant), 6 at 4K.
  - Decidable: PASS iff the median meets the bar; otherwise FAIL.
  - Not decidable after 5 rounds: run 5 more rounds into a second OUTDIR and decide on all 10 rounds together.
  - Still not decidable: the row is UNPROVEN, which is never PASS.
- **UNPROVEN.**
  1. Harmony re-runs G3 once, in a quiet window with no lane building.
  2. If it is still UNPROVEN, merging is allowed ONLY IF 1080 bar (a) PASSED and every UNPROVEN median meets its bar.
     G3 is then recorded as "UNPROVEN, medians under the bars", with the numbers, in the closure.
  3. If any median is over its bar: no merge; run the A' discriminator.
- **4K absolute fps.** It is reported only, and it triggers Q1 when both arms are below 58 fps. It exempts nothing.
- **A' discriminator** (unchanged): it runs on any FAIL, or on any UNPROVEN median that is over its bar.
- **`probe-milkdrop-ab.py` prints, per row:** median A, median B, median d, spread(d), rounds, and a verdict in
  {PASS, FAIL, UNPROVEN}.

**6. S0 pre-flight, offline source and stop rule (gates-A10, live-A4).** This amends plan I1's script and R1.
- **Script order.**
  - (0) **If `PROJECTM_SRC` is set:** use it.
    - It may be a source directory at 03aa8a7 with submodules, or a `.tar.gz` of one; a tarball is extracted into
      `$TMPDIR`.
    - When it is a git tree, verify that HEAD is 03aa8a7ffdf81165136ee64643c6a781f5c6a391.
  - (1) **Otherwise:** run `git ls-remote https://github.com/projectM-visualizer/projectm.git` with a 20 s timeout. On
    failure, print `BLOCKED: no network -- set PROJECTM_SRC=<dir|tarball>` and exit 3.
  - (2) Clone, checkout and init submodules, as in the plan.
  - (3) Before patching, archive the unpatched tree with its submodules to `$HOME/.local/src/projectm-03aa8a7.tar.gz`
    (`PROJECTM_SRC` accepts it on any later run).
  - (4) The rest as in the plan.
- **Stop rule.** Exit 3 means S1 is BLOCKED: the builder reports BLOCKED and stops. S2 never starts until S1's GREEN
  bar (lib-probe, symbol and install name) has passed.

**7. Merge sequencing (live-A4).** This replaces the plan's "Merge sequencing" paragraph and I2's stale-cache row.
- **Merge precondition.**
  - The prefix is installed and verified on this Mac (G0.2 on the prefix).
  - G0.1 through G0.6 pass on the lane branch, in scratch build dirs.
  - The lane report quotes them.
- **I2's stale-cache row never touches main's `build/`.**
  - The row: a scratch configure of the lane tree with `-DprojectM4_DIR=$HOME/.local/lib/cmake/projectM4` (the value
    main's cache holds) must exit 0, and its STATUS line must name `projectm-4.1.1-fbo1`.
  - Main's own `build/` is re-configured only after the merge, by Harmony (G0.1).
- **S1's projectM compile** starts only when no other lane holds the rig's quiet lock. If one does, it waits.
- **No opt-out flag.** F3(a) stands.
- **Post-merge notice** (Harmony to every active lane): "After rebasing onto bf10, re-configure. The 'libprojectM:'
  STATUS line must name projectm-4.1.1-fbo1. Nothing else to do on this Mac."

**8. Sampler restore, a production change (live-A5).**
- **Code.** In `ProjectMSource::restoreGLState` (`src/sources/ProjectMSource.cpp:298-322`), append at the end:
  ```cpp
      // libprojectM leaves its own sampler objects bound (units 1-4 every frame, 0-11 during a soft cut;
      // ruling-bf10). The app binds no sampler object anywhere, so the caller's state is sampler 0 on every unit.
      for (GLuint unit = 0; unit < 16; ++unit)   // 16 = the GL 4.1 minimum GL_MAX_TEXTURE_IMAGE_UNITS
          glBindSampler(unit, 0);
  ```
- It runs on every return path, because every path calls `restoreGLState` (plan I3 step 1).
- **Header.** Append one sentence to the `ProjectMSource.h` header comment: "... and leaves no sampler object bound
  (restoreGLState)."
- **No other production change** is added by this ruling.

**9. New G2 GUARD row: m10_layer_over (live-A5).**
- **Composition** (7070 `load_composition`), at 1920x1080:
  - deck 0 L0 = a MilkDrop clip with `bf10_solid`, triggered;
  - deck 0 L1 = an image clip of a probe-made OPAQUE 1920x1080 PNG with four quadrant colours, triggered, Normal blend,
    opacity 1.
- **Captures:**
  - after 1.5 s;
  - again after a canvas round trip 1920x1080 -> 1080x1920 -> 1920x1080, with 1.5 s at each size.
- **GREEN, both arms:** each capture has ≥ 99.9 % of its pixels within ±3 per channel of the PNG.
- It catches any MilkDrop GL state that leaks into the layer drawn after it: framebuffer, viewport, blend, scissor,
  colour mask or sampler.

**10. T1 strengthened, and symptom-keyed fixes (live-A6).** This replaces plan T1 and F6(a)'s trigger.
- **T1 procedure.**
  - Sizes: 1280x720, 1920x1080, 1080x1920 and 3840x2160.
  - Fixture `bf10_solid`, with `setPresetLocked(true)`.
  - The first `render()` at each size is the init or resize frame.
  - Then 30 frames. Before EACH one, the test clears the returned texture to the sentinel (255, 0, 255, 0) through its
    own FBO, then calls `render()`.
  - Read back after frames 1, 2, 10, 20 and 30.
- **PASS iff, at each read:**
  - the texture is exactly the requested size;
  - alpha is 255 on 100 % of pixels;
  - ≥ 99.99 % of pixels lie within ±6 of the per-channel median;
  - the median is within ±6 of the bf10_solid colour frozen in S1 (amendment 13);
  - 0 pixels equal the sentinel.
- **Pre-registered fixes.** Apply the matching fix, re-run T1-T6b, and record which fix fired.
  - (i) Alpha < 255 and no sentinel colour in the result: projectM wrote alpha < 1.
    -> Add F6(a)'s alpha-only clear AFTER the call: colour mask (0,0,0,1), clear to alpha 1, then restore the colour
    mask.
  - (ii) The sentinel survives in a REGION.
    -> Add `glViewport(0, 0, w, h)` inside the patch, right after its bind (the plan's I3 viewport rule).
  - (iii) The sentinel tints the WHOLE frame: the final draw blended.
    -> Add an opaque clear of `outputFBO_` right after binding it, BEFORE the render call.
- No other conditional fix is allowed.

**11. T6 in wall time, plus new T6b (live-A7; plan defect R-E12).** This replaces plan T6.
- **T6.**
  - Setup: `bf10_solid`; `getPresetSelector().setBlendSeconds(1.0)`; `loadPreset(bf10_solid_b, true)`.
  - Render frames paced at about 16 ms until 2.0 s of WALL time have passed since the `loadPreset` call (about 120
    frames).
  - Read every 10th frame and the last 5.
  - PASS iff:
    - every read is alpha 255 everywhere;
    - every pixel lies inside the per-channel box [min(A,B) - 6, max(A,B) + 6];
    - at least one read's median lies ≥ 10 levels from both A and B in some channel;
    - every read after 1.5 s equals B (±6).
  - A and B are the frozen fixture colours.
- **T6b** is T6 with one change: about 0.3 s after the `loadPreset` call, `render()` switches from 1920x1080 to
  1080x1920.
  - Extra reads at change +1, +2 and +3 frames.
  - PASS iff:
    - every read is the requested size, with alpha 255 everywhere;
    - from +3 frames on, the box clause holds;
    - every read after 1.5 s equals B (±6).
  - RED on main: by absence.

**12. T5, the caller's state survives (gates-A6, live-A5).** This replaces plan T5.
- **Entry state, for every variant.**
  - The test binds its own 64x64 sentinel FBO with `GL_FRAMEBUFFER` (READ and DRAW), cleared to (17, 34, 51, 255).
  - Viewport (1, 2, 33, 44).
  - Texture unit 2 active, with a test texture bound on it.
  - `GL_BLEND` on, with `glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE)`.
  - Scissor, depth, stencil and cull off: the app's state.
  - Sampler 0 on units 0-15.
  - GL errors drained.
- **Variants:**
  - (a) a steady frame;
  - (b) the init frame (the first render() on a fresh source);
  - (c) a resize frame;
  - (d) a pending HARD load on the measured frame: `loadPreset(bf10_solid_b, false)` just before it;
  - (e) a pending SOFT load: `setBlendSeconds(1.0)` + `loadPreset(bf10_solid_b, true)`, measured on that frame and the
    next 10.
- **PASS iff, after `render()`:**
  - DRAW and READ binding == the sentinel FBO;
  - the viewport is (1, 2, 33, 44);
  - program and VAO are unchanged;
  - the active texture unit is unit 2, with its `TEXTURE_BINDING_2D` unchanged;
  - blend enable and the four blend funcs are unchanged;
  - depth / stencil / cull / scissor enables and the depth func are unchanged;
  - the colour write masks of draw buffers 0-3 are unchanged;
  - `GL_SAMPLER_BINDING` == 0 on units 0-15;
  - the sentinel FBO's pixels are unchanged;
  - `glGetError()` == `GL_NO_ERROR`. The one exception: codes the S1 lib-probe recorded inside the fbo call (amendment
    13).
- **INFO** (printed, not asserted): texture bindings on units 0-15 other than unit 2. That is a pre-existing leak, F3.
- **RED on main:**
  - (b) and (c): binding 0 (code);
  - every variant: 0x506 (rig);
  - (e): sampler bindings ≠ 0.
- **If the fixtures leave no sampler bound on main in any variant,** add variant (f): a steady frame with the bundled
  `Geiss - Confetti (Jelly).milk` (rig: units 1-3). It is named here and is not a tuned choice.

**13. Fixture validation and freeze in S1 (gates-A7).** This replaces the plan's "The builder iterates the fixtures only
against the FIX".
- **The lib-probe gains a fixture mode.**
  - Patched library, fbo mode.
  - One FBO of 320x180 and one of 180x320.
  - 90 frames per fixture.
- **Validation bars:**
  - `bf10_solid` and `bf10_solid_b`:
    - ≥ 99.9 % of pixels within ±6 of the median;
    - median max channel ≥ 64;
    - alpha 255 on 100 %;
    - the two medians differ by ≥ 40 in at least one channel.
  - `bf10_time`: the frame mean changes by ≥ 2 levels between reads 300 ms apart (paced).
  - `bf10_circle`: one lit blob, with bbox |w/h - 1| ≤ 0.04 and its centre within ±2 % of the canvas centre, at BOTH
    320x180 and 180x320.
- **Record:**
  - the medians A and B, and the circle's bbox;
  - every glGetError code after each fbo-mode call (this is T5's baseline);
  - the sha256 of the four `.milk` files, in the lane report and in a header comment of the test. The test asserts
    against the recorded colours.
- **Freeze.** Fixtures may be iterated ONLY here, before T1-T6b are written. After the freeze, any T-case failure is a
  code failure: STOP and report. Never retune a bar or a fixture.

**14. m6 INVALID (gates-A8).** An m6 control that is INVALID again after one rerun is a G2 FAIL, never "report and
continue".

**15. G5 bars (gates-A3).** This replaces plan G5.
- **Presets:**
  - P1 = m2's pinned preset, named in the calibration report before any AFTER capture;
  - P2 = `flexi - grind my glitch up [319].milk`, the comp-shader path;
  - P3 = `EVET - Superstrings I.milk`, which has no comp shader: the VideoEcho + Filters path.
- **Captures.**
  - Window-only, by Quartz window id: BEFORE on the pre-lane app, AFTER on the lane app.
  - Each preset at 1920x1080, 3840x2160, 1080x1920 and 640x360.
  - Plus P1's change 1920x1080 -> 2560x1440, captured at +0.1, +0.5 and +2 s.
  - Every window capture is paired with an 8080 render_frame canvas capture taken right after it.
- **Decoded metrics.** The probe computes them and prints them as DATA lines BEFORE any critic sees an image:
  - canvas alpha-255 coverage: AFTER must be 100.000 % at every canvas and every change time; BEFORE shows the box
    fraction;
  - the m6 strip metric on each window capture: AFTER must have std ≤ 3 and a mean within ±6 of the m6 control.
- **GATE:** every decoded AFTER metric meets its bar.
- **Critics:** visual-design, UX, graphic-design, logic, and interaction-logic (for the change sequence).
  - Each returns exactly one verdict line, `PASS` / `SHOULD: ...` / `MUST: ...`, citing the decoded PNG paths it judged.
  - **G5 FAILS on any MUST.**
- **What the critics judge:**
  - the picture is edge to edge;
  - the bars are plain;
  - there are no seams, blocks or stale strips;
  - round shapes stay round;
  - at 640x360, the only size whose BEFORE picture is complete, each preset keeps its character (colour, motion, texture)
    BEFORE vs AFTER. This is a judgment, not a pixel diff (R-E7). The deterministic "same engine" proof is G0.6.
- **Then** the artifact page for Boris: before/after pairs, one plain sentence each.

**16. G0.6 engine identity (live-A10, gates-A3).** This is a new gate, run in S1 and again after the merge; see the gate
list.

**17. m3 re-warm INFO and Q2 numbers (live-A2, live-A8).**
- **Procedure.** m3 adds an INFO phase, run in BOTH arms during calibration and again in G2:
  1. Load P1 at 1920x1080 and wait 3 s.
  2. Capture the reference. L0 = mean luminance over the alpha-255 pixels.
  3. Read and discard 8080 `peak_frame_time_ms`.
  4. Set the canvas to 2560x1440.
  5. Capture at about +0.1, +0.25, +0.5, +1.0 and +2.0 s, recording the real times from the responses.
  6. Read `peak_frame_time_ms`.
- **DATA line:** `DATA m3_rewarm arm=<A|B> t=<s>:<L/L0> ... peak_frame_ms=<n>`.
- **Use.**
  - The lane arm's numbers fill Q2.
  - The pre-lane arm's numbers show that the re-warm is pre-existing.
  - No gate.

**18. Output and recorder coverage, stated in the gate list (gates-A5).** See the "Coverage statement" below the gates.
Erratum: plan I4 m7's text "This is what an Output, the recorder and Syphon read" becomes "This is what Output windows
read".

**19. Scope statement (live-A7).** Add to plan section (1):
- bf10 changes only three things:
  - what `ProjectMSource::render` draws into;
  - its GL-state save/restore;
  - the build.
- Untouched: save/load, undo, routines, autopilot / PresetSelector, REST / OSC / MIDI preset loads, Ableton Link, deck
  switching and the canvas policy.
- They reach MilkDrop only through the queued `loadPreset` and through `render()`. The callers are VERIFIED in H2.

**20. Docs.** These amend plan section (6).
- **pitfalls.md entry:**
  - append: "libprojectM also leaves GL sampler objects bound (units 1-4 every frame, 0-11 during a soft cut);
    restoreGLState binds sampler 0 on units 0-15 -- the app binds no sampler objects.";
  - the Guards line becomes: "tests/test_projectm_canvas_gl.cpp (T1-T6b); live .harmony/probe-milkdrop.sh m1-m10".
- **build-other-platforms.md:**
  - `PROJECTM_SRC` (offline build) and the archive `$HOME/.local/src/projectm-03aa8a7.tar.gz`;
  - the network pre-check, and "exit 3 = BLOCKED".
- **testing-eyes.md:**
  - one diagnostic line: "a solid alpha box at the picture's bottom-left, the size of the Preview panel, means something
    drew into framebuffer 0";
  - and: "after `load_milkdrop_preset`, an empty deck shows MilkDrop through the legacy fallback -- use `load_source` to
    de-confound".
- **APP-INVENTORY.md:** the counts read 7 unit cases (T1-T6b) and live rows m1-m10.
- Everything else as in plan section (6).

### FINAL BUILD STAGES

One builder, one worktree, one merge.

- **S0 -- pre-flight.** No repo change, minutes.
  - The network check, or `PROJECTM_SRC` (amendment 6).
  - Record the stock identity: exports, GLSL set hash, VCS SHA (R-E8).
  - Exit: S1 may start, or the lane is BLOCKED (exit 3).
- **S1 -- I1, plus amendments 6 and 13.**
  - The patched library goes into `$HOME/.local/opt/projectm-4.1.1-fbo1`.
  - The lib-probe meets the plan's I1 GREEN bars.
  - Fixture validation, the freeze, and the error baseline.
  - G0.2 and G0.6 run against the prefix.
  - The compile runs outside other lanes' quiet locks (amendment 7).
- **S2 -- I2 + I3, plus amendments 1, 7, 8, 10, 11 and 12.** In this order:
  1. Commit the test target (inserted after `tests/CMakeLists.txt:831`), the frozen fixtures and the I2 wiring.
  2. Run `ctest -R test_projectm_canvas_gl` on the PRE-FIX `ProjectMSource.cpp` and quote the RED for each case.
  3. Apply I3: saveGLState first; restore on every return; render into `outputFBO_` via `_fbo`; delete the blit; add the
     sampler loop (amendment 8); add the header sentences.
  4. GREEN, applying only amendment 10's symptom-keyed fixes.
  5. Run the full ctest suite.
  6. Run the scratch stale-cache row and the negative-control configure.
- **S3 -- I4 + I5, plus amendments 2, 3, 5, 9, 14, 15, 17, 18, 19 and 20.**
  - Probe rows m1-m10 and the perf rows.
  - Calibration on the pre-lane app FIRST, under the void/stop rule; then the lane app.
  - The docs.
- **Merge** when amendment 7's precondition holds. Then Harmony:
  - runs G0-G5 on merged main;
  - sends the post-merge notice;
  - relays H1-H4 to bf9b.
- **Dependencies.** bf10 has no ordering dependency on any other lane.

### FINAL CONSOLIDATED GATE LIST

Pre-registered. Harmony copies gate strings ONLY from this list. Shared definitions:
- **"uniform"**: alpha 255 on 100.000 % of pixels; ≥ 99.99 % of pixels within ±6, per channel, of the per-channel
  median; median max channel ≥ 64.
- **"box fingerprint"**: as defined in amendment 2.
- **Live capture rules:**
  - every capture is an 8080 render_frame WITHOUT a width/height override;
  - the canvas is set through 8080 `set_composition_params`, then the probe waits ≥ 1.5 s (m3's capture comes 0.5 s
    after a change, by design);
  - every request uses `Connection: close`;
  - no Output window, no synthetic input.

**G0 -- build hygiene and engine identity** (merged main, Harmony)
- **G0.1** Fresh configure + build of merged main in a scratch dir AND re-configure of main's existing build/: both exit
  0, and each prints "libprojectM: <...projectm-4.1.1-fbo1...> (render_frame_fbo: yes)".
- **G0.2** `nm -gU $HOME/.local/opt/projectm-4.1.1-fbo1/lib/libprojectM-4.dylib` lists
  `_projectm_opengl_render_frame_fbo`; `otool -D` prints `@rpath/libprojectM-4.4.dylib`.
- **G0.3** The built app's `otool -l` LC_RPATH contains `$HOME/.local/opt/projectm-4.1.1-fbo1/lib` and NOT
  `/Users/boriskarpman/.local/lib`; `otool -L` lists `@rpath/libprojectM-4.4.dylib`.
- **G0.4** build/compile_commands.json for `src/sources/ProjectMSource.cpp` has `-isystem
  $HOME/.local/opt/projectm-4.1.1-fbo1/include` and no other `.local/include`.
- **G0.5** Negative control: configuring a scratch dir with `-DAUDIODNA_PROJECTM_PREFIX=/nonexistent
  -DprojectM4_DIR=$HOME/.local/lib/cmake/projectM4` exits non-zero, and its output contains "build-projectm.sh".
- **G0.6** Engine identity, stock `~/.local/lib/libprojectM-4.4.1.1.dylib` vs patched
  `$HOME/.local/opt/projectm-4.1.1-fbo1/lib/libprojectM-4.dylib`:
  - (a) `nm -gU <dylib> | awk '{print $3}' | sort` differs by exactly one added line,
    `_projectm_opengl_render_frame_fbo` (stock: 52 symbols);
  - (b) `strings -n 8 <dylib> | grep -E 'void main|#version|uniform ' | sort -u | shasum -a 256` is equal for both
    (stock prefix bffe0439eff25046);
  - (c) both contain `03aa8a7ffdf81165136ee64643c6a781f5c6a391`.

**G1 -- unit**
- **G1.1** `ctest -R test_projectm_canvas_gl` on merged main: T1, T2, T3, T4, T5 (a-e, plus f if added), T6 and T6b
  all PASS. A SKIP is a FAIL.
- **G1.2** The full ctest suite is green.
- **G1.3** The lane report quotes all three of:
  - the RED run of the committed test on the pre-fix code: T1 sentinel 100 %; T5 binding 0 on init / resize, error
    0x506, and sampler ≠ 0 in (e) or (f);
  - the S1 fixture freeze: sha256, the A and B colours, the circle bbox, and the error baseline;
  - which amendment-10 fix fired, if any.

**G2 -- live** (hold the live lock; `.harmony/probe-milkdrop.sh m1..m10` on merged main)
- **G2.0** Precondition: the lane report's calibration on the pre-lane app shows every RED row failing WITH the box
  fingerprint (amendment 2 values) and every GUARD row passing. Otherwise the lane stopped, and G2 is not run.
- **G2.1 m1_fill_legacy.** Load `bf10_solid` with `load_milkdrop_preset`. At 1280x720, 1920x1080, 3840x2160, 1080x1920
  and 640x360: the PNG is exactly the canvas size AND "uniform".
- **G2.2 m2_live_no_stale.** P1 at 1920x1080 and 1080x1920; two captures 0.5 s apart:
  - alpha 255 on 100 % in both;
  - every 64x64 tile has ≥ 20 % of its pixels changed by more than 8 levels.
- **G2.3 m3_runtime_change.** `bf10_solid` 1920x1080 -> 2560x1440 -> 1920x1080, capture 0.5 s after each change: each
  PNG is the new size AND "uniform". The re-warm series is INFO (amendment 17).
- **G2.4 m4_comp_load_clip.** As plan I4, at 1280x720 then 1080x1920: both "uniform".
- **G2.5 m5_preset_switch.** At 1920x1080: `bf10_solid` -> capture A; `bf10_solid_b` -> 0.5 s -> capture B. B is
  "uniform", AND its median differs from A's by ≥ 40 in at least one channel.
- **G2.6 m6_panel_bars.** Window-only captures, control first, at 640x360, 3840x2160 and 1080x1920 (as in plan I4).
  PASS iff:
  - the control is valid (strips std ≤ 3 and max channel ≤ 40);
  - inside the present rect, ≥ 99.5 % of pixels are within ±10 of the rect's median;
  - the strips have std ≤ 3 and a mean within ±6 of the control's strips.
  - An INVALID control after one rerun = FAIL.
- **G2.7 m7_output_tap.** `set_output_tap` on, `bf10_solid` at 1920x1080. `output_probe` 1920x1080 has ≥ 99.99 % of
  pixels within ±6 of the median, and median max channel ≥ 64.
- **G2.8 m8_context_cycle.** After `/api/debug/gl_context_cycle`, G2.1's bar holds at 1920x1080.
- **G2.9 m9_deck_roundtrip.** As in amendment 3:
  - C0 and C3 are "uniform";
  - per channel, |median(C3) - median(C0)| ≤ 6;
  - the live-phase pair after the return meets G2.2's bar.
- **G2.10 m10_layer_over.** As in amendment 9: ≥ 99.9 % of pixels within ±3 per channel of the quadrant PNG, at steady
  state and after the canvas round trip.

**G3 -- perf.** Interleaved A/B, ≥ 5 rounds.
- **Arms:** A = the merge-base Release app; B = the merged-main Release app.
- **Command:** `LOCK_LIB=<lock helper> LANE=bf10 bash .harmony/probe-vupload-ab.sh <A> <B> 5 .harmony/probe-milkdrop.sh
  perf_md_1080,perf_md_4k,perf_md_1080_2l <out>`, then `.harmony/probe-milkdrop-ab.py <out>/ab.tsv [<out2>/ab.tsv]`.
- **G3.1 perf_md_1080 (GATE):**
  - (a) median fps_B ≥ 58 when median fps_A ≥ 58; otherwise median d_fps ≥ -2;
  - (b) median d_gpu ≤ 1.0 ms.
- **G3.2 perf_md_4k (GATE):**
  - (a) median d_gpu ≤ 2.0 ms;
  - (b) median d_fps ≥ -3.
- **G3.3 perf_md_1080_2l (INFO):** medians for both arms and d, reported to F1.
- **G3.4** The decidability, UNPROVEN and A'-discriminator rules of amendment 5 apply verbatim. UNPROVEN is never PASS.
  Check `ps` for burners before any verdict.

**G4 -- regression.** `.harmony/probe-canvas.sh`, all rows, PASS on merged main.

**G5 -- visual work gate.** As in amendment 15:
- the decoded AFTER metrics meet their bars;
- every critic returns one verdict line citing decoded PNG paths;
- G5 FAILS on any critic MUST;
- then the artifact page for Boris.

**Coverage statement (not a gate).**
- The recorder and Syphon read `canvasFBO_` directly (`Renderer.cpp:1035-1038`, `:1045`). That is the same canvas
  texture every render_frame capture in G2.1-G2.5 and G2.8-G2.10 reads.
- Output windows read the output tap (`publishToOutputs`, `Renderer.cpp:1117-1122`), which G2.7 covers.
- No row encodes a file, because no REST route starts the recorder.
- A real projector and a real recording are Boris's W1.

### ERRATA to plan-bf10

- **ER1.** Plan I3 "All six are RED on main" and the T1 note: relabelled by amendment 1. T5's GL-error RED reason is
  now VERIFIED (0x506), no longer inferred.
- **ER2.** Plan I3 T6 "40 paced frames ... the last frames equal B" cannot hold with a 1.0 s soft cut (R-E12).
  Amendment 11 replaces it.
- **ER3.** Plan I4 m7 "what an Output, the recorder and Syphon read": m7 covers Output windows only. The recorder and
  Syphon read `canvasFBO_` (R-E9).
- **ER4.** Plan I2's stale-cache row said "re-configuring main's EXISTING build/" before the merge. A lane never
  re-configures main's build dir; amendment 7 makes it a scratch configure.
- **ER5.** diag-bf10's "lit %" column is a brightness count, not alpha coverage. Alpha-255 coverage equals the
  panel-box fraction exactly; plan E6 is right. The fingerprint uses alpha.
- **ER6.** Plan (1) "This lane neither serves nor conflicts with [BF9]": true for the code. But bf10 now carries m9, and
  hands bf9b H1-H4.

### HANDOFF to bf9b (relayed request: "decks are boxes of clips"; Harmony relays verbatim)

- **H1 -- acceptance row for bf9b's gate.** Add `m9b_deck_switch_live` to bf10's `.harmony/probe-milkdrop.py`.
  - **Setup:**
    - a composition of 20 decks;
    - deck 0 L0 = a MilkDrop clip, triggered, loaded with `bf10_solid`;
    - decks 1-19 hold untriggered clips;
    - then 8080 `load_source {"source_type":"plasma"}`, so the legacy fallback cannot impersonate a kept-playing layer
      (R-E13).
  - **Walk:** 7070 `switch_deck` 0 -> 1 -> ... -> 19 -> 0, at 0.5 s per deck, with one capture per deck.
  - **PASS iff** every capture is "uniform" and has a median within ±6 of the first capture's. Then repeat the walk with
    P1: every capture pair 0.5 s apart meets G2.2's bar.
  - **RED on pre-bf9b main:** the decks other than 0 show their own content or plasma.
- **H2 -- what a deck switch must never do.**
  - It must never call `ProjectMSource::loadPreset`, `resize` or `releaseGL`.
  - It must never change the canvas (composition-level, `Renderer.cpp:454-455`).
  - Today none does. The only callers are `MainComponent.cpp:1780`, `TestServer.cpp:1746`, `Renderer.cpp:655/669/680` and
    `ProjectMSource.cpp:30-32`.
- **H3 -- the playlist advance must follow the playing layers.**
  - The MilkDrop preset-playlist advance (`Renderer.cpp:594-683`) walks only the ACTIVE deck's layers, inside
    `if (deckActive)`.
  - With one shared playing stack, it must walk the playing layers, whatever deck the grid shows. Otherwise a playing
    MilkDrop clip's playlist freezes while another deck is browsed. That changes "how they are playing".
- **H4 -- one projectM for every MilkDrop clip.**
  - One `ProjectMSource` serves every MilkDrop clip (`Renderer.cpp:1314-1325`).
  - With layers that keep playing across decks, two layers fired from different decks can both play MilkDrop. They then
    share one preset and advance it twice per frame.
  - That is F1. bf9b should note it; no bf9b fix is required.

### FILED (not this lane)

- **F1 (plan R8 / E15b)** "One projectM per playing MilkDrop clip", with the G3.3 numbers attached. Suggested Boris
  question: "If two layers both play MilkDrop, should each have its own preset?" DEFAULT: yes, each its own.
- **F2 (plan E15a)** Stateful sources (Strange Attractor / Gravity Well / Fluid Dynamics) reset to black on a canvas
  change.
- **F3** ProjectMSource leaves texture bindings on units other than the caller's active one (R-E4: units 1-3).
  - INFERRED harmless: the render code binds units 0 and 1 before it samples them.
  - Revisit only on a symptom. T5 prints them as INFO.
- **F4 (plan F8b), only if Q2 = "not fine":** keep MilkDrop's picture through a canvas change.

### BORIS QUESTIONS (each has a default; the build never waits)

- **Q1** (asked ONLY if, at 4K, both the old and the new app fall below 58 fps):
  "On a 4K composition, MilkDrop may be too heavy to stay smooth on this Mac. Would you rather it draw a little softer and
  stay smooth, or keep full sharpness even if it stutters?"
  DEFAULT: keep full sharpness.
- **Q2** (Harmony fills <...> from the lane arm of m3's DATA line, and (...) from the pre-lane arm):
  "When you change the composition size while MilkDrop is playing, its picture goes dark and builds back up. A quarter of
  a second later it is about <L(0.25)/L0 %> as bright, and it is back to normal after about <first t with L/L0 ≥ 0.9>
  seconds. The app also pauses once, for about <peak_frame_ms> ms. The old app did the same (...). Size changes are a
  setup step, so we plan to leave this as it is. Is that fine?"
  DEFAULT: yes, fine. If no, F4 is filed.

What only Boris can check: plan section (8), W1-W5, unchanged, with two notes.
- W2: G0.6 proved that the engine's shaders are byte-identical.
- W3 carries Q2's numbers.

### RISKS (ruling-level)

- **RR1 -- the sampler restore (amendment 8) changes the composite on MilkDrop frames.**
  - After a MilkDrop frame, units 0 and 1 sample with the app's own texture parameters again, instead of projectM's
    CLAMP_TO_EDGE. That is the intended state, the same as on frames without MilkDrop.
  - INFERRED: no visible change, except at the texture edges of effects that wrap.
  - **Strongest counterargument: "scope creep in a bug-fix lane."** It loses because:
    - it is the same function and the same save/restore window the plan already rewrites;
    - the leak is VERIFIED;
    - the change is three lines;
    - T5 needs it to be a truthful "caller state survives" witness;
    - projectM's own rendering cannot change (VERIFIED: it is exactly the rig's condition);
    - a separate lane would re-touch the same code and re-run the same probes.
- **RR2 -- m9 alone does not prove "decks are boxes" for MilkDrop.**
  - m9 gates only the round trip, which holds under both semantics.
  - The user's actual request, for MilkDrop, is witnessed only once bf9b adds H1. If bf9b's plan omits H1, Harmony must
    add it before bf9b's gate.
- **RR3 -- G3 may merge with perf UNPROVEN but under the bars.** This is accepted:
  - the physics estimate is +0.1-0.4 ms at 4K (INFERRED);
  - the 1080 absolute bar still gates;
  - the closure records UNPROVEN, with the numbers.
- **RR4 -- G0.6 can fail for a config reason.** It fails if the rebuilt library used different CMake options than the
  stock build; the option names in I1 are ASSUMED. That drift is exactly what G0.6 exists to catch: match the stock
  options, never waive the gate.
- **RR5 -- T6 / T6b timing.** Both assume projectM times transitions on its own clock (INFERRED from the `std::chrono`
  imports). The wall-time specification holds either way.
- **RR6 -- the box fingerprint assumes the normal window layout.** A different layout changes the box size but not the
  fingerprint (the same box at every RED canvas), and amendment 2 recomputes the fractions from the measured box.
- **RR7 -- the plan's R1-R11 are unchanged.**
  - R1 now has a working fallback (`PROJECTM_SRC`).
  - R12 is closed (amendment 1).

### EVIDENCE INDEX

Scratch, rebuildable. Rigs live in
`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/ruling10/`.

- **`rig.cpp`** with `rig_1080.log` and `rig_360.log`.
  - Shows: R-E1, R-E2, R-E4.
  - Run: `rig "<presetA>" "<presetB>" W H`. Preset A loads hard, and B soft-cuts in at frame 60.
- **`samplers.cpp`** with `samplers.log`: R-E3, steady state.
- **`samplers_cut.cpp`** with `samplers_cut.log`: R-E3, mid soft cut.
- **Build each rig with:** `clang++ -std=c++17 -O1 -DGL_SILENCE_DEPRECATION <rig>.cpp -I$HOME/.local/include
  -L$HOME/.local/lib -lprojectM-4 -Wl,-rpath,$HOME/.local/lib -framework OpenGL`.
- **Presets:** `resources/projectm_presets/`.
- **Disassembly:** `otool -tV ~/.local/lib/libprojectM-4.4.1.1.dylib | c++filt`. The sites cited:

  | site | address |
  |---|---|
  | FinalComposite::Draw | 0xff34-0xfff4 |
  | Filters::Draw | 0xf554-0xf6d4 |
  | CopyTexture::Draw | 0x34040-0x34134 |
  | Framebuffer::MaskDrawBuffer (no callers) | 0x37894 |
  | glViewport | 0x11cd8, 0x8e20, 0x90f4 |

- **Identity (R-E8):**
  - `nm -gU ... | awk '{print $3}' | sort | shasum -a 256`;
  - `strings -n 8 ... | grep -E 'void main|#version|uniform ' | sort -u | shasum -a 256`.
- **Code reads:**
  - `src/sources/ProjectMSource.cpp`: 30-32, 71, 120-195, 278-322;
  - `src/sources/ProceduralSource.cpp`: 273-287;
  - `src/render/Renderer.cpp`: 454-455, 507-511, 594-683, 816-823, 1035-1045, 1117-1122, 1314-1325;
  - `src/test/TestServer.cpp`: 1709-1750;
  - `src/api/ApiServer.cpp`: 178, 205, 493-510, 747-759;
  - `src/MainComponent.cpp`: 1774-1790, 7097-7116;
  - `src/model/Composition.h`: 114.

STATUS: COMPLETE
