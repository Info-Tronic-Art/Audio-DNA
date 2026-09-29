# Reviewer Verdict — idlepaint-juce-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS (one SHOULD-level portability item worth a pre-merge check)
REVIEWED: lane/idlepaint 5c5f21d..42871bd, worktree .claude/worktrees/rta0928b-idlepaint
FOCUS: NativeLayerHost/NativeLayerCache/OverlayWatch vs pinned JUCE 8.0.4; F4 (I2/I3); F6; no mutex; non-Apple unchanged

FILES:
  src/ui/NativeLayerCache.h, src/ui/NativeLayerHost.h/.mm, src/ui/OverlayWatch.h/.cpp, src/ui/UiPaintCounters.h,
  src/ui/LayerStrip.h/.cpp, src/ui/ClipInspector.h/.cpp, src/MainComponent.h/.cpp, src/api/ApiServer.h/.cpp,
  CMakeLists.txt, tests/CMakeLists.txt, tests/test_native_layer_cache.cpp, tests/test_overlay_watch.cpp,
  tests/test_layer_strip_transport_view.cpp, tests/test_clip_inspector_paint_key.cpp, CLAUDE.md,
  docs/claude/{pitfalls,architecture}.md, .harmony/APP-INVENTORY.md, .harmony/probe-idle-paint.{sh,py,json}

VERIFIED AGAINST JUCE 8.0.4 SOURCE (build/_deps/juce-src):
  - drawLayer's clear-rect/CTM-flip/CoreGraphicsContext sequence is byte-identical to
    NSViewComponentPeer::renderRect/drawRectWithContext (juce_NSViewComponentPeer_mac.mm:962-1034).
  - wantsLayer/redraw-policy/drawsAsynchronously/isFlipped/isOpaque setup in NativeLayerHost.mm:73-78,
    ADNANativeLayerView matches the peer's own setup (:216-225, :2131-2135, :2628) exactly.
  - hitTest->nil, acceptsFirstResponder->NO, isAccessibilityElement->NO: matches claimed hit-test/first-responder/
    accessibility behavior.
  - setCachedComponentImage takes ownership via unique_ptr (juce_Component.cpp:552-558) — NativeLayerHost's raw
    `new NativeLayerCache` + nullptr-reset teardown is correct, no double-free/leak.
  - NSViewAttachment hides the hostComp NSView via `[view setHidden: !owner.isShowing()]`
    (juce_NSViewComponent_mac.mm:38-97) — isShowing/visibility propagation confirmed at the JUCE level.
  - MainComponent.h member order (overlayWatch_/waveformLayer_/signalBarLayer_ declared AFTER signalBar_,
    waveformDisplay_, bindingOverlay_, midiLearnOverlay_) gives correct reverse-order destruction: hosts destruct
    before the widgets they reference, and before overlayWatch_ (host dtor calls watch_.removeClient).

ADOPTION RULINGS — CODE-LEVEL CONFORMANCE:
  - I1 (sync fallback): NativeLayerCache::setFallback(true) hides the layer (sink_.layerShown(false)) and pokes the
    peer NSView directly via `[NSView setNeedsDisplayInRect:]` (NativeLayerHost::peerNeedsDisplay, bypassing JUCE's
    deferred repaint queue) in the SAME call — genuinely synchronous, not one AsyncUpdater turn later. Fallback->
    Native only shows the layer after `layerDrew()` fires post-draw (RestorePending). Confirmed by
    tests/test_native_layer_cache.cpp incl. an explicit negative "teeth" test proving the plan-body async variant
    (hideAfterPaint=true / FallbackPending) is DISTINGUISHABLE and would fail v2b's covered-frame gate.
  - I2 (one playhead read/tick): LayerStrip::updateTransportView() reads playheadPosition once via
    `std::atomic_ref<double>(...).load(relaxed)` into transportView_ member; paint() reads only that member, never
    the model again. Named strategy in the code comment referencing s166 L5. Tested
    ("the playhead read at the tick is the one painted").
  - I3 (routine hairline gate): LayerStrip::timerTick() recomputes the hairline width with the SAME formula
    paintRoutineBands() paints, and repaints only on change; tested incl. a Playing->Waiting transition.
  - I4/v2b: probe-idle-paint.json's v2b block (cycles=5, restoreFramesMax=2) and the ADNA_UI_OVERLAY_WITNESS vblank
    counter in NativeLayerHost.mm match the ruling; g4 uses the SAME CFG['winMaxMedMs']/['cpuMainMsPerS'] thresholds
    as i1 (no silent re-threshold) — g4's reported CPU FAIL (163 vs 150 ms/s) is a plan-anticipated STOP (I3: "never
    a re-threshold"), correctly surfaced as a FAIL line + a Decision-for-Harmony item, not hidden or downgraded.
  - No mutex in any new file (grepped). Non-Apple: NativeLayerHost.h's #else branch returns nullptr / no-op;
    NativeLayerHost.mm is gated `if(APPLE)` in CMakeLists.txt; OverlayWatch/NativeLayerCache are plain JUCE C++,
    compiled everywhere as before.

