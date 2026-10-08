# Re-check of answer-tempo-auto.md (Boris's L32: can a hand correction hold in automatic?)

Checker: read-only. Nothing built, run, probed or launched. No lane worktree read. Main HEAD a86cf0a.
Verdict: SOUND_WITH_CORRECTIONS. Every code fact the paper cites that I opened is right. The weak points are the framing of the "ANSWER FOR BORIS", two over-readings in the facts, one missing UI fact, and two simpler options not weighed.

## FACTS RE-CHECKED

All line numbers below are from the files themselves, opened by me.

- F1 CONFIRMED. BPMTracker.cpp:99-104: Manual returns after `updatePhase(false, 0.0f)` with `predictedBeatRegime_ = true`. No heard beat reaches the phase.
- F2 CONFIRMED. cpp:234-236 `if (beat && conf >= kBeatResetConfidence) realignPhaseToZero()`; BPMTracker.h:57 = 0.5f. Independent of the tempo number.
- F3 CONFIRMED. cpp:170-204. h:48 = 200, h:54 = 2.0f. 200 x 512 / 48000 = 2.13 s (arithmetic checked). Strictly "greater than 2.0"; the count only moves on confident hops (conf >= 0.1, h:51), so "in a row" means in a row of confident steps.
- F4 CONFIRMED as an inference (diff > 2.0 test; 128 against a heard 126.x). Sound.
- F5 CONFIRMED. cpp:607-622 sets locked, candidate, lastConfident, STATE_LOCKED, counter = 200; medianBuffer_ untouched (311-326). Note: typed and step entry exist only in Manual today (see MUST/SHOULD 3 below); in automatic only Tap reaches this.
- F6 CONFIRMED. MainComponent.cpp:5824-5827 (tap: setManualBPM only), 5840-5845 (resync: no setManualMode). `manual` and `link` call setManualMode(true) (5832, 5850).
- F7 CONFIRMED. cpp:566-578 does not write lockedDownbeatPos_. It also leaves downbeatLocked_ alone.
- F8 CONFIRMED. cpp:343-346 (scoreBeat only on a heard beat) and 377-389.
- F9 CONFIRMED as text (384-388, 438-453, h:62 = 8). The INFERRED reading is too strong; see finding 4.
- F10 CONFIRMED (cpp:390-401). Scope is narrower than the paper implies; see finding 4.
- F11 CONFIRMED. cpp:293-309 (ratio 1.8..2.2 and 0.45..0.55).
- F12 CONFIRMED. h:41-42 = 60 / 200; cpp:281-291; cpp:614 folds a hand tempo. A hand "/2" below 60 is folded UP today, so L14's 22..480 cannot work without change. The paper's T4 remark is right.
- F13 CONFIRMED. /opt/homebrew/include/aubio/tempo/tempo.h lists only new, do, get_last (_s, _ms), set/get_silence, set/get_threshold, get_period, get_bpm, get_confidence, set_tatum_signature, was_tatum, get_last_tatum, get/set_delay. No tempo-hint or range call. Cellar shows aubio 0.4.9_4. CMakeLists.txt:16 `find_package(Aubio REQUIRED)`.
- F14 CONFIRMED. tempo.h:70, 77, 84. BPMTracker.cpp:52-59 reads only the beat flag, bpm and confidence.
- F15 CONFIRMED (arithmetic). cpp:125-130. 2/128 x 2.133 beats/s = 0.0333 beat/s, so a quarter beat in 7.5 s. Estimate at 128 BPM; the paper does say INFERRED.
- F16 CONFIRMED. h:61.
- F17 CONFIRMED. plan-nudge-row2.md adoption block (heading near line 814): "a hand Resync is a run-queue command that starts a held beat. The nudge survives stop and play." 145 B keeps "nudge X ms" between the buttons. Nothing on hold-in-automatic.
- F18 CONFIRMED. area-tempo.md U-HIS-4 (line 109) and O-17 (line 88): NA-17 rested on the reading R76 and had only inferred consent.
- F19 CONFIRMED verbatim. L14: "The /2 and x2 manual work to both limit as well as the listening clock." and "We only need to display the nudge XMS in automatic mode."
- F20 CONFIRMED. L13 verbatim. realignPhaseToZero never touches beatInBar_ (253-258); Tap realigns the phase (setManualBPM, realign true), the typed and "link" path does not.
- L32 question text quoted in the paper matches the numbered file exactly. L18 quote matches.
- U3 CONFIRMED: area-tempo.md:158 "in Auto a small later nudge holds one hop on 199 of 200 beats"; HANDOFF line 56: nudge S1 at a8afcfb built and gated, NOT merged.
- "his three tracks": CONFIRMED that they are Boris's to supply (binding-decisions.md:1040 "I will get you the 3 audio tracks in another session", :1098 "We will have the dj tracks later today"). They have NOT arrived; the transport lane's measurement stage is blocked on them.
- Word count of the ANSWER block: 144 (limit 150). OK.
- COULD NOT CONFIRM (by rule): U1 (do aubio's beats land on the kick when its number is 2 off), U2, U4. Honestly marked UNKNOWN in the paper.

Checked: 24.

## FINDINGS

1. SHOULD (ANSWER FOR BORIS). It opens "Yes to both" and then says it can only be promised after a measurement. The paper's own fallback says that if U1 fails the answer becomes "no, a correction needs Manual". The load-bearing premise (aubio's beat flag lands on the kick even when its number is off) is unmeasured, and the tracks he promised have not arrived. Fix: lead with "Yes, it can be built so", say once that whether it works on his music is unknown until tested on his tracks, which are not yet here.

