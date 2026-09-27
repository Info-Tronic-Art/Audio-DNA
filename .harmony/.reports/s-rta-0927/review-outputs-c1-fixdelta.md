# Reviewer Verdict — outputs-c1-fixdelta
STATUS: DONE
VERDICT: PASS

REVIEWED COMMITS: e14027c..7253fd2 (5e1b7a1, 79b4410, 67bd544, 52dd980, 7253fd2), read via
`git -C /Users/boriskarpman/projects/RealTimeAudio diff/show` (worktree rta0927-w1 was removed
mid-review, merged as e00b69a; same commits confirmed present in main).

FILE: src/ui/OutputWindow.cpp (commit 79b4410)
  [OK] Spec fidelity: the ctor's added check reads `getPeer()->getStyleFlags()` and, if the
       `windowIgnoresKeyPresses` bit is absent or the peer is null, logs to stderr and calls
       `TopLevelWindow::addToDesktop()` (the zero-arg overload, verified in JUCE source:
       `build/_deps/juce-src/.../juce_TopLevelWindow.cpp:163-166` — it calls
       `Component::addToDesktop(getDesktopWindowStyleFlags())`, virtual-dispatched to
       OutputWindow's override which ORs in `windowIgnoresKeyPresses`). This is genuinely a
       Release-reachable check, not cosmetic: the jassert alone is compiled out of Release, and
       `TopLevelWindow`'s own ctor (called by the `DocumentWindow` base, since the trailing
       `addToDesktop` param defaults `true`) adds the peer using the BASE class's
       `getDesktopWindowStyleFlags()` (virtual dispatch resolves to the base during base-class
       construction) — so the peer's *first* creation lacks the flag. The pre-existing
       `setDropShadowEnabled(false)` line normally repairs this via its own `isOnDesktop()`
       branch, but only if the window is already on the desktop; the new check is a real
       Release-live backstop, not a dead no-op.
  [OK] Fail-open/loop check: `Component::addToDesktop` is a no-op when
       `styleWanted == peer->getStyleFlags()` (JUCE source `juce_Component.cpp:376`), so the
       added block cannot recurse or thrash. It runs once, synchronously, at ctor time, strictly
       before `openOnDisplay()`'s `setBounds`/`setVisible(true)` (called later, from a different
       function) — order is preserved, and `addToDesktop`'s internal `peer->setVisible(isVisible())`
       reuses the pre-existing (not-yet-shown) visibility state, which is the same call the
       already-existing `setDropShadowEnabled` path makes — no new visibility side effect.
       No path was found where a peer can end up on desktop without the flag surviving this ctor.
  [OK] Naming/comments: the added comment ("Checked in every build type... a jassert is a
       Release no-op") accurately matches what the code does — no overclaim.

FILE: tests/test_output_law.cpp (commit 79b4410)
  [OK] Spec/claim fidelity — the new row is a real (if static) behavioral guard, not a toothless
       grep: `Sources::cpp` is already comment- AND string-literal-stripped (`codeOnly()`,
       :36-58 — the `Str` state appends nothing but the delimiter quotes), so the new test's
       `body.find("windowIgnoresKeyPresses")` cannot be satisfied by the added `std::cerr`
       log string (which itself contains that word) — only the real enum reference in the
       `if` condition can hit. Ran the mutant evidence myself line-by-line
       (`fixround-{RED,GREEN-TEETH}-test_output_law-releasecheck.txt`): RED on e14027c pre-fix
       (3/7 assertions fail, matches the three added `CHECK`s), GREEN post-fix (7/7), and three
       named teeth mutants each fail a distinct assertion subset (jassert-only: 3 failed;
       log-without-repair: 1 failed; repair-without-reading-the-flag: 2 failed) — consistent
       with what each mutant should defeat. Brace-matching to isolate the ctor body is correct
       (initializer list contains no `{`, so the first `{` found is the body open).
  [OK] Complexity: the jassert-stripping logic is a few lines of straightforward char scanning,
       consistent with the file's existing `codeOnly()` pattern — no new abstraction introduced.

FILE: .harmony/probe-outputs.py / .harmony/probe-outputs.json (commit 67bd544)
  [OK] "Compares like with like" claim verified: `o_probe_survives_resolution_change` now calls
       `trig(0)` + `sleep(settleAfterTrigger)` (the same fixture/constant already used by other
       rows, not a new magic value) BEFORE `set_composition_params`, forcing col0=A onto the
       canvas before the resolution change, then asserts `d(f_720, imageA stretched) <= contentTol`
       in addition to the pre-existing `d(probe, f_720 upscaled) <= tol` check. `contentTol` is
       set to 6.0 — identical to the existing `tol`, not loosened.
  [OK] Teeth confirmed live in evidence: the mutant (trig(0) line removed, i.e. round-1's
       behavior) FAILs the new assertion with d=29.425 (matches the independently-known
       d(A,B)=29.4 baseline in the file's own docstring) against the lane app, while the fixed
       probe passes at d=0.013 — a real resize-content check, not a loosened threshold.
  [OK] The 720p "border" MUST is correctly refuted on the probe side only, no app-code change:
       decoded-pixel evidence (`fixround-720p-border-check.txt`) shows fixture B
       (`P16_02_Screen_Split_2x2.png`) itself carries a (15,15,15) margin at both 1080p (fB) and
       720p (f720) — consistent with "the canvas is full-bleed, the border is B's own content."

FILE: .harmony/.reports/s-rta-0927/outputs-c1.md (sections "Fix round 1a" / "Fix round 1b")
  [OK] Scope/evidence-truth check: every FACTS/METHOD/evidence-file claim in 1b was opened and
       cross-checked against the actual commit diffs and the actual evidence-file contents
       (ctest tail = "691" exactly matches `fixround-ctest-tail.txt`; RED/GREEN/mutant PY 1/10/9
       PASS counts match the three probe-outputs run logs verbatim; the test_output_law
       RED/GREEN/mutant assertion counts match the two releasecheck evidence files verbatim).
       No overclaim found.
  [OK] 1a correctly marked SUPERSEDED-where-differing and is genuinely a no-code-change commit
       (`git show 5e1b7a1 --stat`: two evidence files + the report, no src/test/probe file).

FILE: docs/claude/pitfalls.md, .harmony/notebook.md (commit 52dd980)
  [OK] Pitfall 40 text was updated to name the Release-path check, matches 79b4410's actual code
       (verified: "the constructor re-reads the peer's flags in every build type -- a jassert is
       a Release no-op -- and logs + re-adds the window to the desktop if the flag is missing").
       No numbering collision found post-merge (grep for "Pitfall 40"/"41" across pitfalls.md,
       rendering.md, integration.md shows a single consistent "40").

TREE / HYGIENE
  [OK] All 8 new/changed evidence files in 7253fd2 and 79b4410/67bd544/52dd980 are `100644`
       regular blobs (`git diff --summary`), no symlinks or executable-bit anomalies.
  [OK] No `.venv`, `build-lane`, or other build-dir path appears in `git ls-tree -r 7253fd2`.
  [OK] Current main-checkout `git status` is clean apart from an unrelated `.harmony-version`
       bump and pre-existing untracked `.claude/`/`AGENTS.md` scaffolding — neither touched by
       this range.
  [OK] The reviewed worktree (rta0927-w1) was removed mid-review per the coordinator's note;
       verified the same 5 commits are reachable from main (merge e00b69a) before continuing —
       no gap in the reviewed range.

SUMMARY: 5 commits / 8 substantive files reviewed (OutputWindow.cpp, test_output_law.cpp,
probe-outputs.py, probe-outputs.json, outputs-c1.md, pitfalls.md, notebook.md, plus evidence
files), 0 issues (0 blocking, 0 suggestions). Both the GL Release-safety fix and the probe
"like-with-like" fix are real, correctly targeted, evidence-backed, and do not loosen any
tolerance or introduce a fail-open path. The new test_output_law row is a genuine (if static)
behavioral guard, not defeated by its own log message. Report sections 1a/1b are truthful to
the commits and evidence files. Tree is clean; nothing stray committed.

METADATA: reviewer=reviewer-outputs-c1-fixdelta, builder_packet=outputs-c1-fixdelta, date=2026-09-27T00:00:00Z
