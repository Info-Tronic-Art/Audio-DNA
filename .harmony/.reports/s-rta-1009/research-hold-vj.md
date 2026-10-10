# Hold / momentary trigger: what VJ programs and VJs actually do

Researcher: VJ software and VJs (one of three). Accessed 2026-10-09.
Note: firecrawl was down (errors) -- used Exa + WebFetch. The Resolume forum, reddit (via WebFetch), HeavyM help centre and vjun.io return 403 to my fetch tools; where a page could only be read as a search extract that is said in the finding.

## HEADLINE

Every VJ program aimed at live play that I could read has a hold-while-pressed trigger, and almost always as a per-control switch next to a normal/toggle mode: Resolume ("Piano", also called Momentary/Flash), MadMapper (keys momentary by default), HeavyM (Toggle tick-box), GrandVJ (hold is the default in synth mode), Visibox ("Gate"). VJs use it for flashes, strobes, blackouts and "slams" on a held pad, and to play clips like chords. I found no VJ who dislikes it; the complaints are about its limits (hard cut on release, no soft fade) and about controllers that default to hold.

## FINDINGS

1. Resolume: ANY button or key shortcut can be set to hold ("Piano"): on while held, off on release; the maker says it can be called Momentary or Flash.
   URL: https://resolume.com/support/en/keyboard-shortcuts (also https://resolume.com/support/en/7/midi-shortcuts)
   Quote: "on for as long as you hold the key down, and turn off when you release the key again" / "you can call this option Momentary or Flash"
   HOW SURE: READ ON THE MAKER'S PAGE.

2. Resolume clips have three trigger styles per composition or per clip: Normal (retrigger restarts), Toggle (second trigger turns it off), Piano (stops on release).
   URL: https://resolume.com/support/en/7.17/clips
   Quote: "When triggered the clip starts playing. When released, it stops playing."  Toggle: "When it is triggered again, it turns itself off."
   HOW SURE: READ ON THE MAKER'S PAGE.

3. Resolume's maker pairs hold with "Free Layer" to play clips as chords, and has a training chapter titled "Visual Chords".
   URL: https://www.resolume.com/index.php/support/en/7.17/clips (chord line, read as search extract); https://resolume.com/training/2/11/68 (extract)
   Quote: "play 'chords' of clips with the keyboard or a MIDI device"; training: "play Resolume like an instrument"
   HOW SURE: READ ON THE MAKER'S PAGE (Trigger Style text opened in full; the chord line and training line only as search extracts).

4. Resolume's own examples of what hold is for: hand-played strobe at different rates, and a temporary "hectic part" change that snaps back on release.
   URL: https://resolume.com/support/en/keyboard-shortcuts
   Quote: "Now you can strobe the invert effect at different rates." / "during an especially hectic part in the music"
   HOW SURE: READ ON THE MAKER'S PAGE. Manual also: Piano on an item "will switch it back to whatever is was set to" (search extract).

5. Performer: Zunayed Sabbir Ahmed (We Are VJ Bangladesh) holds an APC40 MK2 pad to strobe, sets Piano + Invert so the effect is off until pressed, and does the same on keyboard Num keys; he mentions using an APC25 for hand-mapped "slams" (flash clips).
   URL: https://zunayed.com/strobe-with-emotion/ (2022-07-25); https://zunayed.com/slams-in-your-hand-midi-keyboard-mapping-resoume-tricks/ (2022-08-01)
   Quote: "Now we can hold this button to strobe and ctrol the frequency!"
   HOW SURE: READ FROM A PERFORMER (a tutorial, not a gig report).

6. Forum voices (reddit r/vjing, June 2015) recommend hold for a screen flash: map a button to a strobe layer/effect bypass in Piano mode.
   URL: https://www.reddit.com/r/vjing/comments/3b1hbw/screen_flash_in_resolume/
   Quotes: u/TheCheeks "make sure it's set to Piano mode so you can hold it for strobe, then let go to stop it"; u/dsquareddan "a MIDI button mapped to the bypass with Piano mode turned on".
   HOW SURE: ONE FORUM VOICE (two voices, same thread; same thread also says Modul8 had a flash on a MIDI key).

7. A newcomer (Jan 2024) binds a bloom effect to a held key in Piano mode and wants it to fade in while held; a reply shows a hold-style effect clip plus layer transition time gives slow blend-in on hold and blend-out on release.
   URL: https://www.reddit.com/r/vjing/comments/1958glp/fade_in_out_bound_effect_resolume/
   Quote: "If you press and hold the key to trigger the effect clip, it will slowly blend in. If you release the key it will slowly blend out."
   HOW SURE: ONE FORUM VOICE.

8. MadMapper: keyboard controls are momentary BY DEFAULT; a per-key Toggle tick-box makes it latch. MIDI notes map to clips/cues/surface visibility.
   URL: https://docs.madmapper.com/madmapper/6/11.-live-performance-and-control
   Quote: "By default, keyboard controls are mapped as momentary switches"
   HOW SURE: READ ON THE MAKER'S PAGE.

