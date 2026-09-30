# ATTACK plan-btguard -- VJ seat (blind). V = verified in source today, I = inferred.
Checked OK: AE:14, AE:81-87, AE:95-126, MC:485-498 match the plan; ADM:193-232, :709-727, CA:2190-2216 read as described; file-mode clear() is undone by updateSetupChannels (V).

## MUST
M1. FAIL-OPEN ON A FAILED TRANSPORT READ = guard silently off for the very device that crashes the app (plan 2.2:66, 4.1 "Unknown allowed", 4.2 "error -> 0"). A read error and a truly-Unknown device both become 0 -> allowed. No gate ever reads a real Bluetooth transport (rig has only 'bltn'; test-denied bypasses classify()), so an enumerator bug ships GREEN 23/0.
    Fix: separate read-ERROR (fail CLOSED, hidden, stderr line) from a reported Unknown 0 (allow). E1 asserts every enumerated device has a successful transport read; probe A2/A3 assert transport == 'bltn' (not 0); ctest with an injected property-read error.
M2. THE PROMISED SENTENCE IS TRANSIENT (2.3, 4.6). It goes through setFileLabel, the shared label overwritten by MC:309/328/404/536/554/725/731/1550/1629/1779/2811/2838/2864/3183 (V). Load a clip, save, capture a slot: the "No wired mic found" text vanishes and the show runs on flat features with no words. That is the silent failure the seat forbids.
    Fix: a persistent status indicator independent of setFileLabel while state != ok; gate row: arm B, trigger a label-writing action, assert the sentence is still shown (/api/debug/ui_text or a new field).
M3. C3 CHANGES THE NORMAL MIC PATH WITH NO BEFORE/AFTER BASELINE AND EXCEEDS THE RULING. binding-decisions (2026-09-24): "any fix is at most a guard that keeps the app off Bluetooth devices". Removing the setSourceMode re-open (AE:107-110/119-122) changes every mode switch for everyone; A2-A5/A7 compare the app only to itself (main has no endpoint, so "identical to today" is untestable).
    Fix: make C3 separable and get an explicit decision that it is in scope; if kept, run probe-btguard at C2 (re-open still present), save opened{} + sourceSampleRate, byte-diff at C3 (names, rate, buffer, channels).
M4. NO-DEVICE (rate 0) IS PROVEN ONLY FOR render_frame. sourceSampleRate 0 also reaches MC:4039, MC:5683 (`armOpts.deviceRate` perf-take arm) and RecorderHost CaptureFacts.rate (RecorderHost.h:98) (V grep; behaviour unexamined I). "Launches and renders" is not "safe at a gig with no device".
    Fix: arm C rows: arm+stop a perf take, start/stop video recording, /api/audio/source both ways, assert 200/alive/no .ips; or clamp deviceRate at those sites.

## SHOULD (9)
S1. Silent device swap: Reapply after a wired interface unplug (or mic appearing) reopens with no word to Boris, and an active take/AudioTap gets prepare() with different args (plan argues only equal args, Risk 3). Fix: one non-modal "Input: <name>" line on each Reapply; gate/document take behaviour across Reapply.
S2. Plugging a wired interface while built-in is open is ignored ("Keep") with no indication; VJ thinks it is broken. Fix: status line "Wired input available: <name>" or adopt it (decide with Boris).
S3. Mic-permission (TCC) denied = the commonest real "audio cannot open": device present, state "ok", silence, no words. Not covered though the plan claims no silent failure. Own it or defer explicitly.
S4. Arms B/C hard-code "MacBook Pro Microphone/Speakers" (4.10); breaks on Mac mini / other rigs. Take names from arm A's opened{}.
S5. ctest RED is synthetic (pass-through stub) and the count is self-contradictory (sec 5 says 14 FAIL/5 PASS while listing 16 ids and calling several vacuous); P2 passes on a stub. Fix: define the stub (Bluetooth default) and a real count; make P2 use a non-first allowed default.
S6. Live RED (13 FAIL) is a 404 on the missing endpoint, not the regression named; "Bluetooth default -> built-in opened" is unit-only (M1/D1, M2 control). Say so; do not claim live RED on the regression.
S7. Message-thread cost: enumerateCoreAudioDevices repeats per-device HAL queries on every list change, duplicating JUCE's scan while earbuds negotiate (possible UI hitch, I). Fix: time the scan, log > 50 ms, assert a bound in the probe.
S8. Scope: ~250 lines, 3 files, reconcile/Reapply (new close+initialise on unplug), status/label semantics change, CLAUDE.md content move -- vs "at most a guard". Justify Reapply (ii)/(iii) against path #4 or drop.
S9. Dedup-name drift: removal renumbers "X (2)" -> "X"; reconcile (ii) sees the opened name "missing" and Reapplies needlessly (I, unmodelled). Add a ctest.

## NIT
N1. AirPlay/Continuity denials show "Bluetooth is never used."; use "wireless audio is never used" or a reason-specific text. Plan does label AirPlay/Continuity as its own reading (2.2, 7.5, 11.1) -- good.
N2. A4 (skipped==[]) fails with earbuds connected; note rig-state dependence.
N3. Boris attribution: only the two permitted quotes appear (plan lines 9-10); no over-attribution found beyond N1's UI copy.
N4. Label "No wired mic found" is wrong when the mic is merely unreadable (M1 fail-closed will make that possible); add the reason.
