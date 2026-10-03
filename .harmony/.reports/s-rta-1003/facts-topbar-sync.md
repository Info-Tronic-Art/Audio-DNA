# FACTS: top bar (Gain, where Sync goes) and how the Sync setting is stored (s-rta-1003)

Read-only recon. Source = RECON = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (a7491d4, lane/bf9b).
Sync-dial lane = BF2 = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2 (lane/bf2, 4a1f240). Nothing was built, run or edited.
Plan/ruling files cited as B:plan-bf2.md / B:ruling-bf2.md are really /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b/plan-bf2.md and ruling-bf2.md.
Labels: VERIFIED (read; file:line), INFERRED (reasoned), UNKNOWN-NEEDS-A-RUN (with the cheapest test).
Paths below are relative to RECON unless they start with BF2 or .harmony. "R:" = RECON, "B:" = BF2.

Boris, verbatim (backlog "Boris feedback of 2026-10-03"):
- "For sync control we should have a way to remember it as part of a composition save. I imagine if a user is doing this professionally, they will set up the venue and save it in case of a computer crash or something and if they come back to that venue, they have the settings already."
- "Sync lives in top bar"
- "Gain slider in top bar could be twice as long. I have very little space to move it because I start at a quarter from the left edge and move down from there so I have very little distance to make a lot of fine adjustment."
- "The music is in time and the visuals should be delayed or a little ahead depending on how the system is wired."
- "All the other defaults are good." (so the BF2 ruling defaults Q2/Q3/Q5/Q6 stand: .harmony/boris-feedback-backlog.md BF30 lists sync Q11 bar-not-dial, Q12, Q13, Q15.)

======================================================================================================
## 1 QUESTIONS ANSWERED
======================================================================================================

### Q1 TopBar layout, Gain slider, min window width, what the Fade removal freed, what would have to shrink

**1a. Sections left to right (R:src/ui/TopBar.cpp `TopBar::resized()` :529-631). All FIXED pixels; nothing is proportional.** VERIFIED.
TopBar is `area.removeFromTop(34)` of MainComponent (R:src/MainComponent.cpp:2578), full width minus 4 px each side (MainComponent `reduced(4)` :2565), then TopBar `reduced(4, 2)` (R:TopBar.cpp:531). Net usable width = window width - 16.

Left run (each `removeFromLeft`, in order):

| # | Section | Width rule | Line | Running x (no Manual field) |
|---|---|---|---|---|
| 1 | "Audio:" label | 38 | :534 | 38 |
| 2 | Audio source combo | 90 | :535 | 128 |
| - | gap | 4 | :536 | 132 |
| 3 | "Gain:" label | 30 | :537 | 162 |
| 4 | **Gain slider** | **70** | :538 | 232 |
| - | gap, separator | 6 + 2 | :539, :542 | 240 |
| 5 | Play / Pause / Stop | 3 x 24 + 2 x 1 gaps (+6 gap, +2 sep) | :545-553 | 322 |
| 6 | Beat wheel | 26 (reduced by 1) + 2 | :557-558 | 350 |
| 7 | Bar readout "Bar 1..4" | 44 + 2 | :561-562 | 396 |
| 8 | Tempo number | 50 | :565 | 446 |
| 8b | reserved slot after it (holds the tracker-state label, absolute 60 x 14 at tempo.right+2) | 64 | :566-569 | 510 |
| 9 | Tap / Resync / Manual / Link | 32, 50, 80, 50 each + 2 gap | :572-579 | 730 |
| 9b | BPM edit field, ONLY in Manual mode | +60 +4 | :582-586 | (+64) |
| 10 | /4 /2 x1 x2 x4 | 5 x 26 + 4 x 1 + 6 | :589-599 | 870 |
| 11 | "Quantize:" label + combo | 55 + 100 + 6 | :602-604 | 1031 |

Right group (everything left over, built with `removeFromRight`, so it SHRINKS toward zero when the budget runs out; it never overlaps): dsp label 55, fps label 45, gap 6, Outputs button 100, gap 4, Master slider 90, "Master:" label 42, gap 4, Master Signal slider 90, "Master Signal:" label = measured text width + 6 (GlyphArrangement, :628-630). (R:TopBar.cpp:606-630.) VERIFIED. The label width (about 72-78 px at 11 pt) is INFERRED, not measured.

