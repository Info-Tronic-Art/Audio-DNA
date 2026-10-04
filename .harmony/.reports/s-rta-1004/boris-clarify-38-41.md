# Clarifying questions 38-41 to Boris (asked in chat 2026-10-04, session s-rta-1004) — the per-screen Delay that replaces the sync dial
Why: his four messages of 2026-10-04 12:08:21 / 12:09:11 (verbatim in .harmony/boris-feedback-backlog.md, "Boris REPLACES the
sync dial with a per-output Delay"). New numbers; nothing shown earlier is re-worded. Each has a default A.
The text below is the text AS ASKED. Once asked, a question is never edited in place. Questions 26-37 (boris-clarify-26-37.md) stay open.

## QUESTIONS AS ASKED

38. Where is each screen's Delay remembered?
    A (default) With the screen, as Resolume does: the app remembers it between launches, whatever show is open.
    B With the show: opening a show sets each screen's delay. (Yesterday: "every show remembers it's sync".)

39. Resolume's Screen window also has Opacity, Brightness, Contrast, Red, Green, Blue for each screen.
    A (default) Only Delay now.
    B The whole window.

40. What is delayed?
    A (default) Only the output windows on the displays. The preview inside the app, video recordings and Syphon are not delayed.
    B Something else. Say what.

41. "Move the beat forward or back to match the image" — with what?
    A (default) With what the app has today: tap the tempo, press Resync on the first beat of a bar. I write that up in the manual. Nothing new is built.
    B Two new controls that slide the beat a little earlier or later, plus the manual.

## MY READINGS (told to him; he corrects only what is wrong)
- R10 What goes away with the dial: the SYNC number and button in the top bar, the room list, "Sync earlier / later" on a key or a pad, the "earlier" half, "every show remembers its sync" (unless 38 B).
- R11 The Delay runs from 0 to 500 ms in 1 ms steps; Resolume's own maximum is being looked up.
- R12 Audio-DNA has no output properties window today (INFERRED from the docs: outputs are a list of displays that are on or off). One is added: Device and Delay per output, opened from the Outputs list.
- R13 There is no user manual yet (INFERRED: none found in the repo). One is started; its first entry is how to line up beat and picture from where the VJ stands.
- R14 The two things that rode in the sync branch and are not the dial (the music-beat wheel, the longer Gain) stay on the list.
- R15 The dial as built on the branch was never in his app; the branch is kept only as a source of parts until the new plan is ruled.

## NOT ASKED (Harmony's to settle, with evidence)
- How a per-screen delay is built (holding each screen's last N pictures; its cost in video memory at 500 ms; where it sits
  between the composition canvas and each output window): recon fact sheet, then an architect plan, a blind council, a ruling.
- What of lane/bf2 / lane/bf2-keys is carried over (tests, probe rig, docs fixes) and what is dropped: the same recon.
- What Resolume's Delay does exactly (range, what it is saved with, what it affects): research fact sheet with sources.

## ANSWERS
39, 40, 41 answered (stamp in .harmony/boris-feedback-backlog.md "Boris's answers to questions 39-41"; verbatim and readings there). 38 not named: default stands. Follow-ups 45, 46: boris-clarify-45-46.md.
