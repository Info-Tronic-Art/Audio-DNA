# Reviewer Verdict -- one-save S1, lens data-safety, round 2
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS: 0 MUST, 0 SHOULD, 2 NIT)
REVIEWED: lane/one-save head 93b49ab (code 608d6fd), fix round = git diff 25d5dc9..93b49ab. Read through git objects only; nothing built or run.

Round-1 had 0 MUST. Its data-safety SHOULDs, checked at the head:
- SHOULD-1 (versionOf int truncation): FIXED as found. src/core/ShowFile.h:27-36 clamps with jmin to INT_MAX (never 0 / 2 / negative); backupTagFor :67 uses it, Composition.h:1009-1010 reads it once. SF-10 (tests/test_show_file.cpp:336-352) adds 2147483648, 3000000000, 4294967296, 4294967298 and checks versionOf, backupTagFor ("v2147483647") and fromVar (loadedVersion = INT_MAX, migrationNote empty); it drives real code; report shows RED (13 failed assertions on unchanged source: -2147483648, -1294967296, 0, 2) then GREEN, plus MU-OS-22 re-run. VERIFIED by reading. Values above int64 max are INFERRED to parse as double -> "no version" -> still copied (v0/v1), no data loss.
- SHOULD-2 (gate string reaches no lint row): not a code fix; Harmony's. Builder stop item 2 + hand-over line give the working command (tests/test_one_save_lint). Open for Harmony.
- SHOULD-3 (test_show_model.cpp:1305 helper edit): needs Harmony's written ratification (stop item 1). Open for Harmony; unchanged.
- SHOULD-4 (probe header claim): FIXED. probe-one-save.sh header and .py docstring now say AUDIODNA_LIBRARY_DIR is inert until S4b and the app only READS his library. Comments only. VERIFIED.
- SHOULD-5 (docs "four other writers"): FIXED. integration.md and pitfalls.md NN now list deck files, PresetManager, Export Bindings, Save Layout, and say Save Deck As can still overwrite any file. Matches `git grep replaceWithText 93b49ab -- src` (MainComponent.cpp:3871, :7389; BindingManager.cpp:297; PresetManager.cpp:156/:528/:581; plus take, audio sidecar, favourites, MilkDrop). VERIFIED.
- AS-4 POSIX guard (other lens): test_app_settings.cpp #ifndef _WIN32 around includes + case; SIGXFSZ disposition restored. VERIFIED.

New defect from the fix: none. The only src change of the round is ShowFile.h (+4/-1). Write paths unchanged: writeShow -> saveWithBackup (backup first, fail-closed) -> Composition::saveToFile -> verified write; src grep at head shows saveToFile( only at MainComponent.cpp:3565. settings.json mend untouched. Files changed in the round are all inside the stage fence; no stray mutant / instrumentation / .venv link / build dir in the tree diff. No on-screen text added (src diff of the round is one header).

NIT-A tests/test_app_settings.cpp (AS-4): the SIGXFSZ restore at the end sits after REQUIRE(restored == 0); a failing REQUIRE earlier would leave SIGXFSZ ignored for the rest of the binary. Test-only, cosmetic.
NIT-B The clamped tag v2147483647 is shared by every version >= INT_MAX; distinct bytes still get "(n)" names, so nothing is overwritten. Builder stop item 7 leaves the wording to Harmony.

Open items for Harmony (not findings against the builder): SHOULD-2 gate string, SHOULD-3 ratification, stop items 6-8 (probe app reads his library until S4b; Save Deck As unverified until later stages).

Confidence: VERIFIED by reading git objects; the RED/GREEN lines are the builder's report, not re-run by me.
