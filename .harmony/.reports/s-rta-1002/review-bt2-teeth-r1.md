# Reviewer Verdict — bt2-teeth-r1
STATUS: DONE
VERDICT: PASS
HEAD: 7c14e26 (base f89b1fa). Method: git archive of 7c14e26 into a scratch dir (the reviewer fence blocks `git worktree add`), fresh configure (FETCHCONTENT_FULLY_DISCONNECTED, deps from build/_deps), test_device_policy only, `-r junit`, each mutant applied by me to src/audio/DeviceGuard.cpp in the scratch copy and reverted from a pristine copy.

- Unmutated head: 55/55 cases pass. VERIFIED.
- src/ untouched: `git diff --name-only f89b1fa..7c14e26 -- src` is empty (diff touches only tests/test_device_policy.cpp, docs/claude/pitfalls.md and .harmony reports). VERIFIED.
- X1 (`if (true)` at :238): killed. Failing R16 `CHECK(reconciler.lostInput().isEmpty())` and R18 `CHECK(reconciler.lostInput() == "USB Mic")`. VERIFIED.
- X2 (`if (false)` at :180): killed. Failing R16 `CHECK(count(rig.spy.createdPairs, "Built-in Speakers + USB Mic") == 2)`. VERIFIED.
- X5 (`setAudioDeviceSetup(keep, true)` at :230): killed. Failing R11 and R14, both `CHECK(rig.manager->createStateXml() == nullptr)`. VERIFIED.
- X6 (drop `haveDevice &&` at :220): survives 55/55, as claimed. Judged EQUIVALENT. I read JUCE 8.0.4 juce_AudioDeviceManager.cpp: every no-device outcome of setAudioDeviceSetup goes through deleteCurrentDevice(), which clears both setup names (:663-667; call sites :745, :761, :785, :835). The "No such device" return at :771 follows the :761 clear. closeAudioDevice() (:889) keeps the names, and its only src caller is DeviceGuard.cpp:181, which re-opens at once. So with no device `keep` is empty and the keep.isNotEmpty() guard already blocks the restore. The premise CHECKs (M5c :878-879, R10 :1139-1140) pass and would go red if JUCE ever kept the names. INFERRED from reading JUCE source, plus the executed premise checks.
- Post-revert run: 55/55 pass, scratch DeviceGuard.cpp byte-identical to pristine. Scratch wt and build dirs removed (rm -rf; no worktree was created, `git worktree list` shows no extra entry).
- GATE-11 counts in head docs/claude/pitfalls.md, each exactly 1: "the one exception is `DeviceReconciler`", "the open device is STOPPED and nothing restarted it", "tried once per distinct list of allowed inputs", "lost - now listening on", "M5r (JUCE". VERIFIED.
- GATE-11 string "R13 (20 list changes": count is 0 at head AND at base f89b1fa, so this round did not break it. The 6th string in the dispatch is stale. The AM-ruling's intended insert was merged into the existing Guards list as "R6 / R7 / R8 / R13 / R14 / R15 / R16 (adopt a mic / a device, the open mic gone, 20 list changes -> ...)". Both "R13" and "20 list changes" occur there. bt2.md's own GATE-11 list omits this string. Not a regression, so not FAIL. Finding SHOULD: the dispatch's expectation of "exactly once" is wrong, and the lane's GATE-11 text should not be re-cited with it.

FINDINGS: 0 blocking; 1 SHOULD (stale GATE-11 string, above).
METADATA: reviewer=independent-mutant-recheck, date=2026-10-02
