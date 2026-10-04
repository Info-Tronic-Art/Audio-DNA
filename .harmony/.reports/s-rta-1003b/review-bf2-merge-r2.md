# Reviewer Verdict -- bf2 merge-in (M0) r2
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
PINNED: lane/bf2 head 68abc16d71f3825cb31fbbb0a8a00939a4ee63e6 (fix round db950ab..68abc16, 10 files; merge 8a5831d unchanged)
ISSUES: 0 blocking / 1 SHOULD / 1 NIT

## Round-1 findings (merge lens)
r1 had no MUST. Its one SHOULD (testing-eyes.md:72 by-name `pkill -f` recipe) is FIXED as found, not documented away: dc59573 rewrites
step 5 to "quit only the pid YOU launched ... source .harmony/probe-quit-ours.sh, record_ourpid, quit_ours" (VERIFIED, diff read).
r1 NIT (APP-INVENTORY 16/17 OSC) untouched, pre-existing, still a NIT.

## Re-checks at the head (VERIFIED: git grep / show on 68abc16)
- Fix round touched src only in BeatLead.cpp:195-196 and BeatLead.h:35-36: comment-only hunks (the diff shows no code token changed). No merge-resolution file (CMakeLists, MainComponent, ApiServer, Renderer, routes) is in the fix diff, so r1's route/both-sides findings stand.
- Harmony's H-1 command `git grep -n 'Pitfall NN' HEAD -- . ':!.harmony/.reports' ':!.harmony/HANDOFF-ARCHIVE.md' ':!.harmony/s-rta-1002b-work.md'` prints nothing (rc 1).
- CLAUDE.md = 24,370 bytes (<= 25,000); pitfall index order 63, 64, 65, 66, 67, 68 (CLAUDE.md:219-224).
- [tsan] count re-pinned with shown arithmetic (probe-tsan-unit.sh:27-33): 5 + 6 = 11 (M0), + the 5th test_sync_witness case (pipelineUs, S3f) = 12. I counted the tagged TEST_CASEs at the head: layer_runtime_race 4 + manual_scalar 1 + analysis_sync_thread 1 + sync_witness 5 + sync_offset_controller 1 = 12 = EXPECTED_TSAN_CASES=12; TARGETS lists all five. (r1's "11" was true at c45b579; the 12th case is the S3f addition, consistent.)
- BY-NAME-QUIT SWEEP re-run on the whole head (*.sh, *.py outside .harmony/.reports and probe-quit-ours.sh; docs; tests/visual):
  every code-line osascript / kill is by pid ("first process whose unix id is $PID" in gate-s165.sh:62, probe-deck-path.sh:133, probe-tempo-silence.sh:81), by ask_ours_to_quit behind refuse_foreign_start + OURPID (gate-s165.sh:44-65), or a self-forked osascript child (probe-async-load.sh:75, probe-btguard.sh:90: `pkill -P "$osa"`, kills only the script's own child). All other pkill/pgrep hits are comments, the "APP STILL RUNNING" message text, or `pgrep -x clang` busy-checks. tests/visual/test_output_window_level.py kills only pids passing _is_ours(pid, launch_time, known) (:1055-1078). The lane scripts probe-sync.sh / probe-sync-venues.py / probe-sync-selftest.py have no quit-by-name; the self-test has no subprocess / os.system / kill / osascript at all. Zero unguarded by-name quit: no MUST.
- Nothing stray: ls-tree of the head has no .venv, .orig, .rej, .bak, build-mut-*, mutant file or symlink outside the dated .harmony/.reports records. `/build-mut-*/` is now in .gitignore (VERIFIED, diff); the untracked mutant dir build-mut-r7 stays on disk only, as the lane report says.
- New test: .harmony/probe-sync-selftest.py (27 cases) imports probe-sync.py behind `if __name__ == "__main__"` (probe-sync.py:1291), launches nothing, and has a stated RED path (PROBESYNC_SELFTEST_TARGET=<mutated copy>; 9 mutant REDs reported at bf2-delta.md:900-903). It reads back the probe's own PASS/FAIL counters and printed lines, so it drives the real verdict functions. The RED runs were not re-run by me (no execution allowed): INFERRED from the lane report. It is tracked despite .gitignore:65 (`.harmony/*`): it appears in the diff stat (force-added).
- Docs: analysis.md [timing] note now states the observed readings (0.65-0.82, then 0.97-1.01) and "cause not established" (matches H-8); performance-controls.md / APP-INVENTORY say what a user can reach for the Relative CC (engine only; learn makes every CC Absolute) -- an honest claim, the door left to Harmony.

## Findings
SHOULD .harmony/gotchas.md:122 ("Kill with `pkill -f Audio-DNA`") and :136 (`pkill -9` and re-`open`), plus the dated pkill remedies in other .harmony notes, are by-name kill advice agents copy. Pre-existing on main, not a merge defect and not a script; the lane filed it itself (bf2-delta.md:788, :817). Fix (Harmony's, any time): reword to "kill the pid you launched / probe-quit-ours.sh". Not a MUST unless Harmony counts doc recipes in the sweep.
NIT BeatLead.cpp:194-199 still carries the vestigial `flags.resyncs != lastResyncs_` branch (both arms now give the same state; comment says so; filed as debt in d172156). Not a merge issue.

## Not verified
ctest totals (1330/1330, 12/12) and the self-test's REDs are lane-reported, not run by me (read-only review). G1 [timing] cause is not established; H-8 covers it.

METADATA: reviewer=claude-sonnet-5-5, builder_packet=bf2-merge-r2, date=2026-10-03, confidence: VERIFIED for the fix diff, H-1 grep, size, order, tsan count, sweep, stray; INFERRED for test runs.
