# Reviewer Verdict - outputs S1, lens fence-unchanged, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (no MUST)
PINNED: worktree outputs-a, lane/outputs-core, base 8b464a6, report head a364bad, code head 0f4c73c. Read through git objects only. Nothing built or run (INFERRED for every runtime claim; the mutant log is the builder's own, not re-run by me).

## FENCE (VERIFIED: git diff --name-status 8b464a6..a364bad)
src: A FrameHistory.h, A OutputLook.h, M SharedFrameSet.h, M SurfacePool.cpp (all four in Owns).
tests: A test_frame_history.cpp, A test_output_look.cpp, M test_surface_pool.cpp (append-only: 242 insertions, 0 deletions; the 5 old cases unedited), M tests/CMakeLists.txt (+17, new targets only).
Not touched: src/ui, src/analysis, FeatureSnapshot, src/recording, AppSettings.cpp, TopBar, root CMakeLists, any doc, OutputPresenter/SharedFrameSet.cpp/Renderer/OutputManager (VERIFIED by name-status).
Outside Owns: three files under .harmony/.reports/s-rta-1004b/ (outputs-S1.md, outputs-S1-mutants.py, outputs-S1-mutants.log) = the stage's report and its evidence runner; report path only.
No on-screen text added: the only text function is settingsStateText (ruled A-12), included by no product file (VERIFIED: grep of src at a364bad finds OutputLook.h/FrameHistory.h only in SharedFrameSet.h:22,62). No mutant, no instrumentation, no .venv link, no .new file in the tree (VERIFIED: ls-tree).

## APP UNCHANGED (VERIFIED by reading; no run)
- kSlots still 4: SharedFrameSet.h `SurfacePool::kSlots = output::kSlots` (=4, FrameHistory.h:23); test_shared_frame_gl.cpp:366 still asserts it.
- Only caller of ensure: SharedFrameSet.cpp:19 `const bool newGen = pool_.ensure(w, h);` -> slots defaults to 4; EnsureResult converts to bool (true iff Created), so file untouched and same value for Unchanged/Created. Failed reads false, as before.
- surface()/retainSurface() bound by the generation's own count = 4 for every generation any caller can create; generation number lastGen_+1 == old current_.gen+1 as long as releaseAll never runs (no caller yet).
- Nobody calls trimRetired / releaseAll(setCreateFn/allocBytes/...) except the destructor (now releaseAll, same effect + counters).
- One intended difference: a failed create is remembered 300 ticks (ruled, A-3 / SF-2). Disclosed by the builder (report "THE ONE INTENDED DIFFERENCE").
- Tests: pool cases are additions only; U-P1..U-P6 names equal the ruling's section 5 text; 10 + 4 + 6 = 20 new ctest entries (counted TEST_CASEs) = 1252 + 20 = 1272 as reported.

## FINDINGS
See the structured output. No MUST. Items: SHOULD probe-tsan-unit.sh gap; SHOULD U-P2 named arm equivalent; NITs (SharedFrameSet.h include+static_assert outside "pool part"; report/mutant files outside Owns; D-2 ordering deviation to be written into the docs; releaseAll leaves failure memos; unused deepSlots / reachMs(w,h) overload; 16 vs 15 entries wording).
