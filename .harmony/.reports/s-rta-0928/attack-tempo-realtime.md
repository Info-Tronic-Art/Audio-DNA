# ATTACK -- real-time/concurrency seat vs plan-tempo.md (against main @ 6db8d67)

VERDICT: SOUND_WITH_FIXES

## Core mechanism: verified correct
The release-fetch_add / acquire-load sequence-counter (2.1) matches the codebase's own idiom:
BPMTracker.h:279-296 shows the existing request atomics already use *relaxed* ordering because
"the counter change IS the message" (BPMTracker.cpp:555-556) -- no separate payload write needs
synchronizing there. requestSeq_ as an added release/acquire fence over three independently-
relaxed atomics is the standard seqlock-counter technique: fetch_add is an RMW, so every release
fetch_add joins one release sequence, and any acquire-load >= N happens-after every write
sequenced-before any fetch_add that produced <= N. runPipeline's real first statement is the
tempoRequest_.load(relaxed) check at BPMTracker.cpp:74, so the latch insertion point is exactly
what the plan claims. Resync's late application (BPMTracker.cpp:352-357) vs an early latch is
correctly reasoned as conservative (applied-but-not-claimed), confirmed by the code at 352.
bpmTracker_ is constructed unconditionally at AnalysisThread.cpp:33 and never reset (no
analysisThread_ reassignment anywhere), so getBpmTracker() is never null in practice -- the
plan's null-check is dead defensiveness, not a live bug. FeatureSnapshot offsets check out:
totalBeatCount really sits at 324 (FeatureSnapshot.h:178), so trackerRequestSeq at 328 with
sizeof staying 384 (FeatureBus.h:127-132, kSnapshotWords=96) is correct -- it rides the existing
seqlock for free. tick() is confirmed message-thread-only (RecorderHost.h:19-20; invoked only
from MainComponent::tickFeaturePipeline via a 120Hz juce::Timer, MainComponent.h:311), so the new
unique_ptr<FeatureSnapshot> allocation there is not a cross-thread hazard. Audio callback and
analysis hot path gain zero allocation/locks -- verified, one extra acquire load per hop only.

## MUST -- the 100ms fallback has a near-zero margin the plan's own numbers expose
Section 2.2 computes the *legitimate* wait as "~95 ms at 4096 @ 44.1 kHz" against a fixed
kStartWaitFallbackSeconds = 0.1. That is the exact failure mode the plan uses to reject option A
("50ms is shorter than the legitimate wait with 2048-frame buffers ... A would fall back
routinely on such devices") -- but chosen design C has the identical problem at larger, common
buffer sizes (Bluetooth/AirPods, "safe" CoreAudio settings routinely negotiate 2048-4096). A ~5ms
margin against message-thread scheduling jitter (NORMAL priority, competing with UI paint/timers)
will trip the fallback under ordinary operation, not just "a stall" -- silently reproducing the
exact bug this plan fixes (bpm-0 start / unknown grid) for users on larger-buffer hardware, with
only a stderr line no non-technical user will see. Section 6's probe (W1-W4) never varies device
buffer size, so this near-miss is untested by the plan's own gate; R4 treats the fallback as a
stall-only edge case, conflating it with the normal-operation risk at common buffer sizes. Fix:
scale the fallback off the observed device-buffer period (deviceRate/ArmOptions already carries
it), not a fixed constant -- or explicitly test the live probe at 2048/4096-sample buffers before
claiming GREEN.

## SHOULD -- the "squash window" for onset markers and Decaying-gesture ends widens quietly
RecorderHost.cpp:433-515 today runs onsetCountBaseline_ setup, synthesizeIdleDecayingEnds, and
periodic/early-tempo saves unconditionally on the *first* message-thread tick after arm (<=~8ms
window). The plan (3.5) moves this whole block behind clock_.started(), consistent
architecturally, but stretches the "silently absorbed into t=0" window from <=8ms today to
~10-100ms whenever a tempo command races Record -- onsets/gesture-releases in that widened window
are dropped from markers or mis-timestamped exactly when a user Taps right before Record. No
G-case in 5.3 measures marker loss under the wait; section 4/10 assert "still consumes
onsetCount deltas (G6)" without quantifying the widened blind spot.

## Self-counterargument
The fallback and the squash-window both already exist today (arm-to-first-tick gap, R3), and the
plan honestly scopes them as "the same class, just longer," not an omission. If Boris's actual
rig runs small buffers (128-512, matching CLAUDE.md's stated 2.67ms real-time budget), the 100ms
margin is enormous and the MUST is moot in practice. I hold it anyway: the near-miss is the
plan's *own* 2.2 arithmetic, not my speculation, computed at a buffer size CoreAudio does
negotiate, and the live probe suite never exercises that configuration -- so the gate cannot fail
on exactly the scenario the plan itself derived as risky.
