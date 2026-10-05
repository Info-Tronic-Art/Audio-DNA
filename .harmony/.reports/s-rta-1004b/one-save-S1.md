# one-save S1 "the show file" -- builder report (lane/one-save, base 8b464a6)

### STATUS
STATUS: DONE_WITH_CONCERNS

Every item of the stage row is built and its unit rows are green; nothing was run live (no app launch is allowed in
this stage), so the two new routes, `writeShow` inside the app and the probe's fixtures against the real app are
COMPILED AND UNIT-PROVEN ONLY. Five things need Harmony's ruling (section STOP ITEMS). Written 2026-10-04 22:12:54.
INBOX-RECHECK: none (this run has no inbox; no addendum arrived).

Compaction header
- TASK: lane one-save, stage S1 (ruling-one-save.md section 4 row "S1 the show file").
- HEAD: 2d3cdbb + the commit of this report (code head 2d3cdbb: `git -C /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave log --oneline -5`).
- BUILD: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-lane = the LANE app (Release, test server, Syphon); 4 mutant apps in build-mut-os4 / os5 / os23 / os21.
- CTEST: `100% tests passed, 0 tests failed out of 1278` (1252 on main + 26 new), through the mutex, rc 0.
- LIVE: nothing run. No app launched, no Output window, no probe run, no screen capture.
- OPEN: 5 stop items, 6 found-not-fixed (below).
- NEXT: Harmony's G-OS1 rows (HAND-OVER block at the end).
- RISK: see RISKS.

## WHAT WAS BUILT (one entry per ruling item of S1)

