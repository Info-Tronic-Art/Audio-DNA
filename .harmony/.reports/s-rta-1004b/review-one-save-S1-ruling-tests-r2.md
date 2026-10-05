# Reviewer Verdict - one-save S1, lens ruling-tests, round 2
STATUS: DONE
VERDICT: APPROVE (PASS) -- 0 MUST, 0 SHOULD, 1 NIT
REVIEWED: lane/one-save head 93b49ab (code head 608d6fd), fix round = diff 25d5dc9..93b49ab; read through git objects only. Nothing built, run or launched.
METADATA: reviewer=sonnet-5-5, date=2026-10-04

## Round-1 findings, each checked at the head (VERIFIED by reading)
- r1 had 0 MUST. Fixed in code:
  - NIT-1 / data-safety SHOULD-1 (version int cut): src/core/ShowFile.h:27-35 now clamps with juce::jmin to INT_MAX (real fix, not documented away); backupTagFor (:62-68) and Composition::fromVar (Composition.h:1009) both go through versionOf, so the tag becomes "v2147483647" and loadedVersion INT_MAX. SF-10 (tests/test_show_file.cpp:336-352) extended under its unchanged title with 2147483648, 3000000000, 4294967296, 4294967298: asserts isInt64, versionOf, backupTagFor, loadedVersion and an empty migrationNote -- each cut-down value (negative, 0, 2) fails the old code; report shows RED 13 failed assertions then GREEN 274, and MU-OS-22 re-run RED then GREEN with an object recompiled after the revert.
  - SHOULD-1 (AS-4 POSIX): tests/test_app_settings.cpp:11-14 and :207-242 guarded #ifndef _WIN32; SIGXFSZ disposition saved and restored (:219, :228). Ctest count unchanged (1278 = 1252 + 26 on macOS).
  - SHOULD-3 (probe overclaim): probe-one-save.sh header and probe-one-save.py docstring reworded: AUDIODNA_LIBRARY_DIR inert until S4b, the app reads (never writes) his library, guard = G-OS4-0 checksums. Comment lines only; selftest 36 ok per the report.
  - data-safety SHOULD-5 (docs "four other writers"): integration.md and pitfalls.md NN now name all ten replaceWithText callers' classes and say Save Deck As can still overwrite a show.
- Left to Harmony by design, not findings: SHOULD-2 (the four out-of-ruling edits to existing tests / the ruling's "three call sites" is two), NIT-2 (gate string reaches no lint row; builder measured 21 rows, 0 lint). Stop items 1-2 stand; the report's hand-over now says to use the per-binary commands.
- No new defect from the fix: the diff touches only ShowFile.h (+limits include, jmin), two test files, two docs, two probe comment blocks and the report. No mutant, build-mut, .venv or .new in the tree (git ls-tree grep: none). No on-screen text added. Mutant apps rebuilt per report (kept outside the tree).

## Findings
NIT-1 (carry-over, unchanged) docs/claude/architecture.md tree glyph after SafeFileWrite.h/ShowFile.h and the duplicated TempDir/bytesOf (OneSaveFixture.h vs test_safe_write.cpp) -- the builder left r1 NITs untouched by choice. Not blocking.

## Notes
- Windows side of the AS-4 guard is unbuilt (builder says so); four older tests include <unistd.h> unguarded, so the test tree is not Windows-clean either way.
- INFERRED: the clamped-version tag ("v2147483647") wording is Harmony's to rule (builder stop item 7).