9. HeavyM: each keyboard assignment has a Toggle tick-box; unticked it is a "push" button (max on press, min on release). The maker's live-control page lists "triggering a strobe effect" as the MIDI example.
   URL: https://help.heavym.net/hc/en-us/articles/360015325180-Controlling-HeavyM-with-a-Keyboard ; https://www.heavym.net/interacting-live-with-a-video-mapping/ (2026-07-16)
   Quote: "Toggle is unchecked: if you press the key, it will set the slider to the max value. When you release it, it will set it to the min value."
   HOW SURE: READ ON THE MAKER'S PAGE, but as a search extract only (both pages 403 to me).

10. GrandVJ (ArKaos): in synth mode hold is the DEFAULT; the toolbar has "latch" (toggle) and "hold" (keep everything running after release).
    URL: https://cdn.inmusicbrands.com/arkaos/downloads/ArKaos_GrandVJ_UserGuide.pdf (read locally; user guide, 2nd edition, undated, old product)
    Quote: "Normally, a cell runs as long as the corresponding keyboard/MIDI key is held down. If latch is activated, triggering a cell will work in a toggling fashion."
    HOW SURE: READ ON THE MAKER'S PAGE. Also: "This is where you play visuals like you would play an instrument."

11. Visibox (Spaceage) has a per-clip Launch Mode with exactly three values, Trigger / Gate / Toggle; Gate is hold-to-play with a Transition Out on release; clips with no setting default to Trigger (mono songs) or Toggle (poly songs).
    URL: https://manual.spaceage.tv/6.0.0/launch-modes/ (undated manual, v6.0.0)
    Quote: "Gate is hold to play." / "This is how you punch a stab in on a hit and take it straight back out"
    HOW SURE: READ ON THE MAKER'S PAGE. It also says releases "are ignored by every mode except Gate", and a Gate clip clicked on screen with no release behaves like Trigger.

12. VDMX has no named hold mode in its media-bin docs: default is play-until-replaced; a "Note Offs" auto-eject option makes it behave like a gate. A VDMX user (Sept 2025) found his momentary pads play only while held and asked for a way to ignore note-off.
    URL: https://docs.vidvox.net/vdmx/vdmx_plugins ; https://discourse.vidvox.net/t/media-bin-midi-triggers-dont-echo-back-properly/2736 (2025-09-10)
    Quotes: docs "continue to play until a new clip is triggered or the file is ejected"; user "the clip only plays when I hold the pad"; user wish "don't listen to note-off" (search extract).
    HOW SURE: READ ON THE MAKER'S PAGE (docs, the "gate" reading is my inference) + ONE FORUM VOICE.

13. Synesthesia: controls are Toggle or Bang; note-on/off drive Toggle, and Bang deliberately ignores note-off (a press-only type).
    URL: https://synesthesia.live/docs/faq/midi_osc.html
    Quote: "Bang type ignores Note Off entirely."
    HOW SURE: READ ON THE MAKER'S PAGE.

14. TouchDesigner (a toolkit, not a VJ app) offers Momentary and Toggle as button types; one VJ (Aug 2022) replaced his MIDI controller with a touch panel of momentary buttons.
    URL: https://docs.derivative.ca/Button_COMP ; https://forum.derivative.ca/t/issues-with-touchscreen-and-momentary-button/280210 (search extract)
    Quote: "A momentary button that is switched on when pushed down."; user "replace the MIDIcontroller I use for VJing by a touch interface"
    HOW SURE: READ ON THE MAKER'S PAGE + ONE FORUM VOICE (extract).

15. Modul8: its own module buttons have a Toggle tick-box (default for buttons) and Down/Continuous/Up values; separately, a 2010 user found his Trigger Finger pads "set to hold by default" and could not make them toggle for visual filters.
    URL: https://www.garagecube.com/documentation/modul8/modules_manual/ ; https://forum.djtechtools.com/t/help-modul8-vj-trigger-finger-toggle-hold-midi-map/10215 (2010-04-05, old)
    Quote: "the pads are set to hold by default and I can't for the life of me switch them to toggle."
    HOW SURE: READ ON THE MAKER'S PAGE (module buttons only; I found no media-slot hold mode in Modul8) + ONE FORUM VOICE.

16. MixEmergency (DJ-video app, Serato DJ add-on) has a per-sample Play Mode of three values, one of them "Play While Held".
    URL: https://www.inklen.com/mixemergency/manual/sampleplayerwindow/ (undated)
    Quote: "Play While Held (plays while the sample trigger is held)"
    HOW SURE: READ ON THE MAKER'S PAGE.

