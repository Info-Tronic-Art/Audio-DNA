# Reviewer Verdict - btguard gates r2
STATUS: DONE
VERDICT: APPROVE (no MUST; 4 SHOULD, 4 NIT)
REVIEWED: lane/btguard head 9f4623e (fix round over 647ad69; whole lane over d88d2ea read via git show/diff). Mutation proofs in $TMPDIR copies built from `git show 9f4623e:` blobs, relinked against build-lane's JUCE/Catch objects; the worktree was never touched.

## Answer
No defect ships and no test in the fix round is unable to fail. Every mutant the lane claims is killed reproduces exactly (VERIFIED, executed); one sibling default-selection mutant class survives all 36 cases (SHOULD). The live GREEN of probe-btguard on the head app was never run (lane STOPPED at a TCC prompt), so the new notice wording and C5b are unproven live.

## VERIFIED (executed by me)
- Real head sources rebuilt from blobs: 36 cases / 201 assertions, all pass.
- Lane mutation table reproduced with identical failing sets: MA passthrough default -> D1 D4 M6 M7; MB no built-in rung -> D1 M6; MC no coalescing -> R1; MD no seq gate -> R4b; ME no rescan -> D5; MF/MF2 default flags false -> E1 (real HAL); MG unfiltered index -> D6.
- Extra mutants I added, all killed: create-no-input-check -> D2; seq never incremented -> D3 R4b; lastReappliedSeq never set -> R4b; min-interval removed -> R4; re-scan result not remapped -> D5; USB rung removed -> P12; ccwl allowed -> P6; guard default ignores direction -> D1 D4 M7.
- SURVIVORS (all 36 green): policy default flag direction `return input ? d.isDefaultInput : d.isDefaultOutput` replaced by `d.isDefaultInput` (MQ) or `d.isDefaultOutput` (MR). Fixture PX (Built-in Mic default in; duplex USB Interface default out; Built-in Speakers): passes on real code (37/37), FAILs on MQ and MR (PX only).
- BG5: Spy records created/opened/started names incl. JUCE's temporary devices; every M/R case asserts !spy.saw(kDenied); control M2 asserts the plain manager DID open the denied name. Tests drive the real GuardedAudioDeviceManager + DeviceReconciler over a mock inner type; E1/E2 use the real CoreAudio HAL.
- BG8: probe tally is `grep -c '^PASS'` / `'^SKIP'` separately; D1/D2 emit `row SKIP` when the rig lists < 2 allowed outputs; a SKIP is never a PASS. The GREEN banner still prints with SKIPs (D is not proven live on this rig).
- BG9: every getenv of ADNA_AUDIO_DENY_DEVICES (DeviceGuard.cpp:207) sits in #if AUDIODNA_TEST_SERVER; Config::testDeniedNames (DevicePolicy.h:72) and "test-denied" (DevicePolicy.cpp:61-63) too; the fix round touches none of it (2 string literals in MainComponent only). Full TEST_SERVER=OFF strings check is the lane's/r1's run, not repeated.
- E1 + A2/A3: E1 asserts uid "BuiltIn*" devices read transport == 'bltn' EXACTLY, transportReadOk, != Unknown; probe A2/A3 assert transport == 'bltn', transport_read_ok true, len(rows)==1.
- No stray: git status = only untracked build-lane/; no .venv in tree, no env-var hook outside TEST_SERVER; fix round diff = tests, probe, 2 literals, docs. CLAUDE.md = 23,818 B (<= 25,000). Real-time: audio callback untouched; reconciler/guard message-thread only; no new mutex.
- Docs: pitfalls.md guards text updated (M6/M7, R1 settle, R4b, E1 flags, new wording); APP-INVENTORY 1003 / 108 = 967 + 36. No stale old-wording copies outside .harmony/undo-v1-ledger.md (historic).
- C5b teeth (by reading, INFERRED): without the perfRecord clamp, arm() with audio + rate 0 either refuses (no take.json -> FAIL) or finalizes with mode "input" / lastError set (-> FAIL). PASS on the pre-fix (clamped) app was observed by the lane. C5b RED not observed.

## SHOULD
S1 Policy default-flag direction untested (above): add PX to tests/test_device_policy.cpp; kills MQ and MR.
S2 Head app never run under probe-btguard (0 of 5 GREEN runs after the fix): notice wording B3/B3b/C2 GREEN and C5b are static-only; pre-fix RED (22/3/2, exactly B3 B3b C2) is the lane's executed line. Harmony's gate must run it, after the microphone-permission dialog is cleared (a live UserNotificationCenter window makes every arm's quit_check FAIL).
S3 New notice may clip at the default/minimum window width (INFERRED, PIL font metrics, not JUCE's own): MainComponent.cpp:2630 caps the notice at row1.getWidth()/2 = ~353 px at 1280 (area 1272 - 565 px of left controls), while the new sentences measure ~355-420 px at 14-15 px Helvetica/SF (old ~275-300). The plan comment says "keeps its whole sentence". BG6's critic judged shots of the OLD text; re-shoot BTGUARD_SHOTS=1 at 1280 wide, or raise the cap (e.g. 2/3).
S4 BG7 video-recording start/stop with no device is not gated live: verified there is no REST route (only /api/perf/record and /api/snapshot) and VideoRecorder reads no audio rate; Harmony must accept the gap explicitly.

## NIT
N1 The lane's "old suite 30/30 on 8 mutants" rests on scratch mutate.py outside the tree; I re-derived the new-suite half independently.
N2 E1 hard-requires builtIn >= 1 and flagged == 1 per direction: fails on a host with no built-in audio (pre-existing, CI continue-on-error).
N3 Real-HAL aggregate `membersReadOk` branch is unit-only (declared).
N4 A C5b-RED run against the clamp mutant would leave an orphan Audio Store asset (the probe removes only the take folder).

METADATA: reviewer=reviewer-btguard-gates-r2, builder_packet=btguard-fix, date=2026-09-30