**1b. Gain slider.** VERIFIED:
- `ResettableSlider`, LinearHorizontal, range 0.0 to 4.0, step 0.01, value 1.0, `setDefaultValue(1.0)`, NoTextBox (R:TopBar.cpp:25-30). Right-click resets to 1.0 (R:src/ui/UniversalParamControl.h:49-53).
- The default 1.0 sits at 25 % of the travel (1.0 / 4.0). This is exactly Boris's "I start at a quarter from the left edge".
- Width 70 px total; the track is drawn from x to x+width (R:src/ui/LookAndFeel.cpp:172-196, 4 px track, 14 px thumb). The pixel length JUCE actually maps to 0..4 is the 70 minus the thumb indent that LookAndFeel_V4 reduces by: INFERRED about 56-70 px (JUCE source is not in RECON). So Boris's working range 0..1 is about 14-18 px of mouse travel for 100 steps (0.01 each). UNKNOWN-NEEDS-A-RUN for the exact figure: launch a test-server build, GET /api/debug/ui_paint `topbar_rect` (R:src/api/ApiServer.cpp:1933) plus a window capture, and measure the track pixels.
- Wiring: MainComponent sets `audioEngine_.setInputGain(value)` on every value change (R:src/MainComponent.cpp:577-579) -> `CombinedCallback::inputGain` atomic (R:src/audio/AudioEngine.h:69, CombinedCallback.h:30, :99).
- NOT persisted anywhere: no composition key, no settings.json key, no preset (grep "inputGain|input_gain" finds only the slider, AudioEngine, and the old PresetManager deck-preset field R:src/ui/PresetManager.cpp:506/558). No REST or OSC route sets it. VERIFIED (grep). It resets to 1.0 at every launch.
- MainComponent still owns an OLD hidden v1 gain slider (R:src/MainComponent.cpp:333-341, hidden :2622-2623). Not the one on screen.

**1c. Minimum window width.** `setResizeLimits(1280, 720, 3840, 2160)` (R:src/Main.cpp:53); default content size 1280 x 800 (R:src/MainComponent.cpp:2329). VERIFIED. Boris's real maximized width is 1728 (R:tests/test_master_signal_link.cpp:14-17, "this app's real maximized-window width on Boris's hardware"; test claim, not independently confirmed).

**1d. Width budget (my arithmetic from 1a; VERIFIED sums, INFERRED right-group label width).**
- Left run = 1031 px (1095 in Manual mode). Right group wants about 514 px (436 fixed + about 78 label).
- At 1728 (usable 1712): left 1031 + right about 514 -> about 167 px spare (about 103 spare in Manual mode).
- At 1280 (usable 1264): only 233 px remain for a right group that wants about 514. The right group is already clipped there: dsp 55 + fps 45 + 6 gap leave 127; Outputs then gets 100 -> 21 left; the Master slider gets about 17 px; "Master:", the Master Signal slider and its label get 0 px. The file's own test says so (R:tests/test_master_signal_link.cpp:17-26, 331-358: "at 1280 the WHOLE right side of TopBar already overflows ... pre-existingly"; written before the Fade removal, when the shortfall was 136 px worse). Cross-check: 1264 - (1031 + 136) = 97, which matches the test's "~105px left" comment. So at 1280 the removal helped by 136 px but the right group is still about 280 px short. VERIFIED logic, INFERRED label width.

**1e. What the "Fade:" removal freed.** Commit 5cdf218 (git -C RECON show 5cdf218): the Fade label 30 + slider 100 + gap 6 = **136 px** on the left run (also the `fadeSlider_` ResettableSlider, `composition_.globalTransitionSpeed` field deleted; the Master Signal group now follows the Quantize combo, seam `quantizeSelectorBoundsForTest`, R:src/ui/TopBar.h:61-62). VERIFIED. It freed width only on the left run; the left run is a fixed block, so the freed 136 px is spare space at the right of the Quantize combo, taken up by the right group's `removeFromRight`.

