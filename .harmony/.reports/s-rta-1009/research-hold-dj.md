# RESEARCH: press vs hold on pads and keys -- the DJ and DJ/producer side
Researcher 1 of 3 (DJs, DJ/producers, their decks, controllers and software). Accessed 2026-10-09. Read-only; no app touched.
Tool note: firecrawl errored on its first call, so the search ran through exa and WebSearch/WebFetch (firecrawl -> exa fallback). Some maker pages gave HTTP 403 to WebFetch; they were read through exa fetch.

## HEADLINE
On decks, pads and DJ software, "works only while held" is everywhere, but mostly as a fixed behaviour of one function (CUE, loop roll, Slip, XY-FX, pad FX), not as a setting on every pad. Where it is a setting, it sits on a clip, a pad mode or a whole function (Ableton Gate, Traktor Gate/Latch, Denon "Momentary/Trigger", Resolume "Piano", Rekordbox "GATE playback"), and DJs keep asking for the way back: hold turned into latch. I found no DJ or DJ/producer who holds a pad to drive visuals; the held-visual evidence is VJ-side only (Resolume).

## FINDINGS
Confidence labels as asked. "Extract" = I saw the quoted text in a search-result extract of the maker's PDF, not the whole file.

1. A CDJ/club-deck CUE button is a held button by design: hold = play from the cue point, release = back to it. Tapping it while playing jumps back and pauses.
   URL: https://virtualdj.com/manuals/hardware/pioneer/cdj3k/controls.html
   Quote: "Press and hold this button to preview track while held. While held, press PLAY to continue playing or release to return to the CUE position."
   Also: https://www.deejayplaza.com/en/articles/what-is-cue-button -- "you need to hold the Cue Button to keep playing the track, otherwise the player jumps back".
   Also Pioneer's own CDJ-3000 manual (extract, https://rental-supplier.nl/wp-content/uploads/2024/11/CDJ-3000_manual_EN_AV_Supplier.pdf): "Playback continues until you release the [CUE] button."
   HOW SURE: READ ON THE MAKER'S PAGE (VirtualDJ page is a third party's description of the CDJ-3000; the Pioneer manual itself is extract only).

2. The same hold is built so you can turn it into latch in the middle of the gesture: hold CUE, then press PLAY, then let go of both, and it keeps playing. On the XDJ-AZ the same is done on a hot-cue pad.
   URL: https://downloads.support.alphatheta.com/manuals/all-in-one-dj-systems/XDJ-AZ/html/en/000COV_en/Using_the_Performance_Pads/Using_the_Performance_Pads.htm
   Quote: "Press the [] button during Gate playback to continue playback even if you release the Performance Pad." (the play glyph dropped out of the page text)
   Same idea on the CDJ: finding 1 ("While held, press PLAY to continue playing").
   HOW SURE: READ ON THE MAKER'S PAGE (AlphaTheta XDJ-AZ manual).

3. Hot cues: the maker's own choice between hold and press is offered as ONE setting for all pads, and only when the deck is paused. Denon: "Paused Hot Cue Behavior" Momentary or Trigger. AlphaTheta: a separate "Gate Cue" pad mode. Rekordbox on a DDJ-400: a checkbox "During Pause, GATE playback is applied". While a track is playing, a hot cue simply jumps.
   URL: https://cdn.inmusicbrands.com/engine/43/Engine%20DJ%20-%20User%20Guide%20-%20v4.3.0.pdf (extract)
   Quote: "Select Momentary for the hot cue to play only while the pad is held, or select Trigger for the hot cue to continue playing once the pad is pressed."
   AlphaTheta (same URL as finding 2): "Press and hold a Performance Pad that has a Hot Cue set during pause. Playback starts from the Hot Cue point and continues until you release the Performance Pad."
   Rekordbox: https://www.reddit.com/r/Rekordbox/comments/opiubl/how_to_i_play_hotcues_only_while_holding_buttons/ -- 'check or uncheck "During Pause, GATE playback is applied"'. The asker says Serato already plays hot cues only while pressed.
   HOW SURE: READ ON THE MAKER'S PAGE (Denon: extract; AlphaTheta: read whole page); Rekordbox setting: ONE FORUM VOICE (3 replies agree on a setting that exists, wording differs).

