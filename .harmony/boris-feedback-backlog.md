# Boris feedback backlog — app evaluation of 2026-10-01 (received 2026-10-02 14:01:06, session s-rta-1002b)

Boris's message, VERBATIM:
> here is some feedback as I evaluted the app yesterday. Ask me questions on any item you don't understand then add to the
> work we need to do. There are other issues but these stand out the most:
>
> Sampling to other layers/ clip. Think how to do this, like set a clip to record output of the composition and they be able
> to use it as soon as it is done. This would be good to have it automatically start and end on bars or beats, quantized
>
> We need a nice clean way to adjust the delay or speeding ahead for each room so we can set it per room. Like if the audio
> is slower than the video, this delays or advances signals and bpm with one very sensitive dial.
>
> Codec display for each video and easy access to that video in finder
>
> Need to manually change envelopes. Need to expand envelope controls and setup format
>
> Make it so when you load a sample into the envelope that it will emulate the envelope of the audios behavior. So basically
> use a sample to extract the envelope live, and then different envelopes can be pulled out of that sample with the envelope
> creator
>
> Timeline control is not working for parameter control
>
> Add bars to all places we have beats
>
> Need to be able to rename each deck with double click
>
> Clips in the layer should not persist between deck changes. Whatever is in the layer should be what is playing and there
> shouldn't be anything from other decks.

## Items (Harmony's reading — consequence text, not Boris's words). Status: OPEN-Q = waiting on Boris's answer; READY = defaults taken, plannable now.

BF1 [OPEN-Q] Record-to-clip ("resample"): record the composition output into a clip that is playable the moment it ends;
    start / stop snapped to the bar / beat. Reuses the existing FFmpeg video recorder (docs/claude/integration.md).
    Questions sent: source (whole output default / one layer / one deck); length (preset N bars default / press-start-press-stop);
    destination (arm an empty cell default); keep files beside the composition (default yes).
BF2 [OPEN-Q] One fine "sync" dial, saved per room: shifts audio-driven signals + beat clock / BPM later or earlier.
    No such control exists today (docs grep: no sync / latency offset). Physics constraint (Harmony, by causality): EARLIER
    can only move beat-locked things (beat clock, BPM-synced oscillators / envelopes, beat-snapped triggers, autopilot) via
    the beat grid; loudness / onset reactions cannot happen before the sound is heard. LATER can delay everything.
    Questions sent: "room" = venue profile (default) or per output screen; range default -300..+300 ms, 1 ms steps.
BF3 [READY] Codec display per video + Show in Finder. Defaults: codec + resolution + fps in the Clip inspector and the clip
    cell tooltip; "Show in Finder" in the clip cell right-click menu and a button in the Clip inspector.
BF4 [OPEN-Q] Envelope editing: today the Envelope signal (SignalInspector) has 3 curve types, 5 beat durations, amplitude,
    phase, loop / one-shot and a PAINT-ONLY curve editor (NOT draggable — APP-INVENTORY:75). Default reading: drag / add /
    delete points, bend segments, more durations incl. bars. Question sent: what "setup format" means.
BF5 [OPEN-Q] Envelope from an audio sample: load a sample, extract its loudness envelope, pull several envelopes out of it
    (default: lows / mids / highs) in the envelope creator. Questions sent: sample audible or silent (default silent);
    stretch to the set beat / bar length (default yes); what "different envelopes" means.
BF6 [BUG, investigate now + Q] "Timeline" as a parameter source (UniversalParamControl source picker lists Timeline,
    APP-INVENTORY:105; binding-decisions ruling 13(a) "A TIMELINE IS A CONNECTION SOURCE") does not drive the parameter.
    Question sent: which parameter, what he did, what happened. Diagnosis lane starts regardless (definitive-fix loop).
BF7 [READY] Bars wherever beats appear: every beat-count setting also offers bar values (4/4: 1 bar = 4 beats); beat
    counters read bar.beat. Needs an inventory of every beat-valued control first.
BF8 [READY] Double-click a deck tab = rename in place (Enter saves, Esc cancels). Today: right-click > Rename Deck...
    (docs/claude/performance-controls.md:51).
BF9 [OPEN-Q, POSSIBLE REVERSAL] Clips must not carry over between deck changes. CONFLICT CHECK: BORIS_DECISIONS.md
    2026-09-26 "Deck switch mid-fade": "finish the fade. when we load a new deck that does not touch the clips playing in
    the layer"; persistent layers (2026-09-26): "just keep the persistent clip in the layer strip, nowhere else."
    Question sent: what he saw (non-persistent clip from the old deck? an old-deck fade finishing? a persistent layer?) and
    whether this replaces the 09-26 rule. No build until answered (binding-decision change).

## Boris's answers (2026-10-02 14:30:49) — recorded verbatim in .harmony/binding-decisions.md "2026-10-02 (s-rta-1002b)"
All nine items are now plannable: BF1 (output or selected layer, press start / stop snapped to the bar, next empty cell
of the top layer, video files), BF2 (venue profiles, -500..+500 ms, typed value + +/- fine tune), BF3 / BF7 / BF8
defaults, BF4 (bigger editor; own window later — recommendation), BF5 defaults, BF6 (Timeline follows the clip playhead;
layer params follow the layer's clip playhead; composition not needed), BF9 (REVERSAL: deck change stops the old deck,
Persistent removed, ignore-column stays). Open follow-up: BF9 what a deck shows when you come back to it.

