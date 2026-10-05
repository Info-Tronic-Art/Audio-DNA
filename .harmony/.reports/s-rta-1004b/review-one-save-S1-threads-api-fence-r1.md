# Reviewer Verdict - one-save S1, lens threads-api-fence, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) - no MUST
REVIEWED: lane/one-save head 25d5dc9 (code head 2d3cdbb), base 8b464a6; read through git objects only; nothing built or run.

THREADS (VERIFIED by reading): every new write runs on the message thread. writeShow's callers are saveComposition (menu / browser callback),
saveCompositionTo (chooser callback, or the route's MessageManager::callAsync), Collect Media (chooser callback). AppSettings::update's two callers
(MainComponent.cpp:2488, OutputManager.cpp:301) are message-thread UI code and were replaceWithText writers before. No new mutex/atomic/lock in any
changed file; SafeFileWrite.h / ShowFile.h are header-only juce_core with a "message thread only" contract. Composition::toVar (HTTP thread,
GET /api/composition) gained only the constant "version" write and lost outputDisplay; loadedVersion/loadedExtras are written by fromVar/loadFromFile
on the message thread (loadComposition, MainComponent.cpp:3495) and carried through composition_ = std::move(s->comp) (MainComponent.cpp:3360).
GET /api/debug/show_file builds its JSON ON the message thread behind a WaitableEvent with a 2 s wait (ApiServer.cpp handleDebugShowFile) - the same
pattern as handleDebugUiText (ApiServer.cpp:2074). POST then GET are FIFO on the message queue, so a GET sees the finished save.
ROUTE GUARDS (VERIFIED): route registration lies inside #if AUDIODNA_TEST_SERVER (ApiServer.cpp 299-346), the handler inside 1837-2505, the handler
declaration inside ApiServer.h 302-336. The std::function members (ApiServer.h) and the MainComponent assignments/helpers (debugPlainSave,
showFileVar, lastSave_) are unguarded - the existing pattern for onDebugSaveComposition - and unreachable without the route.
ON-SCREEN TEXT (VERIFIED): none added. The only boxes are the existing "Save failed: <path>" (suppressed in test mode) and the "Saved: ..." label.
DOCS (VERIFIED): integration.md, architecture.md, pitfalls.md ("NN.") changed; CLAUDE.md not touched (the row does not name it).
PROBE (VERIFIED): probe-one-save.sh sources probe-quit-ours.sh, uses refuse_foreign_start / record_ourpid / quit_ours; no pkill/killall/kill-by-name
(grep over all five probe files: only the selftest kills its own stub's $!). Process matching by ucomm. Everything written is under <out> or ONESAVE_FULLDISK.
STRAY (VERIFIED): no mutant/instrumentation in the src diff; no .venv/build-mut in the tree; /build-mut-*/ added to .gitignore.
SOURCE-vs-RULING: A-1 (read-back before swap), A-2 (copy first, fail-closed, "other" tag), A-3 (integer >= 2), A-4 (only outputDisplay), A-15
(.unreadable then verified write) all implemented as ruled. LINT-1 position check and LINT-7 are real (cannot be satisfied by a bare call).

FINDINGS
1 SHOULD  File outside the S1 row: tests/test_show_model.cpp:1305 (+1 line, helper legacyShow removes "version"). Ruling line 646 says M2/M3/M6/M7... stay
  GREEN UNCHANGED. The edit is in a fixture helper, no case body or expectation; it is forced by A-3 (a toVar-derived fixture is now a version-2 file).
  Builder reported it as stop item 1. Harmony to rule it in.
2 SHOULD  probe-one-save.sh:12-13 and :67 overclaim. AUDIODNA_LIBRARY_DIR is read by NOTHING in src at this head (grep: only plan/ruling/report/probe text);
  CompDecksBrowser::getCompositionsDir()/getDecksDir() (CompDecksBrowser.cpp:322-331) still point at ~/Library/AudioDNA/{compositions,decks}.
  The launched app lists that folder after every successful save (MainComponent.cpp:3613 and :3638 refresh()) and scanForFiles PARSES each deck file
  there (isV2DeckFile -> loadFileAsString). Read-only, pre-existing in every probe (builder N1, which says only "lists"), but the header's "SCRATCH library
  folder ... not named by this probe" is untrue for Harmony's run. Fix: say it is a no-op until S4b and that the app reads (never writes) Boris's folder.
3 SHOULD  Gate string. Ruling G-OS1 `ctest -R "safewrite|showfile|showbackup|onesavelint"` matches no lint row (ctest -R matches case TITLES; onesavelint
  is only the Catch2 tag; titles at test_one_save_lint.cpp lines 103 and 135 do not hold it) - a green run proves nothing about LINT-1/7. INFERRED
  (ctest semantics), builder's stop item 2. Harmony should use the builder's command (test_one_save_lint "[onesavelint]").
4 NIT  .gitignore (+/build-mut-*/) is outside the row; harmless and reported (F6).
5 NIT  showfile::versionOf (ShowFile.h, versionOf): static_cast<int>(int64) narrows. A hand-edited "version": 4294967296 wraps to 0 and would be
  converted as an old-shape file (4294967297 -> 1). A backup is still made first (tag v0/v1), so no data loss. INFERRED (not run). Fix: clamp or return
  the int64 compare result, e.g. cap at INT_MAX.
6 NIT  docs/claude/architecture.md core/ tree: UndoManager line keeps its "└──" glyph although two siblings now follow.
7 NIT  Probe files are mode 100644 (probe-one-save.sh/.py/-stub.py/-selftest.sh) while probe-boxes.sh is 100755; every instruction invokes them with bash.
8 NIT  probe-one-save.sh:6-7 comment "the /api/debug/* routes are test-mode only": they need the test-server BUILD, not --test-mode (ApiServer.cpp:296-300 says so).
9 INFO  Collect Media's writeShow result is dropped (MainComponent.cpp:6751) - as before and as ruled (no new failure text). N4 (LINT-1's bindingManager_
  exemption) and N6 (AppSettings.h stale comment) stand as the builder listed them.

NOT RUN (labelled): RED-then-GREEN of each row rests on the builder's raw lines; I read the SB, AS and LINT test bodies (they drive real
saveWithBackup / Composition::saveToFile / AppSettings::update on temp files, and AS-4 cuts the write with RLIMIT_FSIZE and restores it) and found no toothless case.
SUMMARY: 27 files, 0 blocking, 3 SHOULD, 5 NIT, 1 INFO. Confidence VERIFIED for threads/guards/fence/probe quit rules, INFERRED for items 3 and 5.
METADATA: reviewer=reviewer, builder_packet=one-save-S1, date=2026-10-04
