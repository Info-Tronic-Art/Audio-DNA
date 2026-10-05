# Questions 125-129 to Boris (asked in chat 2026-10-04 15:35:36 at the close of session s-rta-1004) — from the tempo-row ruling (ruling-nudge-row.md section 7)
New numbers. The text below is the ruling's section 7 VERBATIM (questions as asked, readings told with them; its reading
numbers R74..R84 are the ruling's own and overlap Harmony's R74..R77 of boris-clarify-101-106.md: cite them as 'tempo-row R74' etc.).

His words this delta rests on, verbatim: "61 b but lets call it "nudge X ms""; "do a similar to resolume: beatWheel play
pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2 tap resync"; "good (this is just nudge amount)"; "just the bpm
timer. If most of the show is set up to BPM, and the BPM goes stop, the BPM goes to zero nothing moves. If there are
clips that are not BPM based, then they play just as they were and are unaffected"; "If I tapped the tempo again, to set
the tempo, the time does not change. If I press re-sync, then it does re-sync and that changes by how far off the beat
we are."; "49 default".
125. The tempo "-" and "+" buttons -- how big is one step?
     A (default) Whole numbers: from 127.6, "+" goes to 128, then 129; "-" goes to 127. For anything finer you type it.
     B Small steps of 0.1: 127.6, 127.7, 127.8 ...
126. The BPM timer is stopped. What should the tempo number show?
     A (default) The tempo it will run at when you press play (so you can still tap, type, halve and double while it
       is stopped). The lit stop button and the still circle show that it is stopped.
     B 0, until you press play.
127. Today there are three small buttons left of the beat circle: play and pause run every clip on every layer, and
     the square stops all routines. Your row now has its own play, pause and stop for the BPM timer.
     A (default) The old play and pause for all clips leave the top bar (each layer keeps its own). The "stop all
       routines" button stays, to the left of the row, labelled "R[]" in the routines' green so it cannot be taken
       for the row's stop.
     B All three old buttons stay, to the left of the row.
     C All three go; routines are stopped from their own bands and pads only.
128. The BPM timer is stopped or paused, and you tap a tempo or press Resync.
     A (default) The tempo you tapped is taken, Resync puts it on the "1" -- but it stays stopped until you press play.
     B Tapping or Resync starts it running again.
129. You have set a nudge, say "nudge +12 ms". You press stop, and later play. What does the nudge read?
     A (default) 0. Stop then play is a fresh start by hand, like Resync: the "1" is where you press play, nothing on top.
     B Still +12. Stop and play never touch the nudge; only Resync puts it back to 0.
Readings to tell him with the questions (he corrects only what is wrong):
 R74 Pause never touches the nudge number.
 R75 A take does not record the timer's play / pause / stop, and replaying a take never stops your beat. While the
     timer is held, a routine that is playing waits with it; a take that is replaying follows its own audio and plays on.
 R76 With the app listening, "-", "+", "/2", "x2" or a typed tempo switch Manual on.
 R77 The tempo you set by hand can go from 30 to 400; what the app hears by itself stays between 60 and 200. Above
     about 200, beat-driven looks flash fast (400 is almost 7 times a second).
 R78 "/2" and "x2" never move the "1": the beat just runs half or twice as fast.
 R79 The "Bar 1..4" text sits just left of the circle; the LOCKED word sits beside Manual.
 R80 The nudge buttons read "<<" (back = later = minus) and ">>" (forward = earlier = plus), because "-" and "+" are
     now the tempo's.
 R81 Stopped or paused with Quantize on: a clip you fire waits and starts when you press play -- from stop on your
     press, from pause on the next beat or bar line. With Quantize off it fires at once.
 R82 Opening a show, or New, never starts or stops the BPM timer. The app always launches with it running.
 R83 A routine pad pressed while the timer is stopped starts on the first bar line after play: one bar after your
     press.
 R84 Resync while stopped or paused: the beat goes to the "1" and the nudge reads 0, as after any Resync.
Not asked, because his words or a reading settle them: the plan's draft 129 (a quantised fire while stopped: R81,
NA-10); 49 ("49 default"); whether Open restarts the timer (R82, NA-8).
The line that changes for each B: 125 -> `bpmStepUp` / `bpmStepDown` (and T-G11's, T-G13's and LR4a group C's numbers).
126 -> one branch in the label's text source (the five tempo controls still act on the real tempo). 127 -> B: two
buttons and their two callbacks stay left of the row (+ 50 px: DSP is shed at 1728); C: the routines stop and its
tooltip case go (- 36 px). 128 -> `kGesturesStartTimer`. 129 -> the two statements in `setBeatTimer`'s Stop path are
deleted; T-N19 is then the live path and LR2 (a) reads 40 in rounds 4-5.


## ANSWERS
(none yet)

RECORDED 2026-10-04 21:03:09 (session s-rta-1004b). Boris's whole message verbatim: boris-feedback-backlog.md, same stamp. His lines for this file:
"All defaults good except for these:"
"125 whole numbers"
"126 and 127 stop clears all clips from layer strips, pause stops them, tempo setting stays the same. Make thee new buttons as I asked"
"128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button push"
"129 b"
and his words on readings R75, R76, R81, R83 (in the backlog entry). Harmony's reading: 125 A; 126 A; 127 not a letter (new questions 135, 136; reading R85); 128 not a letter (Tap never starts, Resync starts); 129 B; R81 void (Quantize leaves the top bar).
