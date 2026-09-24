# Reviewer Verdict — cleanup
STATUS: DONE
VERDICT: APPROVE
FILES: src/recording/RecorderHost.h, src/recording/RecorderHost.cpp, src/MainComponent.cpp, src/api/ApiServer.h, tests/test_recorder_host.cpp, .harmony/probe-step3.sh
ISSUES: none blocking.
- Field removal verified: `grep -rn analysisRate|rateMismatch src/ tests/` shows zero live code refs (only comments/docs); removal is comment-only + field deletion, no behavior change (VERIFIED).
- JSON: rateMismatch was already retired from onPerfStatus's published keys pre-lane (JSON uses rateChangedSinceArm); this cleanup only fixes now-stale comment wording in MainComponent.cpp/ApiServer.h — no JSON shape change (VERIFIED).
- New tests: cap test asserts markers==8 then ==10 (2 carried) matching RecorderHost.cpp:22/438-444 exactly (kMaxOnsetMarkersPerTick=8, baseline advances by n only) (VERIFIED by reading impl). Wrap test asserts unsigned delta (kNearMax->3 = 6) with no flood, matches unsigned-subtraction comment at :438 (VERIFIED).
- Baseline-limitation comment (RecorderHost.h onsetCountBaseline_ block) accurately scopes the ~1-tick window and correctly declines to fix it in-lane; consistent with test 20's existing "first tick = 0 markers" behavior.
- probe-step3.sh: bash -n passes; both SKIP->FAIL edits name the exact remediation (venv/pip install line); crash-readability opt-in SKIP (line 670, STEP3_RUN_CRASH_TEST gate) is untouched, confirmed via grep.
METADATA: reviewer=reviewer-agent, builder_packet=cleanup, date=2026-09-24
