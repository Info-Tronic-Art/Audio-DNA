# Clarifying questions 26-37 to Boris (asked in chat 2026-10-04, session s-rta-1004)
Why: his message of 2026-10-04 11:59:06 ("Ask questions to clarify till you are 100% confident you understand what needs
to be done"; verbatim in .harmony/boris-feedback-backlog.md, same stamp). New numbers; nothing shown to him earlier is
re-worded. Each has a default A. His answers are filed verbatim under this file's "ANSWERS" heading and in the backlog.
The text below is the text AS ASKED. Once asked, a question is never edited in place.

## QUESTIONS AS ASKED

26. (your answer 1) A paused clip you fire "stays paused". Where?
    A (default) It jumps to its beginning and waits there, paused.
    B It stays on the frame where you paused it.

27. (your answer 4) Your screenshots show six modes. Which do we build?
    A (default) Timeline and BPM Sync. The mode menu lists only those two.
    B Also SMPTE and / or Denon DJ / Pioneer DJ. Say which. Each is a large build of its own and would come later as its own lane.

28. (your answer 4) The manual text names two things your screenshots do not show: Random (R) and BeatLoopr.
    A (default) Leave both out for now.
    B Build Random.   C Build BeatLoopr.   (B, C or both)

29. (your answer 4) Resolume's second small menu lets each clip choose: fire "from the start" or "carry on from where it was".
    You ruled earlier: every fire starts a video from its beginning.
    A (default) Build the menu as Resolume has it. Every clip starts set to "from the start".
    B No menu. Every fire starts from the beginning, always.

30. (your answer 4) Resolume's BPM Sync has no BPM box on the clip, only Speed and Beats. Our earlier plan had a BPM box (starts at 120).
    A (default) As Resolume: no BPM box. The rows are Speed and Bars. The clip follows the app's tempo. 120 is used only to work out the first Bars number.
    B Keep a BPM box on the clip as well.

31. (your answer 20) You switch a clip to BPM Sync. Its length is not a whole number of bars at 120. Example: 7.3 seconds = 3.65 bars.
    A (default, my reading of your words) The app puts the out point on the last whole bar: 3 bars = 6.0 s. It plays at its
      natural speed. The last 1.3 s sits outside the out point until you drag it.
    B The app keeps the whole clip and rounds to the nearest whole bars: 4 bars. It plays about 9 % slow to fit.

32. (your answers 7 and 20) Which Bars numbers can a clip have?
    A (default, your words "whole bars") 1, 2, 3, 4, 5 ... /2 stops at 1.
    B Also 1/2 and 1/4 of a bar (2 beats, 1 beat) for very short loops.
    C Only 1, 2, 4, 8, 16, 32.

33. (your answer 24) You drop the playhead in the middle of a bar.
    A (default) It plays on from there and slides onto the nearest BEAT within a few seconds. The clip's bar start may then sit
      1 to 3 beats off the music's bar start until you fire it.
    B It slides until the clip's bars sit on the music's bars. That can take up to about 15 seconds of running slightly fast or slow.

34. (your answer 12) You press Save (show or deck) and the file could not be written.
    A (default) The "Save failed" box still shows (your ruling of yesterday).
    B Nothing shows.

35. (your answer 12) "Recording the show" means:
    A (default) Both: the performance take (Record tab, Record Take) and the video recording of the output.
    B Only one of them. Say which.

36. (your answer 12) "if the user is changing things around settings ... and they do not save the composition, nothing is saved".
    A (default, my reading) No reminder and no auto-save for the show: not saved = gone, silently. The few things the app keeps
      by itself today (I will list them, not guess them) stay kept as today.
    B Those are also kept only when you save the show.

37. (your answer 19) The "not enough disk space to record" sign:
    A (default) A line in the Record tab, where "Ready. Last take: ..." sits, shown while it is true. No pop-up. The same
      place says so when the recordings folder cannot be written.
    B A pop-up box.

## MY READINGS (told to him; he corrects only what is wrong)
- R1 The panel copies Resolume's layout and controls, drawn in Audio-DNA's own colours and fonts.
- R2 In BPM Sync the second row is labelled "Bars" (Resolume says "Beats"). One bar = 4 beats. Minus and plus step by one bar; /2 and x2 halve and double.
- R3 The timeline shows one line per beat, the bar lines drawn stronger.
- R4 The S fader in BPM Sync steps through halves and doubles; the exact list is read from Resolume, not guessed.
- R5 A click inside the in-to-out range moves the playhead there (as Resolume); a click outside does nothing (his rule).
- R6 "Recording a clip live" = the record-to-clip feature (not built yet).
- R7 Everything else that fails to write shows nothing: Save preset, FX Save, Save Routine, a snapshot.
- R8 He asked for a list of everything that can be saved: owed to him as a page (what saves it, where it goes, what shows if it fails), built from the code.
- R9 Timeline mode's second row is "Duration" in seconds with minus, plus, /2, x2, as Resolume.

## NOT ASKED (Harmony's to settle, with evidence)
- What Resolume does at each point it is cited for (first Beats number on switching to BPM Sync, the Speed steps in BPM Sync,
  a paused clip that is triggered, a scrubbed BPM-synced clip): a research fact sheet, sources cited; any difference from his
  answers above is shown to him before it is built.
- Consequence for the plans: plan-transport.md + ruling-transport.md + ruling-transport-delta1.md are re-opened by BF38-BF40
  (a second delta ruling); ruling-notices.md by BF41-BF42. Neither lane builds before the sync dial merges, as before.

## ANSWERS
Answered 2026-10-04 12:21:01 (26, 27, 28, 29 = "can you clarify?", 31, 32, 33, 35, 36 + the bars-only line); verbatim and readings: .harmony/boris-feedback-backlog.md "Boris's answers to clarifying questions 26-36". Not named, defaults stand: 30, 34, 37. Re-asked under new numbers: 29 -> 42, 32 -> 43 (boris-clarify-42-44.md).