| item | verdict | where | key evidence |
|---|---|---|---|
| A-1 the verified writer | BUILT | src/core/SafeFileWrite.h (`writeBytes`, `holdsExactly`, `writeTextVerified`, `copyVerified`, seam `WriteFn`) | SW-1..4 green (77 assertions); MU-OS-21 -> SW-2 `CHECK_FALSE( safewrite::writeTextVerified(target, text, kHalfAndSaysOk) )` `!true` |
| A-2 the copy before an overwrite | BUILT | src/core/ShowFile.h (`backupTagFor`, `backupFileFor`, `backupBeforeOverwrite`, `saveWithBackup`) | SB-1..7 green (161 assertions); MU-OS-4 -> SB-1, MU-OS-5 -> SB-4, MU-OS-23 -> SB-6 all RED |
| A-3 the version reader | BUILT | ShowFile.h `versionOf`; Composition.h `fromVar` reads it once (`loadedVersion`) | SF-3, SF-4, SF-5, SF-7, SF-10 green; MU-OS-2 -> SF-3 `1 == 0`, MU-OS-3 -> SF-5 `0.8 == 1.0f`, MU-OS-22 -> SF-10 `0 == 1` |
| A-4 dead keys (only outputDisplay goes) | BUILT | Composition.h: member, write, read removed | SF-8 green; `grep -rn outputDisplay src` before the edit: Composition.h:196 / :745 / :892 and PresetManager's own deck field only (no other reader) |
| "version": 2 first; saveToFile(file, extras); loadedVersion / loadedExtras | BUILT | Composition.h (`toVar`, `saveToFile`, `loadFromFile`, `takeLoadedExtras`, `initDefault`) | SF-1, SF-2, SF-6, SF-9 green; MU-OS-1 -> SF-1 `"name" == "version"` |
| A-15 the settings.json mend | BUILT | src/model/AppSettings.cpp `update` | AS-2..4 green; MU-OS-34 -> AS-2 `REQUIRE( kept.existsAsFile() )` false |
| writeShow, its three callers, lastSave | BUILT, NOT RUN LIVE | src/MainComponent.cpp/.h (`writeShow`, `showHasFile`, `debugPlainSave`, `showFileVar`, `lastSave_`); `saveComposition`, `saveCompositionTo`, Collect Media call it | LINT-1 green (one call of `saveToFile(` in src, inside `MainComponent::writeShow`, which calls `showfile::saveWithBackup(`) |
| save_composition {"plain"}, GET /api/debug/show_file | BUILT, NOT RUN LIVE | src/api/ApiServer.cpp/.h | compiles; `strings` of the lane app holds `/api/debug/show_file` (1 hit) |
| LINT-1, LINT-7 | BUILT | tests/test_one_save_lint.cpp | green (350 assertions in 2 cases); RED on the tree before the stage (2 of 2 cases) |
| probe-one-save.sh (L13, L14, L14b, L21) + self-test | WRITTEN, NEVER RUN AGAINST THE APP | .harmony/probe-one-save.sh / .py / -stub.py / -selftest.sh | `SELFTEST 36 ok / 0 FAIL` (stub server; each row red on its mutant mode) |
| probe-boxes K7's expectation | WRITTEN, NEVER RUN AGAINST THE APP | .harmony/probe-boxes.py `k7_saved_shape` + one `check(...)` in `k7_save_reload` | self-test part 2: the new shape passes, 6 other shapes fail |
| docs | DONE | docs/claude/integration.md ("The show file"; settings.json), architecture.md (2 files), pitfalls.md ("NN.") | CLAUDE.md NOT touched (the stage row does not name it; the index line is Harmony's at the merge) |
| mutant apps for the live RED arms | BUILT | build-mut-os4, -os5, -os23, -os21 (target AudioDNA only) | each `mutant app build rc=0, objects compiled 124`; after each `git diff --quiet -- src tests rc=0` |

Files: NEW src/core/SafeFileWrite.h, src/core/ShowFile.h, tests/OneSaveFixture.h, tests/test_safe_write.cpp,
tests/test_show_file.cpp, tests/test_show_backup.cpp, tests/test_one_save_lint.cpp, .harmony/probe-one-save.sh,
.harmony/probe-one-save.py, .harmony/probe-one-save-stub.py, .harmony/probe-one-save-selftest.sh. EDITED
src/model/Composition.h, src/model/AppSettings.cpp, src/MainComponent.cpp, src/MainComponent.h, src/api/ApiServer.cpp,
src/api/ApiServer.h, tests/test_app_settings.cpp, tests/test_composition.cpp (2 call sites), tests/test_show_model.cpp
(1 helper line), tests/CMakeLists.txt, .harmony/probe-boxes.py, docs/claude/integration.md, architecture.md,
pitfalls.md, .gitignore (1 line).

## RED FIRST (raw lines; logs in /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad/one-save-S1/)

Arm 1, the tree BEFORE the stage's source edits (red.sh; the two new headers moved aside for that build):
```
== build test_safe_write rc=2
tests/test_safe_write.cpp:7:10: fatal error: 'core/SafeFileWrite.h' file not found
== build test_show_file rc=2
tests/test_show_file.cpp:6:10: fatal error: 'core/ShowFile.h' file not found
== build test_show_backup rc=2
tests/test_show_backup.cpp:7:10: fatal error: 'core/ShowFile.h' file not found
== run test_one_save_lint rc=42
test cases:   2 |   0 passed | 2 failed
== run test_app_settings rc=42
tests/test_app_settings.cpp:158: FAILED:      (AS-2: REQUIRE( kept.existsAsFile() ))
tests/test_app_settings.cpp:196: FAILED:      (AS-3: CHECK_FALSE( s.update("a", 1) ))
tests/test_app_settings.cpp:224: FAILED:      (AS-4: CHECK_FALSE( ok )  with expansion: !true)
tests/test_app_settings.cpp:227: FAILED:      (AS-4: CHECK( after == before ))
test cases:  10 |   7 passed | 3 failed
```
Arm 2, the named mutants on the finished tree (mutants.sh, mutants2.sh; each: edit, build, RED, revert, touch,
rebuild with >= 1 object, GREEN; source checksums after all nine equal the snapshot taken before the first):
```
=== MU-OS-1 -> SF-1   RED run rc=42 (test cases: 1 | 0 passed | 1 failed)  CHECK( keys.front() == "version" )  "name" == "version"
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (34 assertions in 1 test case))
=== MU-OS-2 -> SF-3   mutant build rc=0, objects compiled 1   RED run rc=42  CHECK( showfile::versionOf(old) == 0 )  1 == 0
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (25 assertions in 1 test case))
=== MU-OS-3 -> SF-5   RED run rc=42  CHECK( c.migrationNote.empty() ) false;  CHECK( l.opacity == 1.0f )  0.8 == 1.0f
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (11 assertions in 1 test case))
=== MU-OS-22 -> SF-10 mutant build rc=0, objects compiled 1   RED run rc=42  CHECK( showfile::versionOf(withLayers) == 1 )  0 == 1
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (77 assertions in 1 test case))
=== MU-OS-4 -> SB-1   RED run rc=42  CHECK( outcome.backup == Backup::Made )  0 == 1;  REQUIRE( copy.existsAsFile() ) false
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (18 assertions in 1 test case))
=== MU-OS-5 -> SB-4   RED run rc=42  CHECK_FALSE( outcome.saved )  !true;  CHECK( writes == 0 )  1 == 0;  CHECK( bytesOf(target) == original )
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (13 assertions in 1 test case))
=== MU-OS-23 -> SB-6  RED run rc=42  CHECK( outcome.backup == Backup::Made )  0 == 1;  REQUIRE( copy.existsAsFile() ) false
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (56 assertions in 1 test case))
=== MU-OS-21 -> SW-2  RED run rc=42  CHECK_FALSE( safewrite::writeTextVerified(target, text, kHalfAndSaysOk) )  !true;  CHECK( bytesOf(target) == before )
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (10 assertions in 1 test case))
=== MU-OS-34 -> AS-2  RED run rc=42  REQUIRE( kept.existsAsFile() )  false
                      restore build rc=0, objects compiled 1   GREEN run rc=0 (All tests passed (20 assertions in 1 test case))
```
Two first attempts in mutants.sh were INVALID and are superseded by mutants2.sh (reported, not hidden): MU-OS-2's
mutant build compiled nothing (the edit landed in the same mtime second as the previous restore build -- the rig's
restore trap bites the MUTANT build too; fixed with a sleep + touch before the mutant build and a printed object
count); MU-OS-22's Catch2 filter held commas, which Catch2 reads as "or" (rc=2, no case ran; the filter is now
"showfile: a version of 0*").

