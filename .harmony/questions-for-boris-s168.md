# For Boris — s168 (2026-09-06)

Plain English. Three explanations he asked for, then the things actually waiting on him.

---

## 1. What a "tempo-locked oscillator" is

An **oscillator** here is a shape generator that produces a value going up and down over time —
a sine wave, a triangle, a ramp, a square. You connect it to a knob (opacity, scale, rotation)
and the knob then moves by itself in that shape instead of you moving it.

**Tempo-locked** means its speed is set in MUSICAL time, not clock time. You do not say
"one cycle every 1.6 seconds". You say "one cycle every 4 beats" or "one cycle every 2 bars".
The app watches the music, works out the BPM, and stretches or squeezes the shape so that one
full cycle lands exactly on the musical boundary. Speed up the track and the wobble speeds up
with it, still landing on the bar line.

That is the whole point of the feature: a free-running oscillator drifts against the music and
looks wrong within about ten seconds. A tempo-locked one stays glued to it.

Everything below is about the same thing: **what the oscillator uses as its clock.**

---

## 2. "Freeze in silence, or keep running from the last BPM?" — ALREADY ANSWERED BY YOU, ALREADY BUILT

You already chose this in the last session (your answer was "B", keep running). It is built,
proven, and pushed. This is the explanation you asked for of what you chose.

**The situation:** the app derives BPM by listening. Between two tracks — DJ pulls the fader
down, the room goes quiet for four seconds — the app hears nothing. It has no beat to follow.

**Option A, freeze:** the beat counter stops. Every tempo-locked oscillator stops where it is.
Every knob it drives holds still. The whole visual goes dead for those four seconds, then
lurches back to life when the next track comes in.

**Option B, keep running (what you chose):** the app remembers the last BPM it knew — or the one
you tapped in by hand — and keeps counting beats off an internal clock as if the music were
still playing. Oscillators keep breathing. The room never sees the visual freeze. When audio
comes back, the counter re-locks to whatever the new track is doing.

**Why it mattered more than it sounds:** the previous behaviour was worse than "freeze on
silence". The detector's "is there any audio" threshold was set at -160 dB, which is not silence,
it is mathematical zero. A real room's noise floor is around -45 dB. So the app almost never
believed it was in silence, and the bug bit in the opposite direction from where anyone was
looking. Two separate faults had to be fixed: the counter had to advance from the predicted beat,
AND a structural "phrase reset" was zeroing the counter again right after. Both are fixed.
Proof: bars advance with no audio at all, and a tapped tempo takes effect with no audio —
5 out of 5 checks pass, and 10 bars measured at exactly 20 seconds at 120 BPM.

**What is NOT fixed, and it is the reason I am asking you question A below:** the same class of
problem still bites during a track, not just between tracks. See below.

---

## 3. "The queued trigger question" — in detail. ALSO ALREADY ANSWERED BY YOU, ALSO BUILT

**What a queued (quantized) trigger is.** When Quantize is on and you hit a clip pad, the clip
does not start the instant your finger lands. The app waits for the next musical boundary — the
next beat, or the next bar — so the visual change lands ON the music instead of a fifth of a
second early. That gap between your press and the actual start is real time: at 120 BPM, waiting
for the next bar can be up to two seconds.

**During that gap, a promise is sitting in memory:** *"at the next bar, start clip 3 on layer 2
of deck 1."* That promise is the queued trigger.

**The question is what happens to that promise if the world it referred to changes before the
boundary arrives.** You switch decks. You clear the clip. You load a different composition. You
hit the global Stop. The promise was made about a situation that no longer exists.

Two possible answers, and the failure mode is the same either way if you get it wrong: the clip
fires two seconds later, on the deck you just switched TO, in front of an audience.

**Your ruling was A: a cue does not survive a Stop. Stop means stop.** Both the clip's own stop
and the big global Stop button now cancel anything pending. That is built.

