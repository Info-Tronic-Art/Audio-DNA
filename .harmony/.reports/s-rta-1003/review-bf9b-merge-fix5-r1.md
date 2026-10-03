# Review bf9b FIX-5 r1 (4137f60..fc51063)
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST)

(1) No freed binding - VERIFIED by reading: src/MainComponent.cpp:3956-3992 removeDeck no longer calls Layer setLayer(nullptr) / selectLayer(-1); Layer inspector binds into Composition::layers, which RemoveDeckCmd does not touch; LayerInspector holds no deck/clip pointer (grep). Clip inspector still emptied before the command (MainComponent.cpp:3963); the onFencedEdit hook (clearClipInspectorIfUnowned, ui/InspectorRepoint.h:31) keeps owned-or-clear for redo/undo; the AM-1/AM-2 forget-first in setLayer (LayerInspector.cpp:734) untouched.
(2) Tests - VERIFIED: AS8 (tests/test_show_model.cpp:~2032) drives RemoveDeckCmd through the real fence and AppInspectors::wire, which calls the production repointInspectorsAfterStackMove / clearClipInspectorIfUnowned (not a copy). It has NO RED arm and is blind to the mutant (report admits it, STOP ITEM 1); the tooth is text lint B4k (test_render_thread_lint.cpp:~528, RED at 4137f60 and under MU11 per report - INFERRED, not re-run) plus live rows. No-layer case has a RED line (3 failing assertions at ef06be2 per report - INFERRED).
(3) No-layer state - VERIFIED: LayerInspector.cpp:588-591 only setVisible on nameLabel_/macroPanel_ in resized() before the early return; setLayer calls resized() on every path (LayerInspector.cpp:744). paint() already prints "No layer selected". Visibility only.
(4) Files changed - VERIFIED (git diff --stat): .harmony/.reports/s-rta-1003/bf9b-fix.md, BORIS_DECISIONS.md, docs/claude/performance-controls.md, src/MainComponent.cpp, src/ui/LayerInspector.cpp, src/ui/LayerInspector.h, tests/test_render_thread_lint.cpp, tests/test_show_model.cpp. No render-thread, compositor, audio, core/ or model/ file. The 4137f60 performance result stands.
(5) Stray - none in the delta (no mutant, hook, .venv link; build-asan-app/ untracked in worktree, not in the commit). Boris quotes ("The layer strip does not need to show the deck a clip is playing from.", "drop") match .harmony/binding-decisions.md:611,659 verbatim.

NITS
- N1 docs/claude/performance-controls.md Deck tab row: "its Undo and its Redo ... while the Clip tab is emptied" overclaims: refreshAfterUndoRedo (MainComponent.cpp:5350-5385) re-points the Clip inspector by the selected cell, so after Undo/Redo the Clip tab can show a still-owned clip. Safe (valid pointer) but the sentence should say "Remove Deck itself empties the Clip tab".
- N2 B4k is a text match on removeDeck's body: moving the emptying into a helper would evade it (live rows still catch). Acceptable, disclosed.
- N3 Pre-existing, not in delta: Clip inspector's EffectScope carries a deck index that a redo/undo of Remove Deck can shift when the Clip tab shows a still-owned clip (INFERRED).