## GREEN
```
test_safe_write: All tests passed (77 assertions in 4 test cases)
test_show_file: All tests passed (254 assertions in 10 test cases)
test_show_backup: All tests passed (161 assertions in 7 test cases)
test_one_save_lint: All tests passed (350 assertions in 2 test cases)
test_app_settings: All tests passed (124 assertions in 10 test cases)      (7 existing + AS-2, AS-3, AS-4)
ctest -N: Total Tests: 1278                                                 (1252 + 4 SW + 10 SF + 7 SB + 3 AS + 2 LINT)
ctest-mutex rc=0 / 100% tests passed, 0 tests failed out of 1278 / Total Test time (real) = 21.78 sec / ctest rc=0
regression rows: ctest -R "^(M2 |M3 |M6 |M7 |T6g|T6h|T6i|AS1|AS6)" -> 100% tests passed, 0 tests failed out of 13;
                 ctest -R "output law" -> 100% tests passed, 0 tests failed out of 15
```
The FIRST full ctest of the stage had one failure (1 of 1278): `safewrite: a writer that fails leaves the target
unchanged and no temporary file behind`, green when run alone. Cause (inferred from the code, then tested): my temp
folders came from `getNonexistentChildFile`, and ctest runs the cases of one binary as parallel processes -- two asked
for "the next free name" at once and shared a folder. Fix: a Uuid in the folder name (test_safe_write.cpp,
OneSaveFixture.h, and the existing `TempSettings` of test_app_settings.cpp, which my three new cases use). After it:
10 runs of the 32 rows at `-j 12`, 10 of 10 `100% tests passed, 0 tests failed out of 32`; then the full ctest above.
The nine mutant runs pre-date this fixture change (case bodies are unchanged by it).

