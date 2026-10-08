# Answer to Boris's L32 (his reply to question 189): can a hand correction hold while the app keeps listening?

His question, verbatim (L32): "189 is it possible to correct the app listening to the music if it is off by two BPM's and keep it on automatic mode, or will that correction necessitate going back to manual mode? This is a question for you.  Same question if I correct, where the one is."

Author: Architect (tempo row). Read-only paper: nothing built, run, probed or launched; no lane worktree read. Code pin: main HEAD a86cf0a. Labels: VERIFIED = I opened the line; INFERRED = reasoned from lines I opened, not run; UNKNOWN = not established.

## ANSWER FOR BORIS

Yes to both. Your correction can hold while the app stays on automatic; it does not have to go to manual. It needs a change to the listening, and I can only promise it after it is measured on your three tracks.

Tempo: your number becomes the centre. The app keeps listening, but only close around your number, and uses what it hears to keep the beat on the kick. "/2" and "x2" work the same way: the listening carries on at half or double.

The "1": your Resync says which beat is the 1. The app counts on from your 1, and the listening only keeps the edge of each beat on the kick; it stops guessing the 1 itself. The nudge moves it a hair.

I recommend exactly this. Manual stays as your own switch for when the listening is no use.

## QUESTION FOR HIM

Your corrected tempo is holding in automatic. The music then moves to a clearly different tempo (a new track that was not beat-matched). What should the app do?
- A (DEFAULT): it lets go of your number by itself and follows the music again; your "1" keeps counting until you press Resync.
- B: it keeps your number until you change it yourself.

## FACTS

