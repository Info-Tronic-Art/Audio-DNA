# Shutdown SIGBUS scout — .ips 2026-07-30-125725 (queue candidate 10)
2026-07-30, read-only. Byte-level verified against the on-disk binary.

## VERDICT
NOT destructor ordering, NOT a leaked listener, NOT a shared/dangling
ValueSource. Mechanism: **corruption of a single 8-byte word inside a live
juce::Label** — teardown merely DISCOVERS it. Confidence HIGH on mechanism
(instruction-level proof); the WRITER is unidentified (MEDIUM-LOW candidates
below). Member reordering / explicit ~EffectsRackPanel teardown would only
move the crash — do NOT do that.

## Chain (offsets matched byte-for-byte)
- Faulting insn: `ldaddal` 64-bit atomic decrement = libc++ shared_ptr release
  of ListenerList::iterators control block (juce_ListenerList.h:362-363,
  ctrl at this+0x18). Value read 0x600001094e01 — ODD; malloc ctrl blocks are
  16-aligned ⇒ garbage pointer ⇒ EXC_ARM_DA_ALIGN. Single-word damage: all
  neighboring fields read valid.
- Frame chain: EffectsRackPanel D2+344 ⇒ the crashing Label is a
  categoryHeaders_ label (EffectsRackPanel.h:71, LAST member destroyed);
  Label::~Label+220 = textValue dtor; Value::~Value+256 → ICF-folded
  ListenerList D2+196 (matches <deduplicated_symbol>+196).
- Ruled out BY OFFSET: listener-not-removed (removeListener already ran,
  juce_Label.cpp:52); shared ValueSource (already deref'd OK; also zero
  shared Values in src/ by grep); double-destroy (would SEGV aligned, not
  ALIGN-trap); GL-thread-paints-components (painting disabled, Renderer.cpp:28);
  static-destruction order (all member/heap teardown).
- appWillTerminateByForce = NORMAL macOS quit teardown (not force-kill);
  consequence: message loop stopped, no MessageManagerLock grants during
  teardown.
- Threads live during teardown: Thread 18 "OpenGL Renderer" still in
  renderFrame/flushBuffer — Renderer::detach() only in ~PreviewPanel
  (PreviewPanel.cpp:25) and previewPanel_ (MainComponent.h:192) destroys
  AFTER effectsRackPanel_ (:196) ⇒ GL thread runs through entire UI teardown.

## Candidate WRITERS found (INFERRED, not tied to this crash)
1. Unsynchronized cross-thread EffectChain: initEffectChain() on GL thread
   (Renderer.cpp:81,88) does vector push_back/realloc while
   EffectsRackPanel::timerCallback reads at 10Hz on message thread
   (EffectsRackPanel.cpp:197,209) with no lock — same family as the 07-28 GL
   race fixed by 8bd09ba.
2. Unchecked indexing: EffectsRackPanel.cpp:472-473 indexes
   sections_[capturedEi]/paramKnobs[capturedPi] unchecked; capturedEi is an
   EFFECT index but sections_ is COMPACTED (nulls skipped :314) ⇒ wrong index
   whenever any effect is null. Sibling lambdas at :358/:377 DO check.

## Smallest safe fixes (no code written)
1. HIGHEST VALUE, one line: detach GL renderer at TOP of ~MainComponent
   (MainComponent.cpp:1561) instead of relying on ~PreviewPanel ~48 members
   later — ends GL activity before any UI teardown. Correct regardless of
   which writer is real.
2. Fence/move initEffectChain() off the GL thread (8bd09ba pattern).
3. Bounds-check the :472 lambda to match siblings.
4. Do NOT reorder members / add explicit dtor teardown.

## Verification reality-check
One occurrence in a 12h52m session (mach-tick verified) — repeated scripted
quits prove nothing. DO: (a) add an ASan build variant (repo currently has
ZERO sanitizer wiring) + long UI-churn session; (b) zero-build interim:
MallocNanoZone=0 MallocScribble=1 MallocPreScribble=1 MallocGuardEdges=1
relaunch turns UAF into a deterministic fault at the real site; (c) regression
signal for fix 1: "OpenGL Renderer" thread absent from any future teardown .ips.

## Reachability correction (vs ledger PM6 note)
The PATH runs on every graceful quit once a GL context existed (categoryHeaders_
populated via rebuildUI from the GL-filled effectChain_; a no-GL fresh instance
skips the loop). The CRASH is state/timing dependent — "any graceful quit
CRASHES" is an overstatement; "any graceful quit runs the risky path" is right.
