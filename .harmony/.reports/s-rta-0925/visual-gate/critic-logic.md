# Interaction-Logic Critic — Record panel state walk (s-rta-0925 visual gate)

Source of truth: `src/ui/RecordPanelModel.h` (`deriveRecordPanelView`, lines 128-281).
Evidence: PNGs in `.harmony/.reports/s-rta-0925/visual-gate/`.

## State walk (D1 -> D8, plus A/B/E master link)

| Shot | Recorder state | Model predicate | Screenshot match |
|---|---|---|---|
| D1 idle, nothing | `idle=true, loaded=false` | Record="Record Take"(Neutral,enabled); Play=disabled("Load a take first"); Load enabled; Reveal disabled(no takeFolder); Repair disabled; Record-audio checkbox enabled+checked; Play-with-audio disabled+unchecked; status="Ready. No take loaded." (Neutral/white) | MATCH — `D1_record_idle_nothing.png` |
| D2 recording | `recording=true, armed=false, overdub=false` | Record="Stop Recording"(Recording/red,enabled); Play disabled; Load disabled; Reveal enabled (revealFolder=takeFolder); Repair disabled; Record-audio checkbox disabled (dimmer border); Name field disabled; status="Recording 0:09 from audio file · 1 lane · 1 move" (red) | MATCH — `D2_record_recording.png`. Status string byte-for-byte matches `"Recording " + t + " from audio file" + recordingCounts(s)` |
| D3 idle, saved | back to idle, `takeFolder` non-empty, `loadedTakeFolder` still empty | status="Ready. Last take: shot_rec" (Neutral); Play still disabled (`!loaded`); notice="Saved: shot_rec" (cyan, noticeLive) | MATCH — `D3_record_idle_saved.png` |
| D4 refused notice | same idle/not-loaded state as D3 (an out-of-band REST refusal, no Status field changed) | notice replaced by refusal text; every button/checkbox state identical to D3 since no `noticeKeyOf` field changed | MATCH — `D4_record_refused_notice.png`. Buttons unchanged from D3 as expected (the refusal is a notice-only event, not a state transition) |
| D5 loaded | `loaded=true, audioReady=true, idle=true` | Play enabled; Play-with-audio now enabled+checked (`idle&&loaded&&audioReady`); Reveal enabled (revealFolder=loadedTakeFolder); Repair disabled (audioStatus=Resolved, not Incomplete); status="Loaded: shot_rec — 0:17 · 1 lane · Audio: ready"; notice cleared (situation changed -> key mismatch) | MATCH — `D5_record_loaded_notice_gone.png`. Status string matches the `"Loaded: "+takeName+dash()+formatClock(loadedDuration)+dot()+count(loadedLanes)+dot()+"Audio: "+audioWord(...)` template exactly |
| D6 playing (withAudio) | `playing=true, playMode="withAudio"` -> `withAudio=true` | Record relabels "Record Over"(Neutral,`recordSendsOverdub=true`); Play="Stop Playback"(Playing/green,enabled, `canStop=!(recording&&overdub)=true`); Load disabled (`idle=false`); Record-audio checkbox now disabled (`withAudio` locks it) — confirmed dimmer border pixel sample vs D5's bright border; Play-with-audio checkbox disabled but still checked (`playing?withAudio:...`); status=playingReadout green; warning="Replaying with the take's audio..." (yellow); notice="Record Over starts a new take..." (cyan, non-noticeLive branch) | MATCH — `D6_record_playing.png` |
| D7 overdub-recording during playback | `recording=true, overdub=true, playing=true` | Record="Stop Recording"(Recording/red) — recording branch takes priority over the withAudio relabel, correct per code order (`if(recording) ... else if(withAudio) ...`); Play="Stop Playback" but **disabled** now (`canStop=!(recording&&overdub)=false`) — pixel-verified: button RGB (49,106,67) vs D6's enabled (105,227,130), i.e. dimmed ~2x, matching JUCE's disabled-alpha convention, not a stale enabled green; status="Recording over shot_rec 0:09 · 0 lanes · 0 moves — Playing 0:17/0:17 · unresolved: 0" matching the overdub+playing template with dash() join; warning unchanged (withAudio still true) | MATCH — `D7_record_over_during_playback.png` |
| D8 stop during overdub | `recording=false` again, `playing=true` still (per F6: "Stop Recording saves the take but leaves the replay running") | Record reverts to "Record Over"(Neutral) — no stale "Stop Recording"/red left over; Play reverts to enabled bright green (`canStop` now true since `recording=false`) — pixel-verified bright green restored; status/warning/notice all revert to the D6 playing-state text | MATCH — `D8_record_stop_during_over.png`. This is the transition most likely to leave a stale disabled/colored element (recording->not-recording while playing stays true) and it did not |

No stale element was found across any D1->D8 transition: every button/checkbox/tooltip-relevant state (enabled flag, tone, label) recomputed correctly off `RecorderHost::Status`, matching `deriveRecordPanelView`'s pure-function contract (no shadow booleans per the file's own header comment).

## Notice lifecycle (D3 -> D4 -> D5)

`RecordPanelNoticeKey` (recording, playing, overdub, playMode, loadedTakeFolder) governs notice survival. D3's "Saved: shot_rec" and D4's refusal notice share an identical key (no tracked field changed between them — the refusal is a same-state REST rejection). D5's Load changes `loadedTakeFolder`, which IS in the key, so the notice correctly disappears the instant the situation changes, per the file's own invariant comment (lines 15-27). Matches observed behavior exactly.

## Master link (E)

`A_topbar_master.png` / `B_composition_inspector.png` show baseline 1.00 on both the TopBar fader and the Composition Inspector "Master" knob/slider. After the OSC `/audiodna/master 0.3` send, `E_A_topbar_master_0p3.png` and `E_B_composition_master_0p3.png` both show 0.30 (fader thumb ~35% travel, "Master 0.30" in the inspector) — pixel-consistent with the shooter log's REST readback (`masterLevel`/`masterOpacity` = 0.300000011920929 both). Two independent UI surfaces (`TopBar` fader, `CompositionInspector` Master knob) agree, confirming the single underlying value the CLAUDE.md changelog describes ("TopBar fader = masterOpacity, Video Opacity twin removed"). `B_composition_inspector.png` also confirms no separate Video/Opacity section exists anywhere in the panel (scrolled to bottom, "Output Settings" is the last section) — no orphaned duplicate control to go stale against the master value.

## MUST / SHOULD / NICE

**MUST** (blocking): none found.

**SHOULD**:
1. `RecordPanelModel.h`'s warning-line priority chain (`recording&&lastError` > `armed&&overtime` > `withAudio` > `recording&&humanRefused` > `playing&&continuousUnavailable`) is only exercised at one branch (`withAudio`, D6/D7/D8) across the captured shots — the other four warning branches (lastError, armed-timeout, humanRefused, continuousUnavailable) have no screenshot evidence either way. Not a defect in what was captured, but the state walk cannot vouch for those four rows from this evidence set alone. (`src/ui/RecordPanelModel.h:257-266`)

**NICE**:
1. D7's disabled "Stop Playback" is distinguishable from D6's enabled one only by a ~2x brightness dim (pixel-verified: (105,227,130) vs (49,106,67)) with identical green hue — visually subtle at a glance; a disabled-state cue independent of hue (e.g. desaturation) would make the D6->D7 transition easier to eyeball live, though the model/data is correct and this is a rendering-affordance nit, not a logic bug.

## VERDICT=PASS

No MUST items.