How the listening takes a hand correction (kept here as my notes, not for his page):
- F1 VERIFIED src/analysis/BPMTracker.cpp:99-104 -- with Manual on, the whole listening stage is skipped: the beat free-runs from the hand tempo and no heard beat moves it.
- F2 VERIFIED BPMTracker.cpp:233-236, BPMTracker.h:57 (`kBeatResetConfidence = 0.5f`) -- with Manual off, every heard beat the listener is sure of puts the beat edge back to 0, whatever the tempo number is.
- F3 VERIFIED BPMTracker.cpp:170-204, BPMTracker.h:48,54 (`kHysteresisHops = 200`, `kBPMChangeThreshold = 2.0f`) -- with Manual off, a heard tempo replaces the held one only when it is MORE than 2.0 BPM away for 200 analysis steps in a row (about 2.1 s).
- F4 INFERRED from F3 -- a hand correction of exactly "two BPM" sits on the edge of that band: 126 -> 128 may hold or may be taken back depending on decimals of what is heard. Not run.
- F5 VERIFIED BPMTracker.cpp:607-622 -- a hand tempo (tap, typed, step) writes the held tempo and marks it "locked"; it does not clear the 48-value history of heard tempos (`medianBuffer_`, BPMTracker.cpp:311-326), so the next heard value is judged against the old history.
- F6 VERIFIED src/MainComponent.cpp:5824-5827 -- Tap does not switch Manual on (it only posts the tempo and a realign). MainComponent.cpp:5840-5845 -- Resync does not switch Manual on.
- F7 VERIFIED BPMTracker.cpp:566-578 -- Resync sets beat edge 0, beat-in-bar 0, bar 0, phrase 0. It does NOT write `lockedDownbeatPos_` (the listener's own idea of where the 1 is).
- F8 VERIFIED BPMTracker.cpp:343-346, 377-389 -- with Manual off, the beat-in-bar count advances once per HEARD beat (`scoreBeat`), not once per beat of the clock. INFERRED from that: a missed or an extra heard beat moves the "1" to another quarter of the circle; a passage with no clear beat that is not silence stalls the count.
- F9 VERIFIED BPMTracker.cpp:384-388, 438-453, BPMTracker.h:62 -- every 16 heard beats the listener re-checks its own guess of the 1; after 8 disagreeing checks in a row (128 beats, about one minute at 128 BPM) it recomputes the beat-in-bar count from ITS guess. INFERRED: that is the moment a hand "1" is thrown away in automatic.
- F10 VERIFIED BPMTracker.cpp:390-401 -- before the listener has locked its own "1", every heard beat sets beat-in-bar to (heard beats mod 4): a Resync is overwritten at the next heard beat. INFERRED consequence.
- F11 VERIFIED BPMTracker.cpp:293-309 -- a heard tempo near double or half of the held one is folded back onto the held one (ratio 1.8..2.2 and 0.45..0.55). INFERRED: the tempo NUMBER of a "/2" or "x2" would survive the listening; the beat EDGE would not (F2: the heard beats still arrive at the heard rate and each one resets the edge).
- F12 VERIFIED BPMTracker.h:41-42, BPMTracker.cpp:281-291, 614 -- every tempo, heard or by hand, is folded into 60..200. His L14 asks 22..480 by hand.
- F13 VERIFIED /opt/homebrew/include/aubio/tempo/tempo.h:63-249 (aubio 0.4.9, the listening library; CMakeLists.txt:16 `find_package(Aubio REQUIRED)`) -- its public functions set only silence level, threshold, delay and tatum signature. There is NO call that tells it "the tempo is near N" or "search only between X and Y". So "listen only close around his number" cannot be asked of the library; it has to be done in the app's own stage after it (BPMTracker.cpp stages 1-5).
- F14 VERIFIED tempo.h:70-84 -- the library reports the position of each heard beat (`aubio_tempo_get_last`, `_s`, `_ms`). The app does not read it (BPMTracker.cpp:52-59 reads only the beat flag, the BPM and the confidence).
- F15 VERIFIED BPMTracker.cpp:125-130 -- in silence the app coasts on the held tempo. INFERRED: a held tempo that is 2 BPM wrong coasts 2 beats per minute off: a quarter of a beat after about 7.5 s of breakdown. This is where "off by two" is seen on stage.
- F16 VERIFIED BPMTracker.h:61 -- 4 beats per bar is fixed.
- F17 VERIFIED .harmony/.reports/s-rta-1004b/plan-nudge-row2.md:814-824 (adoption block) -- the adopted row keeps "nudge X ms" between the two nudge buttons; the nudge survives stop and play; a hand Resync starts a held beat. The block says nothing on whether a hand tempo or a hand "1" holds in automatic.
- F18 VERIFIED .harmony/.reports/s-rta-1005/area-tempo.md U-HIS-4, O-17 -- the earlier reading "a tempo step switches Manual on" (NA-17) was never confirmed by a line of his. His L32 does not pick A, B or C of question 189; it asks whether automatic can be kept.
- F19 VERIFIED his L14: "The /2 and x2 manual work to both limit as well as the listening clock." and "We only need to display the nudge XMS in automatic mode." -- he already expects "/2", "x2" and the nudge to act on the listening clock.
- F20 VERIFIED his L13: "/2 and x2, tempo change do not move the 1." -- a tempo correction must never move the "1" (it does not: F5 with `realign == false`; Tap is the exception, F6, question 190).

UNKNOWN:
- U1 Why the listener reads 2 BPM off on his music, and whether its heard BEATS still land on the kick when its NUMBER is 2 off. Cheapest: feed his three tracks to the tracker offline and log number, beat times and confidence against a hand-counted grid (a probe; not run here by rule).
- U2 How often the listener misses or adds a beat on his tracks (F8), and how often its own guess of the "1" disagrees with his (F9). Same log as U1.
- U3 How the nudge is applied in automatic on the unmerged nudge lane (lane/nudge a8afcfb; I did not read the lane, by rule). The area sheet reports STOP-N2 "in Auto a small later nudge holds one hop on 199 of 200 beats": the lane's offset arithmetic and the soft pull of Option T1/O1 below must be one mechanism, not two. Cheapest: the architect line on STOP-N1 / STOP-N2 that is already owed.
- U4 Whether Resolume itself has any listening mode to compare with. Not checked (no web tools; never operate his Arena). Cheapest: Researcher, Resolume manual "Tempo".

## OPTIONS

### (a) The tempo is about two BPM off

T1 HOLD-NEAR (RECOMMENDED). His number (tap, typed, "+" / "-") becomes the centre. Automatic stays on. The app accepts a heard tempo only inside a narrow band around the centre (starting value to be measured, about plus or minus 3 percent), lets the centre follow a slow drift inside the band (a DJ riding the pitch), and uses the heard beats only to pull the beat edge gently onto the kick, never to jump it. It lets go when the music stays clearly outside the band for a set number of bars (his question above).
 - On stage: the number stays where he put it (it may creep by tenths with the pitch fader); the circle stays on the kick; in a breakdown the app coasts at HIS tempo, so it comes back on the beat.
 - Size: M. All inside the app's own stage after the library (F13): a "held by hand" centre and band in the lock logic (F3), the heard-tempo history cleared on a hand value (F5), the hard reset of F2 replaced by a bounded pull.
 - What can go wrong: (i) he sets a wrong number: the heard beats keep pulling against a clock that runs at the wrong rate, seen as a circle that wobbles each beat; the band must be wide enough that a 1 BPM hand error is absorbed by the drift-follow. (ii) it lets go too early in a busy passage and the number jumps back; (iii) it lets go too late on a real change and runs a few bars at the old tempo. (iv) if U1 shows the library's beats do not land on the kick when its number is off, the pull has nothing true to pull to and T1 falls back to T2 in effect.
 - Works when: on each of his three tracks, after one correction, the number stays within the band and the beat edge within about 30 ms of the kick for the whole track including the breakdown, with no jump.

T2 HIS HAND SWITCHES MANUAL ON (the old default A of question 189). The app stops listening at the first correction.
 - On stage: number and beat are exactly his; nothing fights him. A number that is 0.1 BPM off slides a quarter of a beat in about two and a half minutes; he corrects with the nudge or Resync through the night.
 - Size: S. What can go wrong: only the slide, and that he forgets to switch back for the next track. Works when: nothing moves that he did not move.

T3 AS IT IS (option B of 189). Not offered: F3 / F4 show his number may or may not survive, by decimals. Unpredictable on stage.

T4 "/2" and "x2" IN AUTOMATIC (his L14). The app keeps a factor (.. 1/4, 1/2, 1, 2, 4 ..) between what it hears and what it runs: heard 128 with "/2" runs 64, the listening carries on. At "/2" only every second heard beat is his beat (which one is fixed by his "1"); at "x2" a beat is placed between two heard beats. This factor is also what lets a hand tempo outside 60..200 (L14: 22 to 480) live with a listener that hears 60..200 (F12).
 - Size: S on top of T1 (it needs T1's pull instead of the hard reset: F11), M alone.
 - What can go wrong: at "/2" a missed heard beat flips onto the wrong half (the circle is then two beats out); O1 below is the guard. Works when: "/2" and "x2" pressed in automatic change the speed of everything by exactly 2, the circle's "1" does not move (L13), and the number shows no return within a full track.

### (b) Where the "1" is

Two different corrections hide in "where the one is": WHICH of the four beats is the 1 (a whole-beat correction: Resync), and a HAIR early or late (the nudge, already ruled; L13 "it moves the one a hair forward or back").

O1 HIS "1" IS CARRIED BY THE CLOCK (RECOMMENDED). After a hand Resync in automatic, that beat is the 1. The count of beats and bars is driven by the app's clock (one count per beat the clock passes), not by heard beats (F8). The listening only pulls the edge (as T1). The listener's own guess of the 1 (F9, F10) is muted until he switches it back on or a new "1" is given by him.
 - On stage: he presses Resync roughly on the 1; the lit quarter is his at once; the edge settles onto the kick within a beat or two; the "1" stays his through breakdowns and through a beat-matched mix into the next track.
 - Size: M (the count moves from "heard beats" to "clock beats" in automatic, and every reader of bar and phrase follows; one writer for the count, as the code already insists at BPMTracker.cpp:260-277).
 - What can go wrong: (i) a whole-beat slip: the listening loses the beat, catches it again more than half a beat away, and the pull goes the short way round: his 1 is then on the 2. The bounded pull (never more than a small part of a beat per beat) makes this rare; U2 says how rare. (ii) his press lands more than half a beat late: the edge settles on the NEXT kick. Same rule as any Resync.
 - Works when: on his three tracks one Resync at the first drop keeps the lit quarter on the true 1 to the end of the track, breakdown included, and through one beat-matched transition.

O2 THE HAIR IS AN OFFSET THE LISTENING CARRIES (the nudge; already ruled, F17, F19). "nudge X ms" is added on top of wherever the listening puts the beat, and stays until Resync zeroes it. Size: in the nudge lane already (U3). What can go wrong: the offset and the pull of T1 / O1 fight if they are two mechanisms; they must be one sum (heard position + his offset). Works when: nudge +20 ms in automatic reads +20 ms against the kick one minute later.

O3 RESYNC SWITCHES MANUAL ON (old default A). Size S. His 1 and his edge are exact and nothing moves them; the listening is off, so the slide of T2 applies.

O4 AS IT IS. Not offered: F9 / F10 take his 1 back at a moment he cannot predict.

### Recommendation

T1 + T4 + O1 + O2 as ONE piece of work on the listening stage ("automatic with a hand hold"), size L together because every beat reader's test set moves with it. Manual stays as his own switch and behaves as T2 / O3. This replaces default A of question 189 and the unconfirmed reading NA-17 ("a tempo step switches Manual on", F18): in automatic no press of his switches Manual on.

Strongest argument against it: T2 / O3 is small, cannot fight him, and is what a performer who trusts his own ear needs; a listening that is wrong by 2 BPM may also be wrong about where the beats are (U1), and then the "hold" is a Manual with extra wobble. Why it loses for now: his question asks for automatic to be kept, the coast-through-breakdown gain (F15) is real, and T2 / O3 remain as the fallback the same work leaves intact. If the measurement of U1 fails, the answer to him changes to "no, a correction needs Manual", and that must be said before any build.

Order of work (for Harmony): BPMTracker.cpp already has a queue (nudge S1, S1r, transport S4t; adoption block). This work touches the same lines as the nudge's offset (U3), so it is planned WITH the owed STOP-N1 / STOP-N2 line, not after it.

## WHAT HAS TO BE MEASURED WHEN IT IS BUILT

On his three DJ tracks, each with a hand-counted beat grid (tempo to 0.1 BPM, the true "1" marked):
1. BEFORE any build (U1, U2): the listener's number, its beat times (F14) and its confidence across each whole track. Three numbers decide the design: how far the number sits from the truth, how far the heard beats sit from the kick (ms, median and worst), how many beats per track are missed or added.
2. Band width for T1: the smallest band that keeps the true tempo inside it on all three tracks and still rejects the wrong number the listener offered.
3. Hold test (T1): one correction at the start; the number and the edge-to-kick distance every bar to the end; count of jumps (target 0).
4. Coast test (F15): the edge-to-kick distance at the first kick after the longest breakdown of each track.
5. Let-go test (his question): two tracks played one after the other, not beat-matched, 4 or more BPM apart; bars until the app follows the second; and a busy passage of one track to show it does NOT let go.
6. "1" test (O1): one Resync at the first drop; the lit quarter against the true 1 every 16 bars to the end; one beat-matched transition.
7. "/2" and "x2" (T4): pressed in automatic mid-track; the "1" before and after (L13); the number one minute later.
8. Nudge (O2): +20 ms and -20 ms in automatic; the measured offset one minute later.
9. A wrong hand number on purpose (1 BPM and 3 BPM off): what he sees, so the page can tell him.

## NOT DONE / UNSURE

- Nothing was run. Every "on stage" line above is INFERRED from the code lines named; the measurements of the section above turn them into facts.
- I did not read the nudge lane (rule), so how T1 / O1 share one mechanism with the nudge offset is a design debt (U3), not a spec.
- The band (about plus or minus 3 percent), the pull size and the let-go time are starting guesses, not values.
- I did not check what Resolume does (U4); nothing here claims "as Resolume".
- Tap in automatic (L18: "the beat reacts instantly to the tapping on the second tap") is treated as a hand tempo for T1; whether a tap also moves the beat edge is question 190, not answered here.
- Time signatures other than 4/4 are out of scope (F16).
- No spec for builders is in this paper: it answers his question and sizes the work. The stage plan follows his answer to the one question.

Written (system clock): Wed Oct  7 22:51:27 EDT 2026
STATUS: DONE