**1f. What would have to shrink to double Gain (+70 px) and add a Sync control.** Facts only (no design):
- Doubling Gain = +70 px. A Sync number box + "-" / "+" buttons + a drag bar has no existing size; for scale, the existing house text-box width is 35 px (Master sliders, TopBar.cpp:214, 250), a transport button is 24 px, a Tap button 32 px.
- Spare today: about 167 px at 1728 (about 103 in Manual mode); 0 at 1280 (already negative, see 1d).
- Blocks with fixed slack a planner could draw from (each is a fixed number in resized(), nothing adaptive): the 64 px reserved tempo slot (the tracker-state label inside it is 60 x 14 at 9 pt and BF2's ruling AM-14 already claims that slot for a "SYNC +42" badge, B:ruling-bf2.md:310-316); Quantize combo 100 + label 55; Audio combo 90; the five multiplier buttons 5 x 26; Tap/Resync/Manual/Link 214; dsp 55 + fps 45 (stats); Outputs 100; two 90 px sliders with a 35 px readout each.
- Nothing in TopBar re-flows under a narrow width except the right group's clamp. A new left-run block pushes the right group left by its width; at 1280 it would push more of the right group to zero width.

**1g. Constraints on any new top-bar widget.**
- **ResettableSlider rule** (CLAUDE.md UI Patterns): every slider must be a `ResettableSlider` with `setDefaultValue` at setup (right-click resets). The Gain, Master and Master Signal sliders already are. VERIFIED (TopBar.cpp:28, 209, 246). `ResettableSlider` also relays a right-click from a nested child except a `juce::Button` child (R:UniversalParamControl.h:59-64), so a text box inside it resets but a -/+ button does not.
- **Pitfall 57** (R:docs/claude/pitfalls.md:123): never add a 15-30 Hz `repaint()` without a layer or a change test; the TopBar stays in-peer, its wheel repaints a union rect at 15 Hz (R:TopBar.cpp:309-313, `startTimerHz(15)` :289, `getWheelRepaintBounds` TopBar.h:69). A new widget must not add its own timed repaint.
- **Pitfall 59** (:127): a model-driven widget repaints only when what it PAINTS changes (compare the painted value, not the read value). `Slider::setValue` already repaints only when the snapped value moves.
- **Pitfall 41** (:91): a widget showing a model value that REST, OSC, MIDI or a routine can write must follow the model, set with dontSendNotification, skipping while the mouse is down. The Sync value is writable from REST and OSC (Q2), so the top-bar control must be re-pulled when those change it. The existing pattern in this bar: `syncMasterFromComposition()` from the 15 Hz timer (TopBar.cpp:315-316, 376-396) skips while dragging. BF2's plan wants a listener notification instead of polling (B:ruling-bf2.md:315 "no polling").
- **Pitfall 34**: a headless widget test must `setVisible(true)` explicitly (the BF2 plan cites it for its indicator test).
- TopBar widgets that take keyboard focus stay in-peer (Pitfall 59 tail: "its own layer would put focus-taking controls behind a hit-test-transparent native view"). A number box needs focus, so it must live in the TopBar's normal peer painting.
- Test seams already pinning this bar's layout: tests/test_master_signal_link.cpp (1728 and 1280 cases, quantize->signal ordering, "no Fade"), tests/test_topbar_link_toggle.cpp, tests/test_master_opacity_link.cpp, tests/test_topbar_model.cpp. Any resize of the Gain slider or insertion of a left block will need these re-anchored (the Fade removal did the same, 5cdf218).

**Stale-widget caveat (VERIFIED by absence).** TopBar's Quantize combo and the BPM multiplier highlight are written ONLY by the user's clicks (R:TopBar.cpp:177-188, 401-427); nothing re-reads `composition_.quantizeMode` / `bpmMultiplier` after a composition load (the header admits "TopBar's known stale-widget issue", TopBar.h:154-158). So today a loaded composition changes `quantizeMode` in the model but the combo keeps showing the old value. A Sync control saved with the composition would hit the same gap unless it follows the model (Pitfall 41).

------------------------------------------------------------------------------------------------------

### Q2 The sync-dial lane (BF2: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2, 4a1f240)

**Status of the lane.** Stages S1a, S1b, S2, S3 are committed (git log: S3 head 4a1f240). S4 (keys/MIDI) is NOT in the tree (grep `SyncNudge` finds nothing); S5 (the on-screen control) is NOT built. BF2's base is main eff2b1c, BEFORE bf9b: its TopBar.cpp/h are unchanged from eff2b1c (`git diff --stat eff2b1c HEAD -- src/ui/TopBar.*` is empty), so they still contain the Fade section. VERIFIED. The skeleton .harmony/.reports/s-rta-1003/plan-bf2-delta.md is headings only (no content yet).

**Where the offset lives today on BF2.**
- Value type: integer ms, range -500 to +500 (B:src/analysis/SyncOffset.h:12-13, `clampMs` :21-29). VERIFIED.
- Per-venue list: `SyncVenues` (pure data, juce_core only; B:src/sync/SyncVenues.h). Venue = `{name, ms}`; JSON `{"version":1,"current":"Default","venues":[{"name":"Default","ms":0}]}`; names 1..40 chars, unique case-insensitive, at most 64 venues; ops select / create (copies the current offset) / rename / remove (refused for the last) / setMs / nudge. VERIFIED.
- Its own header states: "Saved in the machine's settings.json under AppSettings::kSync (never in a composition: the same show plays many rooms)" (B:SyncVenues.h:7-9). Key constant: `AppSettings::kSync = "sync"` (B:src/model/AppSettings.h:20). The plan's F4 explicitly REJECTED storing it in the composition ("(b) The composition. REJECTED: the same show plays many rooms; a saved offset would impose the last room's timing on every load", B:.harmony plan-bf2.md:166-170). **This is the direct conflict with Boris's "remember it as part of a composition save".**
- Glue: `SyncOffsetController` (B:src/sync/SyncOffsetController.h): loads venues at construction (never writes on load), pushes the current offset to a sink (`analysisThread_.setSyncTargetMs`, B:MainComponent.cpp:1857-1858, built BEFORE the analysis thread starts so hop 1 already uses it), every op updates the model, pushes the target, publishes a lock-free status copy, notifies `Listener::syncOffsetChanged()`, and schedules a save 500 ms after the last change (`kSaveDelayMs`; `save()` = `AppSettings(file).update(kSync, venues.toVar())`, B:SyncOffsetController.cpp:134-146). Message thread only (status copy readable from any thread). Not undoable, not recorded into takes/routines, not a ControlPath. VERIFIED.

**REST / OSC routes that set it (B:src/api/ApiServer.cpp:346-351; B:src/osc/OscHandler.cpp:195-210; B:MainComponent.cpp:2211-2243, 2330-2331).** VERIFIED:
- GET /api/sync (targetMs, appliedMs, venue, venues, persist, analysisRunning, hopDelayMs, queuedHops, lineOverruns, leadEngaged, lastError; contract B:plan-bf2.md:356-358).
- POST /api/sync/set {"ms":N}; POST /api/sync/nudge {"ms":N}; POST /api/sync/venue {"name","create"}; POST /api/sync/venue/rename {"from","to"}; POST /api/sync/venue/remove {"name"}.
- OSC (UDP 8000): /audiodna/sync <ms> (set), /audiodna/sync/nudge <ms>.
- TEST-ONLY: POST /api/debug/sync_persist {"enabled":false}; env AUDIODNA_SYNC_TEST (unset = normal, integer N = persistence off and session starts at N, other = persistence off). (B:SyncOffsetController.h:28-35, MainComponent.cpp:2243.)
- Keys/MIDI nudge (S4, `SyncNudge` binding) is PLANNED, not built.

**How it is applied (B:src/analysis/AnalysisThread.cpp).** VERIFIED:
- LATE (D > 0): an analysis delay line. Audio hops (512 samples @ 48 kHz) are drained from the resampler into a 64-hop line stamped with arrival time (:127-135) and each is processed only when `now >= stamp + D` (:150-156); the applied D slews toward the target at 0.25 ms per ms (`SyncSlew`, :143; plan F3, first value snaps). So EVERY bus reader (render, the 120 Hz pipeline, REST, TopBar, projectM PCM, the waveform) sees the whole analysis D late. The beat clock runs in sample time per processed hop.
- EARLY (D < 0): `BeatLead` rewrites ONLY nine beat fields of the snapshot after the 14 stages, before the publish (:447-465; B:src/analysis/BeatLead.h:38-50): beatPhase, totalBeatCount, beatInBar, downbeatDetected, barPhase, barCount, totalBarCount, resyncBarOrigin, phrasePhase. Onsets, loudness and bpm are untouched. The tracker itself is never touched. Identity (byte-identical) while D >= 0 and the lead is not engaged.
- Provenance: `snap->syncOffsetMs = appliedMs` (:447 region).
- In `--test-mode` the analysis thread is NOT started (B:MainComponent.cpp:1862-1869 `if (!testMode_)`), so the dial moves nothing on the bus there (features are injected over HTTP). INFERRED consequence for gates: sync needs a PRODUCTION-mode test-server build (plan F8 / probe-sync).

**What plan-bf2 / ruling-bf2 say about on-screen control "S5" and venues.**
- Plan F5: control lives in the TimingWindow "BPM" tab via a new per-tab content slot (I10) and a new `SyncPanel` (I11: venue button + menu, value box "+42 ms", -/+ with repeat, a bipolar `ResettableSlider` bar, `setDefaultValue(0)`, caption "Visuals 42 ms later"). The TopBar was REJECTED as "over-full (E14)" (B:plan-bf2.md:171-176). Ruling Q4 default: the BPM panel; "Other choice: a small button in the top bar that opens it" (B:ruling-bf2.md:520-525).
- Ruling AM-14 (I14, stage S5): in the TopBar only an always-visible, display-only badge `syncIndicator_` (9 pt, bounds (tempoLabel_.getRight()+2, area.getY(), 60, 12) inside the existing 64 px tempo slot, text "SYNC +42" / "SYNC -30", hidden at 0, tooltip names the venue; "Click-to-zero rejected", B:ruling-bf2.md:97-101, 310-321). Driven by the controller's change notification, not polling.
- Harmony adoption (B:plan-bf2.md:714-731): S5 was "NOT in this run"; it waits for Q1 (bar vs round dial) and becomes a follow-up stage; defaults taken: Q2 come back on the last venue + "SYNC +N" badge, Q3 glide, Q4 the BPM panel (S5 only), Q5 a new venue starts from the current setting, Q6 the beat lands where you pressed.
- **Boris's 2026-10-03 answers change S5:** "Sync lives in top bar" (the control, not just a badge), and the setting is to be remembered "as part of a composition save". Both contradict the adopted design (panel in the BPM tab; venues in settings.json). The form "a number box with - / + buttons and a drag bar" comes from the task brief, not from Boris's verbatim text; it matches the form BF2's plan already chose as the default (I11: value box, -/+, bar; ruling Q1 default).

------------------------------------------------------------------------------------------------------

### Q3 Composition save / load at a7491d4

**Serializer.** `Composition::toVar()` / `fromVar()` and `saveToFile` / `loadFromFile` are inline in R:src/model/Composition.h:735-1115 / 1117-1135 (toVar :735, fromVar :872, saveToFile :1119, loadFromFile :1125). `saveToFile` = `file.replaceWithText(JSON::toString(toVar()))` (plain JSON, no wrapper); `loadFromFile` parses and calls `fromVar`, then sets `filePath`. VERIFIED.

**File location.** Compositions dir = `userApplicationDataDirectory/AudioDNA/compositions` (R:src/ui/CompDecksBrowser.cpp:322-325), `*.json`; Save = `MainComponent::saveComposition` (R:MainComponent.cpp:3464, Cmd+S at :6547), Save As = :3507, `saveCompositionTo` :3490. Production REST has `POST /api/load_composition` (R:ApiServer.cpp:251) but the save route is TEST-ONLY `POST /api/debug/save_composition` {"path"} (R:ApiServer.cpp:328, handler :2173). No autosave / crash-recovery exists for compositions (grep "autosave|recover" finds only audio/BPM recovery code). VERIFIED.

**Top-level keys written by `toVar()` (R:Composition.h:735-868).** name, activeDeckIndex, masterOpacity, bpmMultiplier, quantizeMode, outputWidth, outputHeight, outputDisplay, masterSpeed, masterSignal, crossfader{Phase,BlendMode,Behaviour,Curve}, compPositionX/Y, compScale, compRotation, compAnchorX/Y, autopilotDirection/DurationMode/ClipLoops/Loop/MasterLayer, 11 `pta*` per-type-autopilot keys, autoPresetOnGenre, smartAutopilotEnabled, structuralSceneEnabled, genreDeckAssignment, genrePresetNames, **layers** (the shared layer stack), **decks**, globalEffects, conns (sparse), **connect** {gripHoldMs, handBackGlideMs}, routines, routineBank. VERIFIED. Not saved: tempo/BPM, manual-BPM mode, audio source, audio gain, tap state, the Quantize/multiplier TopBar widgets' own state (the model fields are saved, the widgets are not re-read on load).

**Version / migration mechanism.** There is NO version key in a composition file (R:src/model/ShowMigration.h:15-16: "no version key exists; none is needed"). Migration = (1) `hasProperty` guards in `fromVar` (a key added later is simply absent in an old file and keeps its default; e.g. :883-931), and (2) `ShowMigration::isLegacyShow` = "no top-level `layers` array" -> `convertShow` folds per-deck layers into the shared layer stack and returns one human note (R:ShowMigration.h:14-18, :63-179; used in `fromVar` ~:990-1000). `ShowMigration` is "the ONLY src reader" of the legacy keys "persistent" and "globalTransitionSpeed" (:6-8). Two unguarded reads exist: `bpmMultiplier` and `quantizeMode` (:879-880, plain getProperty), `name`, `activeDeckIndex`, `masterOpacity`. VERIFIED. (The `"version": 2` at R:src/ui/PresetManager.cpp:77 belongs to FX presets, a different file type.)

**Where a per-composition "venue name + sync ms" pair would sit.** Natural homes, all VERIFIED to exist as patterns (no design chosen here):
- A new top-level key in `toVar()` with a `hasProperty`-guarded read in `fromVar()`, like `masterSignal` (:901-902) or the nested `connect` object (a sub-object with two floats, :850-852, read :1090-1096 region). Old files lack it; no migration code needed.
- Precedent for machine-ish settings already riding in the composition: `outputWidth`, `outputHeight`, `outputDisplay` (R:Composition.h:743-745).
- Key name `sync` is free in the composition (BF2's "sync" is a settings.json key, a different file).
- UI side: `Composition` is pure model; the live offset lives in BF2's `SyncOffsetController` (a MainComponent member, not in Composition). A composition-borne value needs a hook that pushes it to the controller/sink after a load: the natural place is where the swap lands (R:`finishStagedLoad` :3229-3280 -> `swapCompositionModel` :3009 -> `refreshUiAfterModelSwap` :2946). INFERRED.

**What happens today to app-level settings when a composition loads.** VERIFIED:
- Load is staged on a private `Composition incoming` (R:MainComponent.cpp:3395-3460), validated, clip ids re-minted, source params reconciled, then `composition_ = std::move(s->comp)` inside the GL fence (:3262-3265); routines stopped, undo cleared, load notice cleared, inspectors/grid rebuilt (:3009-3054, :2946-3006).
- AppSettings (settings.json: `milkDropPresetDir`, `outputs`, plus BF2's `sync`) is NOT touched by a composition load: MainComponent uses it only at construction/outputs (R:MainComponent.cpp:1759, 2365, 2371). The composition's own `outputDisplay` etc. are read from the model, not pushed to settings. Audio gain, audio source, tempo/Link/manual-BPM are untouched by a load (never in the file).
- Consequence for a composition-borne sync: a FRESH `Composition incoming` carries field defaults for every key the file lacks, so an old file without the key would load as the default value and (if applied) overwrite the live offset; likewise `kCompNew` -> `initDefault()` (R:Composition.h:200-225). This is a behaviour decision for the planner (apply on load / only if present / ask). INFERRED from fromVar's guarded-default shape.

------------------------------------------------------------------------------------------------------

### Q4 What the beat wheel reads, and what the offset shifts on lane/bf2

**Wheel source.** `TopBar::timerCallback` (15 Hz) does `featureBus_.read()` and copies bpm, trackerState, beatInBar, barPhase, beatPhase, barCount into `displaySnap_` (R:TopBar.cpp:297-306); `paintBeatWheel` uses `beatInBar` (which segment lights), `beatPhase` (its fade) and `bpm`/`trackerState` (lit or dim) (:458-512); the "Bar N" readout uses `barCount` (:514-527, TopBarModel.h:16-19). So the wheel reads the SAME FeatureBus snapshot (the analysis thread's beat clock) as every other consumer, not a separate wheel clock. VERIFIED. The wheel is sampled at 15 Hz (every about 67 ms) and repainted only on that tick, so a 1 ms dial move is invisible on the wheel at that granularity (VERIFIED cadence; the visual consequence INFERRED).

**What the offset shifts on BF2.** VERIFIED (code above):
- LATE (D > 0): the audio into the analysis is delayed D, so the WHOLE snapshot, wheel included, is published D late. Beat fields, bpm, tracker state, onsets and loudness all move together. The wheel FOLLOWS THE SHIFTED beat.
  - Nuance (INFERRED from plan E6/F7, not run): a Tap / Resync / typed tempo is applied at the next processed hop, i.e. "at the press" in show time; so in Tap/Manual mode the wheel is anchored where Boris pressed while onset-driven flashes are D late (this is the ruling's Q6 default "where you pressed", B:ruling-bf2.md:528-531). In Auto mode the tracker re-aligns to the (delayed) detected beats within about a beat.
- EARLY (D < 0): `BeatLead` rewrites the nine beat fields (beatPhase, beatInBar, barPhase, barCount ... see Q2) to run |D| ms ahead; the wheel reads beatInBar / beatPhase / barCount / barPhase, all in the nine, so it LEADS. `bpm` and `trackerState` are not shifted.
- So on BF2 the wheel is NOT unshifted: it moves with the visuals. This contradicts Harmony's reading BF16(c) ("confirm: beat wheel unshifted, output shifted", .harmony/boris-feedback-backlog.md:154) and arguably Boris's "the user can see it pulsing exactly to the time of the music". A wheel that stays on the unshifted beat would need the wheel to read something other than the bus snapshot (the shifted snapshot has no raw/unshifted beat field; BeatLead keeps "RAW beat fields" only in the test-server witness, B:plan-bf2.md:292). INFERRED about what that would take; UNKNOWN whether Boris wants the wheel unshifted: asked of Boris, not answerable from the files.
- Boris's phrase: "The music is in time and the visuals should be delayed or a little ahead depending on how the system is wired." LATE = visuals delayed (the analysis delay line); EARLY = only beat-locked things lead (loudness cannot run early; Boris accepted that, B:plan-bf2.md:20).

======================================================================================================
## 2 TABLES
======================================================================================================

### 2a Width budget by window width (fixed-px sums above)
| Window width | Usable (W-16) | Left run | Right group wants | Spare / (short) |
|---|---|---|---|---|
| 1280 (minimum) | 1264 | 1031 (1095 Manual) | about 514 | about (281) short; right group clipped to 0 from "Master:" on |
| 1728 (Boris's maximized) | 1712 | 1031 (1095 Manual) | about 514 | about 167 spare (about 103 Manual) |
| 1280 before the Fade removal | 1264 | 1167 | about 514 | about (417) short (test comment said about 105 left) |

### 2b Where the sync value could be stored (facts, not a recommendation)
| Location | Exists at a7491d4 | Survives a composition load | Survives a crash / recall at a venue | Precedent in code |
|---|---|---|---|---|
| settings.json "sync" via AppSettings (BF2 as built) | key + controller on BF2 only, not in RECON | untouched by load | yes, per machine | `outputs`, `milkDropPresetDir` (R:AppSettings.h:17-19) |
| Composition JSON top-level key | no | replaced by file (default if the key is absent) | only if the show was saved | `masterSignal`, `connect`, `outputDisplay` |
| Both | n/a | n/a | n/a | would need a rule for which wins on load |

### 2c What each dial sign moves on BF2
| Offset | Mechanism | Moves | Does NOT move |
|---|---|---|---|
| D > 0 (later) | analysis delay line (AnalysisThread.cpp:127-156) | every bus field, every reader incl. TopBar wheel, bpm, onsets, loudness, projectM PCM, waveform | Tap/Resync/typed-tempo anchors (applied at the press) |
| D < 0 (earlier) | `BeatLead` on 9 beat fields (AnalysisThread.cpp:447-465) | beatPhase, totalBeatCount, beatInBar, downbeatDetected, barPhase, barCount, totalBarCount, resyncBarOrigin, phrasePhase (wheel, bar readout, beat-locked consumers) | onsets, loudness, bpm, tracker |

======================================================================================================
## 3 WHAT EXISTS TODAY vs WHAT BORIS ASKED (gaps, no design)
======================================================================================================
- Gain "twice as long": today 70 px, 0..4, default 1.0 at 25 %. No width setting, no other mapping. Gap = a wider slider and about 70 px found in a bar that is already 281 px over budget at 1280 and has about 167 px spare at 1728.
- Sync control in the top bar: RECON has no sync of any kind (grep: no sync dial on RECON; LinkSync is Ableton tempo only). BF2 has the engine, REST, OSC and venues but NO on-screen control (S5 not built) and, when built, planned it in the TimingWindow BPM tab plus a display-only badge. Gap = a top-bar number box with -/+ and a drag bar; ResettableSlider default 0 for the bar.
- Sync remembered with the composition: BF2 stores venues in settings.json only and its header/plan say "never in a composition". RECON's composition has no sync key. Gap = a composition key + the load/new hook that pushes it to the controller + the rule for an old file without the key. Also the venue NAME: BF2 has named venues with a current pointer; Boris's wording is one setting per composition ("set up the venue and save it"); whether the composition carries a venue name, a list, or only ms is open.
- Wheel: BF2's wheel follows the shifted beat; Harmony's backlog note expects the wheel unshifted. Not settled.
- TopBar stale-widget issue (quantize / multiplier) exists today; a composition-borne Sync would need to avoid repeating it.
- No Gain persistence, no Gain REST: a "Gain twice as long" change does not need either.

======================================================================================================
## 4 UNKNOWN-NEEDS-A-RUN
======================================================================================================
1. Exact mouse travel of the Gain track (and of a doubled one): build is forbidden here; cheapest test = launch a test-server build, GET /api/debug/ui_paint (`topbar_rect`, ApiServer.cpp:1933) and capture the window, measure the track from thumb at 0 to thumb at 4 (or read JUCE's LookAndFeel_V4 `getSliderThumbRadius` / `getSliderLayout`).
2. Real width of the "Master Signal:" label at 11 pt (my 72-78 px is an estimate): `GlyphArrangement::getStringWidthInt` is what the existing test already calls (tests/test_master_signal_link.cpp:319); run that case, or a capture at 1280 and 1728.
3. Which controls are actually visible at 1280 today (my arithmetic says Master / Master Signal collapse to 0 px): capture at 1280 x 720; GET /api/debug/ui_paint gives `topbar_rect` and `main_w`.
4. Whether the wheel follows the shifted or unshifted beat in practice, in Tap vs Auto mode: production-mode test-server build of lane/bf2, POST /api/sync/set {"ms":200}, POST /api/set_bpm / tap, then GET /api/features (beatPhase) vs GET /api/sync (appliedMs) while a click track plays (probe-sync.sh row R4/R5 style). Nothing in RECON can answer it.
5. What a composition load does when the key is absent and a non-zero live offset is set (a design rule, then a REST sequence: POST /api/sync/set, POST /api/load_composition of an old file, GET /api/sync) once built.
6. Whether BF2 merges before or after bf9b and the TopBar conflict: BF2's TopBar.cpp still has the Fade section and bf7 also edits TopBar.cpp (B:ruling-bf2.md:321, RR5). Needs a merge dry run, not a read.

======================================================================================================
## 5 FILES A BUILDER WOULD TOUCH (paths only)
======================================================================================================
RECON paths (post-bf9b):
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/TopBar.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/TopBar.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/MainComponent.cpp (gain wiring :577; TopBar creation :532; layout :2578; swapCompositionModel :3009 / refreshUiAfterModelSwap :2946 / finishStagedLoad :3229 if a load must push Sync)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/MainComponent.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/model/Composition.h (toVar / fromVar / initDefault if Sync rides in the composition)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/model/ShowMigration.h (only if an old-file rule needs a note)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/LookAndFeel.cpp (only if the bar is a bipolar fill-from-zero slider; BF2's I11 changes drawLinearSlider for a "bipolar" property)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/UniversalParamControl.h (ResettableSlider; read only)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/Main.cpp (line 53 window minimum; only if the 1280 minimum is to change)
- Docs: docs/claude/integration.md, docs/claude/pitfalls.md, .harmony/APP-INVENTORY.md, CLAUDE.md (UI Patterns / index)
BF2 paths the Sync control must connect to (they merge separately):
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/src/sync/SyncOffsetController.h and .cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/src/sync/SyncVenues.h and .cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/src/model/AppSettings.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/src/analysis/AnalysisThread.cpp and .h, BeatLead.h/.cpp, SyncOffset.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/src/api/ApiServer.cpp and .h; src/osc/OscHandler.cpp and .h
Tests that pin this area: see section 6.

======================================================================================================
## 6 EXISTING TESTS / PROBES AND REST ROUTES THAT CAN DRIVE THIS
======================================================================================================
Tests, RECON (VERIFIED present): tests/test_master_signal_link.cpp (layout at 1728 and 1280, ordering, "bf9b S3.3 no Fade control"), tests/test_topbar_link_toggle.cpp, tests/test_master_opacity_link.cpp, tests/test_topbar_model.cpp, tests/test_right_click_reset.cpp (ResettableSlider), tests/test_composition.cpp, tests/test_show_model.cpp (uses ShowMigration, bf9b load/convert), tests/test_app_settings.cpp.
Tests, BF2 (VERIFIED present): tests/test_sync_venues.cpp, test_sync_offset_controller.cpp, test_osc_sync.cpp, test_sync_slew.cpp, test_sync_witness.cpp, test_analysis_sync_thread.cpp, test_analysis_sync_alloc.cpp (plus test_beat_lead, test_binding_sync_nudge planned in the ruling's lists; test_topbar_sync_indicator / test_sync_panel are S5, not yet written).
Probes: RECON .harmony/probe-idle-paint.sh (TopBar wheel repaint, ui_paint / ui_passes, topbar_rect), .harmony/probe-mastersignal.sh (Master Signal fader through REST/OSC), probe-manual-bpm.sh, probe-resync.sh, probe-beatclock.sh, probe-tempo-start.sh, probe-composition.json, probe-async-load.sh. BF2 .harmony/probe-sync.sh and probe-sync-venues.sh (+ .py).
REST that drives this area without synthetic mouse input (7070 production unless noted):
- Read tempo/wheel inputs: GET /api/bpm, GET /api/features, GET /api/state, GET /api/composition (master fields incl. masterSignal), 8080: GET /api/composition_params / POST /api/set_composition_params (R:src/test/TestServer.cpp:229-233).
- Set tempo: POST /api/set_bpm, POST /api/resync (R:ApiServer.cpp:215-224). No production route sets the composition Quantize mode or the Gain slider; neither is readable over REST.
- Load/save a composition: POST /api/load_composition (production, R:ApiServer.cpp:251); save is TEST-ONLY POST /api/debug/save_composition {"path":"/abs/file.json"} (:328). Reading the saved JSON is the way to prove a new top-level key round-trips (no UI needed).
- TopBar geometry: GET /api/debug/ui_paint (`topbar_rect`, `main_w`, `top_bar_wheel_repaints`) and GET /api/debug/ui_passes (TEST-ONLY build, R:ApiServer.cpp:310-313, 1930-1945), then a window capture for pixels.
- Sync (BF2 only): GET /api/sync, POST /api/sync/set|nudge|venue|venue/rename|venue/remove, OSC /audiodna/sync and /audiodna/sync/nudge, TEST-ONLY POST /api/debug/sync_persist; env AUDIODNA_SYNC_TEST. Production-mode test-server build needed for the dial to move anything (--test-mode skips the analysis thread).
- Window width for a 1280 vs 1728 capture is set by the window, not by REST: no route found that resizes the main window (INFERRED; grep not exhaustive).
