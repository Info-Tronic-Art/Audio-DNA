# Seat papers, verbatim -- lane "transport-answers" (blind council, 2 of 2 seats returned)

Source: the dispatch text of the architect ruling (the workflow passed the papers as one JSON array, WHOLE, 18130 characters
as stated by the dispatch; the array is closed). Below: the same JSON, pretty-printed, one section per seat. Nothing is
added, removed or re-worded inside the JSON. The ruling on these attacks is `ruling-transport-answers.md` (same folder).

Transcription check: the raw array as written here is 18130 characters (the dispatch states 18130).

## Seat: gates (8 attacks)

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "TR25 (h), MU-103, TL-U81, Boris check 26 (Cmd+Z never changes the pause)",
      "claim": "The Undo arm cannot go red. The order it prescribes (pause A first, then rename A, then undo) puts paused=true into the rename command's BEFORE snapshot. MU-103 (undo lands the snapshot's pause) then lands true over true and passes. Check 26 has the same order, so Boris's own check is blind to the defect he named in B6.",
      "evidence": "plan-transport-answers.md:637-639 'with A paused, rename A (a command), POST /api/debug/undo: A paused still true'; :719-721 check 26 'Pause a clip, change something else (rename...), Cmd+Z'. src/core/ClipCommands.h:80-110: SetClipCmd holds whole-Clip value snapshots before_/after_ and undo() does 'cell = *state'. Boris BD:764-766 (via plan B6): cmd-z does not affect pause.",
      "severity": "MUST",
      "proposed_change": "Reorder (h) and check 26: rename A while it is NOT paused, then pause A through the route, then undo (and redo) the rename. Assert A still paused and its place unchanged, both live and idle. State in TL-U81 that its snapshot's pause differs from the cell's. Run MU-103 against that order and show it RED."
    },
    {
      "id": "GA-2",
      "target": "TA1 'THE BPM TIMER', TL-U44 / TL-U88 / MU-107, check 27, TR24b (B10: 'the BPM goes to zero nothing moves')",
      "claim": "The plan says tempo 0 already stands a synced clip still ('OFF row (tempo 0 -> rate 0)'). No gate can show that. TL-U44 asserts only 'speed pushed is exactly the rate' at tempo 0, which passes at whatever rate the code computes. At the pin, bpm<=0 skips setSpeed, so the player keeps its last speed and the clip keeps running. `beatRunning` is built as constant true, so TL-U88 and MU-107 test a parameter nothing ever sets false. TR24b and check 27 are outside the pin or wait for another lane.",
      "evidence": "src/render/Renderer.cpp:1657-1665: 'if (snap.bpm > 0.0f && clip->beatDivision > 0.0f) player->setSpeed(...)' with no else, so tempo 0 leaves the old speed. plan-transport-answers.md:177-181, :148-150, :551-553, :567-571, :785-786. The ruling's OFF row (plan-transport-delta2.md:423) says 'speed pushed = the rate exactly' and never says rate 0. Boris BD:918-920.",
      "severity": "MUST",
      "proposed_change": "Add one case, RED-first on the stage base: bpm 0 in BPM Sync pushes speed 0 and asks for no seek. Add a second that fills `beatRunning` from a fake snapshot (the Renderer line, not a constant). Otherwise strike the sentence 'already stands the clip still' and list B10 as undelivered by this lane."
    },
    {
      "id": "GA-3",
      "target": "TR32 timeline_speed_ten vs O6t / X6t",
      "claim": "TR32 hard-codes 10 (stored 10, content rate [9.5,10.5], S fader travel 1.0), but the plan's own row O6t lets the top fall to 8 or 4 as data (`kTimelineSpeedMax`) and K-S1 predicts late frames at x10. If O6t fires, the merge-2 row cannot pass. TR32's 5 % band is also looser than X6t's 3 % for the same quantity, against 'a bar is met or reported, never loosened'. The wrap rate is not defined for a 0.8 s loop (reads straddle wraps).",
      "evidence": "plan-transport-answers.md:643-647 (TR32), :399-404 (X6t 3 %, O6t lowers the top), :781-782 (K-S1), :500-502 (never loosened). TA-9 REST clamp 0..10 is the only written source of the 10.",
      "severity": "SHOULD",
      "proposed_change": "Write TR32 in terms of `kTimelineSpeedMax` read from the build (speed = Max, 12 -> Max). Take its rate band from X6t (3 %). Say how the rate is unwrapped across loop points. Say whether the row is expected to fail when O6t fires."
    },
    {
      "id": "GA-4",
      "target": "TA-5 / TA-6 verdict 'met': what the probe measures vs what ships",
      "claim": "X2..X8 on M1..M8, including the Tap, Auto and Resync bars, are measured on the probe's branch, which 'issues the seek itself'. That is a re-implementation, not applyCut / transportSpeed / syncMedia. After S4c only 'M2 at one tempo and M4 again' plus TR20-TR23 run on the shipped path. The Tap (X5: at most 2 cuts over 8 Taps), Auto realign (K-A4, the plan's own weak spot) and the K-A6 three-clip Resync (no bar owner, 'reported') are never re-run on the lane build, so a live row can pass without reaching the shipped code.",
      "evidence": "plan-transport-answers.md:222-226 (probe branch issues the seek itself), :479 and :680 (only M2 and M4 re-run), :238 (X5), :763-764 (K-A4), :767-770 (K-A6).",
      "severity": "SHOULD",
      "proposed_change": "After S4c re-run M3 (Auto), M5 (Taps and Resync with three synced clips) on the lane build with the same bars. Give K-A6 a bar now (peak frame time <= 50 ms, as X6) and make it a gate, not a report."
    },
    {
      "id": "GA-5",
      "target": "TL-U49 and TL-U43 wording",
      "claim": "TL-U49 is conditional: 'if the change leaves it more than 0.10 beat off, exactly one seek'. A fixture whose change leaves the clip in time satisfies it vacuously, and MU-71, MU-76 and MU-105 then lose their teeth on that case. TL-U43's second clause ('is within 0.05 beat inside 0.10 beat of music') has no checkable meaning. TL-U85's 'way back gives the same travel for 0, 0.5, 1, 2, 4, 10' lists speeds as if they were travels.",
      "evidence": "plan-transport-answers.md:565-566 (TL-U49), :509-511 (TL-U43), :518-520 (TL-U85).",
      "severity": "SHOULD",
      "proposed_change": "Make TL-U49 one fixture per change with the off-by figure asserted first (>0.10), then the one-seek assertion. Rewrite TL-U43: the 0.08-off clip never gets a seek, its |e| <= 0.05 within N beats of music, and the pushed speed leaves exactly 1 only while |e| > 0.05. Test the travel round trip on travels (0, .25, .5, .75, 1) and the speed round trip on speeds."
    },
    {
      "id": "GA-6",
      "target": "STATUS pins: EXPECTED_ROWS=31 at merge 2 vs RA-11",
      "claim": "The count pin is stale on its own fallback. Line 259-260 lets merge 2 go without S4c ('TR20..TR22 wait'), and TR23 and TR29's re-stated clauses depend on the cut. Line 481 and 5.3 still pin 31 rows with no alternative count. A merge without S4c would either fail the pin or tempt someone to edit it by hand. The same applies to G-U1's N >= 121 and the 'MU-60..MU-110' mutant set when S4c is absent.",
      "evidence": "plan-transport-answers.md:259-260, :481, :599, :579 (113 cases) and :594-596 (N >= 121).",
      "severity": "SHOULD",
      "proposed_change": "Pin the merge-2 row and case counts as a table keyed by 'S4c built / not built' (31 / 28 rows; 113 / the count without U44-U49, U88). Have S0 recount both."
    },
    {
      "id": "GA-7",
      "target": "TA1 hold band (0.05-0.10) vs 71 B; F-A1, D1 / H1 as INFO",
      "claim": "Boris chose 'No slide: the clip plays on from where it is and cuts once'. The hold is a speed correction (factor 0 to 2) that stands a clip or doubles it, and it is built before anyone knows it is needed. D1 and H1, the only numbers that would justify it, are INFO with no bar, and the plan itself says 'if D1 is at most 1 the hold buys nothing' (K-A1). Its own gate TL-U45c may fail at 0.05 and the builder only stops. So a deviation from his words ships on no measured need.",
      "evidence": "boris-clarify-71-74.md:12 (B as asked); BD:902; plan-transport-answers.md:125-128, :211-212, :749-757, :155-156. Adoption block, plan-transport-delta2.md:1076-1079: 'NO SLIDE IS BUILT ... plays on and seeks once'.",
      "severity": "SHOULD",
      "proposed_change": "Make the hold conditional on a pre-registered number: build it only if D1 > 1 per 10 min on at least one track. Otherwise set kGridDeadbandBeats = kOutOfTimeBeats (the runner-up, one constant). Tell Boris in plain words that a tenth-of-a-beat correction exists, with a yes/no."
    },
    {
      "id": "GA-8",
      "target": "TR24 'outside the pin' and TR25 (f) fallback",
      "claim": "TR24 (the nudge, the clip reading the shifted beat, B9) and TR24b (beat stopped) are run 'by whichever lane merges second'. There is no owner, no row count and no date, so B9 and B10 have no live gate at either merge. TR25 (f) quietly degrades to loading a hand-written file when no save route exists. Then 'the file holds paused exactly once' cannot be checked live and MU-102 and MU-110 keep teeth only at unit level.",
      "evidence": "plan-transport-answers.md:648-653 (TR24), :633-636 (f), :463 (S0 'the REST route that saves a show, if any').",
      "severity": "NIT",
      "proposed_change": "Name the owner (Harmony, at the later merge) and add TR24 to that merge's EXPECTED_ROWS. Make S0 decide the save route now. If none exists, state that (f)'s write half is unit-only and drop 'exactly once' from the live text."
    }
  ],
  "strongest_point": "GA-1. The Undo gate for Boris's explicit Cmd+Z-never-touches-pause rule (B6) is ordered so the mutant it is meant to catch passes. A snapshot taken after the pause already contains the pause. Boris's own check 26 repeats the same order, so neither the machine row nor his check can see the bug. SetClipCmd's value snapshots (ClipCommands.h:80-110) show this is the real mechanism, not a theory.",
  "citations_rechecked": true
}
```

## Seat: stage-hands (8 attacks)

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "TA1 the HELD band (lines 125-128), F-A1, K-A1",
      "claim": "The plan rebuilds a speed law after Boris said no slide. Boris sees a synced clip freeze for about 3 frames (factor 0) or run at double speed whenever it sits 0.05-0.10 beat off. That happens on every Tap, nudge and tempo wobble, and more in Auto.",
      "evidence": "BD:902 '71 b'; Q71 B as asked (boris-clarify-71-74.md:12) 'the clip plays on from where it is and cuts once'. Adoption (plan-transport-delta2.md:1075-1077): 'NO SLIDE IS BUILT... S4c builds the one-cut path'. Plan line 125-128: factor 1 - e/frameBeats clamped 0..2. At 120 BPM and 60 fps a frame is about 0.033 beat, so e=0.10 gives factor 0, a stand-still. The plan concedes at K-A1 (lines 749-757) that 'a hold is a slide under another name'. Its own K-A3 calls the per-frame speed change unproven.",
      "severity": "MUST",
      "proposed_change": "Default S4c to the runner-up: exact rate, no held band (kGridDeadbandBeats = kOutOfTimeBeats = 0.10), one cut on the '1'. Build the hold only if SM-a's D1 shows more than 1 cut per 10 minutes, and then ask Boris as a numbered question before it ships. One constant changes, plus TL-U41/U43/U45c/U46, X3 and MU-100."
    },
    {
      "id": "ST-2",
      "target": "TA-2 event table (lines 160-175); checks 4-7, 14",
      "claim": "No row and no Boris check covers pause then play on a BPM-synced clip. A paused clip is OFF (line 148-150), so after any pause it comes back arbitrarily out of time. Boris sees it run off the beat, then jump up to 2 beats on the next '1'. At e exactly 2 beats the direction of the cut is not stated.",
      "evidence": "Table rows: fire, drop, Tap, Resync, tempo, nudge, Auto, BeatLoopr, BPM timer. Nothing for the clip's own play button. Check 4 covers Timeline only. D-6 lists drop, Tap and fire as the only cut causes. The lock period is gcd(B,4)=4 beats (plan-transport-delta2.md:352), so e is wrapped to +-2 beats and +2 and -2 are the same case. A VJ pausing for a breakdown and releasing on the drop sees the picture leap.",
      "severity": "SHOULD",
      "proposed_change": "Add the row 'play after pause: plays on from its frame, ONE cut on the next 1 if more than 0.10 beat off', add it to D-6, and add a TL-U49-style case. Add a Boris check: pause a synced clip 3 beats, press play, watch for the leap. Specify the tie at +-2 beats (cut forward, say) in TL-U49j."
    },
    {
      "id": "ST-3",
      "target": "TA-2 row 173, D-16, check 19 (BeatLoopr Off without Catch Up)",
      "claim": "Arena's Catch Up toggle is a creative choice: 'Otherwise, it will just continue wherever the playhead was'. The plan makes both settings end in time and differ only in when (at once vs the next '1'). The Arena BeatLoopr row on the same desk then looks identical and behaves differently. Boris never agreed to this.",
      "evidence": "facts-resolume-transport.md:66 (documented Catch Up sentence). Plan D-16 (lines 694-695) admits 'In Resolume it stays where it is.' Q71 B as asked (boris-clarify-71-74.md:7-12) scoped the cut to a drop and fires that are not on the '1', not to BeatLoopr. BD:808-810 only says 'build random and beatloopr'.",
      "severity": "SHOULD",
      "proposed_change": "Do not ship D-16 on Harmony's inference. Add a numbered question for Boris with the Arena behaviour as option A: Without Catch Up the clip keeps its offset, no cut. Option B is the plan's cut. Name which line of applyCut's OFF list changes (the BeatLoopr clause in TL-U44)."
    },
    {
      "id": "ST-4",
      "target": "TA3 last bullet (line 388); X6t, TR32",
      "claim": "Master speed still multiplies a Timeline clip and is not clamped, and X6t measures only a clip at 10. A clip at 10 with the master fader up pushes about 40x into the player, which was never measured. Boris sees black frames, holds or a frozen picture at the top of two faders he will naturally use together.",
      "evidence": "Plan line 388 'The master speed still multiplies... is not clamped with it'. Renderer.h:290-293 effectiveClipSpeed = clipSpeed*masterSpeed. CompositionInspector.cpp:148 masterSpeed = val*4 (range 0..4). Renderer.cpp:1670 pushes the product with no clamp (VideoPlayer::setSpeed stores unclamped, plan E6/E7). M7t (line 396-399) sets only 'speed 10', master 1. The plan itself predicts late frames and holds at 10 (K-S1).",
      "severity": "SHOULD",
      "proposed_change": "Clamp the effective Timeline speed pushed to a player to kTimelineSpeedMax (data, per O6t) in effectiveClipSpeed, with a TL case. Or add an M7t arm at 10 x master 4 and name the result. Say which in the S4a/S5a packet."
    },
    {
      "id": "ST-5",
      "target": "TA3 gates TL-U85, TL-U87, TR32, C17/W23 vs TA-10 'the top is data'",
      "claim": "The plan says the top is data (kTimelineSpeedMax, O6t may lower it to 8 or 4), but TL-U85, TL-U87, TR32, MU-104, W23 and C17 hard-code 10. If X6t fails, the ruled fallback makes merge-2 gates fail for a reason already ruled, and a builder would be tempted to loosen them. Boris would get a slider that tops out at N with a gate row that says 10.",
      "evidence": "Lines 402-404: 'It is data (kTimelineSpeedMax)... top is the highest of 10, 8, 4'. Lines 518-520: 'the top is 10'. Lines 643-647: speed 10 stored 10, travel 1.0 at 10. Lines 673-675: 'W23: at the top, text 10, plus disabled'.",
      "severity": "SHOULD",
      "proposed_change": "Write TL-U85, TL-U87, TR32 and C17 against kTimelineSpeedMax. Fix the law so that 'two = half the travel' holds for any top. Add one row to 5.3 for the lowered-top case so it has an expected value before the measurement, not after."
    },
    {
      "id": "ST-6",
      "target": "TA3 fourth strip button (line 378-379), TL-U54",
      "claim": "The fourth button doubles speed and Timeline speed can sit at 0 (slider, minus). From 0, doubling gives 0, so it clears the pause and lights play but nothing moves. Boris sees a clip that will not go from a play-lit button. There is no pause mark, because Speed 0 is not a pause.",
      "evidence": "LayerStrip.cpp:391 clip->speed = std::min(clip->speed*2.0f, 4.0f). Plan line 378-379 keeps 'doubles up to 10 (1, 2, 4, 8, 10)'. Lines 376: minus/plus 'stopping at 0'. TL-U54 (571-573) tests 'doubles up to 10' with no case for 0. BPM Sync step 0 has the same frozen-with-play-lit look (OFF list, line 149).",
      "severity": "SHOULD",
      "proposed_change": "Add a TL-U54 clause: from a speed below 0.1 the forward button goes to 1 (or 0.1). Add a Boris-check line 'Speed 0, press the fourth button: it moves'. Say in the S5b packet that Speed 0 is shown as a state distinct from pause."
    },
    {
      "id": "ST-7",
      "target": "TA1 deadband 0.05 beat (lines 125, 152-156), TR24, X2",
      "claim": "The nudge exists so Boris can match the image to the sound 'exactly', in whole ms. A synced clip ignores any beat shift below 0.05 beat, which is 25 ms at 120 BPM and 50 ms at 60. Small nudges move the circle and every other beat-driven thing but not the picture. Past that the clip is pulled all at once (hold). Boris sees a 20 ms nudge do nothing and a 40 ms nudge jump in one go.",
      "evidence": "BD:799 'move the beat forward or back to get it to match the image exactly'; BD:836 and BD:911 (nudge X ms); BD:859-861 'everything that is connected to BPM shifts... I mean everything'. Plan: in time = speed exactly the rate for |e| <= 0.05; TR24 (a) accepts abs(e) <= 0.05. The deadband is justified only by arithmetic on the 10.7 ms hop at 200 BPM (INFERRED, line 152-156). A hop is 0.011 beat at 60 BPM.",
      "severity": "SHOULD",
      "proposed_change": "Define the deadband in ms, not beats: for example the larger of 1.5 hops and a few ms, capped near 15 ms. Add a TR24 arm: a nudge of 20 ms at 120 BPM moves the clip's lines by 20 ms within 1 s. If this fails, say so in the Boris check instead of claiming 'everything shifts'."
    },
    {
      "id": "ST-8",
      "target": "TA1 THE BPM TIMER (lines 177-181), TL-U88, check 27",
      "claim": "Boris's stop/pause rule ('the BPM goes to zero nothing moves', BD:918-920) is built as constant true. Merge 2 ships a BPM-synced clip that ignores the BPM timer, and the only live row is TR24b, outside the pin. The plan's fallback assumes the tempo row may publish 0, but the tracker's tempo range is 60..200.",
      "evidence": "Plan lines 177-181: 'S4c builds it as constant true'; line 179-180: 'If that lane publishes a tempo of 0... the ruling's own OFF row already stands the clip still'. ruling-transport-delta2.md:180 V10 (kMinBPM 60, kMaxBPM 200), so 'tempo 0' may be unreachable. TL-U88 drives the input in a unit only. Check 27 is '(when the tempo row is built)'. Frozen with play lit and no mark is what Boris asked for, but nothing in it tells him why.",
      "severity": "SHOULD",
      "proposed_change": "Make the S4c packet carry a gate row in the pin: the probe sets beatRunning false through a test route, and the clip must stand while a Timeline clip runs. Drop the 'tempo 0 already covers it' sentence unless a case shows tempo 0 is reachable. State in the Boris-check that the beat circle is what shows the BPM timer is stopped."
    }
  ],
  "strongest_point": "ST-1 is the strongest. Boris answered '71 b' (no slide), and Harmony's adoption voided the slide. Yet the plan's default behaviour makes a synced clip freeze for about 3 frames or run at double speed whenever it is 0.05-0.10 beat off. That is a speed law again, much larger than the 6 percent he turned down, and it fires on every Tap, nudge and tempo wobble. The plan's own K-A1 calls it 'a slide under another name' and offers the runner-up one constant away, so the default should be the runner-up.",
  "citations_rechecked": true
}
```

