# Reviewer Verdict — opacity-r1
STATUS: DONE
VERDICT: APPROVE

## Scope
7 commits on lane/0925-opacity (a1be09b..2b01280) against plan-opacity.md / diag-opacity.md,
implementing Boris's 3 asks: (1) remove Video-tab "Opacity" twin, keep Master, (2) link
top-right TopBar fader to Composition "Master" knob (one value, no compounding), (3) fix
right-click reset dead on all Composition-tab sliders. Note: the "master signal =
gain + all signals" idea in Boris's verbatim request is explicitly out of scope for
this build (diag-opacity.md RISKS: "Master Signal slider ... out of this dispatch,"
pointer only) — that idea is NOT implemented here and should not be read as done.

## Verification performed
- Read every file in `git diff main...lane/0925-opacity --stat` (18 files) in full,
  against both plan-opacity.md and diag-opacity.md line-by-line.
- Confirmed lane commits: test(RED a1be09b) -> fix(GREEN 41055a5) -> fix(twin c229e4c)
  -> test(RED 507ebc7) -> fix(GREEN 16b133d) -> docs(cb956f6) -> docs/notebook(2b01280).
- `git diff a1be09b..41055a5 --stat` / `git diff 507ebc7..16b133d --stat`: the two new
  test files (test_right_click_reset.cpp, test_master_opacity_link.cpp) are BYTE-IDENTICAL
  between each RED and GREEN commit — confirms staged-test hygiene (dimension 9): the same
  assertions ran against stub prod code (RED) and real prod code (GREEN), never edited to
  pass.
- Read the RED-commit stub (`git show a1be09b:src/ui/UniversalParamControl.h`): matches
  plan C-R exactly (`onResetToDefault` never fired, `resetToDefault()` no-op,
  `childRightClickResets` hardcoded false) — logically produces the claimed 4/5-fail tally
  (RC1 pins on HEAD's pre-existing thumb-reset path; RC2-RC5 fail against the stubs).
- BUILT and RAN both test binaries on the current tree (build dir already configured in
  the worktree): `./build/tests/test_right_click_reset` -> "All tests passed (17
  assertions in 5 test cases)"; `./build/tests/test_master_opacity_link` -> "All tests
  passed (14 assertions in 6 test cases)" — exact case/assertion counts match plan's RC1-5
  / ML1-6 tables. This is direct VERIFIED evidence the GREEN state is green.
- Did NOT reproduce a full from-scratch RED rebuild in a scratch copy: `git worktree add`
  is blocked for this role (HARD-GATE), and a `git archive`-based scratch copy + reconfigure
  failed on an unrelated environment issue (JUCE's `juceaide` sub-build cache hardcodes the
  main-repo's build path, not this worktree's) — an infra/tooling limitation, not a code
  defect. Confidence in the RED claim is therefore INFERRED (stub content read + logic
  traced + GREEN side independently executed) rather than directly re-executed at RED;
  flagging per Rule 4 rather than silently upgrading to VERIFIED.
- Grepped the full lane worktree for `masterLevel_|setMasterLevel|getMasterLevel\b`:
  zero hits outside two explanatory comments (TopBar.cpp:209, Renderer.cpp:679) — B5/B6
  cleanly retired the second multiplier and both REST readers now source
  `composition_.eff(CompScalar::Opacity)`.
- Confirmed `ResettableSlider() {}` default ctor is safe: `grep -rn "ResettableSlider("`
  finds no call site that used the old `using juce::Slider::Slider;` inherited-ctor form.
- Confirmed TopBar's `onDragStart/onDragEnd/gripHeld/release(connNow())` pattern is
  byte-for-byte the same shape as the existing `LayerStrip.cpp:431-436` precedent the plan
  cites (read both).
- Confirmed no changes to `src/osc/`, `src/binding/`, `src/midi/` — OSC/MIDI/binding
  writers keep landing on `Composition::masterOpacity` unchanged, which is exactly why the
  fader-as-second-view design makes them "just work" without new wiring (diag §3 rationale
  holds).
- Confirmed every file in the diff's `--stat` is inside the plan's declared SCOPE fence;
  none of the explicitly-OUT items (REST master writer, LayerInspector twin, OutputWindow,
  PresetManager format, Master Signal slider) were touched.
- CLAUDE.md/APP-INVENTORY.md doc updates match plan §4 exactly (Master slider note, REST
  key semantics note, ResettableSlider UI-pattern paragraph).

## Findings

FILE: src/ui/UniversalParamControl.h / .cpp
  [OK] Readability/Patterns: `ResettableSlider` gains a documented nested-child mouse relay
       (`ChildRelay`) with a clear comment explaining JUCE's `wantsEventsForAllNestedChildComponents`
       semantics; matches plan C1 verbatim.
  [OK] DRY: thumb-path and name-area right-click reset both now funnel through
       `resetToDefault()` — the old duplicated reset logic (setParamValue + setValue +
       manual onValueChanged call + gripTouch) in `UniversalParamControl::mouseDown` is
       gone, replaced by one call (`src/ui/UniversalParamControl.cpp:299-305`).
  [OK] Error handling: `childRightClickResets` explicitly excludes `juce::Button` children
       (documented rationale: Button::mouseUp fires its click on any mouse button) so a
       right-click on a "+"/"-" IncDec button doesn't both step AND reset — correctly
       scoped to the text-box Label only.

