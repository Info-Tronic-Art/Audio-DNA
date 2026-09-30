# ATTACK PAPER — lane bt2, seat coreaudio (s-rta-0930)

Recovered by Harmony from the workflow journal (wf_d5f2da66-bc5): the seat returned its paper as structured output and did not write this file.

VERDICT: AMEND

## A1 [MUST] AdoptInput has no progress guard and can loop every 10 s. The reconcile rule keys on 'device open, input name empty, lists.inputs non-empty'. The once-per-scan gate resets on ANY new scan, and lists need not change for seq to advance.

EVIDENCE: DeviceGuard.cpp:94 does ++scan_.seq on every rescan. CoreAudio fires list-changed on any HAL device-list event, including a hidden Bluetooth device connecting or disconnecting, where the allowed lists are unchanged (CA:1435 hardwareListenerProc -> CA:2266). ADM:518-606 insertDefaultDeviceNames pairs input and output by a common rate. An allowed input can therefore be listed yet leave the open setup input-empty, for example when the rate pairing fails or the input is opened output-only after a failed open. In that state the plan re-applies at every new scan, at most once per 10 s, and each re-apply is a full close/open on the message thread. R6/R7/R10/R11 only test a NEW device. None tests 'adopt ran, result still input-empty, unrelated scan arrives'.

FIX: Record the outcome. If a re-apply for AdoptInput leaves the input empty, suppress further AdoptInput until the inputs list CONTENT (not seq) changes from the list it last acted on. Compare the list hash rather than seq in the gate. Add a unit case: a permanently non-pairable input plus N unrelated fireListChanged calls gives reapplies == 1.

## A2 [MUST] The plan moves the reconciler from 'only when the manager has no device' to 'also while a device is open' (I3/I4, R1). The plan's R1 argument ('plugging a different device fires no DeviceHasChanged on the open device', I3) is INFERRED and never tested. The close/open can race JUCE's own restart paths and the HAL-thread restartAsync.

EVIDENCE: DG:165-199 today acts only on getCurrentAudioDevice()==nullptr. The old comment (BG4) fenced this deliberately. ADM:193-233 audioDeviceListChanged can itself closeAudioDevice() and re-init synchronously just before the reconciler's 250 ms timer, which then reads a stale 'haveDevice' and stale lists snapshot. The plan evaluates dp::reconcile(device != nullptr, setup.inputDeviceName, scan.lists) on a scan that may be older than the device JUCE just re-opened. Nothing re-reads the scan after the timer fires, so a just-re-opened (valid) device could be closed again. The live probe uses the policy-level deny, which does not exercise the HAL timers (R4/I5).

FIX: In timerCallback, force a fresh guarded rescan immediately before evaluating the table, or compare the scan seq against the device's start seq. Only act if the device in hand was opened on an older scan than the lists being evaluated. Add a mock case where JUCE re-inits itself on the same list change (the vanished-name XML branch, M5) and assert the reconciler does NOT re-apply on top of it.

## A3 [MUST] InputLost rests entirely on INFERRED I1/I2. The dead-device close stall is unmeasured and it is shipped by default (H1 defaults yes). The rule can also misfire on live devices.

EVIDENCE: CA:734-757 stop(false) waits up to 40x50 ms on the calling message thread. The plan itself says 'Not measurable on the rig' (R2). InputLost also fires whenever the open input NAME leaves the allowed list. That includes a device that is merely newly hidden or renamed, which is not dead, and renamed devices on a rate change. R12 admits a dedupe-swap case leaves the combiner dead while the name stays listed. InputLost is then silently missed, with no notice and no re-apply. The original deaf-state defect persists in that corner and is not testable.

FIX: Make I4 opt-in behind the TEST-ONLY gate, or ship it only with a message-thread watchdog measurement. Alternatively, detect the dead combiner directly (ADM audioDeviceStopped with no restart, from the ADM:1091-1101 change message) instead of inferring it from list membership. That also closes the R12 corner.

## A4 [SHOULD] C3's claim 'the re-open changed nothing' is only partly supported and is not test-guarded for real devices.

EVIDENCE: ADM:735-738: when newSetup == currentSetup and a device is open, setAudioDeviceSetup returns early with no re-open. The measured opens=2 at launch (V3) therefore implies the setup DID differ, so the mic block changed something, for example the channel mask. The plan never identifies WHAT differed and asserts 'nothing'. AE1 runs with every device denied, so there is no device and it cannot fail on a regression that re-adds a real-device call guarded by getCurrentAudioDevice(). The only real-device guard is the live probe A6/A6b, which is outside ctest. The startup order is also newly exposed. The 8.0.4 CoreAudio overflow path was previously hit on the second open with the default-BT machine. Dropping it moves the first open to be the only one, which is fine for the BT guard, but no test runs a launch with a default-hidden device and asserts a single open.

FIX: State what setup difference forced the re-open (log the diff on the old code). Add a mock-type (TDP) case that drives setSourceMode-equivalent calls against a device manager that holds an open device and counts starts. Keep the real-engine AE1 as a secondary guard.

## A5 [SHOULD] Restore-on-failure path (H2) sends names through setAudioDeviceSetup(keep,false), bypassing the default-selection policy. The BT guard for that new path is asserted but not tested against aggregates.

EVIDENCE: The plan builds `keep` from previous names that remain in lists. An aggregate whose hidden-member status changed between scans, or a name reused by another device, passes the name filter but the guard refuses on open by TYPE. R12 shows names can be swapped. The failure then yields a no-device state that has just lost the device it had. R11 tests only the simple case. No case covers an aggregate with a newly denied member or the restore itself failing. Also, a failed adoption restoring a stale output re-arms the 10 s bound, delaying the next genuine adoption (R9).

FIX: Add a mock case: aggregate listed at launch, member becomes denied, re-apply then restore. Assert that neither Bluetooth nor the aggregate is ever opened (spy.saw). Do not count a restore as a full re-apply against the 10 s bound.

## A6 [SHOULD] Timer restart on every change message can starve the reconciler, and the 'never twice on the same scan' gate can also swallow a legitimate retry.

EVIDENCE: The plan's changeListenerCallback always startTimer(settleMs_) (restarts). Every device start/stop and every list change sends an ADM change message (ADM:1088, :1093), and a dying combiner or a flapping USB hub produces a stream of them. With restart semantics the 250 ms timer may never fire while the stream continues. In the other direction, the openDefaultDevices() launch scan is recorded AFTER the open, but macOS may publish the new default input after the list notification (I4). A mic plugged in at that moment and published on the same scan seq is permanently suppressed until the next list change.

FIX: Add a max-wait cap (fire at most 1 s after the first change in a burst). Add a test with a continuous change stream. For the late-default case, re-scan once on the timer, not only on the notification.

## A7 [NIT] Live rows depend on the policy-level deny, not the HAL, yet the pitfall text and G7 present 'GREEN 37/0/2 5 of 5' as device-lifecycle proof.

EVIDENCE: I5/R4 admit the twin covers the guard decorator, JUCE's list-change handler and the reconciler, but not per-device timers, dead-input combiner shutdown (V10) or restartAsync races. The InputLost HAL path has zero executed coverage.

FIX: Word the pitfall and gates as 'policy path verified; HAL path by source read only', and list a one-time Boris/hardware check (B2) as a release precondition for I4, not an option.

