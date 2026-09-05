# S166-L0 — Stop copying, on the GL thread, a vector the message thread mutates.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-L0-remove-gl-copies.md

## YOU OWN THE BUILD SLOT
You are the ONLY builder running in this repo. `build/` is yours exclusively. Read-only
reviewers may be reading source; they never build. Earlier today three builders shared one
build directory and corrupted each other's results — hence the explicit statement.

## THE LANE IN ONE LINE
`CompositorEngine::applyClipEffects(const Clip&, ...)` is fed, at two sites, by a TEMPORARY
`Clip` constructed every frame on the GL thread purely to carry an effects vector — and
constructing it DEEP-COPIES a vector that the message thread mutates. Change the signature to
take the vector directly and both temporaries disappear.

## WHY THIS IS NOT CLEANUP — read before you judge the size
From this session's architecture pass (`.harmony/specs/s166-universal-connection-architecture.md`,
ADDENDUM §A3 — read it):
- Both sites copy, ON THE GL THREAD, a vector the MESSAGE thread mutates: `layer.layerEffects`
  and `composition_->globalEffects`.
- The effect BYPASS toggle (`EffectStackView.cpp`, around line 320 — RE-GREP BY ANCHOR TEXT) is
  UNFENCED. Only erase/add/undo go through the deck fence.
- Today a torn `float`/`bool` during that copy is harmless, which is why nothing has broken.
  **The moment `EffectSlot` carries a connection struct (a `std::string` and a vector — the next
  lane in this arc), a GL-thread copy racing a message-thread assignment is HEAP CORRUPTION.**
- So this lane is a PREREQUISITE for the connection engine, not a tidy-up, and it must land first.

## WHAT TO DO
1. Change `applyClipEffects` to take `const std::vector<Clip::EffectSlot>&` (plus its existing
   `layerId` and other args) instead of `const Clip&`.
   **VERIFY FIRST, do not take my word:** the architecture pass claims `applyClipEffects` reads
   ONLY `clip.effects` in its body and never reads `clip.id`. Confirm that yourself by finding
   every `clip.` use in the function body. **If it reads anything else from the Clip, STOP and
   report** — the signature change is then wrong and the lane needs redesign.
2. Update every call site. The architecture pass says "at least 6" and names approximate
   locations — treat that as a claim, not a total: grep `applyClipEffects(` yourself and fix all
   of them. Each becomes `x.effects` / `layer.layerEffects` / `globalEffects` as appropriate.
3. Delete BOTH temp-`Clip` constructions — the per-layer one and the one inside
   `applyGlobalEffects` (added earlier today by commit `694f8f3`).
4. Change nothing else. Do not "improve" the effect path while you are in it.

## WHAT DONE MEANS
1. Zero temporary `Clip` objects constructed per frame in the effects path. Prove it with two
   greps of DIFFERENT SHAPE.
2. Rendering is IDENTICAL — this is a pure refactor with no intended visual change.
3. No new copy of the effects vector anywhere on the GL thread. If your change still copies,
   the lane has not achieved its purpose; say so rather than reporting DONE.

## FENCE — you may edit ONLY these
- `src/render/CompositorEngine.cpp` / `.h`
- `src/render/Renderer.cpp` (call sites only)
- a test file under `tests/` and its `tests/CMakeLists.txt` entry
Anything else: STOP and report. Do NOT touch `EffectStackView.cpp` — the unfenced bypass toggle
is a REAL finding but it belongs to the fencing lane, not this one. Report it, leave it.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and READ it before quoting any test number.**
- ctest baseline is **233/233** at HEAD `e49a6ad`, produced on a clean serialized build with no
  concurrent builders. Re-run it; never inherit it. Build dir `build`, Release.
- Note honestly: no ctest target links `CompositorEngine.cpp` or `Renderer.cpp` (headless GL is
  not available in this rig), so ctest CANNOT prove this lane. Do not imply that it does. A green
  suite here means "nothing else broke", not "the refactor is correct". I will run the app.
- Do NOT launch the app.

## REPORTING
STATUS (DONE / DONE_WITH_CONCERNS / BLOCKED), the `clip.` audit result from step 1, the full list
of call sites you found and changed, build exit code, before/after ctest, PACKET QUALITY (what did
this packet get wrong?), and anything outside the fence — especially anything you notice about the
unfenced bypass toggle.
