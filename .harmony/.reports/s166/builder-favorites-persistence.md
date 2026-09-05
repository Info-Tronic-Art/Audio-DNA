# S166-FAV — MilkDrop favorites/user-preset persistence — Builder report

## ADDENDUM (post-report, before team-lead's stop-and-hold message arrived)
**STOP received AFTER I had already built, tested, and committed.** Team-lead's stop-and-hold
message (shared `build/` dir + shared-file collision in `MainComponent.cpp` with the
`kClipReplaceContent`/S166-LEAK lane) arrived after I had already completed build+ctest+commit
below. Per team-lead's explicit ask, reporting this plainly rather than treating my own
build/ctest numbers as trustworthy:

**Commit `ab9b115` accidentally swept in an unrelated, uncommitted hunk from the S166-LEAK
lane.** `git show ab9b115 -- src/MainComponent.cpp` shows 4 hunks, not my 3: the 4th
(`handleMenuCommand`, the Image-branch `closeMediaForClip(existing->id)` call + its comment
block, S166-LEAK) is NOT mine and was never part of my task. My own `git diff -- src/MainComponent.cpp`
review, done immediately before `git add`, showed only my 3 hunks — the 4th hunk must have
landed in the shared working tree in the short window between that review and `git add`/
`git commit` (a live concurrent-edit race, exactly the collision team-lead described). `git add
<file>` stages the file's entire current content, not just my own lines, so it went in
unreviewed by me.

Current state (read-only, no action taken):
- `git status --short -- src/MainComponent.cpp` is now clean (no uncommitted changes) — the
  swept-in hunk is fully captured inside `ab9b115`, indistinguishable from my own work in that
  file's history.
- `src/render/Renderer.cpp`, `src/render/CompositorEngine.{cpp,h}` are STILL modified,
  uncommitted, in the working tree right now (confirmed via `git status --short`) — so the
  S166-LEAK lane's change is now SPLIT: its `MainComponent.cpp` call site is committed under my
  message, while its `Renderer.cpp`/`CompositorEngine.{cpp,h}` counterparts are not yet
  committed by anyone.
- `closeMediaForClip()` itself (the function the swept-in call invokes) was already committed
  pre-existing code (from `5d1e791`), so the swept-in call site did compile and did not cause any
  of my build/ctest failures — but I never reviewed its correctness as part of my task, and it
  is not mine to vouch for.
- I have taken NO git corrective action (no reset/revert/cherry-pick) — holding for team-lead's
  decision on how to un-entangle the commit, per instruction.
- My OWN 3 hunks in `src/MainComponent.cpp` are otherwise exactly as designed and self-checked
  (see FILES CHANGED below); the entanglement is additive contamination from the other lane, not
  a defect in my own edit.
- Also already done, all BEFORE the stop arrived: build ran 4 times (initial pass, mutation
  rebuild, restore rebuild, plus one incremental target-only rebuild), ctest ran 4 times (initial
  226/226 after fixing my own test-isolation bug, mutation run 3/4, restore run 226/226 again).
  Full detail already below — flagging here only that these numbers were produced against a
  shared `build/` tree team-lead says other lanes were also building against concurrently, so
  per team-lead's own framing they should not be trusted as clean-room numbers even though my
  own test target's logic is sound (proven via the mutation-restore proof, which is a
  same-process, same-build-directory-instance comparison and not dependent on any other lane's
  concurrent build).

Not making any further build/test/commit calls per the stop instruction. Awaiting the exclusive
build slot.

### STATUS
DONE_WITH_CONCERNS (as designed/self-checked) — but see ADDENDUM above: BLOCKED on team-lead's
resolution of the commit entanglement before this can be treated as closed.