4. Loop roll, Slip, Censor/reverse and XY-FX are held-only on every maker I read; release restores what would have been playing. No latch variant is offered for the roll itself.
   URL: https://support.enginedj.com/support/solutions/articles/69000882261-denon-dj-prime-4-g2-performance-pad-modes
   Quote: "XY FX are triggered momentarily, and deactivate after lifting your finger off the pad." Roll: "When you release the pad, the track will resume normal playback".
   Also https://downloads.support.alphatheta.com/... (finding 2): Slip Loop "continues until you release the Performance Pad. ... Normal playback starts from the exact point the track would have reached by then." And https://www.ora-dj.com/docs/performing/performance-pads: "Hold a PAD FX pad to apply its effect to the selected deck. Release it to stop that pad."
   HOW SURE: READ ON THE MAKER'S PAGE (Denon, AlphaTheta); Ora DJ is a small third-party app's doc.

5. Pad FX in the Serato/Pioneer DDJ family is mixed on one pad bank: some pads are held, others toggle. That is the maker deciding per pad (by effect type), not the user.
   URL: https://support.serato.com/hc/en-us/articles/11087620792591-AlphaTheta-DDJ-FLX2-Quickstart-Guide
   Quote: "Pads 1-4 can be pressed and held to apply FX, whereas pressing pads 5-8 toggle FX that are applied until the pad is pressed again (toggle)."
   Held ones: Echo, Flanger, Reverb, Repeater. Toggled ones: Echo Out, Backspin, Braker, Rollout (finishing, one-way effects).
   HOW SURE: READ ON THE MAKER'S PAGE (Serato support, FLX2 and same text for DDJ-RB).

