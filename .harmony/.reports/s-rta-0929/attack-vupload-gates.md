# Attack: plan-vupload.md gates (perf-gate skeptic)

VERDICT: ship-blocking gaps in the budget's own concurrency/scale gates and the
QoS fix's lifecycle coupling; the P4a bug and w1c/u4a RED-on-main claims are
independently verified against HEAD (7c95c4e = adf9b8a + asyncload, no
functional drift in the touched files).

## MUST

1. **Budget is not a hard per-frame ceiling once >4 players synchronize, and nothing gates that.**
   `Budget::admit` (4.1) force-admits when `deferredFrames_ >= maxDefer` REGARDLESS of `cap`. The plan's own
   fuzz ctest (4.8 test 6) accepts `used <= cap + (requesters at the bound)` as PASS — it explicitly permits
   `used` to exceed cap when several players hit their defer bound the same frame: the very bunching
   pathology P1 claims to fix, reproduced at N>4. Every gating probe row (w1c, w2c) drives exactly 4
   synchronized clips; risk 1/OUT name 8-16 players "headroom only, unproven" but nothing asserts the
   budget still bounds per-frame uploads there. Fix: a probe row or ctest with 8+ synchronized triggers
   asserting per-frame uploads <= cap+1, or drop the unqualified "a per-render-frame video upload budget"
   language in section 1/12 for an explicit N<=4 guarantee.

2. **peek()/pick() divergence under a live writer is named (risk 8) but never gated.** `Ring::peek` (4.2) is
   const/no-CAS, so the decode thread can publish a new Ready frame between a player's `admit()` decision
   (based on peek) and the later `pick()` — cross-thread in production. The new peek ctests are static/no-
   writer; `test_video_player_gl` runs with NO decode thread ("deterministic"); "the two-thread stress case
   re-run under TSan" (4.8) is the EXISTING `test_video_ring` case, predating peek/Retire. Risk 8's "the
   ctest asserts equality" is a single-threaded claim, not a concurrency proof. Fix: a TSan two-thread case
   racing publish() between peek() and pick(), asserting request/used/deferred accounting stays consistent
   when pick returns something peek didn't see.

3. **QoS fix is bound to the wrong object's lifecycle.** `pthread_set_qos_class_self_np` runs in
   `Renderer::newOpenGLContextCreated` (4.5), but it affects `juce::OpenGLContext`'s process-wide singleton
   `SharedResourcePointer<RenderThread>` (verified: `juce_OpenGLContext.cpp:966`, thread body :888-892
   matches the plan's own citation). Output windows attach their OWN context (`src/ui/OutputWindow.cpp:10-
   71`, unpatched) sharing that thread. If the singleton thread is torn down/recreated while only an Output
   window's context is live (main preview minimized, output windows kept open — legal per the "no stop
   model" ruling, BORIS_DECISIONS.md:342-349), the QoS boost silently reverts to DEFAULT until the
   Renderer's own context next initializes; `gl_thread_qos` stays stale. u2 samples once per launch and
   never re-checks after such a cycle. Fix: re-assert QoS from every context's `newOpenGLContextCreated`
   (incl. OutputWindow::Presenter's), plus a u2b row: detach preview, keep an Output window open, recheck.

## SHOULD
Test 6's bound (`used <= cap + (requesters at the bound)`) is structurally a tautology once force-admit
exists — it cannot fail for any `maxDeferFrames <= kMaxDeferFrames` and should not count as evidence for the
"no starvation, no re-bunching" claim in section 1/12.

## NIT
Citations taken at `adf9b8a`; HEAD already drifted `VideoPlayer.h` by +2 lines (asyncload's `open()` comment;
`kSlots=3` is now :153, plan cites :151). Re-verify against HEAD before commit 1.

## Verified against current HEAD
P4a bug reproduced at `CompositorEngine.cpp:1106-1128`/`:1372-1385` (clipTex==0, pending==false ->
`applyFXOnlyLayer`); `uploadToTexture`(:397-483)/`releaseGL`(:485-500) match exactly, `ring_.release` fires
right after upload (:447-448) — the bug is live today. Pitfall 56 (`docs/claude/pitfalls.md:124`) states the
"never 0/no media" contract this repairs; no conflicting ruling in BORIS_DECISIONS.md :320-360.

## Strongest counterargument to this attack
The plan ships risks 1 and 8 as named, open risks rather than hidden claims, and the only case the app
currently exercises in probes (4 synchronized players) is well-gated. My MUSTs ask for coverage of a scale
and a race the plan's own OUT section already flags as unmeasured "headroom." I hold them because section
1/12's compact description reads as an unqualified guarantee ("never a skipped content frame... no layer
starves") with no N<=4 caveat — and that unqualified sentence is exactly what a future regression is
checked against.
