# Follow-up questions 42-44 to Boris (asked in chat 2026-10-04, session s-rta-1004)
Why: his answers of 2026-10-04 12:21:01 to questions 26-36 (verbatim in .harmony/boris-feedback-backlog.md, same stamp):
"29 can you clarify?", "32 c only multiples of 4", and answer 35's two kinds of recording. New numbers; nothing shown
earlier is re-worded. The text below is the text AS ASKED. Questions 38-41 (boris-clarify-38-41.md) stay open.

## QUESTIONS AS ASKED

42. (question 29, said plainly) A clip is 60 seconds long. You fire it, and at second 20 you fire a different clip on that
    layer. Later you fire the first clip again. Where does it start?
    A (default) At second 0. Every time. No menu. (Your rule of yesterday: every fire starts a video from its beginning.)
    B You choose per clip, with a small menu as Resolume has: "from the start" (second 0) or "carry on" (second 20, where it
      was). Every clip starts set to "from the start".

43. "Multiples of 4" — which set? Two examples at 120 BPM: a 7.3-second clip and a 45-second clip.
    A (default: the C you picked, 4 / 8 / 16 / 32 beats) 1, 2, 4, 8, 16, 32 bars. Doubling only.
      7.3 s -> 2 bars, out point at 4.0 s.   45 s -> 16 bars, out point at 32 s.
    B Any whole bar (every bar is 4 beats): 1, 2, 3, 4, 5 ...
      7.3 s -> 3 bars, out point at 6.0 s.   45 s -> 22 bars, out point at 44 s.
    C Groups of 4 bars: 4, 8, 12, 16, 20 ...
      45 s -> 20 bars, out point at 40 s.   A clip shorter than 8 s cannot reach 4 bars.

44. Today the app can also record the output straight to a video file in a folder (not into a cell).
    A (default) "Record a clip" replaces it: one way to record video.
    B Keep the plain recording as well.

## MY READINGS (told to him; he corrects only what is wrong)
- R16 26: the fired clip stays on the frame where you paused it, still paused, shown in the layer strip. A different clip fired
  on that paused layer shows its first frame and waits, paused.
- R17 27: the mode menu lists SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ greyed out; they are built later.
- R18 36: the quit window appears every time you quit, with Resolume's three buttons; "Save & Quit" on a show that has no
  file yet asks where to save it.
- R19 The timeline shows a line per bar, none per beat.
- R20 Defaults taken, since you did not name them: 30 (no BPM box on the clip), 34 (the "Save failed" box of Save show / Save
  Deck still shows), 37 (the disk-space sign is a line in the Record tab).

## HARMONY'S ANSWER to his question in 33 ("b can you program this reliably or should we change the plan?")
Told to him: B is taken. Whether it is reliable is not known yet: none of it is built or measured. It is measured on a test
build before it is promised (a clip's position worked out from the beat clock, checked against the beat over minutes and
across a playhead drop). The plan does not change on a guess; if the measurement is bad he gets the fallback with its cost.

## ANSWERS
42 = default (A); 43 and 44 answered in his own words (verbatim and readings: .harmony/boris-feedback-backlog.md "Boris's answers to questions 42-46"). Re-asked under new numbers: 43 -> 47, 44 -> 48 (boris-clarify-47-49.md).
