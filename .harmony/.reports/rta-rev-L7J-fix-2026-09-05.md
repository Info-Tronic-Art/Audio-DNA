# Reviewer Verdict — L7-JUKE fix (F1 race remedy)
STATUS: DONE
VERDICT: PASS

FILES: src/render/Renderer.h, src/render/Renderer.cpp, src/ui/MilkDropBrowser.h,
       src/ui/MilkDropBrowser.cpp, src/MainComponent.cpp (fence held exactly; read-only,
       repo /Users/boriskarpman/projects/RealTimeAudio)

Q1 — Is the race closed? YES.
  `ProjectMPresetManager::toggleFavorite(int)`'s ONLY caller anywhere in the repo (excluding
  build dirs) is now `Renderer::toggleFavoritePreset()` (Renderer.cpp:911), confirmed by
  `grep -rn "toggleFavorite"` across src/. `FilesBrowser::toggleFavorite(const juce::File&)`
  (FilesBrowser.cpp:637) is a same-named method on an unrelated class with a different
  signature (File, not int) — confirmed name collision, not a hit, as the packet predicted.
  Also swept for any OTHER direct writer of `PresetInfo::favorite` via
  `grep -rn "\.favorite\s*="`: the only two writes in the repo are Renderer.cpp:911's confined
  call (inside toggleFavorite() itself, ProjectMPresetManager.cpp:227) and one inside
  `loadUserData()` (ProjectMPresetManager.cpp:130) — which is dead code, see Q4. No other
  reachable writer exists. Race is closed.

Q2 — Does toggleFavoritePreset mirror rescanMilkDropPresets branch-for-branch? YES, exactly.
  Side-by-side (Renderer.cpp:868-902 vs :904-937): null-manager early-out (:870-871 / :906-907),
  not-attached inline case (:883-887 / :917-921), marshal case with identical
  `getCurrentContext() != &glContext_` guard and `blockUntilFinished=true` (:894-899 / :929-934),
  already-on-GL-thread inline fallthrough (:901 / :936) — structurally identical, same shape,
  same comment reasoning reused verbatim. No divergence found.
  blockUntilFinished=true is correct: MilkDropBrowser::mouseDown() calls
  `onToggleFavoriteRequested(idx)` immediately followed by `repaint()`
  (MilkDropBrowser.cpp:60-62) — blocking ensures repaint reflects the new favorite state in the
  same frame; async would risk a stale-star paint. No deadlock risk: read the full mouseDown()
  body (MilkDropBrowser.cpp:38-65) — plain member access on the message thread, no mutex/lock
  held across the call, so the blocking round-trip cannot invert against anything the GL thread
  might hold. Self-deadlock on an already-GL-thread caller is explicitly excluded by the
  `getCurrentContext() != &glContext_` branch, same precedent as the sibling function.

Q3 — Removed-fallback claim verified from source: TRUE.
  Wiring (MainComponent.cpp:1628) runs synchronously inside MainComponent's constructor
  (message thread), unconditionally, right after `presetManager_` is handed to the Renderer.
  Checked for any synchronous message-pump between `browserPanel_` becoming a live component
  (`addAndMakeVisible` at MainComponent.cpp:1377) and this wiring (line 1628): only
  `juce::MessageManager::callAsync` and `juce::AlertWindow::showMessageBoxAsync` appear in that
  span (lines 330, 353) — both async, neither reenters the dispatch loop synchronously. JUCE's
  message thread is single-threaded/non-reentrant absent an explicit nested loop, so no mouse
  event can reach browserPanel_ before the constructor returns. Also confirmed
  `onToggleFavoriteRequested` is set exactly once and never cleared anywhere (grep, 4 hits
  total: declaration, the one assignment, and the two read/invoke sites in mouseDown()). If it
  were ever unset, the failure mode is a silent no-op (repaint fires, star doesn't change) —
  correctly the safer trade over a data race; a user sees a right-click that doesn't visibly
  toggle, not a corrupted read.

Q4 — loadUserData/saveUserData zero-callers claim: CONFIRMED independently, two patterns.
  `grep -rn "loadUserData\s*(\|saveUserData\s*("` and a second bare-identifier pass
  `grep -rn "loadUserData\|saveUserData"`, both across the whole repo excluding build dirs —
  both return only the .h declarations and .cpp definitions (ProjectMPresetManager.h:39-40,
  ProjectMPresetManager.cpp:85,111), zero call sites anywhere. Separate, non-blocking finding
  worth surfacing on its own: favorites are never persisted or restored across a restart (no
  save on toggle, no load on startup) — a real UX gap, but out of scope for this race fix and
  not something this fix needs to address.

Q5 — Regression check against the prior review's four items: NONE FOUND.
  The fix's footprint is strictly additive and isolated to the favorite-toggle path
  (`Renderer::toggleFavoritePreset`, `MilkDropBrowser::onToggleFavoriteRequested` + its
  wiring, and the one changed line inside `mouseDown()`). Diffed the onAutoSwitch, Link
  toggle, MIDI-out, and Jukebox Blend/Pool/Mode onChange code in this same working-tree diff —
  byte-identical to what the prior review (rta-rev-L7J-2026-09-05.md) already verified; no
  shared lines touched.

WHAT WAS SWEPT (to support the "nothing else found" claims above):
  - Every reference to `toggleFavorite` in src/ (repo-wide grep, both an exact-call and a
    bare-identifier pass).
  - Every direct write to `PresetInfo::favorite` (`\.favorite\s*=` repo-wide).
  - Every reference to `loadUserData`/`saveUserData` (two independent grep shapes, repo-wide,
    build dirs excluded).
  - Every reference to `onToggleFavoriteRequested` (repo-wide) to confirm single-assignment.
  - Full constructor body between browserPanel_ becoming visible and the new wiring, for any
    synchronous message-pump call.
  - JUCE's `OpenGLContext::executeOnGLThread` doc comment (juce_OpenGLContext.h) — confirms
    blocking semantics and GL-thread requirement match the code's usage.

Machine-side (build/ctest) trusted from the dispatching session's own report per this
packet's "machine half done by me" — not independently re-run (read-only assignment).

METADATA: reviewer=rev-L7J (narrow re-review), builder_packet=L7-JUKE-fix, date=2026-09-05
