# S166-L8 — Make the composition tier gateable. It is currently unverifiable from outside.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-L8-comp-tier-oracle.md

## YOU OWN THE BUILD SLOT
You are the only builder in this repo. `build/` is yours. Reviewers read source, never build.

## WHY THIS LANE EXISTS — a real, twice-observed hole, not a nice-to-have
Twice today a change to the composition tier shipped source-reviewed but **behaviourally
ungated, because no surface in this app can reach it**:
- `694f8f3` made `Composition::globalEffects` composite for the first time. Nothing outside the
  app can add a global effect, so nobody could confirm it does anything.
- The four render-dead fields (`Composition::masterOpacity`, `masterSpeed`, `compOpacity`,
  `Clip::clipOpacity`) cannot be driven from outside either, so the fix for them will land
  equally unverifiable.
And no ctest target links `CompositorEngine.cpp` or `Renderer.cpp` (headless GL is unavailable
in this rig), so there is no unit path either. The result is a whole tier of the app that can
only be checked by a human looking at a screen.
**This lane gives that tier an oracle.** After it, `render_frame` at two states is a real
headless check for composition-tier rendering — for the global-effects lane already shipped, for
the four dead fields, and for the connection engine the next arc builds.

## WHERE TO BUILD IT — the TEST server only
`src/test/TestServer.{h,cpp}` — the `--test-mode` server on port 8080. It already owns exactly
this kind of surface (see its existing `/api/signals` handler, which reports every signal's live
cached value). **Do NOT touch `src/api/ApiServer.cpp`** (the production server on 7070) — keeping
this in the test server means zero production risk, which is why the lane is safe to land now.

## WHAT TO BUILD
Endpoints that let a caller drive the composition tier and observe it:
1. **Global effects:** add an effect to `composition_->globalEffects`, bypass/unbypass one,
   remove one, and list what is there. Address effects the way the existing effect endpoints do —
   find how `ApiServer`'s `set_param`/`set_effect` name an effect and a parameter, and match that
   convention rather than inventing a new one.
2. **The composition scalars:** set `masterOpacity`, `masterSpeed`, `compOpacity`, and a clip's
   `clipOpacity`. **These four currently have NO renderer consumer** — setting them will change
   nothing on screen today, and that is FINE and expected: the point is that the endpoint exists
   so the lane that wires them can be proven. Say so plainly in your report; do not "fix" them
   here, that is a different lane.
3. **Readback:** a way to GET the current values of everything you can set above, so a test can
   confirm a set took effect at the model layer even when the renderer ignores it.

## WHAT DONE MEANS
1. Each endpoint round-trips: set, then read back the same value.
2. Adding a global effect and then rendering a frame produces a DIFFERENT image than rendering
   with an empty global stack — **you cannot verify this yourself (do not launch the app); just
   make sure the endpoints make it POSSIBLE.** I will run it.
3. Bad input (unknown effect name, out-of-range index, missing field) returns a clean error, not
   a crash and not a silent no-op. A silent no-op is the worst outcome — it would make the oracle
   lie, which is worse than having no oracle.
4. Thread safety: these handlers run on the HTTP server's thread, not the message thread and not
   the GL thread. **Look at how the existing handlers in this file deal with that** before you
   mutate model state from one. Adding or removing an entry in a vector the GL thread iterates is
   exactly the hazard this repo has spent two sessions closing — if the existing endpoints use a
   fence or a queued message, use the same mechanism. If they do NOT, say so loudly in your
   report rather than copying an unsafe pattern; that is a finding.

## FENCE — you may edit ONLY these
- `src/test/TestServer.cpp` / `.h`
- a test file under `tests/` and its `tests/CMakeLists.txt` entry
Anything else — especially `src/api/ApiServer.cpp`, the model, or anything under `src/render/` —
STOP and report.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.** Redirect to a file;
  do not pipe through `tail`/`tee` and read `$?` (zsh has no PIPESTATUS — two builders hit this
  exact trap today).
- ctest baseline is **241/241** as of HEAD `a5c9782`. Re-run; never inherit.
- Do NOT launch the app.

## REPORTING
STATUS, the endpoints you added with their exact request/response shapes (I need these to write
the gate — be precise, this section is the deliverable I will actually use), the thread-safety
finding from step 4, build exit code, before/after ctest, PACKET QUALITY, and anything outside
the fence.
