# Reviewer Verdict — s-rta-0928b video rebase (lane/video)
STATUS: DONE
VERDICT: APPROVE

## Scope
Independent review of lane/video's merge onto main (old head c1120bf on 328301d;
new head 99c54f5 on main a56f835 — actually a `git merge main` per the builder's
own report, not a linear rebase, after a rebase attempt hit repeated conflicts
across 5/12 commits; the fallback is disclosed in the rebase-report commit).

## Check 1 — range-diff (lane content preserved)
`git range-diff 328301d..c1120bf a56f835..99c54f5`: all 12 pre-existing lane
commits show `=` (content byte-identical, no conflict-resolution diff noise),
plus one new commit 13: `99c54f5` (the rebase report, additive only). No lane
commit's content changed. VERIFIED.

## Check 2 — `git diff a56f835..99c54f5 -- docs CLAUDE.md tests/CMakeLists.txt src/api src/test`
5 lines changed (not net-removed) in this path set:
- `CLAUDE.md` rule 5 (Shader Rules): shortened, with the removed detail
  ("Shaders are hot-reloadable from disk" / the exact NOTE wording) relocated
  and preserved in `docs/claude/effects.md` (which gained a new sentence in
  the same diff: "Shaders compiled from files are hot-reloadable from disk...;
  the embedded set is not."). Content moved, not lost — confirmed by reading
  both sides.
- `docs/claude/architecture.md` tree comment for `VideoPlayer.h/cpp`: extended
  (adds decode-thread/VideoRing detail), not shortened.
- `docs/claude/effects.md`: sentence added, not removed.
- `docs/claude/performance-controls.md` "Inactive decks keep time" bullet:
  the B2 clause was rewritten to reflect the new threaded video model ("the
  player's decode thread idles off screen and, on return, re-seeks and
  catches up while the layer holds its last frame") in place of the old
  single-threaded wording ("the next on-screen frame seeks and decodes,
  bounded per call"). This is a content update matching the new architecture,
  not a dropped fact — cross-checked against Pitfall 56 / rendering.md
  "Video playback", both consistent.
- `docs/claude/testing-eyes.md` `/api/render_frame` row: extended with
  video-specific HOLD/FAILED semantics, not shortened.

No line in this path set was net-removed. VERIFIED (read full diff, both
sides of each changed line).

Everything main (a56f835) added is present and untouched in 99c54f5:
- mediaopen block: `Pitfall 55` text intact verbatim in both `CLAUDE.md`
  index and `docs/claude/pitfalls.md`; `fence_hold_frames` / `fence_black_frames`
  / `media` (presence_sweeps/presence_changed) fields present in both
  `ApiServer.cpp` and `TestServer.cpp`.
- seqvram block: `Pitfall 54` text intact verbatim; `seq_late_frames` /
  `seq_textures` / `seq_evictions` etc. all present in both servers.
- `MessageHeartbeat` TEST-ONLY routes: `POST /api/debug/heartbeat` registered
  in `ApiServer.cpp:290`, `message_heartbeat_on` / `peak_message_stall_ms`
  present under `#if AUDIODNA_TEST_SERVER` guard, unchanged.
All VERIFIED by direct grep/read of current worktree files, not the diff summary.

## Check 3 — CLAUDE.md size and pitfall numbering
`wc -c CLAUDE.md` = 24,725 bytes ≤ 25,000. VERIFIED.
The video pitfall is consistently resolved to **56** everywhere: CLAUDE.md
index line 233, `docs/claude/pitfalls.md` line 122, `docs/claude/rendering.md`
"Video playback" paragraph's internal cross-reference ("Pitfall 56 / 53"), and
`docs/claude/rendering.md` ordering (55 "Media opens and the fence" precedes
56 "Video playback"). No stray literal "NN" remains in either doc. VERIFIED
via grep across CLAUDE.md, pitfalls.md, rendering.md.

## Check 4 — builder's ctest/probe claims
- `tests/CMakeLists.txt` registers `test_video_ring` and `test_video_player_open`
  as new executables (both blocks present, correctly closed, no interleaving
  with the mediaopen blocks placed immediately above them — read lines
  2690-2782, confirmed no leftover `<<<<<<<`/`=======`/`>>>>>>>` markers
  anywhere in tracked source/doc/CMake files). `grep -c '^add_executable(test_'`:
  91 in the merged tree vs 89 in old main (a56f835) — a net +2, matching the
  claimed "17 new cases" as new *executables* being test_video_ring (many
  Catch2 cases) + test_video_player_open (a few forked-child cases); consistent
  with 897 = 880 (main) + 17 (lane). Not independently re-run (read-only
  review; ctest execution not attempted here) — INFERRED plausible from static
  registration, not independently reproduced.
- The rebase-report commit 99c54f5 (`.harmony/.reports/s-rta-0928b/video.md`,
  appended "## Rebase" section) states the same ctest 897/897 and probe rows
  (video RED 54/3 pre-existing, media-open GREEN 38/0, seq-vram GREEN 66/0)
  given in the task packet verbatim — the on-disk report and the packet's
  builder-supplied summary match. VERIFIED (byte-level match on the key
  numbers: 897/897, 19.82s, 54 PASS/3 FAIL, 38/0, 66/0).
- `.harmony/probe-video.sh`, `probe-media-open.sh`, `probe-seq-vram.sh` all
  exist and are executable in the worktree. VERIFIED present.
- The 3 named probe-video fails (w1 bimodal fps, w2 (a2) floor, w7 one RTT
  overshoot) are each cited as pre-existing/documented in the lane's own
  fix-round-2 report (c1120bf), not new regressions — plausible given the
  merge touched zero runtime logic beyond additive `/api/state` fields and
  doc/CMake resolution (confirmed no runtime `.cpp` beyond `ApiServer.cpp`'s
  additive `/api/state` block appears in the a56f835..99c54f5 diff for the
  reviewed path set). Not re-run live by this reviewer.

## Additional spot-check (beyond the mandated path set, sanity only)
Read `src/render/Renderer.{h,cpp}` and `src/render/CompositorEngine.{h,cpp}`:
both lanes' additions (mediaopen's `FencedPtrSlot`/`detachActiveDeckFenced`
and video's `installVideoPlayer`/`getVideoStats`) coexist and interoperate
correctly (e.g. `CompositorEngine.cpp:399-406` checks both `mediaMissing` and
a video player's never-shown state in the same pending-texture branch) — no
sign either lane's logic was silently dropped or shadowed by the merge.

## Findings
None blocking. No lost line, no hidden behavior change in the conflict
resolution, no missing gate.

SUMMARY: 1 rebase/merge reviewed (12 lane commits + 1 report commit), 7 files
in the mandated path set diffed line-by-line, 0 blocking issues, 0 suggestions.
Confidence: VERIFIED for range-diff/diff/size/marker checks (executed directly
against the worktree); INFERRED for ctest pass-count and probe-script live
behavior (read static registration + the builder's own report; not re-executed
by this reviewer).

METADATA: reviewer=reviewer-video-rebase, builder_packet=s-rta-0928b, date=2026-09-29