FINDINGS:
  [SHOULD] Spec/claim fidelity + portability — src/ui/LayerStrip.cpp (transportViewOf) uses `std::atomic_ref<double>`
  on the read side only (write side, GL/Renderer, still plain — explicitly and honestly noted as "L5's open item").
  This directly contradicts an EXISTING, twice-stated "verified UNAVAILABLE on this toolchain" finding already in
  this codebase for the exact same primitive: src/features/FeatureBus.h:18-20 and
  .harmony/specs/featurebus-thread-safety-design.md:14,26 ("std::atomic_ref verified UNAVAILABLE on this toolchain"
  — the reason FeatureBus uses an atomic-word seqlock instead). LayerStrip.cpp is compiled UNCONDITIONALLY on all
  platforms (not JUCE_MAC-gated), and .github/workflows/build.yml builds ubuntu-22.04 (GCC) and windows-latest
  (MSVC) in addition to macOS. I independently compiled+ran a minimal `std::atomic_ref<double>` program with the
  local Apple clang 17 (Apple clang version 17.0.0) and it worked, so the *macOS* leg is very likely fine (and
  consistent with the lane's own reported 920/920 ctest + working probes) — but nothing in this lane's report
  reconciles the contradiction with the Linux/Windows CI legs, and it is the kind of thing a green macOS-only build
  will not reveal (D9: absence of a build failure here is not evidence for the untested platforms).
  Fix: before merge, either (a) confirm `std::atomic_ref<double>` compiles under the CI's actual GCC/MSVC versions
  (a cheap CI-only check), or (b) if FeatureBus's finding is current, drop atomic_ref here in favor of the other
  s166 L5-sanctioned strategy ("convert the field") or a plain read with the same benign-race rationale the pre-
  existing code already accepted, and note the reconciliation in the report. Not blocking PASS because the actual
  build+test run this lane performed exercises the primary (macOS) platform successfully; flagged because it
  touches "non-Apple builds unchanged" (plan 2.10) and the mismatch was never checked against existing project
  knowledge.

  [NIT] docs/claude/pitfalls.md — Pitfall "NN" is textually inserted BETWEEN existing Pitfall 55 and Pitfall 56
  (which the video lane already merged in), i.e. out of numeric order; CLAUDE.md's index line places "NN" correctly
  after 56. Expected/self-aware per plan Q5/I11 ("Harmony assigns NN at merge"); Harmony must renumber this entry
  to 57 (not 56) at merge time, or move it before 56 in both files for consistency.

  [NIT] ClipInspector::paintKeyNow() reads `clip_->playheadPosition` as a plain double (no atomic_ref), while the
  parallel LayerStrip fix (same lane, same underlying GL-written field) upgraded to atomic_ref. Inconsistent
  treatment of the same hazard within one lane; low risk (pre-existing plain reads of this field already exist
  elsewhere and are unchanged by this diff), but worth a one-line note in the report for L5 to reconcile uniformly.

SUMMARY: 29 files changed (reviewed all touched non-doc, non-probe files plus the 4 new ctest files in full; probe
.py skimmed for gate-threshold fidelity only, not line-by-line). 1 SHOULD (atomic_ref/toolchain reconciliation,
verified concern via source cross-reference + a local compile probe, not blocking), 2 NITs. No MUST findings: I1/
I2/I3/I4 rulings are implemented as specified and independently verified against JUCE 8.0.4 source; no mutex
introduced; non-Apple compile-time gating is structurally correct; deletion order is correct; test coverage for
the FOCUS classes (NativeLayerCache modes incl. a negative "teeth" case, OverlayWatch's 6 rule classes, LayerStrip
I2/I3) is thorough and each test is traceable to a plan/adoption clause.
METADATA: reviewer=reviewer-idlepaint-juce-r1, builder_packet=idlepaint-juce-r1, date=2026-09-29
