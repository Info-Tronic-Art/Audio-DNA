# Reviewer Verdict - one-save S1, lens threads-api-fence, round 2
STATUS: DONE
VERDICT: PASS (no MUST). Round 1 had no MUST; the four fix-round code/doc changes are sound. 2 SHOULD stand (both Harmony rulings, not code), 3 NIT.
REVIEWED: lane/one-save head 93b49ab (code head 608d6fd), fix round = git diff 25d5dc9..93b49ab; read through git objects only; nothing built or run.

FIX ROUND READ (VERIFIED by reading the diff; no builds):
- 9e57a64 src/core/ShowFile.h:27-35 versionOf clamps with juce::jmin(int64, INT_MAX); <limits> included. Only reader of the int is Composition.h:1009 (loadedVersion) and backupTagFor ShowFile.h:62/91; MainComponent.cpp:3600 reports loadedVersion. No other cast of a show "version" in src. SF-10 (tests/test_show_file.cpp:336-352) uses 4 values > INT_MAX, REQUIREs the parsed property isInt64 (so the case cannot pass vacuously), checks versionOf, backupTagFor ("v2147483647") and Composition::fromVar (loadedVersion, migrationNote empty). It can fail: report gives RED raw lines on the unchanged source (-2147483648 / -1294967296 / 0 / 2) and GREEN after; MU-OS-22 re-run RED then GREEN. Thread/fence unaffected: pure function, no I/O.
- 55e1606 tests/test_app_settings.cpp: AS-4 and the two POSIX includes under #ifndef _WIN32; SIGXFSZ old disposition restored (line 228). Test body unchanged otherwise; still drives real AppSettings::update under a real RLIMIT_FSIZE. Windows side unbuilt (reported).
- a0062a6 probe-one-save.sh header and probe-one-save.py docstring: comments only. They now say AUDIODNA_LIBRARY_DIR is inert (grep of src at 93b49ab: 0 hits) and the launched app READS, never writes, the user's library. Round-1 finding 2 is fixed as found.
- 608d6fd integration.md and pitfalls.md NN name the other replaceWithText writers whole (deck files, PresetManager, Export Bindings, Save Layout, take, audio, favourites, MilkDrop) - docs only, still within the S1 doc rows.
- 93b49ab report only.
FENCE: the file list of 8b464a6..93b49ab is identical to round 1 (28 paths); the fix round added no file; all fix-round paths are S1 files (ShowFile.h, test_show_file.cpp, test_app_settings.cpp, probe-one-save.*, integration.md, pitfalls.md, the stage report). No on-screen text added. No new thread, mutex or atomic (the fix is a pure function and comments).
PROBE QUIT RULE (VERIFIED grep): no pkill/killall in the four probe-one-save files; the only kills are the self-test killing its own stub's $! (selftest.sh:22, 40, 66); probe-one-save.sh:20 is a comment about quit_ours. Unchanged from r1.
ROUND-1 ITEMS: r1 #1 (test_show_model.cpp helper line) and #3 (ruled gate string reaches no lint row) are Harmony rulings, left open by the builder as stop items 1 and 2 - not fixed in code, and not MUST. NITs 4-8 untouched by design (builder said so).

FINDINGS
1 SHOULD (Harmony ruling, still open) tests/test_show_model.cpp:1305 is one line outside the S1 row (legacyShow helper drops "version"); forced by A-3. Needs Harmony's written word. VERIFIED.
2 SHOULD (Harmony ruling, still open) ruling G-OS1 string `ctest -R "safewrite|showfile|showbackup|onesavelint"` runs 21 rows and no lint row (builder measured with ctest -N; INFERRED by me from ctest title-matching). Use test_one_save_lint "[onesavelint]".
3 NIT ShowFile.h:33 std::numeric_limits<int>::max() is unparenthesised; a Windows build that leaks the max() macro would break it. JUCE normally sets NOMINMAX; INFERRED, not built. Optional: write (std::numeric_limits<int>::max)().
4 NIT the probe header's "at launch" claim for the library scan: the post-save refresh() calls are VERIFIED (MainComponent.cpp:3613, 3640); the launch-time scan I did not trace (INFERRED). Read-only either way.
5 NIT tests/test_app_settings.cpp:225-228: if REQUIRE(restored == 0) fails, the SIGXFSZ disposition is not restored (the line after it is skipped). Test-only, fails loudly anyway.

NOT RUN (labelled): RED/GREEN lines rest on the builder's raw output; Windows guard unbuilt; no probe or app run (not allowed).
SUMMARY: 8 files in the fix round, 0 blocking, 2 SHOULD (rulings), 3 NIT. Confidence VERIFIED for fix content, fence, probe quit rules; INFERRED for items 2-4.
METADATA: reviewer=reviewer, builder_packet=one-save-S1, date=2026-10-04
