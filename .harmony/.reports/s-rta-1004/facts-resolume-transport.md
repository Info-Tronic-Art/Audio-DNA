# Resolume clip transport: fact sheet (researcher, 2026-10-04; read-only; firecrawl scrape errored once -> exa + WebFetch used; forum pages 403 to WebFetch, read through exa highlights)

Labels: DOCUMENTED = quote on an official resolume.com support page; STAFF = Resolume staff forum post (identified by forum rank/signature); THIRD-PARTY = other; NOT DOCUMENTED = looked, found nothing.
Versions: official page https://resolume.com/support/video (the same text is at /support/en/7/video and /support/en/7.18/video; the page's version tabs read v7.21, v7.18, v7, v6 [WebFetch of the 7.18 page]). The v6 page (/support/en/6/video) is older: it has an "R" button instead of a Random play mode. Quotes below are from the v7 text unless marked.

## 1 QUESTIONS ANSWERED

### Q1 Transport mode menu (Boris's screenshot 8 shows: Timeline, BPM Sync, SMPTE 1, SMPTE 2, Denon DJ, Pioneer DJ)
- Timeline: manual speed (pitch) in seconds. BPM Sync: speed follows the global BPM. SMPTE 1/2: playhead follows an external timecode input. Denon DJ / Pioneer DJ: playhead follows a track playing on a linked DJ player. SMPTE, Denon, Pioneer are Arena-only and unavailable on clips with an audio track.
- DOCUMENTED (v7.18/7.21 era) https://resolume.com/support/video : "On Arena, videos can also be set to SMPTE or Denon. Those playmodes are useful for syncing to a DJ set or show moment."
- DOCUMENTED (v7.13 and v6 tabs) https://resolume.com/article/46 : "You can select your clips to run on SMPTE 1 or SMPTE 2 via the Timeline dropdown." and "Note that SMPTE is not available on clips with an audio track."
- DOCUMENTED https://resolume.com/article/137 (Pioneer): "Resolume will perfectly follow the time of the audio track." and "You can also set a clip to Pioneer playback manually. Simply switch the playback mode to Pioneer and fill out the track name manually."
- DOCUMENTED https://www.resolume.com/support/en/sync-to-denon-players : "Resolume will now sync the playhead of the video to the audio. It will also trigger the video when the track is loaded in the player."
- Screenshots (observed by me, not text): SMPTE 1/2 show Channel, Offset, Duration and no buttons or menus; Denon/Pioneer show Title or File, Player, Fader, Offset, Duration and ONE small menu (an arrow icon), no direction buttons; Timeline and BPM Sync show the 3 buttons + two small menus. Pioneer/Denon/SMPTE are out of scope for Audio-DNA (external clocks).

### Q2 Timeline mode
- Speed: non-linear slider, finer between 0 and 2, running up "towards 10". DOCUMENTED https://resolume.com/support/video : "The Speed slider has a non-linear response. This is a fancy way of saying that you have more precision in the values between 0 and 2... When you go towards 10, it ramps up more quickly". The exact maximum (10?) is only implied; the step of minus / plus is NOT DOCUMENTED (see section 3).
- Duration: typing a length makes Resolume change the SPEED so the clip lasts that long; the Speed slider keeps its own range. DOCUMENTED: "Simply enter 8 as the duration and Resolume will do the math for you and adjust the playback speed so the clip will last exactly 8 seconds." and "Note that this doesn't affect the Speed slider!"
- What minus, plus, /2, x2 on Duration step by: NOT DOCUMENTED (the page only describes /2 and *2 for Beats: "use the *2 and /2 buttons to quickly multiply or divide the value by 2").

