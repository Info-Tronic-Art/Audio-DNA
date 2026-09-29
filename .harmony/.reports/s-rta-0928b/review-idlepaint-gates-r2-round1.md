# Reviewer Verdict — idlepaint-gates-r2
STATUS: PARTIAL
VERDICT: FAIL
FILES:
  - .harmony/.reports/s-rta-0928b/idlepaint.md (fix-round-1 section, 42871bd..b1307c7)
  - .harmony/probe-idle-paint.py (J2/J3 code)
  - .harmony/probe-idle-paint.json (identity.maxDelta/edgeSpan)
  - docs/claude/pitfalls.md (Pitfall NN text corrected)
  - (whole-lane, spot-checked) src/ui/LayerStrip.{h,cpp}, src/ui/NativeLayerHost.{h,mm}, src/ui/NativeLayerCache.h,
    src/ui/OverlayWatch.{h,cpp}, src/ui/ClipInspector.{h,cpp}, src/MainComponent.{h,cpp}, src/api/ApiServer.{h,cpp},
    CMakeLists.txt, CLAUDE.md, docs/claude/architecture.md, .harmony/APP-INVENTORY.md
ISSUES:
  MUST (1): J1 (ruling "critic MUST — ADOPT") is not implemented. The fix round diagnosed the root cause rigorously
    (per-pass CGContext/layer instrumentation, 7 suspect toggles, a scratch main+hook build, halves test) and concluded
    the first-display-pass artifact is identical in main and the lane, with no JUCE patch involved — but it then
    self-STOPPED: no code fix was applied and the required gate row `v4_full_pass_identity` (main idle vs lane after
    `POST ui_repaint_all`, RED on 42871bd / GREEN after) does not exist anywhere in probe-idle-paint.{py,json,sh}
    (grep confirms zero occurrences of "v4"). The FOCUS criterion "v4 RED on 42871bd and GREEN after" is therefore
    unmet, not merely deferred with a stub — the row was never written. This is disclosed honestly (STATUS: PARTIAL,
    HANDOFF-NEEDS option A/B/C for Harmony), and the underlying tension it surfaces is real (any lane-side one-shot
    fix that erases the first-pass look would make lane-idle diverge from main-idle, breaking v1/v3 BEFORE-vs-AFTER
    identity) — this is not corner-cutting, but per the review packet's own bar ("FAIL only for a MUST... or a missed
    ruling"), an ADOPTed MUST with no code change and no gate row is a missed ruling. Fix: Harmony must rule J1
    options A/B/C (idlepaint.md fix-round-1 "Options for Harmony") before this lane can be considered complete;
    whichever option is chosen, a `v4_full_pass_identity` row (or an explicit, ruled redefinition of it) must exist
    and run RED→GREEN before merge.
  SHOULD (1): v1/v1b/v2 FAIL at 8 SignalStrip-corner px (1/255, 3x3 span 14-30 < edgeSpan 32) — correctly measured and
    printed by the (correctly implemented) J3 rule, but not yet ruled by Harmony (HANDOFF-NEEDS (b): accept, re-rule
    edgeSpan, or fix the corner AA). Not a MUST since J3's own FOCUS ask ("criterion applied and printed") is met;
    flagging so it isn't lost before merge.
  SHOULD (2): v3_identity_production_masked flake (33 px at 1/255, same column, in both lane-vs-main and main-vs-main
    arms) is diagnosed as main's own launch-to-launch noise but not yet ruled/re-thresholded (HANDOFF-NEEDS (c)).
  NIT (1): J4 portability check only ran on Homebrew GCC 15 + Apple clang; GCC 11 (CI) and MSVC were correctly filed
    as CI-only pre-merge checks, per the ruling's own "if cheap, else file it" — acceptable, noting for completeness.
VERIFIED (independently reproduced/read, not taken on the report's word):
  - No production/test code changed in the fix round (`git diff 42871bd..HEAD -- src tests CMakeLists.txt` empty) —
    matches the report's "No app code changed this round" claim.
  - i1/i2 GREEN, 5 launches each, thresholds unchanged; g4 window-max PASS + CPU demoted to an INFO line printing
    BEFORE (main) vs AFTER per J2, implemented in `gate(..., cpu_gate=False)` and the `g4_routine_before` arm.
  - J3 identity rule matches its stated definition exactly: `identity()`/`edge_mask()` in probe-idle-paint.py compute
    "differs <= maxDelta(1) AND sits at an edge (3x3 span > 32)"; reran the edge_mask logic standalone in Python
    (flat image -> no edges; a hard step edge -> edges only at the two boundary columns) — matches spec.
  - ctest invoked serially both rounds (`ctest --test-dir ... -j1`), 920/920 reported.
  - CLAUDE.md = 24,980 B (<= 25,000 cap); whole-lane docs diff (CLAUDE.md, architecture.md, APP-INVENTORY.md,
    pitfalls.md) is purely additive — nothing removed except two lines that were REWORDED not deleted (the archive
    notes), content preserved.
  - No new mutex anywhere in the lane's src/ diff; `atomic_ref` used once (LayerStrip transport read), commented with
    the s166/ConnectionEngine.h read-strategy name as I2 requires; paint() consumes the cached `transportView_`
    member, never re-reads the model, matching I2's "paint never re-reads the model" requirement.
  - New TEST-ONLY env switches (ADNA_UI_NATIVE_LAYERS, _TEETH, _OVERLAY_WITNESS) are compiled only under
    `#if AUDIODNA_TEST_SERVER`; no stray `.venv` symlink, no leftover ADNA_TEMP/J1DIAG instrumentation in tracked
    src/tests (`git grep` clean); CMakeLists.txt gates NativeLayerHost.mm under `if(APPLE)` correctly.
  - The pitfalls.md NN text was corrected between the r1 and fix-round versions to match the J1 diagnosis (dropped
    the disproven "main's union used to overwrite it" claim, replaced with the verified "first display pass" finding)
    — no overclaiming left in the doc.
METADATA: reviewer=reviewer-idlepaint-r2, builder_packet=idlepaint, date=2026-09-29T05:30:00-04:00
