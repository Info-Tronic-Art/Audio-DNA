# Reviewer Verdict -- bf10 lens gl, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (APPROVE, no MUST)
PINNED: head e3c73a229b27212fddacf395cbad21a8879b53ce. NOTE: the pinned "base 30364a8" is the S2fin head, not the lane's
merge-base (e89bb5f). All production code (src/sources, CMake, patch) changed before 30364a8, so I reviewed
e89bb5f..e3c73a2 for code and 30364a8..e3c73a2 for the S3 delta. Read via git show / git diff only.

## GL state (ProjectMSource.cpp at head) -- all VERIFIED by reading + executing what is cheap
- projectM draws only into outputFBO_: bind (:176) + glViewport + projectm_opengl_render_frame_fbo(pm_, outputFBO_) (:178).
  Zero reads/writes of framebuffer 0 and the blit is gone. Patched RenderFrame (checked against the archived 03aa8a7 source,
  `git apply --check` OK in a $TMPDIR extract) has exactly ONE DRAW bind, now the target; early returns precede it.
- saveGLState is the first statement of render() (:126-127); restoreGLState runs on the `!pm_` return (:136), the normal
  tail (:197) and the no-projectM #else path (falls to :197). Init / resize / pending preset load are all inside the saved state.
- Alpha-1 clear (:181-187): glGetBooleani_v mask of draw buffer 0, glColorMaski(0,F,F,F,T), glClearBufferfv, mask restored
  to the value read -- restored before restoreGLState, no early exit between. Clear colour untouched. Mask is read AFTER
  projectM's draw (ruling R-E4: libprojectM never changes masks), so no leak path.
- Sampler loop (:329-330): 16 units < GL_MAX_COMBINED limits, no other glBindSampler/glGenSamplers in src (git grep), so
  "caller state = sampler 0" is true; runs on every return because every return calls restoreGLState.
- Threads: no GL call off the GL thread (loadPreset/feedAudio only queue under the existing mutexes). No new mutex, no new
  allocation (GLboolean[4]/GLfloat[4] on stack). No Renderer.cpp change.
- Canvas resize / preset crossfade: resize() recreates outputFBO_ inside the saved state; projectM re-sizes lazily; the
  final full-screen copy covers the FBO, then alpha is forced. releaseGL path unchanged (m8 cycle row covers it).
- Patch: 73 lines, 3 hunks + header decl, identical in shape to plan I1; `types.h` already includes <stdint.h> (plan's
  "add stdint" hedge correctly not needed). Installed prefix dylib: nm differs by exactly _projectm_opengl_render_frame_fbo,
  shader-string hash bffe0439eff25046 equal to stock, VCS SHA embedded, install name @rpath/libprojectM-4.4.dylib (all re-run).
- CMake lookup, EXECUTED in a scratch mini-project built from the head's FindProjectM.cmake + CMakeLists block:
  default -> "libprojectM: .../projectm-4.1.1-fbo1/... (render_frame_fbo: yes)" rc 0;
  prefix=/nonexistent + stock projectM4_DIR -> CMake Error naming build-projectm.sh, rc 1;
  re-configure of that dir with the real prefix -> stale stock cache dropped, picks fbo1, rc 0.

## Plan / amendments
I1-I5 present; amendments 8 (sampler), 10 (fix (i) alpha clear, only the symptom-keyed one), 11, 12, 13 (STOP 1 {0x500} on init
only), docs list, THIRD_PARTY_LICENSES, CLAUDE.md index line 66 (payment by the plan's own fallback because mkvidx spent the
first choice -- report says so). CLAUDE.md 23,963 B at head; main is 23,975 B so a merge lands ~23,962 B (<= 25,000).
Tests: test_projectm_canvas_gl links the REAL ProjectMSource + patched lib; report quotes RED 7/7 on the e89bb5f
ProjectMSource.cpp (red-ctest.log "7 tests failed out of 7"). T5 captures masks 0-3, 16 sampler units, FBO/viewport/blend/etc.
Stray scan: no symlinks in the tree (git ls-tree 120000 empty), no getenv/env hook added under src (existing ADNA_* hooks
are older), no mutant or instrumentation in src; the only env knobs added are probe-only (MILKDROP_MODE/ARM/P1/PY).
Merge preview (`git merge-tree main e3c73a2`): textual conflicts only in CLAUDE.md and docs/claude/pitfalls.md (adjacent
65 vs 66 appends); APP-INVENTORY, testing-eyes, tests/CMakeLists auto-merge.

## NITs (none blocks)
1. docs/claude/rendering.md (new paragraph) says "alpha forced to 1 for presets without a comp shader"; the code forces
   it unconditionally on every frame (an extra masked full-canvas clear, also for comp-shader presets). Say "always".
2. CMakeLists.txt FATAL_ERROR / STATUS print ${projectM4_DIR}, which is empty when ProjectM was found by the pkg-config or
   manual-search fallback in FindProjectM.cmake. Cosmetic.
3. T5 enters with all-TRUE colour masks and scissor off, so a restore that hard-codes TRUE, or a caller with scissor on
   (glClearBufferfv honours scissor), is not exercised. Harmless for the app today (no scissor in the source call path;
   VideoPlayer restores its own); one variant with mask (T,F,T,T) would give the mask restore real teeth.
4. T6's cross-fade branch is satisfied by absence on pre-change code (lane NUANCE 2, already ruled STOP 2/3). Informational.
5. Harmony: resolve the CLAUDE.md / pitfalls.md conflicts keeping 64, 65 (ui), 66 (this lane); G3 perf stays UNPROVEN until
   the quiet-window re-run (lane hand-off), G4/G0.1/G5 are post-merge.

CONFIDENCE: VERIFIED for the GL state, patch, prefix and CMake behaviour (read + executed). INFERRED for live G2/G3 numbers
(not re-run; lane logs only).