### Q3 BPM Sync mode (load-bearing)
- How speed follows BPM: DOCUMENTED: "Resolume will use the global BPM to control the speed of the clip." The clip spans a number of BEATS you set: "you will need to set the number of beats that the clip spans in the Transport section." Resolume shows BEATS, never bars (screenshots 2 and 7: "Beats 16").
- Beats row: minus/plus, /2, x2. DOCUMENTED: "you can click the number and change it, use the + and - buttons or use the *2 and /2 buttons to quickly multiply or divide the value by 2." Step of +/-: NOT DOCUMENTED (probably 1 beat; unverified). Fractions, minimum, maximum: NOT DOCUMENTED.
- WHICH BEATS RESOLUME PICKS BY ITSELF. The current official page says (DOCUMENTED, v6/v7): "By default, Resolume will guess the right amount of beats for you based on the length of the clip. It will guess to the nearest power of 2, so it will set the clip to 1, 2, 4, 8, 16, 32, 64, 128 etc beats for you."
  BUT three other sources disagree on the rounding, so the rule is NOT SETTLED:
  (a) STAFF (Joris, Team Resolume, Feb 2016, Resolume 5 era) https://resolume.com/forum/viewtopic.php?t=13032 : "When a clip is set to BPM Sync, it will 'snap' to the nearest multiple of 4 beats, while keeping the speed at 120 bpm as close as possible to the original playback speed." Example in the thread: 30 s clip -> 60 beats at 120 BPM -> 64.
  (b) forum reply, author not captured in the highlight (Dec 2019, Arena 7.0.x; reply style is staff) https://resolume.com/forum/viewtopic.php?t=19386 : "Clips set to BPM might not play at the original speed. We try to round the clip duration/speed to a near multiple of 8 or 16 beats, so it loops nicely to the music." Label: THIRD-PARTY-unverified-author.
  (c) DOCUMENTED release note (Resolume 4.x blog) https://resolume.com/blog/category/software?page=9 : "Calculate number of beats based on length of clip and default BPM of 120".
  So: the length -> beats guess uses 120 BPM (not the composition BPM) per (a) and (c); the rounding is power of 2 (docs) or multiple of 4 / 8 / 16 (staff 2016 / 2019). A clip that is not near a power of two (the docs' own example: a 12-step robot) gets "set it to play back in 12 beats, or trim it" : DOCUMENTED https://www.resolume.com/support/en/bpm : "you should either set it to play back in 12 beats, or trim it so it only takes 8 steps."
  Resolume changes the clip's SPEED to fit the chosen beats; the in/out points are not moved by the guess (no source says they are). Auto-detect: old R4 manual (THIRD-PARTY-old, official PDF of version 4) https://resolume.com/download/Resolume-4-Manual-English.pdf had a drop-down left of Beats "setting the BPM directly (BPM) or asking Resolume to detect the number of beats (Auto)". Not in Boris's v7 screenshots; v7 pages do not mention it.
- Speed row in BPM Sync: DOCUMENTED: "the BPM Sync Speed is quantised to multiples of 2. So you can ramp a clip from 0, 1/8, 1/4, 1/2, 1, 2, 4, 8 to 16 times as fast." Exact list = 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16 (the page says "multiples of 2"; the nine values are its own list). Screenshots show "1/4" and "1". What minus/plus do here: NOT DOCUMENTED (presumably one step along this list; unverified).
- The evenly spaced lines on the BPM Sync timeline: NOT DOCUMENTED in words. From screenshot 2 (Beats 16) I count about 15 interior ticks = 16 equal segments (INFERRED: one segment per beat; Boris should confirm by changing Beats and watching the tick count). A 2019 user also calls them "slice indicators" (THIRD-PARTY) https://resolume.com/forum/viewtopic.php?t=19347 .

### Q4 In and Out points
- Setting them: DOCUMENTED: "Grab and move the small blue pointers at its end to set the In and Out points of the clip." Shift+drag moves the range keeping its length: "hold down Shift while dragging the range."
- TIMELINE mode, out point moved in: the Duration follows. DOCUMENTED: "when shortening the clip via the In and Out points, the Duration will be adjusted to match the new length. Vice versa, when you change the Duration directly, the In and Out points will not be adjusted." So the clip plays the shorter part at the same speed (INFERRED from "adjusted to match"; Duration was defined as the time the clip plays). It does NOT look faster or slower.
- BPM SYNC mode, Beats unchanged, out point moved: for CLIPS NOT DOCUMENTED. The nearest official text is about animated PARAMETERS (not clips), v7: "In BPM Sync mode, changing the in & out points by default won't update the duration in beats." https://resolume.com/support/en/7/parameter-animation (and /7.12/). That matches Boris's picture (beats stay, marked part fitted into them) but speaks of parameter animation only. One user (Arena 7.0.5, Dec 2019) reports for a clip: "When I try to change the in point on the timeline control it beat stretches the clip." THIRD-PARTY https://resolume.com/forum/viewtopic.php?t=19347 . 
- Playhead dragged outside in..out, and what a click on the bar does: NOT DOCUMENTED by Resolume. One user (Aug 2023, v7 era) writes that you must drag the playhead and cannot click to move it: "being able to click anywhere on the timeline and have it play from there. ... this is an issue for all playheads in resolume everywhere." THIRD-PARTY https://resolume.com/forum/viewtopic.php?t=22647 (Resolume answer there concerns something else). Docs only say you "grab the blue pointer ... and slide it around": DOCUMENTED.

### Q5 The three buttons
- DOCUMENTED: "Use these buttons to set the clip to play forwards or backwards, or pause the clip."
- A PAUSED clip triggered again, and where a backwards clip starts when triggered: NOT DOCUMENTED. Nearest: trigger style "Normal": "When a clip is triggered, the clip starts. When triggered again, the clip restarts" (DOCUMENTED, https://www.resolume.com/index.php/support/en/7.3.1/clips , clip trigger style section; it does not mention pause or direction). A 2010 forum note (Resolume 3, THIRD-PARTY) says a ping-pong clip re-triggered starts in the direction where it stopped: https://resolume.com/forum/viewtopic.php?t=7611 (too old to rely on).

### Q6 Loop-style menu (v7 names in the docs)
All DOCUMENTED https://resolume.com/support/video :
- Loop (default): "just start the clip from the beginning and continue playing for ever and ever and ever."
- Ping Pong: "plays alternately forwards and backwards."
- Random: "jump to random points in your video"; reveals Interval ("how often the playhead will jump to a new random point") and Distance ("the range from which a random point will be picked"), "measured in seconds or in beats, depending on which mode you're in."
- Play Once and Eject: "'one shot' samples that you want to punch in at the right time" (clears the layer at the end; the user-written phrase "play once and clear" = this).
- Play Once and Hold: "it will hold the last frame of the clip when it's done playing."
Five options in v7. (v6 page has Loop, Ping Pong, Play Once and Eject, Play Once and Hold plus a separate R button.)

### Q7 Trigger-style menu (second small menu; the docs call it "Playmode Away")
- DOCUMENTED (v7): "These buttons decide what happens when a clip is triggered when you've been away from it for a bit." (1) "plays the clip from the start. This is the default"; (2) "pick-up": "starts the clip from wherever it was when it was last played"; (3) "relative pick-up": "will start the clip at the same relative position the previously played clip was at."
- The menu labels in the UI: the official page names them "from the start / pick-up / relative pick-up"; Resolume staff in forum threads call the settings "Continue" and "restart": thread t=22247 (Feb 2023; a user writes `changed the play behaviour to "continue" rather than "restart" in transport controls`; the reply signed "Software developer, Sound Engineer" [Zoltan's signature] says "keep the first clip on play once, and continue mode") https://resolume.com/forum/viewtopic.php?t=22247 -> STAFF-by-signature. Third-party: "Restart / Continue / Relative" https://vjacademy.info/resolume-layers (THIRD-PARTY). So the exact UI words are most likely Restart / Continue / Relative (INFERRED); Boris can read them off the menu.
- Which transport modes: the v7 page states no restriction. The Resolume 4 manual said "These buttons are only available in Timeline transport mode" (old official PDF, scribd copy https://www.scribd.com/document/329018263/Resolume-Manual) and an R4 release note says "BPM Sync should ignore "continue" setting from the timeline transport" (https://resolume.com/blog/category/software?page=9). Boris's v7 screenshots 1, 2 and 7 show the menu icon in BPM Sync too (observed), but whether "continue" is honoured in BPM Sync in v7: NOT DOCUMENTED for v7. Pioneer/Denon show one menu; SMPTE none (observed).

### Q8 Scrubbing a BPM-synced clip; drift, Resync, beat snap
- After scrubbing: DOCUMENTED, but Resolume 4/6 text (quickstart v6 https://resolume.com/support/en/6/quickstart-tutorial ; R4 manual PDF): "messing with the clip like this will mean that it is no longer synchronised with the BPM - the tempo will be right but it will be out of phase. You can resynchronise it by clicking the clip thumbnail again - it will start again at the start of the next bar." So: after a scrub the clip keeps the right TEMPO, stays at the dropped phase (does not re-align by itself), until the clip is triggered again. No v7 page repeats this: v7 behaviour NOT DOCUMENTED. Boris's answer 9 ("the app keeps nudging it back by itself") is therefore NOT how Resolume documents it.
- Resync button (global): DOCUMENTED https://www.resolume.com/support/en/bpm : "When you press 'Resync', everything in Resolume that is set to BPM Sync will jump back to the first beat of the first bar of the first phrase." Nudge Up/Down: "temporarily speed the tempo up or down while you have the button pressed". Drift: wrong BPM -> "you'll get further and further out of sync as the song goes on." Resync/Pause are disabled under Ableton Link (https://resolume.com/support/en/link).
- Beat Snap (clip launch quantise): DOCUMENTED (v7 clips page, https://www.resolume.com/support/en/7/clips ): "wait until the next beat, the next bar, in 2 bars, in 4 bars and so on, before it starts." Per clip or composition.
- The clip's playhead vs the global BPM phase is a separate thing for animated parameters: "disable BPM Phase Lock ... the parameter is still in sync with the BPM, but no longer in phase" (https://resolume.com/support/en/parameter-animation). Not stated for clips.

### Q9 Random and BeatLoopr
- Random: v7 = a loop-style option (Q6), with Interval and Distance; in BPM Sync "Interval and Distance are measured ... in beats". The v6/R4 "R button" in BPM Sync: "the clip will randomly jump to a random beat and continue playing from there." DOCUMENTED (v6 https://resolume.com/support/en/6/video).
- BeatLoopr: shown only in BPM Sync. DOCUMENTED: "This enables you to have Resolume automatically loop sections of the clip... just select one of the options - the clip will loop over the relevant number of beats. When you are done, just click the selected option again or the Off button." "With 'Catch Up' active, turning the Beat looper off will continue playback where the playhead would have been if you hadn't used the Beat looper. Otherwise, it will just continue wherever the playhead was."
- Loops from where the playhead is now, not stored: STAFF (Zoltan, Jun 2022) https://resolume.com/forum/viewtopic.php?t=21594 : "BeatLooper loops from the current playback position. It doesn't store loops start and end points permanently in the clip data."
- Jumping to a cue point turns it off: DOCUMENTED (R4 manual) "The Beatloopr ... will automatically turn off if you jump to a cue point." (old).
- The OPTION LIST (values): NOT DOCUMENTED on any page I could read (a forum thread titled "Beatlooper 1/16 not working", https://resolume.com/forum/viewtopic.php?p=94230, shows a "1/16" option exists; the page returned 403 and its date and body were not read).

### Q10 Time readout
- DOCUMENTED: "In the top right, you can see the clip's current time. Clicking on this number will switch to show you the remaining time." Release note (R4 era): "Show time remaining on clip timeline (click the time to switch between two modes)".
- Format: STAFF (Joris, 2016) https://resolume.com/forum/viewtopic.php?t=13032 : "there is a timecode counter which shows you the clip position in seconds:frames in BPM mode". So "00.14" in the screenshots is probably seconds.frames (INFERRED; 14 = a frame count, not hundredths).

## 2 TABLE: what Boris's answers assume vs what the sources say
| Boris's answer (backlog 2026-10-04) | Sources | Verdict |
|---|---|---|
| Clip length in BARS | Resolume shows BEATS ("Beats 16"), *2 and /2; the auto guess can give 1, 2 beats (less than a bar) | Differs: Resolume has no bars row; bars = beats/4 is our mapping |
| "figure out some kind of math ... look at how resolution does it ... nice in and out point and even amounts of bars" | Docs: nearest power of 2 of beats, from clip length (at default 120 BPM per R4 note + 2016 staff); staff 2016: multiple of 4; 2019 reply: multiple of 8/16. It changes SPEED; nothing says in/out points are chosen | Rule disputed; in/out untouched as far as documented |
| Out point pulled in, same bars -> plays slower; pulled out -> faster | Clips in BPM Sync: NOT DOCUMENTED. Parameters: beats not updated by in/out (7). Timeline: Duration follows in/out so speed is unchanged. One user: "beat stretches the clip" | Plausible, not proven for clips |
| Paused clip fired stays paused | NOT DOCUMENTED (Normal trigger style: "the clip restarts") | Unknown |
| BPM-synced clip stays on the beat; app nudges back after a drop | Docs (v4/v6): after a scrub "out of phase", re-sync by triggering again; no auto nudge documented | Differs |
| S fader steps halves/doubles, "smaller fractions to higher multiples" | 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16 in BPM Sync; Timeline Speed is a smooth non-linear slider up to about 10 | Matches |
| Playhead cannot leave the in..out (drag), click outside does nothing | Not documented; user: Resolume does not jump on a bar click | Unknown, partly consistent |

## 3 NOT DOCUMENTED (each a step for Boris in Arena, under a minute)
1. Beats chosen automatically: import clips of exactly 10 s and 12 s (or any), set to BPM Sync with the composition BPM at 120, note Beats; then set BPM to 90 and re-set a fresh copy to BPM Sync, note Beats. 120 BPM = 0.5 s/beat: 10 s = 20 beats, 12 s = 24 beats. Power of 2 gives 16/32; multiple of 4 gives 20/24; multiple of 8/16 gives 16/24.
2. Does the guess change in/out or only speed? After step 1 read the in/out markers (they stay at the ends if untouched).
3. Out point in BPM Sync: in BPM Sync with Beats 16, drag the out point to half and watch the video: speed doubles-slows or Beats changes?
4. Beats +/- step and fractions, min, max: click minus repeatedly from 16 and /2 repeatedly from 1; note values.
5. Speed minus/plus (Timeline and BPM Sync) and Duration minus/plus: click each once, note the value.
6. Timeline ticks: change Beats 16 -> 8 and count the lines.
7. Pause the clip, click its thumbnail again: stays paused? from where? Backwards: set backwards, retrigger, note where the playhead starts.
8. Scrub in BPM Sync, release: does it stay out of phase, snap to a beat, or keep nudging?
9. BeatLoopr: write down every button label.
10. Click the timeline bar outside the playhead: does it move? Drag the playhead beyond the in/out markers: can it?
11. Trigger menu: open it, read the 3 labels; switch to BPM Sync and see whether the entries are greyed.

## 4 SOURCES (all accessed 2026-10-04; official pages carry no date on the page)
- https://resolume.com/support/video (official, version tabs v7.21/7.18/7/6)
- https://www.resolume.com/support/en/7/video ; https://www.resolume.com/index.php/support/en/7.18/video ; https://resolume.com/support/en/6/video
- https://www.resolume.com/support/en/bpm ; https://www.resolume.com/support/en/7/clips ; https://www.resolume.com/index.php/support/en/7.3.1/clips ; https://resolume.com/support/en/link
- https://resolume.com/support/en/7/parameter-animation (also /7.12/, /6/)
- https://resolume.com/support/en/6/quickstart-tutorial ; https://resolume.com/download/Resolume-4-Manual-English.pdf (version 4, old)
- https://resolume.com/article/46 (SMPTE, v7.13/v6) ; https://resolume.com/article/137 (Pioneer) ; https://www.resolume.com/support/en/sync-to-denon-players
- https://resolume.com/blog/category/software?page=9 (R4 release notes)
- Forum: t=13032 (Joris, 2016-02-28), t=19386 (2019-12-15), t=19347 (2019-12-08), t=21594 (Zoltan 2022-06-27), t=22247 (2023-02), t=22647 (2023-08-08), t=7611 (2010), t=13632 (2016) ; forum pages returned HTTP 403 to WebFetch, read via exa highlights only.
- THIRD-PARTY: https://vjacademy.info/resolume-layers ; https://www.scribd.com/document/329018263/Resolume-Manual
- Boris's screenshots resolume-transport-1..8.png (read).

## VERIFICATION (independent source check)
Checked 2026-10-04 by a second researcher. Official pages fetched raw with curl and read as text (firecrawl scrape/search errored: fell back to curl + exa fetch + WebSearch). Forum pages 403 to curl, read in full through exa fetch (author and date visible). Marks: CONFIRMED = quote on the page and carries the claim; CITATION-OFF = page supports the claim but quote/attribution/URL not as written; WRONG = not there.

### Claim-by-claim
- Q1 video page "On Arena, videos can also be set to SMPTE or Denon. Those playmodes are useful for syncing to a DJ set or show moment." -> CONFIRMED (resolume.com/support/video; also /en/7/video and /7.18/video; tabs v7.21, v7.18 present on the page).
- Q1 article/46 "...SMPTE 1 or SMPTE 2 via the Timeline dropdown." + "SMPTE is not available on clips with an audio track." -> CONFIRMED.
- Q1 article/137 "Resolume will perfectly follow the time of the audio track." + "set a clip to Pioneer playback manually..." -> CONFIRMED.
- Q1 sync-to-denon-players "Resolume will now sync the playhead of the video to the audio. It will also trigger the video when the track is loaded in the player." -> CONFIRMED.
- Q1 summary line "SMPTE, Denon, Pioneer are Arena-only and unavailable on clips with an audio track" -> only partly sourced: "no audio track" is stated for SMPTE only; Arena-only is stated for SMPTE and Denon; nothing read says either for Pioneer. (unlabelled summary, not DOCUMENTED; treat as INFERRED for Pioneer/Denon audio-track limits.)
- Q2 Speed non-linear, "more precision in the values between 0 and 2", "towards 10, it ramps up more quickly" -> CONFIRMED.
- Q2 Duration "enter 8 ... adjust the playback speed so the clip will last exactly 8 seconds" + "this doesn't affect the Speed slider!" -> CONFIRMED.
- Q3 "Resolume will use the global BPM to control the speed of the clip." + "you will need to set the number of beats that the clip spans in the Transport section." -> CONFIRMED.
- Q3 "you can click the number and change it, use the + and - buttons or use the *2 and /2 buttons to quickly multiply or divide the value by 2." -> CONFIRMED.
- Q3 "By default, Resolume will guess the right amount of beats ... nearest power of 2 ... 1, 2, 4, 8, 16, 32, 64, 128 etc beats" -> CONFIRMED (v7 and v6 pages both carry it).
- Q3 "the BPM Sync Speed is quantised to multiples of 2. So you can ramp a clip from 0, 1/8, 1/4, 1/2, 1, 2, 4, 8 to 16 times as fast." -> CONFIRMED verbatim (v7 page; the word "quantised" is NOT on the v6 page). The sheet's reading "nine values" is the page's own list. Caveat: "multiples of 2" and a list that starts 0, 1/8 are not the same thing; the page is loose, so the list is the usable fact.
- Q3 bpm page "you should either set it to play back in 12 beats, or trim it so it only takes 8 steps." -> CONFIRMED.
- Q3 (c) release note "Calculate number of beats based on length of clip and default BPM of 120" -> CONFIRMED (in the Resolume 4.1.4 section of blog/category/software?page=9).
- Q3 R4 manual PDF "(BPM) ... asking Resolume to detect the number of beats (Auto)" -> CONFIRMED (read the PDF text).
- Q3 (a) Joris, Feb 2016, t=13032 "it will 'snap' to the nearest multiple of 4 beats ... 30 second clip ... 60 beats ... so it snaps to 64." -> CONFIRMED verbatim (author Joris, moderator/staff; page shows no Team Resolume badge for him).
- Q3 (b) t=19386 "We try to round the clip duration/speed to a near multiple of 8 or 16 beats..." -> CONFIRMED verbatim, and the sheet UNDER-labels it: the author is Zoltan, "Team Resolume" (post of Wed Dec 18, 2019). It is STAFF, not "THIRD-PARTY-unverified-author".
- Q4 "Grab and move the small blue pointers at its end to set the In and Out points" + "hold down Shift while dragging the range" -> CONFIRMED.
- Q4 "when shortening the clip via the In and Out points, the Duration will be adjusted to match the new length. Vice versa ... In and Out points will not be adjusted." -> CONFIRMED.
- Q4 parameter-animation (/7/ and the unversioned page) "In BPM Sync mode, changing the in & out points by default won't update the duration in beats." -> CONFIRMED (parameters only, as the sheet says).
- Q4 t=19347 user "it beat stretches the clip" and "slice indicators" -> CONFIRMED (user Arena 7.0.5, Dec 8 2019; Zoltan replied "ticket made" about a different point, the Sync-Mode default).
- Q4 t=22647 "being able to click anywhere on the timeline ... this is an issue for all playheads in resolume everywhere" -> CONFIRMED, but the user writes it about the playhead in ALLEY (the "drag the playhead arrow at the bottom"), so it is weaker evidence for the clip transport than the sheet implies.
- Q5 "Use these buttons to set the clip to play forwards or backwards, or pause the clip." -> CONFIRMED.
- Q5 7.3.1 clips "When a clip is triggered, the clip starts. When triggered again, the clip restarts" -> CONFIRMED (Normal trigger style).
- Q5 t=7611 (2010) ping-pong retrigger starts in the direction it stopped -> CONFIRMED (user report; Joris: "bug versus undocumented feature").
- Q6 Loop, Ping Pong, Random (+Interval, Distance, "seconds or in beats"), Play Once and Eject ("'one shot' samples"), Play Once and Hold ("hold the last frame") -> CONFIRMED. Small overreach: the sheet's gloss "(clears the layer at the end)" for Eject is not on the page (page says only "eject"); the R-era and third-party page says "clear".
- Q7 Playmode Away: "plays the clip from the start. This is the default", "pick-up ... starts the clip from wherever it was when it was last played", "relative pick-up ... same relative position the previously played clip was at" -> CONFIRMED. (Also on the page: the "relative pick-up" is recommended as a SMPTE backup.)
- Q7 t=22247 (Feb 28 2023, Resolume 7.13.2): user wrote "continue" rather than "restart"; Zoltan (Team Resolume) "keep the first clip on play once, and continue mode" -> CONFIRMED.
- Q7 vjacademy "Restart / Continue / Relative" -> CONFIRMED as third-party text.
- Q7 R4 manual "These buttons are only available in Timeline transport mode." -> CONFIRMED in the official PDF (the sheet's scribd URL was not fetched; the PDF cited elsewhere in the sheet carries it). R4 note "BPM Sync should ignore "continue" setting from the timeline transport" -> CONFIRMED (blog page 9, two entries).
- Q8 v6 quickstart + R4 PDF "no longer synchronised with the BPM - the tempo will be right but it will be out of phase. You can resynchronise it by clicking the clip thumbnail again - it will start again at the start of the next bar." -> CONFIRMED (the page has "Note that messing with the clip like this will mean..." ).
- Q8 bpm page "When you press 'Resync', everything in Resolume that is set to BPM Sync will jump back to the first beat of the first bar of the first phrase." + Nudge sentence + "further and further out of sync" -> CONFIRMED.
- Q8 link page "the Resync and Pause buttons for the BPM are disabled" -> CONFIRMED.
- Q8 7/clips Beat Snap "wait until the next beat, the next bar, in 2 bars, in 4 bars and so on, before it starts" -> CONFIRMED.
- Q8 parameter-animation "disable BPM Phase Lock ... still in sync with the BPM, but no longer in phase" -> CONFIRMED on the unversioned page (v7.23 text); it is NOT on the older /en/7/ page.
- Q9 v6 video "the clip will randomly jump to a random beat and continue playing from there" -> CONFIRMED.
- Q9 Beat Looper "automatically loop sections of the clip", "click the selected option again or the Off button", Catch Up sentence -> CONFIRMED.
- Q9 t=21594 Zoltan (Jun 27 2022) "BeatLooper loops from the current playback position. It doesn't store loops start and end points permanently in the clip data." -> CONFIRMED.
- Q9 R4 "The Beatloopr ... will automatically turn off if you jump to a cue point." -> CONFIRMED (official PDF).
- Q10 "In the top right, you can see the clip's current time. Clicking on this number will switch to show you the remaining time." + R4 note "Show time remaining on clip timeline (click the time to switch between two modes)" -> CONFIRMED.
- Q10 "STAFF (Joris, 2016) ... a timecode counter which shows you the clip position in seconds:frames in BPM mode" -> CITATION-OFF: the words are in t=13032 but written by ZOLTAN (Team Resolume), not Joris. Claim still STAFF-backed.
- Sources list: scribd link for the Resolume 4 manual -> CITATION-OFF (URL not read by me; the same sentence is in the official PDF).

### Refutation attempts on the three load-bearing answers
1. FIRST BEATS NUMBER. The sheet says "NOT SETTLED" (power of 2 vs multiple of 4 / 8 / 16). I could not refute the docs line, and I found a second staff statement that SIDES with it: Joris, Dec 17 2016, https://resolume.com/forum/viewtopic.php?t=14019 : "the default beat value. This is the amount of beats so that clip play back at 120 BPM matches the original speed as much as possible, into exponents of 2 beats (1 beat, 2 beats, 4 beats, 8 beats, 16 beats etc)." STAFF, later than his Feb 2016 post. Also the Feb 2016 "multiple of 4" rule is contradicted by its own example: 60 beats IS a multiple of 4, yet he says it "snaps to 64"; Zoltan in the same thread explains 64 as "a multiple of 4 and 16". So the staff example fits power of 2 (and "multiple of 16"), not "nearest multiple of 4". CONCLUSION: "power of 2, computed at 120 BPM" has the docs (v6 and v7) + Joris Dec 2016 + the 64 example; "multiple of 4" is the outlier and the sheet's section-3 prediction "multiple of 4 gives 20/24" rests on a rule its own source contradicts. Still open (no source): HOW "nearest" is measured for a clip between two powers (linear or log: 24 beats -> 16 or 32?) and clips under 0.5 s. So the sheet is not wrong, but it hedges too much on the rule and too little on the open point.
2. BPM-SYNC SPEED STEPS. Not refuted. The only source is the official video page and it is verbatim: 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16. A WebSearch summary and forum titles about "1/2, 1/4 and even 1/8 BPM Sync Parameter" concern parameter animation, not clip Speed, and were not accepted (third-party). The step of the minus / plus buttons stays NOT DOCUMENTED. Caveat: "multiples of 2" is loose wording, so do not read it as "x2 only".
3. A TRIGGERED PAUSED CLIP. The sheet says NOT DOCUMENTED / "Unknown" (section 1 Q5 and the table row "Paused clip fired stays paused"). REFUTED as a gap: two STAFF posts say it stays paused.
   - Zoltan (Team Resolume), Resolume Avenue 7.9, Feb 2022, https://resolume.com/forum/viewtopic.php?t=21284 : "If you pause a clip, and trigger it again, it will still be paused at the same position."
   - Zoltan (Team Resolume), Resolume 6.1.1, Jan/Feb 2019, https://resolume.com/forum/viewtopic.php?t=18149 : "Auto pilot won't change your clip's settings, so if you had paused a clip, that will be launched by the auto pilot and it will stay paused."
   - Also official, https://www.resolume.com/support/en/7/clips (Fader Start tip): with "pick-up" "the clip will then remain paused where it was the moment you fade down the layer".
   Caveats: the two staff posts do not say which Playmode Away setting was on (screenshots attached, not readable), and the 7.3.1 page says Normal "restarts" without mentioning pause; a forum quote is not a manual. So: Boris's answer ("stays paused") is SUPPORTED by staff; the sheet's "Unknown" should read "STAFF: stays paused, same position (v7.9)". Where a PAUSED clip sits when triggered under the default from-the-start setting is not stated (the staff words are "same position").

### Other gaps found while checking
- No contradiction found with the Q8 claim that Resolume documents the scrub as out of phase until retrigger; it is v4/v6 text only, as the sheet says.
- Not checked (could not read): forum p=94230 (Beatlooper 1/16, 403); the sheet marks it unread.

### Verdict
SOUND_WITH_CORRECTIONS. Every DOCUMENTED quote I checked is on its page and carries the claim; no fabricated quote found. Corrections: (1) Q5/table: paused clip has a STAFF answer (stays paused at the same position); (2) Q3: t=19386 is STAFF (Zoltan), and the "multiple of 4" rule is contradicted by Joris's own 60->64 example while Joris (Dec 2016) and the docs say powers of 2 at 120 BPM; (3) Q10 timecode line is Zoltan's, not Joris's; (4) t=22647 playhead remark is about Alley; (5) scribd URL not read, the PDF carries the quote; (6) "clears the layer" gloss for Play Once and Eject is not on the page.
