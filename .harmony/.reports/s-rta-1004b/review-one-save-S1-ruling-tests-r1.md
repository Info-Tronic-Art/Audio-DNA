# Reviewer Verdict - one-save S1, lens ruling-tests, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- 0 MUST, 3 SHOULD, 5 NIT
REVIEWED: lane/one-save head 25d5dc9 (code head 2d3cdbb, source+tests 4f73cd9), base 8b464a6; read through git objects only.
METADATA: reviewer=sonnet-5-5, date=2026-10-04

## Conformance (all VERIFIED by reading the pinned objects unless marked INFERRED)
- Rows exist under the pre-registered titles, drive real code, and can fail: SW-1..4 (tests/test_safe_write.cpp), SF-1..SF-10 (test_show_file.cpp, 10 cases),
  SB-1..SB-7 (test_show_backup.cpp, 7 cases), AS-2..AS-4 (test_app_settings.cpp:146-237), LINT-1/LINT-7 (test_one_save_lint.cpp:95,130). Titles match ruling s5 / plan 602-661 word for word.
- Mutants MU-OS-1..5, 21, 22, 23, 34: definitions in the builder's mutant.py match the ruling table; scratchpad logs show RED at the named row
  (mu1 SF-1 :37-63, mu3 SF-5 :180/183, mu4 SB-1 :61-64, mu5 SB-4 :153-156, mu21 SW-2 :106-114, mu22 SF-10 :306-318, mu23 SB-6 :222-242, mu34 AS-2 :158)
  and a restore build "Building CXX object" (>=1 object) before each GREEN run. Source sha256 of ShowFile.h, SafeFileWrite.h, Composition.h, AppSettings.cpp at 25d5dc9 equals pre-mutant-sha.txt: no mutant left.
- RED arm 1 (tree before the stage): red-run-test_one_save_lint.log shows `REQUIRE( calls.size() == 1 ) 3 == 1` (the LINT scan really walks src/); AS-2..4 fail on old AppSettings.
- ctest count: 4 SW + 10 SF + 7 SB + 3 AS + 2 LINT = 26; 1252 + 26 = 1278 matches ctest-N.log "Total Tests: 1278".
- Amendments: A-1 (read-back before swap, SafeFileWrite.h:61-72), A-2 (ShowFile.h:76-105, fail-closed, next free name, plain FILE `backups` -> Failed), A-3 (versionOf integer >= 2 only, ShowFile.h:29-37; fromVar reads it once, Composition.h:1006-1010),
  A-4 (only outputDisplay removed; the six other keys kept), A-15 (AppSettings.cpp:36-50) all implemented as ruled. LINT-1 holds at head: one `saveToFile(` call in src, inside MainComponent::writeShow (MainComponent.cpp:3565), through showfile::saveWithBackup; Collect Media (:6751) and Save / Save As go through writeShow.
- No on-screen text added (MainComponent.cpp diff: only the existing "Saved:" label and `!testMode_`-guarded existing box). Docs: integration.md "The show file", architecture.md, pitfalls.md "NN." as the ruling says; CLAUDE.md untouched (S7's).
- Nothing stray: no build-mut-*, no .venv, no .new in the tree (git ls-tree); .gitignore +1 line. Scripts quit only their own pid (probe-quit-ours.sh reused; selftest kills only its stub pid); no script names ~/Library/AudioDNA or ~/Library/Audio-DNA.

## Findings
SHOULD-1 test_app_settings.cpp:11-12,223-231 -- POSIX-only (<sys/resource.h>, setrlimit, SIGXFSZ) in a test built on every platform; SIGXFSZ is left ignored process-wide. Guard with #ifndef _WIN32 (builder's N2). VERIFIED by reading.
SHOULD-2 Three edits to EXISTING tests the ruling does not name, all disclosed: tests/test_show_model.cpp:1305 (legacyShow helper drops "version"), tests/test_app_settings.cpp:17-21 (TempSettings dir name Uuid), tests/test_composition.cpp (2 call sites, not "three": the ruling's count is a miscount, the third and fourth callers are src). I judge each necessary and minimal (a fixture built from toVar() now IS a version-2 file by A-3; ruling says M2/M3/M6/M7 "UNCHANGED") -- Harmony should rule it explicitly (builder stop item 1) rather than let it pass by silence.
SHOULD-3 .harmony/probe-one-save.sh:10-14 and probe-one-save.py docstring -- the header says a SCRATCH library folder (AUDIODNA_LIBRARY_DIR) is passed so his folders are not touched, but nothing in src reads AUDIODNA_LIBRARY_DIR yet (git grep over src: 0 hits). The launched test-mode app still lists ~/Library/AudioDNA/compositions and parses each decks/*.json (CompDecksBrowser.cpp scanForFiles / isV2DeckFile) at launch and after every successful save (builder's N1). Read-only, as every earlier probe, but the comment overclaims isolation; reword ("inert until S4b; G-OS4-0 checksums are the guard"). VERIFIED.
NIT-1 ShowFile.h:33-34 -- static_cast<int>(int64 version) wraps: 2^32+2 reads as 2, 2^31 reads as INT_MIN (tag "v-2147483648"). Clamp or reject above INT_MAX. INFERRED (not run).
NIT-2 Gate string "ctest -R safewrite|showfile|showbackup|onesavelint" (ruling s5 G-OS1) matches no LINT case (ctest -R matches case titles; onesavelint is a Catch2 tag) -- builder stop item 2 is right; Harmony should use the hand-over command. INFERRED from catch_discover_tests defaults.
NIT-3 docs/claude/architecture.md: SafeFileWrite.h / ShowFile.h are listed after "└── UndoManager.h/cpp" under a core/ heading still captioned "Undo/redo scaffold"; the tree glyphs are now wrong.
NIT-4 TempDir and bytesOf are duplicated in tests/test_safe_write.cpp and tests/OneSaveFixture.h (Duplicate Code, small).
NIT-5 MainComponent.cpp:3562 writeShow always writes empty keys/layout blocks (loadedExtras ignored). Harmless before S2 / M1 ordering (no file has blocks), but Composition::takeLoadedExtras has no caller until S2 (builder's N5).

## Notes (not findings)
- AS-4 (RLIMIT_FSIZE) proves the stream-status path, not the read-back; the read-back is proven by SW-2 through the seam, and MU-OS-21's live arm is plausibly undecidable (builder stop item 4; the ruling already says "when decidable").
- The probe selftest's stub re-implements the ruled behaviour (circular by nature); the real proof is Harmony's live run. Nothing was built, run or launched by this review.
