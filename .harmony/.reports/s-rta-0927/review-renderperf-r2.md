# Reviewer Verdict — renderperf-r2 (fix round)
STATUS: DONE
VERDICT: APPROVE
FILES: src/render/Renderer.h, src/render/Renderer.cpp (fix commit 7500718); .harmony/.reports/s-rta-0927/renderperf-evidence/fix/** and
       .harmony/notebook.md (report commit b7ce184). No files outside the plan's already-approved Renderer.h/.cpp
       captureFrame/processPendingCapture fence were touched.
ISSUES: none blocking.
  - [SHOULD] No lasting regression test guards this race: Renderer.cpp is not linked into any ctest target, and the
    capture_race.py repro needs a temporary, never-committed env-var hook to force the window (by design it FAILs on
    a committed build). Consider filing debt to either (a) add a small always-on capture_race stress probe (no hook,
    just N overlapping real captures + byte checks, accepting it is probabilistic) to .harmony/, or (b) get
    Renderer.cpp's capture path under a linked test target so a future refactor of this code can be caught
    mechanically instead of only by code review.
  - [NIT] `hook-strings-count.txt`/`renderer-sha256.txt` in the evidence tree are useful provenance but slightly
    redundant with the sha256 already shown for redhook/greenhook in the race logs; not worth trimming.
METADATA: reviewer=reviewer-agent, builder_packet=renderperf-fix, date=2026-09-27

## Verification performed
- Diff scope: `git diff e617312..b7ce184 --stat` — only Renderer.h/.cpp (46/19 lines) + evidence/report/notebook
  files changed. No probe threshold, test, or unrelated source file touched. Matches the plan's already-approved
  file fence (plan-renderperf.md DECISION/SPEC: "Renderer.h/.cpp (members :617-622, captureFrame :2031-2071,
  processPendingCapture :2073-2149 ONLY)").
- Read the full diff of both files (git diff on the two hunks) and the surrounding unchanged code
  (sed -n on captureFrame/processPendingCapture in the worktree via `git -C <worktree>` + direct file read of the
  worktree source, not the main checkout).
- Confirmed the single MUST from review-renderperf-r1.md (shared `capturePixels_`/`captureReadW_/H_`/`Ms_` members
  could hand caller A a second, overlapping capture's pixels) is the exact defect this commit removes: the four
  shared members are deleted; `std::promise<bool>` becomes `std::promise<CaptureRead>`; `processPendingCapture` fills
  a LOCAL `CaptureRead` and moves it into `set_value`; `captureFrame` retrieves it by value via its OWN
  `future.get()` with no post-signal read of any Renderer member. Traced the exact interleaving the old MUST
  described (A's future unblocks -> before A re-acquires captureMutex_, B arms + is serviced, overwriting the shared
  members -> A reads B's data) and confirmed the new code has no step where a caller reads anything other than its
  own future's value — the bug class is structurally impossible now, not merely narrowed.
- Confirmed the temporary repro hook (`hook.py`) was never committed: `grep -n
  AUDIODNA_TEST_CAPTURE_COLLECT_DELAY_MS|capturePixels_|captureReadW_ src/render/Renderer.*` in the worktree = no
  hits, and the report's own `hook-strings-count.txt` (fix: 0) matches.
- Spot-checked evidence for the specific claims in the builder report: `ctest-fix.log` tail shows
  "100% tests passed, 0 tests failed out of 736" (matches the report's number, +4 over the r1 base of 732/736 that
  the pre-fix plan work had already reached — no test count regression). `sha256-FIX-vs-C3.txt` shows every FIX
  capture's hash equal to the cited C3 hash at 720p, 1080p (x3), and 4K. `window-sampler.txt` in every batch
  (`race`, `fix1`, `fix2`, `tier1`) shows `Output-named Audio-DNA windows []` and max 1 on-screen layer-0 window —
  matches the screen-safety claim.
- Confirmed the disclosed residual race (`found_not_fixed #5b`: a second `captureFrame` arming while the first is
  still pending overwrites the `capturePromise_` pointer, so the first times out safely after 5s) is unchanged by
  this diff (the arm block at the top of `captureFrame` is untouched except for the promise's template type) and is
  correctly described as pre-existing/out of scope, not newly introduced or newly hidden by this fix.
- Confirmed no threshold was loosened: this commit range touches no probe `.json`/`.py` file at all (diff --stat
  has zero hits under `.harmony/probe-*`).
- Sacred rules: the capture path is not the audio callback / analysis steady-state / 60fps render loop — it is a
  once-per-request test/API path, and the plan's C3 already accepted one vector allocation per capture on the GL
  thread; this fix keeps that same cost (a local `CaptureRead::pixels` vector per capture, moved not copied) and
  adds no new mutex or lock. No violation.
- Naming/readability: `CaptureRead` is a clear, minimal struct; comments at both call sites correctly describe the
  by-value handoff and its purpose (matches the actual code, no overclaiming).
- SLIM: no dead/vestigial/duplicate code introduced. The diff is a pure narrowing (four scattered members -> one
  struct passed by value); this is a reduction in surface area, not an addition.

## Conclusion
The fix is minimal (~40 lines across 2 files), correctly targeted at the single MUST raised in round 1, verified by
direct code reading (the race is structurally eliminated, not narrowed) and cross-checked against the report's own
live evidence (byte-identity hashes, ctest count, window-sampler screen safety, hook-removal proof). No plan/fence
violation, no threshold loosened, no stray files. APPROVE.
