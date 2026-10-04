# Questions 121-124 to Boris (asked in chat 2026-10-04 15:36:01 at the close of session s-rta-1004) — from the transport answers ruling (ruling-transport-answers.md section 7)
New numbers. The text below is the ruling's section 7 VERBATIM.


121. You press Resync while BPM-synced clips are out of time.
     A (default) They cut into time at once: your press is the "1".
     B They play on and cut on the next "1", one bar after your press.
122. You save a show with a paused clip and open it another day.
     A (default) The clip is paused on the same frame as when you saved.
     B The clip is paused on its first frame.
123. You fire a BPM-synced clip between two "1"s. On the next "1" it cuts into time. Where to?
     A (default) To its nearest bar line: back to its beginning if you fired late in the bar, forward to the start of
       its second bar if you fired early.
     B Always back to its beginning, so it starts over on the "1".
124. BeatLoopr: you switch the loop off and Catch Up is not lit.
     A (default) The clip carries on from where it is and cuts into time on the next "1", like any clip that is out
       of time.
     B It stays where it is, off the bar, until you fire it again or press Resync. (This is what Resolume does.)
What each B changes: 121 -- one argument of `barOneCrossed` (a Resync is not a "1"), TL-U83's and TL-U89's last
clauses, X5's and TR21 (d)'s Resync sentence, D-14. 122 -- one line of Clip.cpp (the key `pausedAt` is not written),
TL-U80, TR25 (f)'s frame clause. 123 -- `applyCut`'s target for a clip that has not been cut since its stamp was
minted, one clause in TL-U49j, a frame-code clause in TR20. 124 -- TB-14.
Still held back, unchanged: 73 (FM-8). Requests RQ-0 .. RQ-3 stand; RQ-0 (three tracks) is the one the first run needs.
NOT a numbered question, and why: what is built if a clip would cut too often (rows O3 and O3d) is asked only if the
measurement says so, with the measured numbers in the sentence; whether the picture cuts on small nudges, what a replay
does to a pause, the fourth button at 0 and a tempo of 0 are ruled here and shown to him (checks 30, 29, 12, 31).


## ANSWERS
Recorded 2026-10-04 15:59:53: 121 = A; 122 = A; 123 = B; 124 = A; and "BeatLoopr" is renamed "Beat Repeat". Verbatim: backlog, same stamp.