17. A 2026-era performer remote for Resolume (ROGGER, touchscreen/gamepad, GitHub) is built around hold: "Hold to fire, release to clear", a flash/invert/strobe/"bump" grid.
    URL: https://github.com/riegergogi2001/resolume-rogger (no project date)
    Quote: "Hold to fire, release to clear, and beat-synced repeats that follow your tap tempo."
    HOW SURE: ONE FORUM VOICE (one developer's tool; looks AI-assisted; the sections about momentary FLASH bank only seen in a search extract).

## WHAT IT IS USED FOR ON STAGE

- Flash / white screen hit while held (6, 5, 17).
- Strobe only while the pad is down, off the instant it is released, rate chosen beforehand (4, 5, 6).
- Blackout / invert as a manual strobe, held (5, 6; Resolume forum "I usually map a button as a composition invert, nice little manual strobe effect" -- burnalot, 2015, forum t=12745, search extract only).
- "Slam" / punch clips fired by short presses; stab "on a hit and take it straight back out" (11, 5).
- Playing several clips at once as chords, each shown only while its key is down (3, 10).
- A temporary change during a hectic passage that snaps back on release (4).
- Effect on a key while held, e.g. bloom (7).
- Hold as the way to play a visual instrument ("Piano", "play visuals like you would play an instrument") (1, 3, 10).

## AGAINST IT

- No one said "I never use it" or "it gets in the way". Searches run: "piano mode never use / don't use / annoying"; "prefer toggle"; reddit r/vjing hold/flash; VJ interviews (Nox Lumina interview on protoloops.com, Gabe Damast/Zedd note on the Resolume forum) -- none mention hold at all, for or against. Absence of complaint is not proof of use.
- Hold's hard edge: release cuts straight to off. A 2025 Resolume forum post (search summary only, not read) asks for a soft fade-out on release; finding 7 shows the 2024 reddit user hit the same wall and needed a workaround.
- Resolume itself limits it: on clip triggers hold is a property of the clip, not of the input ("a clip trigger is either set to Piano or not. It can't be set to Piano for midi and Normal for something else" -- staff, 2015, https://resolume.com/forum/viewtopic.php?t=12519, search extract). A 2023 thread says clip hold did not work over DMX (https://resolume.com/forum/viewtopic.php?t=22398, extract). Staff declined a click-to-toggle mouse variant of Piano (from WebSearch summary of forum p=50071; not read).
- Controllers that send hold by default annoy people who wanted toggle (15); a VDMX user wanted to ignore note-off (12). The need: the setting is per control, not per hardware.
- Nuance: Zunayed, "Strobe or NOT!" (2025-06-26, https://vjun.io/zsabbir/strobe-or-not-free-wire-patch-vj-tips-resolume-1c2a, search extract; 403): "Strobe CAN NOT be the only effect!" -- a taste warning, not an argument against hold.
- Makers that skip a hold mode: Synesthesia (Toggle/Bang, no hold type, 13), VDMX (no named mode, 12), Modul8 media slots (none found, 15).

## HOW THE PROGRAMS NAME IT

- Resolume: "Piano" (with "Invert"); maker says "Momentary or Flash" works as a name; clip trigger styles Normal / Toggle / Piano (1, 2).
- MadMapper: "momentary" (default) vs "Toggle" (8).
- HeavyM: "push" button vs "Toggle" (9).
- GrandVJ: "latch" / "hold" (hold = keep all running; the key-held behaviour is just the default) (10).
- Visibox: "Gate" vs Toggle vs Trigger ("hold to play") (11).
- MixEmergency: "Play While Held" (16).
- TouchDesigner: "Momentary" vs "Toggle" (14).
- Synesthesia: "Toggle" vs "Bang" (Bang = press only) (13).
- VDMX: "momentary"/"toggle" are controller-side; app side "Note Offs" auto-eject (12).
- Millumin: interaction "transformer" -- staff: "waiting for a Note ON (press) then a Note OFF" with the "switch" transformer (https://forum.millumin.com/discussion/1527/..., read; not a hold mode as such).
- Beyond VJ (search extracts only, not read): Ableton Live clip Launch Mode "Trigger / Gate / Toggle / Repeat" (https://docs.cycling74.com/apiref/lom/clip/); FL Studio "Hold & stop", "Latch"; Novation Launchpad "Momentary / Toggle". The same words the DJ and producer researchers should find.

## NOT FOUND

- Any count or survey of how many VJs use Piano/hold; no usage data from Resolume.
- A VJ saying in their own words that they hold a pad for a logo through a break. The nearest is Resolume's persistent clips for "Promotor logos, flashes or live cameras" (https://www.resolume.com/index.php/support/en/7.17/clips, extract), which is latch, not hold. A logo is a toggle job in everything I read.
- Any working VJ (band or DJ gig) interview that names hold/Piano as part of their rig. Interviews I read or saw do not mention it.
- Docs for hold in Millumin, NestDrop, CoGe VJ (CoGe doc found covers slots/sequencer only), Modul8 media slots.
- The Resolume forum threads and the vjun.io post could not be opened (403); several items rest on search extracts, flagged above.
- Dates: Resolume, Visibox, GrandVJ and MixEmergency manuals carry no date; treat as current (Resolume, Visibox) or old (GrandVJ, Modul8 2010).