BF10 [BUG, READY] (2026-10-02 14:40:58) MilkDrop output not adjusted to the composition-size canvas. Boris (verbatim): "milkdrop output
    has not been adjusted since we changed output to be composition size. Please add to fix list" + a screenshot of the
    Preview/Output panel: the MilkDrop picture fills only a lower-left block (~55 % wide x ~50 % tall); a black band runs
    across above it and a black block sits to its right; stretched streak / garbage imagery fills the top strip and the
    bottom strip. INFERRED: the MilkDrop render target / viewport keeps an old fixed size (pre-canvas) instead of the
    composition canvas (Pitfall 37). Needs instrumented diagnosis (decode a capture of the source at 2 canvas sizes).

BF9 CLARIFIED (2026-10-02 14:44:36): decks are boxes of clips; switching decks never changes what plays (verbatim in BORIS_DECISIONS.md
    "Decks are boxes of clips"). The running plan-bf9 (premise "deck change stops the old deck") is SUPERSEDED -> re-planned
    as bf9b; plan-bf9.md is research input only.

# Boris feedback of 2026-10-03 (received ~13:28, recorded 2026-10-03 13:32:39, session s-rta-1003) — answers to the Oct 2 page (39 questions) + new items
## VERBATIM (his whole message; lead-in: "here is my feedback. organize it as you wish and give me list of questions to clarify, if needed:")
```
Firing a column that is already playing should restart its videos

What does this mean: Momentary pad released before its quantized beat: now cancels

Little correction. When record to clip, we want to see the recording in the layer it is recording to while it is being recorded. Can you do that?

The layer strip does not need to show the deck a clip is playing from.

Recording a clip should be prominently displayed, so it can be recorded as long as the user likes. There's no limit except hard drive Space.

The recording should look exactly like the output. If there's a logo, it should look exactly as it's displayed.

The recording will always play like a regular clip once it is recorded. 

As far as a video clip is concerned, it is in two categories, either BPM synced throughout the whole thing, or just playing with a speed control. If it is BPM synced, regardless of the length, it is synced to the current playing BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change the bars in it by amount. Let me know if you have any questions on that. Regardless of the clip being BPM or speed controlled, if we are in Qantize mode, it is triggered on time by the Qantize method.  There should be no problem playing clips that are set up for BPM and clips that are set up for speed at the same time. Their speeds are just controlled differently.

For sync control we should have a way to remember it as part of a composition save. I imagine if a user is doing this professionally, they will set up the venue and save it in case of a computer crash or something and if they come back to that venue, they have the settings already.

Sync lives in top bar

For sync and setting the BPM. The user of the application will tap tempo and keep the application in time with the music. It is hard to tap ahead or behind the beat. The user will try to have the beat matched exactly to the music so the user can see it pulsing exactly to the time of the music. Sometimes the visual system is a little delayed and will need to be a little ahead or sometimes the visual system is a little faster than the music and that's what this is for. The visuals should be on the delay. The music is in time and the visuals should be delayed or a little ahead depending on how the system is wired. Let me know if you have questions to clarify. 

Long samples: squeeze the whole sample into the envelope length (up to 10 minutes), or pick a section? Default: the whole sample. I want to be able to use the whole thing or to shorten it manually but clicking to timing marks.

For Milk drop there is no timeline, but there are effects.

Explain this to me. Where do I draw the timeline curve? [ After this fix, “Clip Position” and “Timeline” do the same thing: the knob goes from its low end to its high end as the clip plays. Once you can draw the Timeline curve, Timeline follows your drawing and Clip Position stays a straight line. Keep both, or remove Clip Position? Default: keep both.]

Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6

Let’s get rid of beats and just have bars in most places unless beats are necessary. For setting a clips beats, these are done with beats and not bars. For most other things, we will use bars. For the circle at the top that counts off 1234 and then starts over, those are beats as well. 

We should pick the most optimal Kodex to use and not worry about the other ones. Web M is not necessary. I want codecs that decode easily and play well

If there are ‘/‘ characters in a decks file name, do not worry about adding sub folders. They are just characters. If there are characters that are no good, then let me know and we will create a fix together

Can you figure out how to get everything working in the keying menu? Does the keying slider actually do anything? Maybe we get rid of it. Some elements in the keying menu work and some don’t. Should we use the transparency slider to do the work for keying elements? Also review the compositing menu, right slider in layer strip. These elements don’t work: Creative, 3D, 

When I grab the play head and move the timeline, I do not want it to jump back to where it was or where it should be playing before I grabbed it. I wanted to keep playing at the same speed, but play from wherever I drop the play head. Does this make sense?


Testing some of your work. When I switch decks, the clips in the layers disappear. They should remain. The output also goes black when I switch decks in there are clips playing in a layer.


How do I access the bigger envelope, editor? I don't think I need it? I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window.

I have beats and seconds in the milkdrop editor. I only need beats.

Gain slider in top bar could be twice as long. I have very little space to move it because I start at a quarter from the left edge and move down from there so I have very little distance to make a lot of fine adjustment.

All the other defaults are good.
```

