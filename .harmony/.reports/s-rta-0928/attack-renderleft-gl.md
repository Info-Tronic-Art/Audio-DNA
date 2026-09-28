# Attack: plan-renderleft (GL + threading skeptic), s-rta-0928

VERDICT: SOUND_WITH_FIXES

## MUST
1. R1.2 (commit 5)'s capture-gate line "if (compositor_.framePendingImages() > 0 ||
   legacyPendingThisFrame_) return;" (plan ~545-551) reads legacyPendingThisFrame_, but that
   field is declared as a NEW Renderer.h member only in R1.3 (commit 6, plan ~594). The plan's
   own comment flags this ("legacy flag: R1.3 (false until then)") but never resolves it: commit
   5, as literally written, does not compile, breaking the plan's own COMMIT SEQUENCE guarantee
   ("each builds ... each reverts alone", plan line 887). Fix: declare the field (default false)
   in R1.2, or move the gate line into R1.3.

## SHOULD
2. compositor_.pumpImages() -- the only site that drains the Mailbox, uploads decoded textures,
   applies postImageSet eviction (glDeleteTextures), and dispatches the next prefetch job -- runs
   only "the first statement inside if (deckActive)" (plan line 544). Verified: Renderer.cpp:233
   ("bool deckActive = (deck != nullptr)") and the early return at :327-334 (no image/source/deck)
   which still calls processPendingCapture() but, per the plan, never ran pumpImages() that frame.
   postImageSet (fired from swapCompositionModel) is NOT gated on deckActive, so during any
   no-active-deck window (startup, source-only mode) decode/prefetch jobs keep completing while
   drain/upload/eviction/next-prefetch-dispatch all stall -- the exact leak/staleness class R1-d
   claims to fix. Not covered by R1-e ("every caller and path") or R1-f. Fix: run cache
   maintenance unconditionally near the top of renderOpenGL.
3. The per-frame upload-budget spillover for readyImages_ is prose-only ("on Upload if
   (!uploadBudget_->take(bytes)) break; (it stays for next frame)", plan ~490), with no matching
   state in the ImageTexCache::Cache API. onResult (which mutates Cache state) runs BEFORE the
   budget check, so a budget-exhausted item is already consumed from the Mailbox and onResult'd,
   but never onUploaded -- and no ImageTexCache test case (plan 554-562, 667-672) exercises
   "budget exhausted mid-drain, resumed next frame." Worst case: a decoded image never becomes
   resident, contradicting R1-b's explicit "No cap" on hold duration. Fix: keep readyImages_
   un-cleared across the break so the loop resumes at the same entry next frame; add a test.
4. ~Decoder() calls pool_.removeAllJobs(true, 5000) (plan 468, FilesBrowser precedent), run from
   ~Renderer() member teardown -- AFTER detach() already blocked the destructing thread until the
   GL thread closed (Renderer.cpp:21-24, F12). Up to 5 more seconds added to app quit, unverified
   against Boris's own quit-timing rig rules (HANDOFF.md:36-44, quoted in the plan's Builder step
   0). Inherited, not independently justified for this call site.

## NIT
5. "MUST NOT CHANGE / Pitfall 37" claims R2 keeps "same span" for the TEST-ONLY lock (plan
   809-810), but R2's own APPROACH text says the lock "is released before convert + encode, so
   concurrent callers still overlap" (line 33-34) -- strictly narrower than today's span, which
   holds across all of captureFrame including PNG encode (TestServer.cpp:597-605, verified). The
   narrowing is the intended fix, not a bug, but the "unchanged" framing could let a reviewer sign
   off Pitfall 37 without noticing the span shrank.

## Strongest counterargument to this review
Findings 2-3 are narrow-state-window gaps that may never trigger in the app's normal
"composition always has an active deck" usage, and 1 is a one-line fix a competent builder makes
on sight regardless of the spec's literal ordering. The plan's core promises for this temperament
-- no GL call off the GL thread, try_lock-only on the GL side, weak_ptr-guarded job delivery,
removeAllJobs before teardown, retire-list media lifetime unchanged, no new FBO, no history-key
change -- all check out against the current code (Renderer.cpp:161-334, CompositorEngine.cpp:90-306,
Renderer.cpp:2035-2179, TestServer.cpp:580-605). I still hold MUST on (1) because the job here is
to find where the plan AS WRITTEN ships a broken intermediate commit, and that gate is unambiguous
and self-flagged by the plan's own inline comment.
