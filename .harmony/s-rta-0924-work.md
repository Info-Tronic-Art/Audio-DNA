# s-rta-0924 work log (secondary)
- boot: handoff s-rta-0923 read; screen-safety law read. Tree: .harmony-version modified (not ours), AGENTS.md untracked (not ours) — leave.
- #1 comp-position units bug: builder dispatched (TDD, scratch dir fail-first). Gate = probe-lane3.sh COMP check (expect 12/12 after).
- #2 step 3: architect critic dispatched -> .harmony/.reports/s-rta-0924/step3-critic.md (S3-M superseded by Lane 3 ManualWrite seams).
- rig note: MINIMAL-profile read-only REPORT_FILE must be under <repo>/.harmony/.reports/ (dispatch gate blocks memory/.reports/ in a foreign repo).
- Boris 2026-09-24: Bluetooth headset crash re-test DEFERRED ('another time, not that important') — stays on loose-ends as low priority, not owed this session.
- #1 builder DONE: ScalarMath::posPxToCompUniformX/Y (px/1920, px/1080), Renderer applyCompTransform uses them for position+anchor; new test_comp_transform_units (5 cases). ctest 362/362 (builder-reported).
- #1 HARMONY LIVE GATE: probe-lane3.sh 12 PASS / 0 FAIL (was 11/1). COMP frames no longer black: comp_a mean 77 / black band right 8.1% cols; comp_b black band left 2.9% -> real horizontal shift. Looked at comp_a.png. App gone, 0 Output windows. Reviewer pending -> then commit.
- #2 critic DONE: BUILDABLE-WITH-FIXES, 5 blocking, A1-A8 adopted; banner added to plan. Report .harmony/.reports/s-rta-0924/step3-critic.md.
- step3 orchestration: W1 workflow = lanes S3-A (recording), S3-C (REST), S3-G (gate assets) in worktrees, each builder->reviewer->1 fix round. Harmony merges, builds, ctest. W2 = S3-B (MainComponent) + S3-D docs. Then Harmony runs probe-step3.sh.
