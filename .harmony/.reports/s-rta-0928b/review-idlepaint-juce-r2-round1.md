# Reviewer Verdict — idlepaint-juce-r2
STATUS: DONE
VERDICT: APPROVE
REVIEWED: lane/idlepaint fix-round delta 42871bd..b1307c7bd (worktree .claude/worktrees/rta0928b-idlepaint);
whole lane 5c5f21d..b1307c7bd for docs/CLAUDE.md-additivity and byte-cap checks.
FOCUS: J1 diagnosis definitiveness/root-cause-first; JUCE-source claim fidelity vs pinned 8.0.4; ruling
conformance (J1-J5); stray artifacts; real-time rules; docs additivity; CLAUDE.md <= 25000 B.

FILES REVIEWED (full diff read):
  .harmony/probe-idle-paint.py (both fix-round commits, full diff)
  .harmony/probe-idle-paint.json (identity block)
  docs/claude/pitfalls.md (a965a98's 1-line text correction)
  .harmony/.reports/s-rta-0928b/idlepaint.md (both report sections: r1 + fix round 1)
  git diff 42871bd..b1307c7 -- src tests CMakeLists.txt (confirmed EMPTY -- "no app code changed" is literal)
  git diff 5c5f21d..b1307c7 -- docs CLAUDE.md .harmony/APP-INVENTORY.md (whole-lane docs additivity)

VERIFIED ON DISK (re-derived, not taken on report's word):
  - `git -C <wt> diff 42871bd..b1307c7 -- src tests CMakeLists.txt` is empty: the "no app code changed this
    round" claim is literally true.
  - `ctest --test-dir build-lane -N` -> Total Tests: 920 (registered count matches claim); ran
    `ctest --test-dir build-lane -j4` live -> 100% passed, 0 failed, 920/920 (re-executed, not just read from log).
  - JUCE 8.0.4 pinned in CMakeLists.txt (`GIT_TAG 8.0.4`); a matching checkout exists at
    build/_deps/juce-src (commit 51d11a2, `git describe` = 8.0.4). Spot-checked the pitfall-NN citations still
    live in the fix-round text: `repaint(const Rectangle<int>&)` is exactly line 1074; the vblank->
    setNeedsDisplayInRect body is exactly lines 1089-1116; `getRectsBeingDrawn` and the
    BREAKING_CHANGES.md JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS text match. (These citations were not
    edited by the fix round -- a965a98 only replaces the wrong "old union used to overwrite it" narrative with
    the J1 finding -- but FOCUS asked to re-check JUCE fidelity, so verified they still hold.)
  - J5 (rebase no-op): confirmed `git merge-base --is-ancestor 3919ea5 5c5f21d` is true and 5c5f21d is the
    lane's actual merge-base with main -- the "already rebased, no-op" claim is correct, not a skipped step.
  - J4: `std::atomic_ref<double>` read is exactly `LayerStrip.cpp:749` as cited; `ClipInspector::paintKeyNow`
    (`ClipInspector.cpp:1006`) reads `clip_->playheadPosition` plainly as claimed (the filed r1 NIT is
    correctly still open, correctly not silently fixed out-of-scope).
  - No stray artifacts: no `.venv` symlink, no `ADNA_TEMP|J1DIAG` string in tracked src/tests, git status shows
    only build-lane/ (gitignored-build-dir-equivalent, pre-existing, untouched by this round) untracked.
  - CLAUDE.md = 24980 bytes (<= 25000 byte cap, verified with `wc -c`).
  - docs diff 5c5f21d..b1307c7d is purely additive: APP-INVENTORY.md extends one line into a paragraph (no text
    removed), architecture.md adds a tree line + a new "UI Painting (macOS)" section, CLAUDE.md adds a
    Pitfall-NN index line + a "Periodic repaints" UI-Patterns line and tightens two archive-note sentences
    (semantically identical, not a removal of information).

RULING CONFORMANCE (J1-J5):
  - J2 (probe-idle-paint.py `gate(..., cpu_gate=False)` for g4): implemented exactly as ruled -- window-max
    stays the PASS/FAIL gate (still able to fail: printed line asserts `med_wm <= CFG["winMaxMedMs"]`), CPU
    becomes an INFO-only line printing BEFORE (main, re-measured, 5 more launches) vs AFTER. Nothing is hidden:
    163.6 ms/s vs the 150 limit still prints, just not as a FAIL.
  - J3 (identity() replacing array_equal + corner-cluster allowance): `identity.maxDelta=1`,
    `identity.edgeSpan=32` land exactly in probe-idle-paint.json; `edge_mask()` computes a 3x3-neighbourhood
    span via `mode="edge"` padding and ORs both captures ("in either capture", per the ruling text); `viol = m
    & ((d > maxDelta) | ~edge)` is the exact tolerated-iff-<=maxDelta-AND-at-edge logic the ruling states. All
    call sites (`v0`, `v1` S1-S4, `v1n`, `v1b`, `v2`) were migrated; grepped for `corner_rect`/`cornerCluster*`
    residue in the .py -- none found (the ruling's "had no other user and are removed" claim holds). The rule is
    demonstrably able to FAIL: it reports FAIL on v1 S1/S2/S3, v1b SignalBar, v2 A-B (8-21 px each, at the
    SignalStrip top corners, span 14-30 < 32) -- not a rubber-stamp PASS-everything rule.
  - J1 (diagnose-then-STOP): the isolation is definitive by the FOCUS's own bar -- 9 suspects toggled one at a
    time (smoothing, subpixel positioning, interpolation, layer opacity, sync timing x2, partial-vs-full pass,
    native layers off, a JUCE-level snapshot warm-up), every one left the same 24,507/1,648/~9.8k-px classes
    unchanged; a scratch main+hook build (main source untouched) reproduces the identical classes and the same
    persistence, closing the "only the lane shows it" alternative explanation. Root-cause-first was honored: no
    fix was attempted until the mechanism (first-drawRect-vs-later-drawRect, not on/off-screen, not dirty-rect
    size) was pinned down. STOP is not a punt: the report names the actual conflict (a one-shot post-first-pass
    repaint is the only lane-side lever, it is neither periodic nor background per the FORBIDDEN clause, but it
    would make lane-idle diverge from main-idle by the same pixels and break the v1/v3 BEFORE-vs-AFTER identity
    contract) and hands Harmony three concrete options (A/B/C) rather than silently picking one. This matches
    the ruling's own escape hatch ("If the cause is inside JUCE/AppKit ... STOP and report the evidence").
    Root-cause mechanism itself is correctly labeled INFERRED (no public API exposes the CA-internal reason),
    not oversold as verified.
  - J4: cheap check (GCC 15 Homebrew, Apple clang 17) done and passing; GCC 11 / MSVC correctly filed as
    CI-only (no local toolchain) -- matches "do the check if cheap, else file it" literally.

No MUST-level defect found: no ship-blocking regression, no dropped ruling, no test that cannot fail, no
overclaiming spec text left uncorrected (the one wrong claim from r1 -- "old union used to overwrite" -- was
found and fixed this round with disk evidence, which is what this dimension is checking for).

SUMMARY: 2 test-probe files + 1 docs file changed in the fix round proper (3 commits), 0 app-code files (`git
diff` empty, confirmed). 0 blocking issues. 0 suggestions beyond what is already tracked by J4/HANDOFF-NEEDS
(the CI-toolchain atomic_ref check and the v3 flake verdict remain open by design, both correctly surfaced to
Harmony rather than silently resolved).
