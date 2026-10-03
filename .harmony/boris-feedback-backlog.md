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

## Boris's answers to Harmony's 14 clarifying questions (received ~14:02, recorded 2026-10-03 14:03:17, s-rta-1003)
VERBATIM:
```
1 drop
2 restart
3 yes
4 yes
5 beats
6 both
7 not current bpm but have a bpm and beats input so user can do it by numbers and a x2 and /2 control to double or half easily
8 yes
9 yes
10 yes
11 drag handles that snap
12 bars
13 switch to - is fine for all bad chars
14 no more tuning
```
The questions as asked (Harmony's wording) and what each answer settles:
1 Deck tab dot (a small dot on a deck's tab when one of its clips is playing): keep or drop? -> DROP. bf9b: the tab dot
  (ruling-bf9b amendment 16(b), machine check M-e) is removed together with the strip badge (BF14).
2 A video you played, replaced, then fire again: continue where it left off (today) or restart? -> RESTART. Every fire of a
  video starts it from its start (in point). Changes the bf9b C3 "resume" contract (K10 (ii), Boris-page step 8.11) ->
  built in the transport lane after the bf9b merge; bf9b's Boris page drops 8.11.
3 While recording, the destination cell shows a live picture, a REC mark and the running length; it is not played into the
  output. -> YES.
4 Recording one layer with see-through parts keeps them see-through. -> YES (needs an alpha codec; H.264 cannot carry alpha).
5 BPM-synced clip length unit. -> BEATS.
6 "Change it by amount": type a number plus /2 and x2 buttons. -> BOTH.
7 Automatic length = closest of 1, 2, 4, 8, 16, 32 bars at the current BPM? -> NO: "not current bpm but have a bpm and beats
  input so user can do it by numbers and a x2 and /2 control to double or half easily". Harmony's reading: a BPM-synced clip
  carries its own BPM and its length in beats, both typed as numbers, with x2 and /2. OPEN: what the two numbers start at.
8 "1/6" in the Quantize list = 1/16. -> YES.
9 Sync: the beat wheel stays exactly where he tapped; only the picture is shifted. -> YES (lane/bf2's wheel follows the
  shifted beat today: must change).
10 Opening a composition loads its saved venue and sync value, replacing what the app had; the SYNC note shows. -> YES.
11 Trimming a long sample: drag the two end handles; they snap to the bar lines. -> DRAG HANDLES THAT SNAP.
12 MilkDrop preset change labels. -> BARS (seconds removed).
13 Characters a file name cannot hold. -> "switch to - is fine for all bad chars": every bad character becomes "-".
14 Other codecs still open and play as today; no more tuning for them. -> NO MORE TUNING.

## Follow-up on answer 7 (recorded 2026-10-03 14:10:11) — Harmony asked: "What should the two boxes start at when you first switch a clip to BPM sync? Default: read a BPM from the file name if it has one (like \"128bpm\"); otherwise pick the beat count (4, 8, 16, 32, 64) that puts the clip closest to 120 BPM."
Boris (verbatim): "7 follow up: default and bpms for clips to 120"
Harmony's reading (INFERRED, put back to him): the default stands, with 120 as the BPM a clip is assumed to be when its file name gives none. Second reading, if he corrects: the BPM box starts at exactly 120 and the beats box shows whatever the clip's length gives at 120 (2 beats per second, possibly not a whole number).
CLARIFIED by Boris (verbatim, recorded 2026-10-03 14:10:25): "default all bpms to 120" -> every clip's BPM box starts at exactly 120 (no
file-name rule, no nearest-power-of-two rule); the beats box shows what the clip's length gives at 120 BPM; he then types a
number or uses x2 and /2. Harmony's first reading above ("the default stands") was WRONG and is superseded.

## Boris's answers to questions A-E (recorded 2026-10-03 14:55:32, s-rta-1003)
VERBATIM:
```
a Let's not allow control Z to change anything that is live in the layer strip. It changes anything else
b yes it stays in clip tab regardless of deck
c can you clarify?
d yes
e every show remembers it's sync
```
The questions as asked (Harmony's wording):
A "You duplicate or load a deck, a routine starts one of its clips, then you press Cmd+Z. Should that clip keep playing,
  like when you delete a deck? Default: yes. One exception for now: if the loaded deck added layers to your show, Cmd+Z
  takes those layers and their clips away."
B "A clip is selected and you click another deck tab. Should the Clip tab stay on that clip, or jump to the same spot on
  the new deck? Default: stays, as today."
C "After deleting a deck, the 'Undo Remove' button only shows when the tab row has room; with many decks only the text line
  at the top says it. Enough? Default: yes."
D "Gain: twice as long, same scale? Default: yes. Alternative: also stretch the low end so 0 to 1 takes half the slider."
E "A show saved while Sync sits on 'Default' at 0 does not remember a sync setting, so opening it later leaves the room's
  sync alone. Default: yes. Alternative: every show remembers its sync, 0 included — then a show saved at home resets the
  room to 0."
BF31 [NEW RULE, from answer a] Undo (Cmd+Z) never changes what is live in the layer strip; it undoes everything else.
  Harmony's reading (consequence text): a fired clip is not an Undo step; undoing a structural edit (add / load / duplicate
  / remove deck, add / remove layer, clear) leaves what is playing playing. Today a trigger IS an Undo step (TriggerClipCmd)
  and Undo of a Load Deck that added layers stops that deck's clips (ruling-bf9b-merge AM-7's pinned exception, T6h).
  NOT built in the bf9b lane (no regression there: both behaviours are main's today) -> its own lane right after the merge,
  with pre-registered expectation changes (T6h, the trigger-undo tests, Boris-page step 8.6). OPEN: the exact edges (Undo of
  Clear, Undo of Add Layer while that layer plays) -> the plan decides, council attacks.
Answer b: the Clip tab stays on the clip he is working on, whatever deck is shown (= today; ruling Q-C closed).
Answer c: he asks for a clarification of question C (owed in plain words).
Answer d: Gain twice as long, same scale.
Answer e: EVERY show remembers its sync, 0 included (REVERSES the ruling-bf2-delta default "a show saved at Default 0 does
  not remember a sync setting").
