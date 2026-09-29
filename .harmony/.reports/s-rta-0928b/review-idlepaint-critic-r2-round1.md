# Reviewer Verdict — idlepaint-critic-r2
STATUS: DONE
VERDICT: PASS_WITH_NITS

SCOPE: lane/idlepaint, worktree .claude/worktrees/rta0928b-idlepaint, fix-round delta 42871bd..b1307c7
(head b1307c7bdb50029a4d6dca4c148ab3e358df9db5), read via `git -C <worktree> diff/show` only. Delta touches
ONLY `.harmony/.reports/s-rta-0928b/idlepaint.md`, `.harmony/probe-idle-paint.{py,json}`,
`docs/claude/pitfalls.md` — no app/C++ source changed this round (`git diff 42871bd -- src tests CMakeLists.txt`
is empty, confirmed). Focus: critic lens (visual/UX/logic), re-checking round-1's MUST (J1) plus rulings J2-J5.

FILES:
  .harmony/probe-idle-paint.py — new `identity()`/`edge_mask()` comparator (Harmony ruling J3) + g4 cpu_gate=False
    path (J2) + g4_routine_before INFO arm
  .harmony/probe-idle-paint.json — identity{maxDelta:1, edgeSpan:32} added; old cornerClusters/cornerClusterMaxPx/
    cornerMaxDelta keys removed (superseded by J3, not a silent drop — cross-checked against the .py, no residual read)
  docs/claude/pitfalls.md — Pitfall NN text revised (J1 finding folded in), no doctrine removed
  .harmony/.reports/s-rta-0928b/idlepaint.md — fix-round-1 report appended (J1 diagnosis + STOP, J2-J5)

INDEPENDENT VERIFICATION (re-derived, not just read): I do not have a way to launch the app myself, but the
builder's own raw capture PNGs from the diagnosis (S/j1-b1/{base,main}/{s0,s0b,s1,s2}.png) are still on disk, so
I re-ran the pixel-diff math myself (not the builder's script — my own one-off, same J3 formula) on the raw
captures to check the report's central claim independently:
  - lane "base" s0 vs s0b (both natural idle, NO explicit full-repaint trigger, ~1s apart): 76px, all in the fps
    box (live counter). Folders/sliders: 0. => the lane's natural idle capture does NOT drift on its own.
  - lane "base" s0 vs s1 (s1 = immediately after one explicit POST /api/debug/ui_repaint_all): 36002px, folders
    24507 (max 66), sliders 1648 (max 29), text ~9756 (max 1). Reproduces the report's headline number exactly.
  - lane base s1 vs s2 (second repaint_all): only 341px, folders/sliders 0 — confirms it does NOT oscillate;
    once triggered it stays in the new look (matches "any later pass draws the steady look").
  - main s0b (main's own natural, unmodified idle capture, captured ~11s after launch — long enough for dozens
    of main's own periodic full-window-union repaints to have already fired) vs lane base s0 (BEFORE any
    repaint_all): 186px, all fps+1/255 AA noise, folders/sliders 0.
  - main s0b vs lane base s1 (the "after repaint_all" look): 36105px, folders 24507 (max 66), sliders 1648
    (max 29) — i.e. main's real natural idle window matches the lane's PRE-trigger state, not its post-trigger
    state.
  This independently confirms the report's load-bearing claim ("main's 30 Hz union never covered those
  regions" — pitfalls.md NN) rather than just trusting the builder's narrative: main's own continuous idle
  full-window repaints do NOT, in practice, repaint the Files-grid folder icons or the three right-hand TopBar
  slider thumbs back to a different look — main's window sits in the same rendering the lane calls "first pass"
  for its entire idle life, same as the lane.
  - I also confirmed on disk that the two REAL (non-TEST-ONLY) triggers the round-1 critic named —
    `bindingOverlay_->onBindingModeExit = [this]() { repaint(); }` and
    `midiLearnOverlay_->onLearnModeExit = [this]() { repaint(); }` — already exist verbatim in MainComponent.cpp
    at main 5c5f21d (`git show 5c5f21d:src/MainComponent.cpp | sed -n '1830,1850p'`), i.e. pre-existing on main,
    not introduced by this lane.
  - I opened S/j1-zoom.png (before/after folder-icon crop) and S/j1-diffmask-small.png myself: the visible
    difference is edge-antialiasing softness on the folder icon and the three slider-knob rims, not a layout,
    color, or content change — consistent with the report's own characterization.
  Net: round-1's MUST rested on an unverified premise ("main's union overwrites this within a frame" —
  critic-idlepaint-r1.md finding 1) that this fix round's evidence, and my own independent re-derivation of the
  SAME raw files, falsifies. Given the two trigger sites are pre-existing on main and main's own idle window
  never differs from the lane's pre-trigger state, this is not a new visible difference the lane introduces —
  it is a latent, equally-present-on-both-builds AppKit/CoreAnimation rendering quirk, newly exposed (not newly
  created) because nobody had compared a full-pass capture to a resting capture before. I did not myself launch
  a live build to confirm the persistence claim end-to-end (I read only committed disk state + the builder's own
  saved captures); that residual is exactly why this remains a HANDOFF-NEEDS item, not a closed one.