## FACTS MEASURED (each with its raw line)
- F1 ruling V1 holds FOR REAL on this machine, without any seam: on the tree before the stage, AS-4 limits the process
  to 64-byte files for one `update()` (RLIMIT_FSIZE) -- `juce::File::replaceWithText` swapped the cut-off temporary
  file in and `update` answered true. Raw: `test_app_settings.cpp:224: FAILED: CHECK_FALSE( ok ) with expansion: !true`
  and `:227 CHECK( after == before )` whose left side is 64 bytes ending `"big": "0123456789012345` (no closing
  quote or brace), and `:228 CHECK( s.read("outputs").toString() == "the set as it was" )` `"" == ...` (the settings
  then read as EMPTY). This is M-1's outcome O1 shown on a size limit, not on a full disk: the disk run stays Harmony's.
- F2 N: `ctest -N: Total Tests: 1278` on build-lane (N_main 1252 + 26).
- F3 The file's line endings do not change: `juce::JSON::toString` itself emits CR LF (a target written by the new
  raw-bytes writer starts `{ '{', '\r', '\n', ' ', ' ', '"', 'v', 'e', 'r', 's', 'i', 'o', 'n', '"', ':` -- the MU-OS-5 run's
  expansion of the rewritten target).
- F4 The app-only build of a mutant dir compiles 124 objects in about 90 s on this rig tonight (22:04:47 -> 22:06:14);
  the whole tree (app + every test) is 1637 objects and took 33 min under a load average of 185 (21:26 -> 21:59).
- F5 A 2 MB HFS+ image mounts with 1,912,832 free bytes: `hdiutil create -size 2m -fs HFS+` rc 0, `hdiutil attach
  -nobrowse -mountpoint` rc 0, `/dev/disk4s1 2008 140 1868 7%`, detach rc 0, `hdiutil info | grep -c` 0. (Only the
  command lines were checked, in the scratchpad; no app, no save: M-1 itself is not measured.)
- F6 `.gitignore` did NOT ignore `build-mut-*`: `git check-ignore -v build-mut-os4` printed nothing for it (it printed
  `.gitignore:5:/build-lane/`). One line added.
