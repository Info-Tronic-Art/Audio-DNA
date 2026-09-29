# Reviewer Verdict — idlepaint-gates-final
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES

FILES REVIEWED (git -C .claude/worktrees/rta0928b-idlepaint diff 5c5f21d..34a8cf8, 29 files):
plan-idlepaint.md (incl. all 3 HARMONY ADOPTION addenda), lane report idlepaint.md (all 3 sections: r1, fix round 1,
fix round 2), src/ui/{UiPaintCounters.h,NativeLayerCache.h,OverlayWatch.{h,cpp},NativeLayerHost.{h,mm},LayerStrip.{h,cpp},
ClipInspector.{h,cpp}}, src/MainComponent.{h,cpp}, src/api/ApiServer.{h,cpp}, src/ui/TopBar.cpp, CMakeLists.txt,
tests/{test_native_layer_cache,test_overlay_watch,test_layer_strip_transport_view,test_clip_inspector_paint_key}.cpp,
tests/CMakeLists.txt, CLAUDE.md, docs/claude/{pitfalls,architecture}.md, .harmony/APP-INVENTORY.md,
.harmony/probe-idle-paint.{sh,py,json}, plus disk evidence under scratchpad/idlepaint/{runs,fix2}/*.log and
scratchpad/idlepaint/fix2/p1/* (K1 production pairs).

## MUST finding (blocks approval)

**"v4 is INFO after 34a8cf8" is FALSE — disk-verified.** Addendum 3 ruling L2 says: "v4_full_pass_identity becomes an
INFO row (it documents the first-pass class; it is not a lane-vs-main identity)." Commit 34a8cf8's message claims
exactly this ("v4_full_pass_identity is INFO (addendum 3 L2)"). But the diff of that commit
(`git -C <wt> show 34a8cf8 -- .harmony/probe-idle-paint.py`) is a ONE-LINE COMMENT ADDITION only:
```
+    # Harmony addendum 3 L2: v4 documents the first-display-pass class (identical in main); v3p is the identity gate.
```
`row_v4()` still ends with `(ok if all(r[2] == 0 for r in res) else no)(f"v4_full_pass_identity (K3): ...")` — unchanged
from before the addendum. `ok`/`no` (probe-idle-paint.py:97-102) print `PASS `/`FAIL ` and increment the global
PASS/FAIL counters that gate the script's exit code (`sys.exit(1 if FAIL else ...)`, line 1148); there is no `info()`
call for v4 anywhere. `v4_full_pass_identity` also remains in `DEFAULT_ROWS` (only `x1`/`v1b` are excluded from the
default set), so a default invocation of `probe-idle-paint.sh` still runs it as a gate.

Reproduced from the evidence already on disk, no re-run needed: `scratchpad/idlepaint/fix2/runs/F3.log` (the row set
`v4_full_pass_identity,v3p_production_idle_identity,v0_capture_teeth,v1_identity_test_mode,v2_identity_fallback,
v2b_fallback_frames,v3_identity_production_masked` against the FINAL build-lane binary) prints:
```
FAIL  v4_full_pass_identity (K3): test mode, card -- idle look vs after a whole-MainComponent pass outside the fps
mask, 2 launches: r1 35393 px differ (max delta 66), 13507 violate K2 ... | r2 35393 px differ ...
FAIL  v1_identity_test_mode S2 (card): BEFORE vs AFTER outside the fps mask -- 237 px differ (max delta 119), ...
16 PASS / 2 FAIL / 0 SKIP (06:12:32, ...)
PASS  app terminated

PROBE-IDLE-PAINT RED
```
So on the FINAL app, a default gate run is **RED**, printing `FAIL` (not `INFO`) for v4, and the wrapper script exits
non-zero and prints "PROBE-IDLE-PAINT RED" — directly contradicting both the addendum's ruling and the commit message
that claims to satisfy it. This is a claim/reality mismatch on a gate result, exactly the class of finding dimension 7
calls for hunting: the code comment overclaims compliance with a ruling that the code does not implement. Fix: change
`row_v4()`'s terminal call from `(ok if ... else no)` to `info(...)` (or otherwise remove it from `DEFAULT_ROWS`/from
`sys.exit`'s FAIL accounting), matching what the comment and commit message already assert.

A second, related but separate open item in the same log: `v1_identity_test_mode S2` FAILs on the final app (the "Mod 1"
SignalBar meter class, 160 px at max 119) — this is explicitly flagged by the lane report itself as unresolved
(HANDOFF-NEEDS (b), fix round 2) and is NOT covered by any of the three addenda. It is a second reason the shown F3.log
run is RED even independent of the v4 defect above. Not a new discovery, but it means "GREEN on the final app" cannot
be asserted for a straight default run of the probe today — only for the specific ruled-and-passing rows (i1/i2/g4
window max, v2b, v3p, K2 rows other than v1 S2), which I did verify individually (below).

## Checks that PASS (individually verified against raw logs, not just the lane report's prose)

- i1/i2/g4 RED on main, GREEN on final, 5 launches each: confirmed via `runs/R1.log` (i1 FAIL 21.8ms/347ms/s),
  `runs/R2.log` (i2 FAIL 16.4/499.6), `runs/R3.log` (g4 FAIL 19.3/351.8), and `fix2/runs/F1.log`/`F2.log` (i1 PASS
  4.5ms/114.2ms/s x5 launches r1-r5, i2 PASS 4.4/111.2 x5, g4 PASS window-max 5.6ms x5, CPU INFO 158.2 vs main 327.5).
- v2b teeth: `fix2/runs/F3.log` shows `covered 0` on the real (async-hide-off) arm and `covered 11` on the
  `ADNA_UI_NATIVE_LAYERS_TEETH=asynchide` arm — the witness genuinely discriminates, not toothless.
- K2 identity rule (<=1/255 anywhere, J3's anti-aliased-edge clause dropped) is the one actually wired into `identity()`
  in probe-idle-paint.py and used by v1/v1b/v2/v3/v3p/v4 — matches commit fd456e5 and the addendum text.
- v3p PASS with 2 launch pairs: `fix2/runs/F3.log` line 109, raw: `r1 0 px differ ... | r2 33 px differ (max delta 1),
  0 violate K2 ... in 0 cluster(s)` — both pairs pass K2.
- K1 withdrawal evidence is real: `scratchpad/idlepaint/fix2/p1/{main,mh,lane}/*.png` + card.json + logs exist on disk;
  the report's production-pair table (main s0 vs s1 = 0/0, main+hook s0 vs s1 = 24507/66 + 1648/29, main s1 vs lane
  idle = 0/0, main s1 vs lane after full pass = 24507/66) is consistent with those captures being real artifacts, not
  fabricated numbers — the emoji/slider pixel counts recur identically (24507/66, 1648/29) across independently-run K1
  arm rows in both fix-round-1 (`S/j1-b*`) and fix-round-2 (`S/fix2/p1`, `S/fix2/K1.log`) evidence, which is the kind
  of cross-run determinism you'd expect from a real AppKit effect, not a scripted result.
- ctest 920/920 serial: confirmed on disk, `scratchpad/idlepaint/fix2/ctest.log` tail: "100% tests passed, 0 tests
  failed out of 920".
- Docs additive vs 5c5f21d: `git diff 5c5f21d..34a8cf8 -- CLAUDE.md docs/claude/pitfalls.md docs/claude/architecture.md
  .harmony/APP-INVENTORY.md` shows only additions plus two intentional, in-plan compressions (the ARCHITECTURE.md/
  TASKPLAN_V2.md archive notes, ~pre-planned "PAY" lines) that preserve the same information more tersely — nothing
  substantive removed.
- CLAUDE.md size: `git show 34a8cf8:CLAUDE.md | wc -c` = 24,980 B, under the 25,000 B cap.
- No TEMPORARY hook left in the tree: `git grep -n "ADNA_TEMP\|J1DIAG\|firstPassRepaintScheduled_"` at 34a8cf8 across
  src/ and CMakeLists.txt returns nothing — the K1 patch, the J1 diagnostic, and the v1b freeze hook are all absent
  from the committed tree (only ever built into scratch copies per the report, consistent with the disk evidence
  directory names `app-hook`, `app-hook2`, `app-mainhook`, `app-diag` living outside the worktree).
- TEST-ONLY routes (`/api/debug/ui_paint`, `ui_test_menu`, `ui_native_fallback`, `ui_repaint_all`) are correctly gated
  behind `#if AUDIODNA_TEST_SERVER` in both ApiServer.h and .cpp — no production exposure.

## Code quality (secondary to the gate-verification task, spot-checked)

- `src/ui/NativeLayerCache.h`: clear state machine (Native/RestorePending/Fallback/FallbackPending), well-commented,
  matches the plan's I1 synchronous-fallback ruling and the 5db815f fix (non-opaque widget repainted beneath a
  returning layer). No issues found.
- `src/ui/LayerStrip.cpp` (I2): `std::atomic_ref<double>` read once per tick into `TransportView`, comment names the
  s166 read-strategy as required by the ruling ("s166 spec L5 names two acceptable reads ... This is the atomic_ref
  read"). Matches ruling I2 exactly, including the counter-based 95%-advance-ratio witness (g2 gate, verified PASS
  1.000 / 0.993 in two independent runs).
- Test registrations in tests/CMakeLists.txt follow the existing project convention (mirrors adjacent target blocks)
  exactly (include dirs, link libs, compile defs, sanitizers, catch_discover_tests).
- No dead code / no unreferenced surfaces spotted in this diff; all new TEST-ONLY surface is reachable only from the
  probe and gated correctly (Harness-wiring: confirmed via ApiServer route registration and CMake APPLE block for
  NativeLayerHost.mm).

## SLIM

No excess found: every new file/route/counter is read by either the probe script or a ctest; nothing looks
speculative or unreferenced. Not applicable beyond the above.

## Disposition

REQUEST_CHANGES on the single MUST above (v4 must actually become non-gating — `info()` not `ok/no` — to match
commit 34a8cf8's own claim and addendum 3 L2; trivial one-line fix, already isolated and identified). Everything else
checked (i1/i2/g4/v2b RED-main/GREEN-final, K2, v3p, K1 evidence, ctest, docs, CLAUDE.md size, no leftover TEMPORARY
hooks) is disk-verified and PASSES.

SUMMARY: 29 files (diff) + 3 report sections + probe scripts + scratchpad evidence, 1 blocking issue (v4 mislabeled as
INFO but still gates the exit code / still prints FAIL on the final app), 1 pre-existing unruled item noted for
Harmony's attention (v1 S2 Mod-1-meter FAIL, not caused by this commit range, already flagged by the lane report's own
HANDOFF-NEEDS and not addressed by any of the three addenda).