RULING-BY-RULING:
  J1 (critic MUST) — diagnosed and STOPPED with evidence, per the plan's own escape hatch ("if the cause is
    inside JUCE/AppKit and needs a JUCE patch, STOP and report"). No source changed. STATUS remains PARTIAL,
    correctly disclosed (not silently accepted). [OK, with a SHOULD below on phrasing]
  J2 (g4 CPU -> INFO) — done correctly: `gate(..., cpu_gate=False)` still gates on window max (provably RED on
    main, GREEN after) and only demotes CPU to an info line; code-read confirms no early-return bug swallows a
    real FAIL. [OK]
  J3 (identity rule) — `identity()`/`edge_mask()` read correctly implement "≤1/255 per channel AND at an
    edgeSpan>32 AA edge in either capture"; independently re-derived the same boolean logic by hand against the
    raw PNGs above and got matching counts. v0's negative fixture (the 1-px teeth shift) is proven to violate
    the rule (81 clusters, >0) — the rule can fail, satisfying "test that cannot fail" scrutiny. Marginal FAILs
    remain disclosed (8 SignalStrip-corner px at exactly 1/255, S3 also 13 deck-cell px) — HANDOFF-NEEDS (b),
    not hidden. [OK]
  J4 (portability) — checked on Homebrew GCC 15 + Apple clang 17 (both pass); GCC 11 / MSVC unavailable locally,
    correctly FILED rather than asserted. [OK]
  J5 (rebase) — confirmed no-op is correct: `git rev-parse --short main` = 5c5f21d = the lane's own merge-base. [OK]

ALWAYS-ALSO-CHECK:
  - Gates RED on the named arm and able to fail: yes for g4 (main 16.5-17.9ms > 8.0), yes for v0 J3 teeth
    (>0 violations), no regressions found in the gate() code path.
  - Nothing stray: `git status --porcelain` in the worktree shows only the expected `build-lane/` untracked dir;
    no `.venv` symlink; `git grep "ADNA_TEMP\|J1DIAG" -- src tests` is empty (all TEMPORARY diagnostic hooks were
    reverted before commit, as claimed).
  - Real-time rules / no new mutex: unchanged this round (no C++ touched); already reviewed and passed in
    review-idlepaint-juce-r1.md and review-idlepaint-gates-r1.md (both PASS_WITH_NITS) — out of scope for this
    delta, not re-litigated here.
  - Docs additive: `git diff 5c5f21d..b1307c7 -- CLAUDE.md` shows only the two plan-authorized compressions
    (ARCHITECTURE.md/TASKPLAN_V2.md note lines) plus additions; nothing else removed. `wc -c CLAUDE.md` = 24980
    (<= 25000 cap).

FINDINGS:
  SHOULD:
  1. [logic/spec fidelity] idlepaint.md's J1 section states "Root cause: inside AppKit / Core Animation
     (INFERRED)... No JUCE patch is involved" while invoking the STOP clause worded "if the cause ... needs a
     JUCE patch, STOP". These don't literally match (they found no patch fixes it, not that a patch is needed) —
     the STOP is still the right call (any lane-side fix demonstrably breaks v1/v3 identity, per the kick:150
     experiment), but Harmony's Option A/B/C ruling should be made on the actual finding ("unfixable without
     contradicting another gate"), not on the literal STOP wording. Fix: tighten the wording when Harmony rules,
     no code change needed.
  2. [visual] The mechanism is still genuinely INFERRED (medium confidence, per the report's own CONFIDENCE
     line) — 7 toggled suspects (font smoothing, subpixel positioning, interpolation, opaque, sync/syncearly,
     halves, native-layers-off) all left the divergence unchanged, so the actual AppKit/CA step responsible is
     still unknown. Not blocking (the STOP path doesn't require knowing why, only that no lane-side fix is safe),
     but worth naming as the item most likely to surprise a future investigator.
  3. [scope] STATUS is honestly PARTIAL — J1 acceptance, the J3 8px corner-AA fails, and the v3 flake verdict all
     still need an explicit Harmony ruling before this can be considered mergeable. Listed for completeness, not
     as a defect: the report itself asks for exactly this in HANDOFF-NEEDS.
  NIT:
  4. probe-idle-paint.json dropped `cornerClusters`/`cornerClusterMaxPx`/`cornerMaxDelta` (superseded by the new
     `identity` block) — confirmed no other row in the .py still reads those keys, so this is a clean removal,
     not silent dead config.

SUMMARY: 4 files in the fix-round delta (0 app source), 4 findings (0 blocking / 3 SHOULD / 1 NIT). No MUST:
the round-1 critic's blocking finding (whole-window repaint changes folder-icon/slider-rim rendering and it
persists) was diagnosed at the root and shown — by the builder's evidence and by my own independent re-derivation
of the same raw capture files plus a direct disk check of the two trigger call-sites — to be pre-existing,
equally-present behavior on main, not a new regression this lane introduces, and not something a lane-side fix
can remove without breaking the v1/v3 identity gates. Confidence: VERIFIED on the pixel-diff numbers and the
main-5c5f21d source check (re-derived directly by me); INFERRED on the underlying AppKit/CoreAnimation mechanism
(labeled as such by the builder and unresolved by either party). Remaining open items are correctly surfaced as
HANDOFF-NEEDS for Harmony, not shipped silently.
METADATA: reviewer=critic-r2, builder_packet=idlepaint-fix1, date=2026-09-29