### RESULT
`loadUserData()`/`saveUserData()` are now wired up in `MainComponent.cpp` — load at ctor
(message thread, pre-GL-attach, right after `rescan()`/`loadManifest()`), save inside the
existing `onToggleFavoriteRequested` callback, after the (blocking) `toggleFavoritePreset()`
call returns. Build exit code 0, ctest 226/226 (222 baseline + 4 new). Core round-trip test
proven load-bearing via mutation (fails 0==1 with `loadUserData()` neutralized, restored
byte-identical, md5-verified, passes again). The one concern: I cannot behaviorally prove
MainComponent actually invokes these at the right times without launching the app, which this
packet forbids — that gap is verified by code-reading citation instead (see CONFIDENCE+VERIFY).

### FACTS
- `src/sources/ProjectMPresetManager.h:39-40`, `.cpp:85-154` — `saveUserData()`/`loadUserData()`,
  unmodified (confirmed byte-identical post-task, `git diff` empty).
- `src/MainComponent.cpp:50-60` — new `projectMUserDataFile()` helper (anonymous namespace).
- `src/MainComponent.cpp:1622-1633` — new `loadUserData()` call, after `rescan()`/`loadManifest()`.
- `src/MainComponent.cpp:1655-1669` — new `saveUserData()` call, inside `onToggleFavoriteRequested`.
- `tests/test_projectm_preset_manager.cpp` (new, 4 TEST_CASEs) + `tests/CMakeLists.txt:398-428`
  (new target, headless, juce_core only).
- Build: `cmake --build build -j8` exit code 0 (captured directly, not through a pipe — see
  PACKET QUALITY for why that mattered).
- ctest: 226/226 passed, 0 failed (`build/` dir, Release). Baseline was 222; +4 new tests.
- Mutation proof: `loadUserData()` neutralized (`return;` inserted) → rebuilt
  `test_projectm_preset_manager` only → `ctest -R "S166-FAV"` → 3/4 passed, 1 failed
  (`REQUIRE( favorites.size() == 1 )` → `0 == 1`, the round-trip test). Restored via Edit,
  verified `md5 src/sources/ProjectMPresetManager.cpp` == `392c4900d324da4d34a1b10c1f7e434c`
  (pre-mutation snapshot) → full rebuild → ctest 226/226 again.
- `.harmony/notebook.md` — new entry appended documenting the wiring decision and two findings
  (see ISSUES).

### METHOD
Read the two target files + `.harmony/gotchas.md` + the full `5d1e791` commit (the confinement
mechanism this packet pointed at) before writing anything. Grepped `saveUserData`/`loadUserData`
two ways (`grep -rn` and `grep -rwn`) — confirmed zero callers, matching the packet's claim.
Traced `presetManager_`'s lifecycle in `MainComponent.cpp` to find where `rescan()`/
`loadManifest()` run (ctor, pre-GL-attach) and where the confined favorite-toggle callback lives,
then designed load/save placement around those two existing safe points rather than inventing a
third. Checked JUCE source directly for two load-bearing facts rather than assuming: `JSON::parse
(const String&)` catches its own parser exception and returns an empty `var` (never throws) —
`build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:541-549`; `File::replaceWithText` uses
`TemporaryFile`, which stages in the TARGET's parent directory —
`.../juce_core/files/juce_File.cpp:798-803` + `juce_TemporaryFile.cpp:72-80` — which is why the
save call site creates that directory first.

### CONFIDENCE+VERIFY
High confidence the code is correct and matches the confinement invariant; explicit low
confidence on one specific claim, stated plainly rather than asserted: **I have not behaviorally
proven MainComponent calls these methods at the times I describe.** Verify by: (1) reading
`src/MainComponent.cpp:1622` and `:1662` directly — the calls are there, in the described
locations, in the ctor and the toggle callback respectively; (2) an app-level gate (favorite a
preset, quit, relaunch, confirm it's still favorited) — NOT run here, this packet's rules
explicitly forbid launching the app in this build+ctest-only session. The 4 new unit tests
exercise `ProjectMPresetManager` directly and do NOT link `MainComponent.cpp` at all (see
`tests/CMakeLists.txt:405-407` — the target's only sources are the test file and
`ProjectMPresetManager.cpp`), so they cannot and do not prove the wiring is live; they prove the
persistence class's own contract is correct and regression-guarded (proven via the mutation test
above). This is the actual reason for DONE_WITH_CONCERNS rather than DONE.