2. SHOULD (ANSWER FOR BORIS / Recommendation). L130 asks that assumptions be shown as "Decided without asking you -- say so if one is wrong". The answer silently reverses NA-17 ("a tempo step switches Manual on", told to him as R76; he asked what it means and never confirmed it) and decides that his "1" is carried by the app's own clock. Fix: add one such line.

3. SHOULD (ANSWER FOR BORIS, "Tempo"). It says "your number" without saying how he enters it in automatic. Today the typed field and the +/- are visible only in Manual (TopBar.cpp:101-104 `bpmEditField_.setVisible(manualMode_)`; tempoLabel_ shows the heard integer otherwise). In automatic only Tap sets a number. The paper's T1 lists "typed, +/-" as if they exist in automatic. Fix: say "tap, or type it" and carry a one-line question or note on where the field shows in automatic (L8 allows laying it out, ask if unsure).

4. SHOULD (FACTS F9 / F10 / O4). Over-read. downbeatLocked_ is only ever set true (cpp:476; grep shows no reset), so F10 (Resync overwritten at the next heard beat) applies only before the first lock, about 8 heard beats after the app starts. F9's relock fires only when the listener's own best position differs from its own earlier lock for 8 checks in a row (cpp:441-452); disagreeing with his hand does not count. So today a hand "1" survives on a stable track for a long time; the real threat to it is F8 (a missed or extra heard beat) and the one-time relock. The recommendation still stands, but O4's reason ("takes his 1 back at a moment he cannot predict") should be restated this way before Harmony reuses it.

5. SHOULD (OPTIONS). Two simpler or safer options are not weighed. (a) A trim: his correction kept as an offset on top of what the listener hears (+2 BPM, or a one-beat turn of the count), so automatic keeps following tempo changes and there is no band and no "let go" question. It fails only if the listener's error is not constant, which U1 would show. (b) If U1 shows aubio's beat flag is not on the kick, pull the edge with the app's own kick/onset detection (the snapshot already carries onsetCount, Pitfall 30) instead of aubio's beat. Without (b) the paper's fallback is "no".

6. SHOULD (ANSWER FOR BORIS, plainness). "the edge of each beat", "centre", "kick" are the words he would have to ask about; "the listening only keeps the edge of each beat on the kick" is opaque. Replacement below.

7. SHOULD (T2 / F15 numbers). "A quarter of a beat in about two and a half minutes" and "7.5 s" are my-arithmetic estimates at 128 BPM; the paper marks the section INFERRED but does not say "at 128 BPM". Not in the ANSWER, so only a note for the paper.

## A BETTER "ANSWER FOR BORIS"

Yes, I can build it so your correction holds and the app stays on automatic. Whether it works on your music I can only say after I test it on your three DJ tracks, which I do not have yet.

Tempo: you set the number (tap, or type it). The app keeps listening but only accepts tempos very close to yours, so it follows a slow pitch change and ignores the rest. "/2" and "x2" work the same way.

The 1: when you press Resync, that beat is the 1, and the app counts on from it. It no longer guesses the 1 itself. The nudge moves it a hair.

Manual stays as your own switch.

Decided without asking you, say so if one is wrong: Tap, Resync and the tempo buttons will not switch Manual on by themselves.

(about 125 words)

Keep the paper's QUESTION FOR HIM as it is. It is one focused question, not a reading.

Written (system clock): see date line below.
Wed Oct  7 23:03:19 EDT 2026
