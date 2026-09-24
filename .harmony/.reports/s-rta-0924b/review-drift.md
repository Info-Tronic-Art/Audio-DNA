# Reviewer Verdict — s-rta-0924b drift
STATUS: DONE
VERDICT: PASS

## Verdict
PASS. `lane/drift` (commit bffa2d9, worktree `/Users/boriskarpman/projects/rta-wt-drift`) faithfully implements `plan-long-drift.md` E1-E6 plus all three of `critic-long-drift.md`'s MINOR amendments (code amendments #1 lastError-early-break and #2 `s_err = None`; amendment #3 is a plan-doc wording addition, not a code change — see MINOR note below). Scope, thread-safety, and commit hygiene are all clean. Only `.harmony/probe-step3.sh` is touched — no C++, no RT hot path, no other files.

## Findings

FILE: .harmony/probe-step3.sh

[OK] Spec/claim fidelity: `bash -n .harmony/probe-step3.sh` is clean (verified directly, ran twice). shellcheck not installed on this machine — matches builder's own noted deviation; not blocking (no shellcheck anywhere else in this repo's CI either, per grep of the tree).

[OK] Spec/claim fidelity (critic amendment #1, RecorderHost.cpp:403-415 self-stop claim): independently re-read `src/recording/RecorderHost.cpp:395-415` — confirmed the tap self-stop edge detector sets ONLY `lastError_`, never `recording_`. The new poll loop (probe-step3.sh:743-754) parses BOTH `recording` and `lastError` out of a single python one-liner and its `case` breaks early on `True|true` + non-empty `lastError` OR `False|false` — this is exactly the amendment the critic asked for, and it is correctly wired to the failure mode that actually occurs in source. Old (pre-amendment, plan-literal) poll loop would have burned the full wall time on this exact failure mode — confirmed by reading the plan's own unamended snippet (plan lines 272-279) side by side.

[OK] Correctness (critic amendment #2, `s_err` guard): `s_err = None` inserted immediately before `if n_pts > 2:` (probe-step3.sh:470, in the diff at `@@ -426,6 +467,7 @@`), and P2's `minutes_for_0p5ms` computation guards with `len(matched)>2 and drift_stderr_ms>0.0 and s_err is not None` (probe-step3.sh:502-503) — exactly the critic's amendment, not the plan's original (weaker) guard.

[OK] Spec/claim fidelity (E5 hoist, byte-identical claim): re-derived independently, not from the builder's or plan's transcription. Extracted the ACTUAL pre-lane inline python from `git show main:.harmony/probe-step3.sh` (not retyped) and ran it against the run-8 take on disk (`~/Documents/Audio-DNA/Takes/step3gate1.adna-take`, `~/Documents/Audio-DNA/Audio/87e3...` asset): output `mean_offset_ms=33.22 drift_ms=-0.14 drift_stderr_ms=2.26 p95_jitter_ms=12.11 n_markers_raw=135 n_dupes=0 n_matched=135 n_spurious=0 n_grid_in_range=134 pct_matched=100.7 n_peaks=130`. Then extracted the new `t2_align` function from the lane branch via `sed -n '/^t2_align(){/,/^}/p'` and called it with no `M` arg: output byte-identical. Also ran it with `M=2 W=30` and `M=10` — both match the plan's F15/section-6 documented values exactly, including `minutes_for_0p5ms=22.8` and the M=10 "window unavailable" shape (`n_win_last=0`). This is the strongest claim in the builder report and it holds under independent re-derivation, not just re-reading the diff.

[OK] Placement / scope completeness: `git diff main...lane/drift -- .harmony/probe-step3.sh` shows exactly 7 hunks, all within the documented anchors (header comment after :94, config block after :107, WAV-gen after :165, precondition after :170, section-9 hoist :426-501, section 11L insertion between :634 `fi` and :637 `# --- 12.`). Section markers confirm 11L lands after `# --- 11. overdub safety` and before `# --- 12. crash-readability`, matching plan E6 exactly. Sections 10, 12, 13 are outside every hunk range — confirmed untouched (not just by re-reading, the diff itself proves it: git only emits hunks where content differs).

[OK] `gen-click-wav.py --duration-s`: confirmed present and unmodified (`--duration-s`, default 120.0, argparse line 64) — builder's claim that `gen-click-wav.py` needed no changes holds.

[OK] Bash 3.2 compatibility (plan risk R8): directly executed `$(( LLEFT<30 ? LLEFT : 30 ))` under the system's actual `/bin/bash` (GNU bash 3.2.57, confirmed via `bash --version`) with LLEFT=45 and LLEFT=10 — both produce correct results (30, 10). The `set -u` unset-var fallback `${LMIN_NEEDED:-?}` also verified safe under `set -u`.

[OK] Commit hygiene: single commit `bffa2d9`, message accurately describes what changed (does not overclaim — attributes the byte-identical verification, names the specific amendments), only `.harmony/probe-step3.sh` touched (196 insertions / 4 deletions, matches `git show --stat`), no unrelated edits, no orphaned files.

[OK] RT / hot-path rules: N/A — no C++ touched, confirmed by `git diff --stat` (single file, bash script). No new mutex/alloc/thread question arises.

[OK] Default-run no-op: `STEP3_LONG` unset → `LONG="0"` → the new `if [ "$LONG" = "1" ]` block at section 9 (extra WAV gen), section 1 (precondition), and section 11L (record/poll/align) all skip; section 11L's `else` branch prints one `skip` line, matching the existing shape of the crash-test row (:838-ish `STEP3_RUN_CRASH_TEST`). No renumbering of existing sections.

[MINOR] Scope/plan-fidelity nit — critic amendment #3 not literally applied to `plan-long-drift.md`: the critic's third amendment asked for a documentation sentence to be ADDED to the plan's own section 6 ("R5 remains unverified until S5"). `git diff main...lane/drift` touches ONLY `probe-step3.sh` — `plan-long-drift.md` is unchanged (it's a prior docs commit, 10db979). The commit message claims all three critic amendments were incorporated, which is true for the two CODE amendments but the doc-wording amendment was never applied to the plan file itself. Substance is not lost — the builder's own report text explicitly states "R5 ... unverified by any cheap self-test, only by the real S5 run" — but the plan document as a standalone artifact does not carry the critic's requested correction. Non-blocking: no code or test behavior depends on it, and the intent reached the handoff via the builder report instead.

## Not verified in this review (out of scope / requires live app)
- Real `STEP3_LONG=1` run (S4/S5 from the plan) was not executed — task explicitly scopes this to source review, and no app was launched (App/GUI launches are Harmony's live gate, not this review's). R2/R3/R5/R6 (WARN-lines-expected-to-fire, disk pressure, periodic-save jitter, mid-run rate change) remain exactly as flagged by the plan/critic — carried forward, not retired here, consistent with the builder's own "Open risks" section.

METADATA: reviewer=reviewer-agent, builder_packet=s-rta-0924b, date=2026-09-24