**Why this was a bigger job than one line.** It turned out there were FOUR separate routes by
which a queued trigger could outlive its context, and each round of review found another one
after the previous round believed it was finished:
  1. Switching decks **froze** the promise instead of cancelling it — so it survived and fired later.
  2. Clearing the active clip left the promise armed. Fixing that also closed an unrelated
     "phantom active clip" bug that nobody had reported yet.
  3. Adding a deck, and appending a deck from a file, both make a deck active **without going
     through the normal deck-switch code** — so the cancel that had just been added never ran.
     One of those two paths was code that had shipped green earlier the same day.
  4. Undoing a "remove deck" reactivates the restored deck and deactivates whatever was current —
     a fourth route, found only by sweeping all 13 places in the codebase that write the active
     deck index, and reproduced with a deliberately adversarial test.

**The lesson recorded from it, which is why I am telling you the detail:** a quantized trigger is
a promise held across time, so *every* way of abandoning its context is a way for it to fire
wrongly. That shape — "a promise held across time" — is exactly what a recorded performance is
too, which is why it matters for what I am building now.

---

## THINGS ACTUALLY WAITING ON YOU

Nothing here blocks my current work. All of it is feel/taste — yours, not mine.

**A. When the music DROPS, should the oscillator shapes restart, or keep flowing?**
This is the one live musical question. The app detects structural moments in a track (a drop, a
build) and resets its bar counter when it sees one. Because oscillators read that counter, a real
drop mid-track **yanks an 8-beat shape backwards** — a slow breathing motion is suddenly back at
the start, mid-breath. That is a visible jump.
  - *Keep flowing (my default, being built now):* the drop hits, the visuals stay smooth and
    continuous through it. The wobble does not stutter.
  - *Restart on the drop:* every oscillator snaps to the top of its cycle when the drop lands,
    so the whole rig visibly punches in sync with the biggest moment in the track.
Honestly, both are defensible and it is a taste call about YOUR shows. I am building the
mechanism so it is a **switch**, defaulting to keep-flowing, so your answer costs a toggle and
not a rebuild. Answer when you can.

**B. When you fire a saved routine, should it put things back the way they were when you
recorded it, or start from wherever the rig is right now?**
Say you recorded a routine while layer 2 was at 30% opacity and the Invert effect was on. You
fire that routine three hours later with layer 2 at full and Invert off.
  - *Restore first:* it sets things back to how they were, so it looks exactly like it did when
    you recorded it. Predictable, but it will stomp on whatever you had set up.
  - *Start from now:* it just plays its moves on top of the current state. Never stomps, but the
    result depends on where you happen to be, so the same routine can look different every time.
My recommendation: **restore, with a per-routine "start from now" switch** for the ones you want
to layer on top. This is for a later lane, not blocking.

**C. Should a routine start on the next BEAT or the next BAR?**
Same idea as the queued clip trigger above. Next beat is snappier and up to half a second away.
Next bar is more musical and up to two seconds away.
My recommendation: **bar**, with a per-routine override, and the global Quantize setting winning
when it is on. Also not blocking.

---

## CLOSED WITHOUT NEEDING YOU

**Preset migration — the question is moot, there is nothing to migrate.**
You were twice asked to rule on how to migrate old presets to the new connection model, and twice
you said "clarify this". Rather than ask a third time I checked your actual disk. The app's own
data folder (`~/Library/Application Support/Audio-DNA/`) **does not exist**, there are no saved
compositions or presets anywhere under Documents, Desktop or Music, and your Audio-DNA folder
holds three screenshots and an empty Recordings folder. Nothing has ever been saved that a
migration could damage. Closing the question. If you save presets before the connection rewrite
lands, tell me and I will re-open it with real files in hand.

**For the record, the thing that would have been lost:** the old model let two different signals
drive the SAME knob at once, stacked. The new model gives each knob exactly one owner. Any preset
with a double-driven knob would have silently lost one of them. Zero such files exist.
