# Reviewer Verdict — idlepaint-critic-r2
STATUS: DONE
VERDICT: PASS_WITH_NITS
FILES: .harmony/probe-idle-paint.py, .harmony/probe-idle-paint.json, .harmony/.reports/s-rta-0928b/idlepaint.md (delta b1307c7..d072ec3); no src/ changed this round (verified by git diff --stat).

## Three hats

VISUAL — PASS. Opened and inspected on disk (not recalled): diff-v3p-r2.png (final shipped state, production idle main-vs-lane, masked) shows a clean window, no stray highlighted regions. diff-v1-S2.png shows exactly one isolated cluster (the SignalBar "Mod 1" thumb), consistent with the report's Mod-1-is-live-content claim — nothing else in the window is touched. diff-v4-r1.png shows the FAIL is confined to the Files-grid folder icons, matching the documented first-pass-vs-steady-pass root cause (fix round 1 J1), not a new/unexplained regression. look-v3p-k1.png (K1 arm, not shipped) visually confirms the report's claim that K1 changes the folder-icon rendering relative to main. No visible difference beyond the disclosed/masked classes was found in the shipped (non-K1) build.

UX — PASS. No src/ touched this round, so popup-over-signal-bar timing, overlay fallback (I1/I4), and animation rates are unchanged from the already-reviewed r1 state. g1_anim_rates and i1/i2/g4 in the final-app gate table (idlepaint.md line ~530-600) show unchanged rates (wf/sb ~29.1-29.2/s, TopBar ~14.9/s) and window-max/CPU medians consistent with prior rounds.

LOGIC — PASS with one open decision flagged, not a defect. K1 (Harmony ADDENDUM 2, ADOPT) was not implemented; the builder stopped it and reported PARTIAL. I independently re-derived the core claim rather than trusting the prose: git diff b1307c7..d072ec3 confirms zero src/ changes (K1's patch is not in the tree — `grep -rn firstPassRepaintScheduled_ src/` returns nothing), and the K1 patch shown in the report matches what would have been required. The stop is evidenced, not asserted: the production measurement table (main s0 vs s1: 0/0 diff; main+one-shot-kick s0 vs s1: 24,507px/66 + 1,648px/29 — exactly the J1 classes) demonstrates main itself never repaints the Files-grid icons or TopBar thumbs at idle in production, contradicting K1's stated premise ("main's 30 Hz union repaints everything"). The K1-arm run then shows v4 GREEN but v3p/v1/v3 RED on the same 13,507px/max-66 class — i.e., K1 and the plan's own v3p/K3 requirement are mutually exclusive on the current code, a genuine logical conflict the builder surfaced rather than silently resolved in one direction. This matches the "diagnose, then STOP with options A/B/C" pattern Harmony already accepted for J1 in fix round 1 — it is a verified, evidenced deviation (task packet's own carve-out), not a missed ruling swept under the rug: STATUS is PARTIAL, NEXT ACTION explicitly asks Harmony to rule A/B/C.

## Gate/teeth check
- v4_full_pass_identity: confirmed RED on the final (shipped) app via the report table and the diff image (13,507px, max 66) — it is a real, currently-failing gate, not toothless.
- v3p_production_idle_identity: confirmed PASS on the final app (0-33px at max 1/255) via report + diff image; confirmed it goes RED on the K1 arm (evidence the test can fail both ways).
- ctest: tail of S/fix2/ctest.log independently confirms "100% tests passed ... 920/920" (matches claim).
- Nothing stray: `git status --short` in the worktree shows only untracked build-lane/; no .venv symlink present; grep for K1's env-hook/instrumentation (firstPassRepaintScheduled_, ADNA_TEMP_KICK_MS, J1DIAG) in src/ returns nothing — the K1 patch and the main+hook scratch build were correctly kept out of the tree.
- Docs: git diff b1307c7..d072ec3 touches no docs/CLAUDE.md (this round is script+report only); CLAUDE.md is 24,980 bytes (<= 25,000, verified with wc -c). The earlier 5c5f21d..d072ec3 CLAUDE.md diff (already reviewed pre-round-2) is additive (one new UI-Patterns bullet + one new pitfall line) with two adjacent bullets reworded/condensed to make byte-budget room — no information removed, only "Note:" prefixes dropped.

## Open items for Harmony (decisions, not code defects)
1. K1: A (ship as-is, v4 stays RED like main) / B (apply K1, rebase identity references) / C (separate AppKit lane). Reviewer has no basis to prefer one — all three are internally consistent with the evidence; A is what is currently shipped and does not regress anything visible versus main today.
2. Mod 1 test-mode meter: is it live content (mask it) or should the probe freeze beatPhase. Low risk either way (occurred 3/26 sequence captures, 0/30 fresh launches, same magnitude in both apps) — SHOULD, not MUST.

## Minor (NIT)
- probe-idle-paint.json v3p.topbarLiveFromLeftPt/WidthPt (322/196) is a few points wider than the cited TopBar::resized offsets (326..512); this is a conservative direction (masks slightly more, not less) so it cannot hide a real regression narrower than declared — no action needed.

SUMMARY: 2 files (script+config) + 1 report reviewed, delta b1307c7..d072ec3. 0 blocking issues found; 2 open Harmony decision points already self-disclosed by the builder (K1 A/B/C, Mod-1 classification) and 1 NIT (mask margin, non-blocking). Confidence: VERIFIED for ctest count, git diff scope, absence of stray K1 artifacts, and the pixel evidence in diff-v3p-r2.png/diff-v4-r1.png/diff-v1-S2.png/look-v3p-k1.png (opened and inspected directly, not taken on the builder's word). INFERRED: that the K1 vs v3p conflict is a true structural conflict rather than an artifact of the probe's own masking choices — I did not attempt a fourth arm to falsify this, since the report's own 6-row production table already brackets it from both directions (main-alone, main+kick, lane-idle, lane+pass).
METADATA: reviewer=critic-r2, builder_packet=idlepaint, date=2026-09-29