6. The same physical control is sold in both senses: a spring-loaded or touch control that is momentary, with a combination to latch it. DJM Beat FX: the ON/OFF button latches; the X-PAD is momentary; touch X-PAD then press ON/OFF to keep the effect on. Denon Prime main-FX toggle switches: one way latches, the other holds only while pulled.
   URL: https://novelty.fr/wp-content/uploads/downloaded/downloads/materiel_manuels/pioneer_djm-900nxs2_manual_EN.pdf (extract)
   Quote: "When you release your finger from the [X-PAD], the effect turns off. ... To keep the effect on ... while touching the [X-PAD] press the [ON/OFF] button".
   Denon (extract, https://cdn.inmusicbrands.com/denondj/Prime4G2/PRIME%204%20G2%20-%20User%20Guide%20-%20v5.1.0%201.pdf): "Pull and hold this toggle toward ... to activate the selected ... momentarily" and the other direction "latch to that position".
   HOW SURE: READ ON THE MAKER'S PAGE (both extract only).

7. DJs ask for latch on held-only Pad FX, and work around it. A 2018 Pioneer forum post asks for "press a FX-Pad with shift to auto-hold it until the pad is pressed again"; a 2024 Rekordbox thread asks the same; a DJ school article publishes the workaround (switch pad mode while holding).
   URL: https://forums.pioneerdj.com/hc/en-us/community/posts/360001414623-Pad-FX-Hold-option
   Quote: "Even my DDJ-400 has this workaround... I can't understand why pioneer don't solve this with a firmware update." (Pedro Gil, 2022)
   Also https://www.reddit.com/r/Rekordbox/comments/199rwe1/changing_pad_fx_from_momentary_to_toggle_ddj400/ -- "instead of having to hold it down continuously" (Jan 2024).
   Also https://wearecrossfader.co.uk/blog/ddj-400-hidden-features/ (11 May 2022) -- "they require your fingers to be held in place ... Lock the Pad FX in place and allow your fingers to get groovy on other tasks."
   Counter-voice in the thread: "The idea behind the PAD FX is that they're beat-linked FX; a hold would take them out of that scope." (Pulse, 2018; not shown to be Pioneer staff).
   HOW SURE: ONE FORUM VOICE (Pioneer thread, 4 comments); READ FROM A PERFORMER (Crossfader, DJ training outfit); the Reddit thread is 3 replies.

8. A DJ-software pad mapper that offers a toggle/hold choice per pad exists, and a user was still caught by a latched effect he did not know was on (no light).
   URL: https://community.algoriddim.com/t/instant-fx-on-reloop-mixon-8/29575 (Oct 2025)
   Quote: "I keep getting caught out by Echo Out coming on with a push of the button and not releasing, but as no light comes on there is no way to know"
   He says he tried "swappign between toggle and hold" in djay's pad assignment.
   HOW SURE: ONE FORUM VOICE.

9. Ableton Live: Gate is one of four per-clip launch modes, defined as hold-to-play, and the factory default is Trigger.
   URL: https://www.ableton.com/en/live-manual/12/launching-clips/
   Quote: "Gate: down starts the clip; up stops the clip." and "Toggle: down starts the clip; up is ignored. The clip will stop on the next down."
   Default: https://www.soundonsound.com/techniques/creative-clip-launching-ableton-live (2007) -- "the factory default is Trigger"; Gate "will play only while its launch key is held down"; the other modes "are best used with MIDI triggers or keyboard assignments".
   HOW SURE: READ ON THE MAKER'S PAGE (Ableton manual; SoS is a magazine tutorial, old).

10. How performers use Gate in Live: short held hits over a running set. One producer calls it the live secret weapon for "drum fills, FX risers, and vocal stabs"; a finger-drummer demo for Ableton uses gate and trigger side by side; Push's one-shot Simpler has a Trigger/Gate switch for the same reason.
   URL: https://blog.imseankim.com/ableton-live-12-advanced-clip-launching-techniques-electronic-music/ (20 Oct 2025)
   Quote: "hold a pad for a two-beat fill, release, and your main pattern resumes without you touching anything else."
   Also https://www.ableton.com/en/blog/inside-the-track-bmaul/ (2017): "Brian demonstrates the different uses of gate and trigger modes for sample playback." (the why is in a video I could not read).
   Push 2 manual (https://www.ableton.com/en/manual/using-push-2/, extract): "With Gate enabled, the sample will begin fading out as soon as you release the pad."
   HOW SURE: ONE FORUM VOICE for the 2025 blog (a self-described 28-year producer; reads like a how-to, not an interview); READ FROM A PERFORMER for BMaul (no why quoted); READ ON THE MAKER'S PAGE for Push.

11. Traktor Remix Decks / Kontrol F1: every sample cell is Latch or Gate; Latch is the default; Gate is used on purpose for stabs and hits; the F1 has a "Gate Override" that makes ALL cells gate for as long as you stay in that sub-mode without changing the saved setting.
   URL: https://docs.native-instruments.com/ni-tech-manuals/traktor-pro-manual/en/advanced-usage-tutorials
   Quote: "Latch mode is activated by default." and "When deactivated (Gate mode), a Sample will play only when the mouse button is held."
   F1 manual (extract, https://www.manualsdir.com/manuals/857048/native-instruments-traktor-kontrol-f1-dj-controller-for-remix-decks.html?page=42): "very useful for just throwing in short, one-shot type effects, shouts, or hits. In gated mode, you can actually 'play' the pads as you would with a hardware groovebox."
   HOW SURE: READ ON THE MAKER'S PAGE (Traktor manual read; F1 text extract).

12. Maschine: Note Repeat is hold-only (hold a pad, it repeats, release = stops) with TWO opt-in helpers: LOCK (keep the mode on) and HOLD, which means LATCH. Mode buttons (Solo, Pattern ...) are also held by default and can be "pinned".
   URL: https://docs.native-instruments.com/ni-tech-manuals/maschine-mk3-manual/en/playing-on-the-controller (page opened; the Note Repeat text is from the extract of the same page)
   Quote: "HOLD (Button 3) | Allows the repeated notes to be latched. This means that you can release the pads and the repeated notes will continue to play."
   Also https://docs.native-instruments.com/ni-tech-manuals/maschine-mk3-manual/en/quick-reference -- "By default, each of these buttons needs to be held" (mode buttons; extract).
   HOW SURE: READ ON THE MAKER'S PAGE (Maschine). Akai MPC: not found (see NOT FOUND).

13. Novation names the two behaviours in its own guide and tells performers when to use which: hold for short visits, latch for long stays. Custom-mode pads can be toggle, trigger or momentary per pad.
   URL: https://fael-downloads-prod.focusrite.com/customer/prod/s3fs-public/novation/downloads/10594/launchpad-pro-user-guide-en.pdf
   Quote: "Momentary behaviour is great when performing and time is limited."; Custom mode (MK3 page, https://userguides.novationmusic.com/hc/en-gb/articles/25494530115346-Launchpad-Pro-MK3-interface): "Momentary behaviour will turn on a note when the pad is pressed and release the note when un-pressed."
   Note: the 'momentary' there is mostly about VIEWS (hold Volume to see faders, let go to go back), not about sound.
   HOW SURE: READ ON THE MAKER'S PAGE (Launchpad Pro PDF read; MK3 custom-mode sentence from extract).

14. Resolume (VJ side but the one program that DJs with visuals use): hold is called Piano, exists per clip ("Trigger Style": Normal, Toggle, Piano) and per control shortcut, never default. Their own use: strobe/flash while held; a release that restores the previous state. Constraint stated by Resolume's staff poster: a clip is Piano or not, whatever sends the trigger.
   URL: https://resolume.com/support/en/clips -- "When released, it stops playing. With this style you have to physically hold the key, mouse or shortcut down to keep the clip playing."
   URL: https://resolume.com/support/en/keyboard-shortcuts -- "you can call this option Momentary or Flash or whatever floats your boat." and "quickly set the clip to random playback during an especially hectic part in the music, and then release the button".
   URL: https://resolume.com/forum/viewtopic.php?t=12519 (Joris, 2015) -- "Piano mode is not available on clip triggers. For clip triggers, you can set the clip itself to piano via Clip > Triggerstyle > Piano".
   VJ practice: https://zunayed.com/strobe-with-emotion/ (2022) -- "Now we can hold this button to strobe and ctrol the frequency!"
   HOW SURE: READ ON THE MAKER'S PAGE (Resolume); READ FROM A PERFORMER (a working VJ tutorial, one VJ).

15. DJs and DJ/producers who run their own visuals do not describe hold pads. They sequence or automate visuals from the music: Max Cooper sends "MIDI triggers into Ableton to trigger scenes and effects" which become OSC to Resolume; Noisia sequence "triggering video files and adjusting the opacity of a clip"; an electronic live act (Promising/Youngster) triggers visuals from his sequencer and says "I don't have the capacity to manually tweak visuals live". EREZ (Push + APC40) stops clips for a drop effect.
   URLs: https://www.musicradar.com/artists/every-show-is-designed-for-the-space-so-every-show-is-different-max-cooper-explains-the-workings-of-his-unique-3d-av-live-shows (Nov 2024); https://www.resolume.com/blog/14308; https://www.arkestra.app/articles/promising-youngster-on-his-av-set; https://musictech.com/features/interviews/erez-interview-future-forms-madi-tanguay/ (Jul 2026)
   HOW SURE: READ FROM A PERFORMER (extracts only; I did not open the pages whole). The absence of a held pad is INFERRED from what they chose to describe.

## WHAT IT IS USED FOR ON STAGE
- Preview/hit-and-return: CUE, hot-cue gate, Slip: audition or drop a piece and return to the running track (1, 2, 3, 4).
- Rolls, stutters and cuts that must end exactly when the finger ends: loop roll, Trans/cut, XY-FX, reverse-while-held (4, 5, 6).
- Short FX over a running mix, one hand busy: Echo/Flanger/Reverb pads, Beat FX X-PAD (5, 6).
- Drum fills, risers, vocal stabs, finger-drummed hits from clips or samples: Gate in Live, Gate in Traktor (9, 10, 11).
- Temporary views and modes: hold Volume, Solo, Note Repeat button, then back (12, 13).
- Strobe/flash/"hectic part" on visuals, released to restore: Resolume Piano (14).
- Latching the hold when the hands are needed elsewhere: Cue+Play, ▶ in Gate Cue, X-PAD+ON, Maschine HOLD/LOCK, Pad-FX mode-switch trick (2, 6, 7, 12).

## AGAINST IT
- Held is rarely the user's per-pad setting. In nearly every program the maker fixes it per FUNCTION (roll, XY-FX, CUE); pads in one bank are mixed by the maker (5). Evidence of a per-pad user switch on DJ gear: only Traktor cells, Ableton clips, djay pad assignment, Resolume clip/shortcut (8, 9, 11, 14).
- The default is the press/latch. Ableton default Trigger; Traktor default Latch; Resolume default Normal (9, 11, 14). One blog says "Most producers stick with Trigger mode and never look back." (finding 10 URL; one voice).
- Held-only is a complaint. 2018-2024 forum voices want latch on Pad FX; articles publish workarounds (7). Hands are a finite resource.
- A hold setting costs surprise. A 2011 Ableton user thought his clips were broken: "If I hold the mouse down it will play but when I let it go it stops..." (https://forum.ableton.com/viewtopic.php?t=163305). A DJ on djay was caught by a latched effect that gave no light (8).
- Ableton Gate has gaps: the 2018 DOJO post says Gate "doesn't take into account the legato mode" and needs sidechain-gate or mapping tricks to hold-play a running loop (https://steolepanda.com/post/172871023568/dojo-8-gate-legato-in-ableton-live; extract only, ONE FORUM VOICE). A 2011 forum user cannot do a momentary mute with a note-off because "In Live's midi automation, switches are always toggled with Note-On messages, Note-Off has no effect." (https://forum.ableton.com/viewtopic.php?t=168169; extract).
- No DJ/producer source describes holding a pad for visuals (15). Searches run for this: "DJ triggers visuals live from pads Resolume Launchpad flash hold"; "DJ producers use Launchpad or Push to trigger visuals live set own VJ visuals hold pad strobe interview"; the Resolume + DJ-sync pages. Found only VJs (14) and automation (15).
- Hold is not what the hold button does. In several programs "Hold" means LATCH (see next section). A DJ reading "hold" on a pad may expect latch.

## HOW THE PROGRAMS NAME IT
- Ableton Live: Trigger / Gate / Toggle / Repeat (clip Launch Mode). Gate = hold.
- Resolume: Piano (hold, and its Invert); Normal; Toggle. Resolume itself: "you can call this option Momentary or Flash".
- Traktor: Gate vs Latch (sample Trigger Type); Gate Override.
- Denon/Engine DJ: Momentary vs Trigger (hot cue, pad); "triggered momentarily" (XY FX); "latch" (main-FX switch).
- AlphaTheta/Pioneer: Gate Cue (hot cue), Slip Loop, "Hold" = LATCH (Pad FX Hold option forum title); "Cue Point Sampler" (CDJ CUE hold).
- Serato: "pressed and held" vs "toggle" (Pad FX); pad assignment in djay: "toggle" vs "hold".
- Rekordbox: "GATE playback" (hot cue during pause).
- Novation: Latch vs Momentary (mode buttons); toggles, triggers or momentary switches (custom pads).
- Maschine: Note Repeat; HOLD (= latch) and LOCK (= keep mode on); "pin".
- Sequential/Arp synths and Maschine use "Hold" for latch; in Resolume/Ableton "hold" is the action and Gate/Piano the mode.

## NOT FOUND
- Serato's own page on whether Pad FX or hot-cue pads have a user hold/latch setting: found only per-controller quickstarts (5). Pioneer's own reason for no Pad FX latch: only a forum guess (7).
- Akai MPC: no manual text on note-repeat hold/latch read.
- Allen & Heath Xone:96 and Rane: no hold-vs-latch FX evidence; the Xone:96 manual (https://support.allen-heath.com/hc/en-gb/articles/24944524347665-Xone-96-Cue-system-configuration, extract) only offers a CUE "auto-cancel vs latching" choice, which is a different thing.
- Ableton Push manual line on a per-pad "momentary" button was read as an extract only.
- Any DJ/producer using a held pad to drive visuals; any survey of how many DJs use Gate or Momentary.
- Numbers on how often each mode is used; everything here is manuals plus 1-4 voices per claim.
- The video part of BMaul's gate vs trigger explanation (video).
- Firsthand: I could not read the Pioneer CDJ-3000 manual, Denon PDFs and the Launchpad Pro PDF whole through WebFetch (binary/size); those quotes are from search extracts of the same files and are marked.