FILE: src/ui/CompositionInspector.cpp / .h
  [OK] Scope completeness: all 9 twin-removal deletions (A1-A9) present and match 1:1;
       `bindScalarControls` C4 edit present, arming all 7 remaining scalar UPCs with
       their real defaults via the existing `compScalarDefs()` table (no new constants).
  [OK] Patterns: reuses the existing descriptor table instead of hardcoding 1.0/0.25/0.5
       per-control — avoids introducing a second source of truth for defaults.

FILE: src/ui/TopBar.cpp / .h, src/MainComponent.cpp / .h, src/render/Renderer.cpp / .h,
      src/api/ApiServer.cpp, src/test/TestServer.cpp
  [OK] Spec/claim fidelity: diag's ROOT CAUSE claim (two independent multiplies compounding)
       is fully addressed — `Renderer::masterLevel_` and its dim block are deleted, not
       just deprecated; both REST readers (`/api/status.masterLevel`,
       `/api/state.master_level`) now source the single model field via `eff()`, matching
       the "keys kept, meaning changed" tradeoff the diag/plan explicitly call out and the
       docs record.
  [OK] No regressions: OSC/binding/MIDI/recorder-replay paths (all in files NOT in this
       diff) already wrote `Composition::masterOpacity`, so they automatically now drive
       the fader too — verified no code changes were needed or made there.
  [OK] Legacy v1 slider (`MainComponent::masterLevelSlider_` + `masterVideoLevel` deck-preset
       plumbing) fully deleted per the plan's TRADEOFFS choice (drop both persistence
       lines rather than diag's original "apply on load" option) — a deliberate, documented
       plan-level override of the diag's step 4, correctly implemented as chosen.

FILE: tests/test_right_click_reset.cpp, tests/test_master_opacity_link.cpp, tests/CMakeLists.txt
  [OK] Staged-test hygiene: confirmed byte-identical across RED/GREEN commits (see
       Verification above).
  [OK] Test quality: each TEST_CASE matches its plan-table id/assertions; RC2's "second
       right-click when already at default" edge case and ML5's mid-drag guard are both
       present and correctly reasoned.
  [COMMENT] Link closure (harness-wiring-adjacent but not a HARNESS gate item): the CMake
       link list for both targets grew well beyond the plan's authored list (pulls in
       MappingEngine/EffectChain/ShaderManager/TextureManager/FullscreenQuad/LUTLoader
       transitively via SignalRegistry -> AudioSignal's vtable). This is disclosed
       accurately in a CMakeLists.txt comment and a dedicated notebook.md entry, and is a
       correct, minimal application of the plan's own stated rule ("add the ONE .cpp that
       defines the missing symbol, never MainComponent.cpp/Renderer.cpp"). Not a defect —
       flagging only because a future editor of this test file should read the notebook
       entry before trimming source list "cruft."

FILE: .harmony/APP-INVENTORY.md, CLAUDE.md
  [OK] Docs match plan §4 exactly for the touched lines.
  [ISSUE-MINOR, non-blocking] CLAUDE.md:1197 ("Composition-Level Transform (P25)") still
      reads "...before the master level dim," a holdover phrase from when there were two
      dim stages (masterLevel_ then masterOpacity). It is not factually wrong (there is
      still one master-opacity dim) but is now imprecise given masterLevel_ is deleted.
      This line was not in the plan's docs list and the plan/diag never claimed to audit
      CLAUDE.md exhaustively for masterLevel_ mentions (they explicitly grepped only for
      the ResettableSlider paragraph and the two Output-Integration lines) — so this is a
      pre-existing gap outside the plan's own stated scope, not a missed instruction.
      Suggest a one-line follow-up fix, not blocking this review.

## SLIM CHECK
No EXCESS found. Every added line (relay struct, onResetToDefault hook, syncMasterFromComposition,
two test targets) is exercised by the new tests and/or the live-gate checklist; every deleted
line (masterLevel_, legacy v1 slider, Video twin) is a plan-directed removal of code that is now
provably unreferenced (grep-confirmed zero remaining hits). Nothing to disposition as
REMOVED/DEBT_FILED/JUSTIFIED_KEEP/DEFERRED_TO_NEXT_CLOSE — the diff contains no speculative,
duplicate, or vestigial additions.

## Boris's second request ("master audio AND all signals... gain slider adding other signals")
Confirmed NOT implemented in this build, and confirmed the diag correctly scoped it out with a
named pointer (AudioEngine::setInputGain is pre-analysis; SignalRegistry has no post-analysis
depth/gain stage today) rather than silently dropping it. This should be tracked as a follow-up
design item, not treated as done by this lane.

SUMMARY: 18 files reviewed in full, 0 blocking issues, 1 non-blocking doc-staleness nit
(CLAUDE.md:1197) and 1 informational comment (test link-closure growth, already
self-documented by the Builder). Every plan edit (A1-A10, C1-C4, B1-B7) verified present
and matching; RED/GREEN test hygiene confirmed by direct diff; GREEN state confirmed by
direct execution (17+14=31 assertions, 11 test cases, all pass). No scope creep, no
regressions to OSC/MIDI/binding paths, no changes outside the plan's declared file list.

METADATA: reviewer=reviewer-opacity-r1, builder_packet=lane/0925-opacity, date=2026-09-25