### UNKNOWNS-NOT-DONE
- Whether the app, launched for real, actually restores/persists a favorite across a quit —
  unverified here (forbidden by this packet's own build+ctest-only rule); Harmony's behavioral
  gate is the right place to close this, not a re-run of this builder.
- Whether `userPreset` will ever get a real setter/UI — out of scope; it currently has none
  anywhere in the codebase (confirmed by grep), so its persistence half is untested beyond a
  JSON-schema round-trip (see ISSUES).

### NUANCE
`saveUserData()` runs synchronously on the message thread inside the right-click handler (a
small local JSON write, not async). This matches this file's own existing precedent
(`setMilkDropPresetDir()` → `saveMilkDropPresetDirSetting()`, same synchronous-write-on-change
shape) rather than introducing new async I/O infrastructure that doesn't exist anywhere else in
this codebase. It does NOT touch the GL/render thread, which is what I read the packet's "does
not stall ... the render thread" criterion to be centrally about, given the packet's own
extensive framing around GL-thread confinement.

### HANDOFF-NEEDS
None for a Builder retry. For Harmony: this is a real, load-bearing-test-shaped coverage gap
that can only be closed with an app-level behavioral gate (not a re-dispatch of this lane) —
recommend including "favorite a preset, quit, relaunch, confirm favorited" in whatever gate
follows.

---

## SUMMARY
`ProjectMPresetManager::saveUserData()`/`loadUserData()` were fully implemented but never
called — confirmed via two differently-shaped greps before touching anything. Wired both in
`MainComponent.cpp`: `loadUserData()` at startup (ctor, message thread, pre-GL-attach — same
safe zone `rescan()`/`loadManifest()` already use), `saveUserData()` on every favorite toggle
(inside the existing `onToggleFavoriteRequested` callback, after the confined, blocking
`toggleFavoritePreset()` call returns, so `presets_` is stable and the save is a pure read on the
message thread). No new thread touches `presets_`/`.favorite` — the GL-thread confinement
mechanism from `5d1e791` is unchanged and untouched (`Renderer.{h,cpp}` were not edited). Added a
new headless test file (`ProjectMPresetManager` only, no GUI/GL) with 4 test cases covering the
round-trip, missing-file, malformed-file, and on-disk-schema cases from WHAT DONE MEANS, and
proved the core round-trip test load-bearing via a scratch mutation (byte-identical restore,
md5-verified).

## FILES CHANGED
- `src/MainComponent.cpp` — added `projectMUserDataFile()` helper (anon namespace, no header
  change); added `loadUserData()` call in the ctor's MilkDrop-wiring block; added
  `saveUserData()` call (+ parent-dir creation) inside `onToggleFavoriteRequested`.
- `tests/test_projectm_preset_manager.cpp` (NEW) — 4 Catch2 test cases against
  `ProjectMPresetManager` directly (real file I/O, real scanned `.milk` files, no mocks).
- `tests/CMakeLists.txt` — new `test_projectm_preset_manager` target (juce_core only, mirrors
  `test_thumbnail_cache`'s minimal shape). Explicitly commented as distinct from the pre-existing
  `test_preset_manager` target, which covers an unrelated class (`ui/PresetManager`, effect-chain
  deck presets) — same name pattern, different system; flagged so no one confuses the two.
- `.harmony/notebook.md` — appended an entry closing the "if either gets wired up, re-check..."
  expiry clause left by the `5d1e791` session, with the two findings below.
- `src/sources/ProjectMPresetManager.{h,cpp}` — READ, not changed. `git diff` on `.cpp` is empty
  (confirms the mutation-test restore was byte-identical, and confirms no edits were needed —
  the existing implementation already satisfies every WHAT DONE MEANS criterion; see METHOD).

## TESTS
`tests/test_projectm_preset_manager.cpp`, 4 TEST_CASEs (one with 2 SECTIONs):
1. **Round trip** — favorite "Beta" in one manager instance, save; construct a SECOND instance
   (simulates a restart), rescan the same dir, load, confirm "Beta" (and only Beta) is favorited.
2. **Missing file** — `loadUserData()` against a nonexistent path is a no-op: preset count and
   favorites unaffected, no crash.
3. **Malformed file** — two SECTIONs: truncated JSON, and syntactically-valid-but-wrong-shaped
   JSON (`favorites` as a string, not an array). Both: `REQUIRE_NOTHROW`, favorites stay empty,
   preset count unaffected (nothing wiped).
4. **Schema** — `saveUserData()`'s on-disk JSON has a `favorites` array with the favorited
   preset's name and an empty `userPresets` array (nothing sets `userPreset` anywhere in the
   codebase today — see ISSUES).

Build: `cmake --build build -j8`, exit code captured directly (no pipe) → **0**.
ctest (build dir, Release): **226/226 passed, 0 failed** (baseline 222 + 4 new).

**Load-bearing proof:** neutralized `loadUserData()` (`return;` as the first statement) →
rebuilt `test_projectm_preset_manager` only → `ctest -R "S166-FAV"` → **3/4 passed, 1 failed**
(the round-trip test: `REQUIRE( favorites.size() == 1 )` → `0 == 1`; the other 3 correctly stayed
green, since they only assert on empty-favorites/no-crash, unaffected by a no-op load). Restored
`ProjectMPresetManager.cpp` to its exact prior text; `md5` before mutation
(`392c4900d324da4d34a1b10c1f7e434c`) matched `md5` after restore exactly. Full rebuild → ctest
**226/226** again.

I did **not** attempt an equivalent mutation-proof against `MainComponent.cpp`'s wiring, because
it would prove nothing: `test_projectm_preset_manager`'s CMake target does not compile or link
`MainComponent.cpp` at all (`tests/CMakeLists.txt` — its only sources are the test file and
`ProjectMPresetManager.cpp`), so by construction no change to `MainComponent.cpp` can affect
these tests' pass/fail. This is the coverage gap named above, not a testing oversight.

## ISSUES
1. **Pre-existing, not fixed (out of scope):** `saveUserData()`'s favorites array is keyed by
   `PresetInfo::name` (display name), not path. `scanDirectory()`'s own duplicate check is by
   full path, so two `.milk` files with the same base filename in different scanned directories
   (e.g. a bundled preset and a user-added preset dir both containing "Rorschach.milk") are
   treated as distinct presets but collide in the favorites file — the first name-match on load
   wins, possibly the wrong one. This is latent in the ALREADY-IMPLEMENTED code from before this
   packet, not something I introduced or was asked to redesign; flagging per this repo's
   convention rather than silently fixing scope creep.
2. **Untestable via the public API:** `PresetInfo::userPreset` has no public setter on
   `ProjectMPresetManager` — confirmed by grep, nothing outside `ProjectMPresetManager.cpp` itself
   references it. The save/load code for it is correct and round-trips (test 4 confirms empty
   array in, empty array out, no crash), but there is currently no way — test or production — to
   actually set a preset as a "user preset" to test the non-empty case. Noted, not fixed (adding
   a setter/UI concept for "user presets" is a separate feature, not part of "make favorites
   persist").
3. Nothing found outside the fence. `Renderer.{h,cpp}` were read (for the confinement mechanism)
   but not edited — the design deliberately avoids needing to touch them (see NUANCE on why save
   isn't inside `toggleFavoritePreset()`'s `doToggle()`).
4. `.harmony/binding-decisions.md` and `.harmony/gotchas.md` show as modified in `git status` but
   I did not touch either — their mtimes (14:34/14:46) predate my first build command (14:50:19),
   consistent with the concurrent recon-* agents active in this session. Confirmed via `git diff
   --stat`, not edited by me.

## SKILL_PROPOSALS
None — this was a straightforward wire-up task fully covered by feature-build discipline; no
5+-step procedure emerged that isn't already covered.

## RISKS
- The message-thread synchronous save-on-toggle (see NUANCE) is a tiny, local JSON write — not
  expected to be perceptible, but it IS synchronous I/O on the UI thread, matching this codebase's
  own existing accepted pattern for `milkDropPresetDir_`. If Boris/Harmony wants a stricter
  reading of "does not stall the UI" (not just the render thread), this would need to move to a
  background write — flagging the design choice explicitly rather than silently picking one
  reading of an ambiguous criterion.
- The name-keyed favorites collision (ISSUES #1) is a real, if narrow, correctness risk for users
  with duplicate-named presets across directories. Pre-existing, not introduced or worsened here.

## METRICS
- Files touched: 2 source (`MainComponent.cpp`, `.harmony/notebook.md`), 2 test infra
  (`tests/test_projectm_preset_manager.cpp` new, `tests/CMakeLists.txt`).
- Lines added to `MainComponent.cpp`: ~45 (2 call sites + 1 helper + comments).
- New tests: 4 TEST_CASEs, 6 SECTIONs total (2 in the malformed-file case).
- Build time (incremental, full `AudioDNA` + all tests, 8 threads): well under 5 min.
- ctest full-suite wall time: 5.05s (226 tests).

## KNOWLEDGE CONTEXT
No KNOWLEDGE_TOOLS block in the work packet, and no `graphify-out/GRAPH_REPORT.md` consulted for
this narrow, well-scoped lane (2 call sites in one already-read file) — grep-only, treating
impact conservatively per the C3 safety rule. Impact authority: grep (not authoritative), but the
only surface touched (`onToggleFavoriteRequested` lambda, ctor's MilkDrop block) was read in full
before editing, and the two greps for `saveUserData`/`loadUserData` callers (before) were
confirmed empty in two shapes, satisfying this repo's own standing "two greps of different shape"
rule for zero-callers claims.

## PACKET QUALITY
- **Clarity:** CLEAR. The packet correctly named the exact defect (zero callers, verified),
  pointed precisely at the confinement mechanism to respect (`5d1e791`, `PresetSelector`), and
  gave an unambiguous fence.
- **Missing context:** The packet didn't anticipate that a load-bearing test for THIS SPECIFIC
  fix (an integration wiring change in `MainComponent.cpp`) can't be proven via `ctest` alone,
  because `MainComponent` isn't unit-testable headlessly in this repo (no GL/GUI context) and
  launching the app is explicitly forbidden by the same packet. This isn't a flaw in the packet —
  it's a real structural gap between "build+ctest only" and "prove an integration fix is wired,"
  worth naming for future packets of this shape (a call-site-wiring fix inside a
  GUI-construction-time code path).
- **Unused context:** None — every section (fence, threading constraint, build/test rules) was
  directly relevant and used.
- **Self-brief files:** `.harmony/gotchas.md` — read in full, useful (confirmed the "two greps"
  rule, the stale-binary trap, and the confinement-order lesson from the same session that
  produced the invariant this packet points at). No DEPARTMENT field in the packet, so no
  self-assembly step applied.

## STATUS
DONE_WITH_CONCERNS

## NEXT ACTION
None required from another Builder. For whoever runs the behavioral gate: an app-level check
(favorite a preset via right-click in the MilkDrop browser, quit, relaunch, confirm it's still
favorited) is the one thing this build+ctest-only session could not itself prove, and is the
natural next step to close the loop this packet opened.