- F7 Lane app: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app`, binary sha256 ea3c1a2481fc...49da225; a
  final `cmake --build build-lane` after the mutant apps compiled 0 objects (the mutated headers got their content AND
  mtime back). Mutant binaries: os4 ced43f4eef76..., os5 4e265838c2dd..., os23 f26dc7e02dda..., os21 df29c5ea51ca...
- F8 `strings` of the lane app: `/api/debug/show_file` 1 hit; `outputDisplay` 1 hit (PresetManager's own deck key,
  src/ui/PresetManager.cpp:505 -- it leaves with the class in S7).

## STOP ITEMS FOR HARMONY (each: what I found, what I did, what needs a ruling)
1. THE REGRESSION ROWS' FIXTURE CARRIED "version": 2. The ruling says tests/test_show_model.cpp M2, M3, M6, M7 ... "must
   stay GREEN UNCHANGED". Its helper `legacyShow()` (tests/test_show_model.cpp:1298-1311) builds an "old" show from
   `Composition::toVar()` and removes "layers"; toVar now writes "version": 2, so by A-3 that fixture is a version-2
   file and is NOT converted. Raw (before my one line): 14 failed assertions at test_show_model.cpp:1380, :1382, :1383,
   :1386, :1395, :1397, :1400, :1401, :1409, :1495, :1496, :1635, :1636, :1637 (log showmodel-before-fixture-fix.log).
   DONE: one line in the HELPER -- `obj->removeProperty("version");` -- no case body, no expectation touched. NEEDS A
   RULING: is a fixture-helper line inside "unchanged"? (A real old file never has the key; the model behaviour is A-3's.)
2. THE GATE STRING `ctest -R "safewrite|showfile|showbackup|onesavelint"` (ruling section 5, G-OS1) MATCHES NO LINT ROW:
   ctest -R matches case TITLES, and the two ruled titles are "`saveToFile(` on a Composition is called only inside
   MainComponent::writeShow" and "Composition.h, AppSettings.cpp and ShowFile.h name no `replaceWithText`" -- neither
   holds "onesavelint" (that is the Catch2 TAG). Titles kept as ruled. Use the command lines of the HAND-OVER block.
3. K7 MAKES NO BACKUP. The plan's Table 2 says K7 turns red because "a backup appears beside it". K7 saves to
   `<out>/k7_saved.json`, a NEW file (.harmony/probe-boxes.py `k7_save_reload`): the copy rule has nothing to keep. The
   expectation I wrote: first key "version" = 2, "keys" and "layout" blocks present, NO backups folder in the out dir.
4. MU-OS-21's LIVE ARM IS PROBABLY NOT DECIDABLE (inferred, not run): the mutant removes only the read-back
   comparison; the production writer still checks `write()` / `flush()` / `getStatus()`, and on a real full disk those
   report the short write, so the MU-OS-21 app would still answer "failed" and OS-L21 would stay green on it. The
   ruling already says "OS-L21 when decidable"; the arm that can go red is the 185147b app (F1 shows the path is real).
   build-mut-os21 exists if she wants to try it.
5. FIVE CHOICES THE RULING DOES NOT SPELL OUT (each small; say if any is wrong): (a) `showfile::saveWithBackup` is a
   new function in ShowFile.h -- the copy-then-write ORDER lives there so that MU-OS-5 turns red both SB-4 and OS-L14;
   `MainComponent::writeShow` calls it with `composition_.saveToFile` as the write. (b) `{"plain": true}` on a show
   with NO file records lastSave result "cancelled" and opens no chooser (a test route must never open one).
   (c) `lastSave` carries one extra field, `ms` (the whole of writeShow), for M-2. (d) Collect Media's show copy goes
   through `writeShow`, and its result is dropped as before: no box is added there (his rule: no new failure text).
   (e) AppSettings.h was NOT touched (S2 owns it): AS-4 needs no seam because it cuts the write with RLIMIT_FSIZE.

## FOUND, NOT FIXED
- N1 A test-mode app still LISTS the user's real compositions folder when a save succeeds (`saveComposition` /
  `saveCompositionTo` call `getCompDecksBrowser().refresh()`; ruling SF-13): unchanged here, ends in S4b. The probe
  passes `AUDIODNA_LIBRARY_DIR` already; nothing reads it before S4b.
- N2 tests/test_app_settings.cpp now includes `<sys/resource.h>` and uses `setrlimit` / `SIGXFSZ` (AS-4): POSIX only.
  A Windows build of the tests needs a guard around that one case.
- N3 The four other `replaceWithText` writers (Take.cpp:293, AudioStore.cpp:251, FilesBrowser.cpp:634,
  ProjectMPresetManager.cpp:108) are as they were (ruling SF-4); F1 says the hazard is real for them.
- N4 LINT-1 exempts `bindingManager_.saveToFile(` (BindingManager's own writer, MainComponent.cpp:7348): S2 removes
  Export Bindings and should then drop the exemption from tests/test_one_save_lint.cpp.
- N5 `Composition::takeLoadedExtras()` (PL:169) exists and is unit-tested (SF-2) but has no src caller until S2 / S3.
- N6 src/model/AppSettings.h's header comment still says a corrupt file is "rewritten" without naming the
  `.unreadable` copy (the file is S2's; docs/claude/integration.md has the truth).
- Smells seen, not touched: MainComponent.cpp is a god file (8,028 lines; every stage of this lane edits it);
  `saveComposition` / `saveCompositionTo` duplicate the "Saved:" label + list refresh.

## RISKS
- R-a NOTHING HERE RAN IN THE APP. `writeShow`, the "plain" branch, `GET /api/debug/show_file`, and whether the real
  app loads the probe's F-OLD / F-A JSON (written from reading Clip / Layer / Deck `fromVar`, image clips on a 16 x 16
  PNG the probe writes) are unproven until OS-L13 runs. If a fixture is refused the row prints
  `FAIL  OS-L13: NOT the old-shape show loads (HTTP ...)` -- a fixture fault, not a product fault.
- R-b The copy rule reads and parses the target on the message thread at every Save (ruling R8): M-2 prints it.
- R-c `loadedExtras` are two `juce::var` on Composition (ruling R16). I found no copy of the LIVE composition off the
  message thread (`grep -rnE` over src for a Composition copy-constructed or assigned from `composition_`: 0 hits); the HTTP thread's pre-check
  (ApiServer.cpp:1137) loads a PRIVATE Composition, whose vars live and die on that thread. A grep, not a proof.
- R-d Commits are grouped by buildable unit (source + tests; probes; docs), not one per ruling item: A-1 / A-3 / A-4
  share Composition.h and tests/CMakeLists.txt, and a per-item split would leave commits that do not configure.

## PROGRESS LOG (appended after every item; a successor resumes from the last line)
- [21:12:04] step 0: worktree clean at 8b464a6; no earlier report (fresh stage); df 296Gi free; configure rc=0.
- [22:00] nine unit mutants RED then GREEN (mutants.sh + mutants2.sh); sources back to the pre-mutant checksums.
- [22:03] first full ctest 1277 / 1278 (parallel temp-folder race in my fixtures) -> Uuid folder names -> 10 x 32 rows green at -j 12 -> full ctest 1278 / 1278, rc 0.
- [22:04] commits 4f73cd9 (source + tests), 47c25c5 (probes), 2d3cdbb (docs + the ignore line).
- [22:10] four mutant apps built (build-mut-os4 / os5 / os23 / os21), tree clean after each; final build-lane build compiled 0 objects; `git status --short` empty; no Audio-DNA process (none was ever started by this stage).

## PACKET QUALITY
- Clarity: HAD_TO_INFER (who builds the mutant apps for the live RED arms; what `{"plain": true}` does on a show with
  no file; whether a fixture helper counts as "unchanged").
- Missing context: the fact that `legacyShow()` derives old fixtures from toVar(); that `.gitignore` has no
  `build-mut-*` line (the rig rules say it has); the LINT rows' titles against the `-R ... onesavelint` gate string.
- Unused context: ruling sections 6, 7 and the S2..S5 rows beyond their names; binding-decisions beyond the save /
  failure-text lines.
- Self-brief files: no DEPARTMENT / SELF-BRIEF field in the packet. Read: plan-one-save.md (adoption blocks, OS1,
  section 5, Table 2), ruling-one-save.md whole, CLAUDE.md, docs/claude/integration.md + architecture.md +
  pitfalls.md (edited sites), .harmony/probe-boxes.sh / .py, probe-async-load.sh, probe-quit-ours.sh + selftest,
  ~/Harmony_Main/memory/pulse.json (GREEN; claims "general" by two harmony sessions, none on this lane).
- No knowledge tools -- grep-only; no deletion rests on "no callers" except `outputDisplay`, whose removal the ruling
  orders (A-4) and whose only other hits are PresetManager's own field.

### SLIM CHECK
Changed lines trace to the stage row; no refactor of neighbouring code; no new abstraction beyond the two ruled
headers and `saveWithBackup` (stop item 5a). No on-screen text added.

## HAND-OVER TO HARMONY
Head: the commit of this report on lane/one-save (code head 2d3cdbb; `git -C /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave log --first-parent --oneline -5`).
Build dirs: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-lane = the LANE app + every test (current: a rebuild compiles 0 objects).
  /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-mut-os4  = MU-OS-4  app (backupBeforeOverwrite always NotNeeded)        -> RED arm of OS-L13
  /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-mut-os5  = MU-OS-5  app (a Failed copy is ignored, the write goes on)   -> RED arm of OS-L14
  /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-mut-os23 = MU-OS-23 app (a target that does not parse needs no copy)    -> RED arm of OS-L14b
  /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-mut-os21 = MU-OS-21 app (swap without comparing the read-back)          -> RED arm of OS-L21 "when decidable" (stop item 4)
  each app: <dir>/AudioDNA_artefacts/Release/Audio-DNA.app (target AudioDNA only; no tests in those dirs).
Full ctest: `bash /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad/lib/ctest-mutex.sh /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave/build-lane -j 6`
  prints `100% tests passed, 0 tests failed out of 1278` and `ctest rc=0`.

G-OS1, row by row (APP = the bundle path; L = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/onesave; the caller holds /tmp/audiodna-live.lock; no Audio-DNA running):
1. unit rows: `ctest --test-dir L/build-lane -R "^(safewrite|showfile|showbackup|appsettings):"`
     PASS line: `100% tests passed, 0 tests failed out of 24`  (4 SW + 10 SF + 7 SB + 3 AS)
   lint rows: `L/build-lane/tests/test_one_save_lint "[onesavelint]"`
     PASS line: `All tests passed (350 assertions in 2 test cases)`
   the settings file's ten cases: `L/build-lane/tests/test_app_settings`
     PASS line: `All tests passed (124 assertions in 10 test cases)`
   RED arms: the named unit mutants (section RED FIRST); `python3 /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad/one-save-S1/mutant.py apply|revert <id>` is the edit.
2. the probe's self-test (no app): `bash L/.harmony/probe-one-save-selftest.sh`
     PASS line: `SELFTEST 36 ok / 0 FAIL`
3. OS-L13, OS-L14, OS-L14b: `ONESAVE_APP=L/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app bash L/.harmony/probe-one-save.sh <out-base> os_l13,os_l14,os_l14b`
     PASS lines: `PASS  OS-L13: ...`, `PASS  OS-L14: ...`, `PASS  OS-L14b: ...`, then `PROBE-ONE-SAVE GREEN`
     M-2: `INFO  OS-L13: M-2 the time of writeShow on F-OLD (<n> bytes after the save): first save (...) <ms> ms; plain save (no copy) <ms> ms`
     RED arms (same command, another ONESAVE_APP):
       build-mut-os4  -> `FAIL  OS-L13: ...` (it also turns OS-L14 and OS-L14b red: no copy is ever made)
       build-mut-os5  -> `FAIL  OS-L14: ... NOT old.json's sha256 unchanged ...`
       build-mut-os23 -> `FAIL  OS-L14b: ... NOT backups/x.other.json holds exactly the 10 bytes ...`
       the 185147b app -> `FAIL  OS-L13: ...` naming `backups folder: None` (it has no show_file route: the facts say
                          `show_file HTTP 404`; that arm proves only "no backups folder").
4. OS-L21 = M-1 (Harmony makes the image; these lines ran clean in the scratchpad, F5):
     `hdiutil create -size 2m -fs HFS+ -volname onesave-m1 <scratch>/m1.dmg`
     `mkdir -p <scratch>/m1 && hdiutil attach -nobrowse -mountpoint <scratch>/m1 <scratch>/m1.dmg`
     `ONESAVE_FULLDISK=<scratch>/m1 ONESAVE_APP=<app> bash L/.harmony/probe-one-save.sh <out-base> os_l21`
     `hdiutil detach <scratch>/m1` ; `hdiutil info | grep -c m1.dmg` -> 0
     new app, PASS line: `PASS  OS-L21: ...; the show's sha256 is unchanged (...); lastSave result 'failed' ...`
     185147b app: `FAIL  OS-L21: ... NOT the show's sha256 is unchanged ...` + `INFO  OS-L21: this app has no show_file route (HTTP 404): the 185147b arm. Outcome O1 (...)` (or `Outcome O2`: then the row is INFO by the ruling)
     without ONESAVE_FULLDISK: `INFO  OS-L21: not run -- ...` (never a pass).
5. probe-boxes K7: `BOXES_APP=L/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app bash L/.harmony/probe-boxes.sh <out-base> k7_old_show`
     new PASS line among K7's: `PASS  k7_old_show save: the saved file is a version-2 show (first key 'version', version 2, keys + layout blocks True); backups folder beside a NEW target: False`
     RED arm: main's app (first key 'name', version None).
6. G-OS-HIS and G-OS4-0 are Harmony's own scripts; the routes they need: `POST /api/debug/save_composition {"plain": true}`,
   `GET /api/debug/show_file` (lastSave.backup "made", lastSave.ms for M-2), `GET /api/composition` (now with "version",
   without "outputDisplay").
Before returning: `git -C L status --short` empty; no app running that this stage started (none was started); `git
worktree list` unchanged (7 lines, none added by this stage).
