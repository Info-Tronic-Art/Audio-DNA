# Critic — s-rta-0925 step4 polish plan
STATUS: DONE
VERDICT: PASS

## Scope
Adversarial disk-verification of `.harmony/.reports/s-rta-0925/plan-step4polish.md` (Architect, s-rta-0925)
against source in /Users/boriskarpman/projects/RealTimeAudio. Every "Facts the plan rests on" (F-a..F-p) row and
every file:line citation in sections 1-4 was opened and diffed against current disk content; JUCE-source claims
(F-m, F-l) were opened in `build/_deps/juce-src`; the D2 pure-function arithmetic examples were re-executed in
Python; call-site completeness for `setNotice`/`refresh`/`RecordPanelView` consumers was greped project-wide.

## What checks out (VERIFIED on disk, not just plausible)
- F-a..F-p: every cited line number is correct, including exact single-line hits (`RecorderHost.cpp:714`,
  `test_record_panel_model.cpp:316-319`, `test_recorder_host.cpp:174`, `BrowserPanel.h:62`,
  `LookAndFeel.cpp:50`). This is unusually precise for a plan document.
- F-b/F-c/F-d/F-j/F-k confirmed byte-for-byte (ApiServer.cpp:1233-1315 callAsync pattern, MainComponent.cpp
  2012-2018/3384-3408, BrowserPanel.cpp:64-87, LookAndFeel.cpp:78-86 incl. `useEllipsesIfTooBig=false`).
- F-f (arm()/repairLoadedAudio() never publish) confirmed by grep of every `publishStatus()` call site in
  RecorderHost.cpp: none inside arm() (143-262) or repairLoadedAudio() (700-716); present in every other
  transition (:364,614,666,680,696). Insertion points (after RecorderHost.cpp:256 `res.takeFolder=...`, before
  :258/:259 provisional-save-and-notify; after RecorderHost.cpp:714) are causally correct — recording_/assetId_/
  takeFolder_ are all already set by the insertion point, so the published Status is consistent, not partial.
- F-m (tooltips reach disabled components) independently re-derived by opening the actual JUCE 8.0.4 sources:
  `TooltipWindow::getTipFor` (juce_TooltipWindow.cpp:161-171) gates on foreground+no-mouse-down+modal only;
  `Component::getComponentAt` (juce_Component.cpp:1115-1122) and the underlying `ComponentHelpers::hitTest`
  (juce_ComponentHelpers.h:74-79) gate on visibility only, no `isEnabled()` anywhere in the hit-test/hover path;
  `Button` only consults `isEnabled()` in click/keyboard handlers (juce_Button.cpp:382,394,643). The plan's
  central untested-until-Boris claim (R5) is honestly flagged rather than asserted as fact.
- F-l (`GlyphArrangement::getStringWidthInt` static, `Font::getStringWidth` `[[deprecated]]`) confirmed at the
  cited lines.
- D2's `tabWidthsFor` pure function: re-implemented in Python from the plan's own listing and run against all
  six worked examples in section 3.3 (cases 1, 3, 5, 6) — every expected output matches the plan's stated
  numbers exactly (e.g. `{33,18,52,78,44,58}`,pad 8,total 428 → `{58,42,76,102,68,82}`, sum 428).
- `RecordPanelNoticeKey` field types match `RecorderHost::Status` exactly (RecorderHost.h:190-218); a
  defaulted `operator==` does not break aggregate-initialization in C++20, so the braced constructions compile.
- Call-site completeness: `setNotice` has exactly one call site today (MainComponent.cpp:2017), matching
  section 1.3's scope; `RecordPanelView`/`deriveRecordPanelView` are consumed only by RecordPanel.{h,cpp} and
  the test file, matching the plan's file list — no missed consumer.
- No existing (untouched) test case checks `playWithAudioTooltip`/`recordAudioTooltip`/`nameTooltip` in a way
  that would be broken by D1's new branch ordering (checked rows 1, 3, 4, 7, 8 — the row 8 case in particular,
  where the tooltip text DOES change under the new logic, has no existing assertion on that field, so there is
  no silent regression).
- FetchContent names in the scratch-build command (`FETCHCONTENT_SOURCE_DIR_JUCE/_HTTPLIB/_CATCH2`) match the
  declared dependency names (`JUCE`, `httplib`, `Catch2` — CMake uppercases automatically) and all three
  `build/_deps/{juce,httplib,catch2}-src` directories exist on disk today.
- Threading analysis (section 6): every touched line is message-thread only; the only lock is the pre-existing
  `statusMutex_`; no new mutex, no audio-callback/analysis-thread/render-thread/GL code touched — accurate
  against CLAUDE.md sacred rules 1-4.
- Untestable claims (D5's visible-at-once flip, D1's tooltip-on-hover) are explicitly and correctly labelled
  "Boris's to see" rather than claimed as ctest-covered — no overclaiming.

## Minor nits found (non-blocking)
1. Off-by-one line citation: "arm's provisional-save notify (:258)" — the `if` guard is at RecorderHost.cpp:258
   but the actual `dispatch.notify(...)` call is line 259. Does not change the causal-ordering argument (the
   new `publishStatus()` insertion point still lands before both lines).
2. Section 5's RED-protocol grep gate (`grep -n "recording_ =\|playing_ =\|..."`) is intended to catch a FUTURE
   key-field write without a following `publishStatus()`, but as written it only lists matches — it doesn't
   fail the build if a match isn't followed by a publish. That's an acceptable manual reviewer gate (explicitly
   framed as "for the reviewer"), not an automated CI check, so it's a process nit, not a defect in the plan's
   own patch.
3. `perfRepair()` on failure both calls `dispatch.notify(msg)` (setting the notice once, mid-transition) and
   returns `msg` to `runAction`, which then calls `setNotice(result, fresh)` again with the same text and key —
   a harmless double-set that already exists in every other refusal path today (pre-existing pattern, not
   introduced by this plan).

## Hunted-for and NOT found
- No missed consumer/call site: grepped every `setNotice`, `.refresh(`, `RecordPanelView` reference in `src/`.
- No RT-rule violation: every changed function is message-thread-asserted or JUCE UI callback; no touch of the
  audio callback, analysis thread, render thread, or any lock-free channel.
- No race: the only cross-thread structure touched is `statusMutex_`, already guarding `publishStatus()`/
  `status()`; the plan adds call-frequency (user-rate), not new concurrent writers.
- No untestable claim asserted as verified: D5/D1's UI-visible behavior is correctly deferred to Boris; the
  plan's ctest additions (+8) only pin what's actually pure-function/model-testable.
- No scope creep: D6, the per-button caption (D1's deferred half), and the B7 rename are explicitly left alone,
  matching the task's boundaries.

## Verdict
PASS. No blocking defect found after opening every cited file:line, re-deriving the JUCE tooltip-gating claim
from JUCE's own source rather than trusting the plan's citation, and re-executing the D2 arithmetic
independently. The three items listed above are cosmetic/process nits, not correctness or safety issues, and
none would block a builder from implementing this plan as written.