## Items (Harmony's reading — consequence text, not Boris's words). Page = .harmony/.reports/s-rta-1002b/boris-checks.html (questions 1-39).
BF11 [NEW, READY] Firing a column that is already playing restarts its videos (today a column re-fire leaves them running).
BF12 [EXPLAIN] "Momentary pad released before its quantized beat: now cancels" (s-rta-1002 page, feel check) — explanation owed, no build.
BF13 [bf1 DELTA] Record to clip: (a) the recording is visible in the layer / cell it records into WHILE recording (OPEN-Q: how —
     a live picture in the destination cell vs played into the layer; a whole-output recording played into its own layer would
     record itself); (b) page Q7 ANSWERED: no automatic end; REC shown prominently; the only limit is disk space (a disk-space
     guard is Harmony's consequence); (c) page Q8 ANSWERED: "should look exactly like the output ... logo ... exactly as it's
     displayed" (OPEN-Q: single-layer recording keeps see-through parts? default now YES); (d) page Q9 = default (plays like a
     regular clip; Quantize decides).
BF14 [bf9b DELTA, READY] The layer strip does NOT show the source deck: remove the strip's source-deck badge (ruling-bf9b
     amendment 16(a)-(c): badge + badge click) before the merge. OPEN-Q: the deck TAB dot (default keep).
BF15 [NEW MODEL STATEMENT] A video clip is either BPM-synced (always follows the current BPM; a length the user can set; the app
     sets it automatically, the user changes it) or speed-controlled; Quantize times the START of both kinds; both kinds play
     side by side. (Today: transport mode "BPM Sync" with beat-division presets exists — docs/claude/history.md:55,
     architecture.md:92 beatDivision.) OPEN-Q: unit (his message says "bars" here and "beats" under BF21), how "by amount"
     works, the automatic length rule.
BF16 [bf2 DELTA] Sync: (a) saved as part of the composition (venue recall after a crash / return visit); (b) page Q14 ANSWERED:
     it lives in the TOP BAR; (c) page Q16: the user taps in time with the music; the beat stays in time with the music; the
     VISUALS are what gets delayed / advanced (confirm: beat wheel unshifted, output shifted).
BF17 [bf45 DELTA] Long samples (page Q21): whole sample by default, AND shorten it by hand by clicking timing marks.
BF18 [bf6] Page Q24 = (a): MilkDrop has no timeline (Timeline greyed out); its effects remain.
BF19 [EXPLAIN] Page Q25: where the Timeline curve is drawn — explanation owed; the default (keep both) stands until he says.
BF20 [bf7 DELTA] Quantize menu: 1 bar, 1/2 bar, 1/4, 1/8, "1/6" (OPEN-Q: read as 1/16).
BF21 [bf7 DELTA] Bars in most places; a clip's length is set in BEATS; the top circle counts BEATS (1-2-3-4).
BF22 [NEW] Codecs: pick the one that decodes easily and plays best; do not chase the others; WebM not needed (page Q39: no job).
BF23 [BUG, page section 3] "/" in a deck name is just a character — never a sub-folder. Characters a file name cannot hold:
     tell him, fix together.
BF24 [NEW AUDIT] Keying menu: make every entry work or propose removal; does the Keying slider do anything (maybe remove; maybe
     the transparency slider does the keying amount); compositing (blend) menu on the layer strip's right slider: the
     "Creative" and "3D" groups do not work.
BF25 [BUG] Dragging the playhead: after the drop the clip plays on from the drop point at the same speed; it must not jump
     back to where it was / would have been.
BF26 [REPORT] "When I switch decks, the clips in the layers disappear ... output also goes black" = the app WITHOUT the deck
     change (lane bf9b is built, not merged: main 5abdf01 vs lane/bf9b a7491d4; the lane's own probe shows the pre-change app
     changing the picture on a switch). Merge = this session's first job.
BF27 [bf45 REVERSAL] No bigger envelope editor. Every control works in the Signal tab's envelope editor (supersedes BF4
     "bigger envelope" and page Q17 / Q18).
BF28 [bf7] MilkDrop editor: beats and seconds shown today; only the musical unit is needed (OPEN-Q: label in bars or beats).
BF29 [NEW, ui] Top bar Gain slider twice as long (he works in the lower quarter of its travel).
BF30 "All the other defaults are good." -> every page question not named above takes its default (decks Q1-Q4; record Q5, Q6,
     Q10; sync Q11 bar-not-dial, Q12, Q13, Q15; envelopes Q19, Q20, Q22, Q23; Timeline Q26; bars Q27, Q29, Q30 (except clip
     length = beats); deck rename Q32, Q33; picture info Q34; Q35 on-screen test strip NOT OK'd; MilkDrop Q36, Q37; MKV Q38).
