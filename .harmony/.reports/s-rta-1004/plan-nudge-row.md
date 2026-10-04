# PLAN nudge-row -- DELTA on Boris's answers 61, 62, 111: the sign, the text "nudge X ms", and the tempo row of the top bar (lane "nudge-row", s-rta-1004)
Architect (opus), 2026-10-04. A position for the blind council to attack; an architect ruling follows; Harmony decides.
Read-only. Pins checked: main HEAD = 185147b and `git status --short -- src tests docs CMakeLists.txt` printed nothing
(plain-file reads below are reads of 185147b); lane/bf2 = 740b6d6 clean; lane/bf2-keys = 9eab9bd clean. Nothing was built,
no test was run, no app or probe was launched. All widths and sums below are arithmetic on the lines cited, not pixels.
Shorthands: M: = main at 185147b (paths under /Users/boriskarpman/projects/RealTimeAudio). RN: = .harmony/.reports/
s-rta-1004/ruling-nudge.md (A1..A24 = its amendments). PL: = plan-nudge.md. BD: = .harmony/binding-decisions.md.
BL: = .harmony/boris-feedback-backlog.md. FB: = facts-beat-controls.md. FR: = facts-resolume-screen-delay.md.
RT: = ruling-transport-delta2.md.
Labels: VERIFIED (read at the pin), COMPUTED (arithmetic on verified lines), INFERRED, ASSUMED.
Precedence: Boris's verbatim words > Harmony's adoption (PL:765-820) > this delta, once ruled > RN > PL's body.
This delta AMENDS RN. Where it is silent, RN stands.

## 1 GOAL
Boris, verbatim (BD:910-921; BL "Boris's answers to questions 61-63" and "Boris's answer to question 111"):
- "61 b but lets call it "nudge X ms""
- "do a similar to resolume: beatWheel play pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2 tap resync"
- "good (this is just nudge amount)"
- "just the bpm timer. If most of the show is set up to BPM, and the BPM goes stop, the BPM goes to zero nothing moves. If
  there are clips that are not BPM based, then they play just as they were and are unaffected"
Earlier words this delta leans on (BD:855-862): "If I tapped the tempo again, to set the tempo, the time does not change. If
I press re-sync, then it does re-sync and that changes by how far off the beat we are."; "If we are shifted forward or back,
everything that is connected to BPM shifts forward or back. I mean everything. If the user twist the knob in real time, or
triggers a clip, that is not affected unless it's set to be quantized". And BD:868: "49 default".
Harmony constraint (nudge row): the row is HIS list in HIS order -- beat wheel, play, pause, stop, BPM number, BPM minus,
BPM plus, nudge back, nudge forward, /2, *2, tap, resync; nothing is added to it and nothing left out; Manual, Link and
Quantize stay to its right (reading R69). Every control in it does something. A stopped or paused BPM timer is a STATE
he can see in the row; "nothing moves" covers every beat-driven reader at once; what is not BPM-based is untouched. The
engine stages of RN (G-N0, S1, S2) and the key / pad stage S4 are not re-opened except where the stopped timer needs a field.
What this delta delivers: (1) the sign constant and every string, case and row it changes; (2) what play, pause and stop
do, where the state lives, and the smallest engine addition that makes ONE stopped beat; (3) what the BPM number, minus,
plus, /2 and x2 do against the 60..200 fold; (4) the row's layout, widths, narrow-window rule, tooltips, keys and the
visual gate's states; (5) the change list against RN, stage by stage.
THE THREE THINGS A READER MUST KNOW FIRST
 a. The top bar ALREADY has three buttons ">", "||", "[]" left of the beat wheel (F3, F4). ">" and "||" play and pause
    every layer's playing CLIP; "[]" stops all routines. None of them is the BPM timer. He chose "just the bpm timer" over
    "Every clip" (question 111 A over B). So the three widgets become the timer's; where the old jobs go is question 127.
 b. The timer is stopped INSIDE the tracker (its phase accumulator is the BPM timer), at four sites, not by freezing the
    published snapshot above a tracker that runs on. Reason: pause must resume from where it was held, and play from stop
    must make "now" the "1"; a freeze above a running tracker can do neither without a second offset machine (NR2).
 c. /2 and x2 cannot work today even if wired: every tempo request is folded into 60..200 (F9), so 128 x 2 = 256 folds
    back to 128. A hand-set tempo gets its own range, 30..400, clamped and never folded; the detector keeps its fold (NR3).

## 2 ESTABLISHED FACTS (verified lines only; anything else is labelled where it is used)
The top bar today
F1  VERIFIED M:src/ui/TopBar.cpp:529-630 (`resized()`), left to right, px: "Audio:" 38, source 90, gap 4, "Gain:" 30, gain 70,
    gap 6, sep 2 | ">" 24, 1, "||" 24, 1, "[]" 24, gap 6, sep 2 | wheel 26, 2 | "Bar N" 44, 2 | tempo number 50, state
    word (60, drawn inside a reserved 64) | Tap 32, 2 | Resync 50, 2 | Manual 80, 2 | Link 50, 2 | [BPM field 60, 4: only
    while Manual is on] | "/4" "/2" "x1" "x2" "x4" 26 each with 1 between, gap 6 | "Quantize:" 55, combo 100, gap 6 |
    then from the RIGHT edge: DSP 55, FPS 45, gap 6, Outputs 100, gap 4, Master slider 90, "Master:" 42, gap 4, Master
    Signal slider 90, its label (measured from the text, :628-630). The bar's inner width is the window's minus 8 (:531).
F2  COMPUTED from F1: the left block is 1031 px (Manual off) / 1095 px (Manual on); the right block is 436 px + the Master
    Signal label (ASSUMED about 86 px: it is measured at run time). Total about 1553 / 1617. At 1280 (inner 1272) the bar
    is OVER its width by about 281 / 345 px TODAY. `removeFromRight` clamps to what is left (JUCE), so at 1280 the right
    block is cut from its far end: Master and Master Signal get 0 px with Manual on. VERIFIED M:tests/
    test_master_signal_link.cpp:17-23 and :348-352 say the same in words ("at 1280 the WHOLE right side of TopBar ...
    already overflows its budget pre-existingly"). So RN's FM-5 is answered YES by the code and by that test's comment;
    the pixels are still VG-0's to capture.
F2b VERIFIED M:tests/test_master_signal_link.cpp:15-16: 1728 is "this app's real maximized-window width on Boris's
    hardware"; the window's minimum is 1280 x 720 (M:src/Main.cpp:53). RN's V11 (it opens maximized) stands.
F3  VERIFIED M:src/ui/TopBar.h:100-102 and TopBar.cpp:33-39: three TextButtons ">", "||", "[]"; the "[]" tooltip is "Stop all
    routines". M:src/MainComponent.cpp:603-628: `onPlay` / `onPause` call `applyClipPlaying(..., "play" | "pause", ...)` for
    every shared layer's playing clip; `onStop` calls `routineEngine_.stopAll()` and nothing else (comment :624-626, his
    ruling of 2026-09-26; M:docs/claude/recording.md:73-76 says the same; M:tests/test_topbar_link_toggle.cpp:95 pins the
    tooltip).
F4  VERIFIED M:src/MainComponent.cpp:7584-7588, :7852-7861: the bindable target "Play / Pause" (`GlobalPlayPause`) plays /
    stops the AUDIO FILE (`applyAudioTransport`), not clips; "Stop" (`GlobalStop`) = `routineEngine_.stopAll()`. No key or
    pad target runs the three top-bar clip buttons' play / pause.
F5  VERIFIED M:src/ui/TopBar.cpp:319-370 (`updateBpmDisplay`): Manual on -> the big number shows the TEXT of the BPM field
    (or "120"), then RETURNS -- so in Manual it never shows the published tempo, its colour is never set, and the FPS / DSP
    labels (set at :365-369, after the return) stop updating. Manual off -> `int(bpm + 0.5)` or "---", coloured by the
    tracker state (red / yellow / green), with the state word beside it.
F6  VERIFIED M:TopBar.cpp:97-119, :140-154: the BPM field is a `juce::TextEditor`, visible only while Manual is on;
    digits and ".", 6 characters; Enter -> `onManualBpmChanged(true, bpm)` -> `applyTempoCommand("manual")` ->
    `followExternalTempo` (never realigns). Manual ON applies the field's old text or 120, THEN refills the field with
    the detected tempo truncated (FB T1). The field keeps the keyboard after Enter (RN SF-4).
F7  VERIFIED M:TopBar.cpp:156-170, :401-427; M:src/model/Composition.h:142, :229, :741, :879; M:tests/test_composition.cpp:
    157, :194: the five buttons only write `composition_.bpmMultiplier` and call a callback nobody assigns; the value is
    saved and loaded and read by nothing (FB Q1, VERIFICATION item 8).
F8  VERIFIED M:TopBar.cpp:289, :297-317: one 15 Hz timer reads the bus and repaints `getWheelRepaintBounds()` = the union
    of the wheel and the "Bar N" rect, expanded by 2 (M:TopBar.h:69). The wheel is painted by the bar, not a child.
The tracker
F9  VERIFIED M:src/analysis/BPMTracker.h:41-42 (kMinBPM 60, kMaxBPM 200), M:BPMTracker.cpp:281-291 (`foldBPMToRange`:
    halve while above 200, double while below 60, then clamp), :607-622 (`applyTempoRequest` folds EVERY requested tempo:
    Tap, typed, REST, OSC, Link), :119 (the detector's raw estimate is folded too).
F10 COMPUTED from F9, what a tempo REQUEST does today: 240 -> 120; 256 -> 128; 201 -> 100.5; 59 -> 118; 50 -> 100; 30 ->
    60. So "x2" as a request changes the tempo only from 60..100 (to 120..200) and "/2" only from 120..200 (to 60..100);
    between 100 and 120 neither can change anything; 128 x 2 comes back as 128.
F11 VERIFIED M:BPMTracker.cpp:69-104: per hop, in this order: the request sequence is latched (:74), a pending tempo
    request is applied (:79-84), then in Manual `updatePhase(false, 0)` and return (:97-102) -- the detector's pipeline is
    skipped in Manual. :213-251 (`updatePhase`): the phase free-runs from `lockedBPM_`, whole beats crossed are counted,
    a confident onset realigns (Auto), and in the predicted regime a wrap advances the bar position. FB VERIFICATION
    item 3: the ONLY writers of `phase_` and `totalBeatCount_` are lines 217, 223, 230 and 256-257 -- all inside
    `updatePhase` and `realignPhaseToZero`.
F12 VERIFIED M:BPMTracker.cpp:253-258 (`realignPhaseToZero`: +1 beat when the phase is >= 0.5, then phase 0), :343-346
    (`scoreBeat()` runs on a detected beat outside the predicted regime: it moves the bar position), :349-352 (bar phase
    and phrase are recomputed every hop from the beat fields), :354-362 and :566-578 (`applyResync`, last in
    `feedDownbeatFeatures`: realign, bar position 0, level true, barCount 0, phrase 0, origin = totalBarCount).
F13 VERIFIED M:BPMTracker.cpp:592-604: `postTempoRequest` is one compare-exchange loop on one atomic word, then
    `raiseRequestSeq()` AFTER the write (Pitfall 48). :632-636 `setManualMode` is one atomic store + the raise.
F14 VERIFIED M:BPMTracker.cpp:293-309, :147: in Auto an estimate near half or double the locked tempo is "corrected" to
    the locked octave. INFERRED: a locked tempo left outside 60..200 when Manual is switched off would be KEPT by this
    correction for as long as the music stays near its half.
The readers
F15 VERIFIED (FB T2, its VERIFICATION "Other lines spot-checked, all CONFIRMED"): every beat consumer reads the published
    snapshot -- count deltas (quantised fires, Autopilot, routines, slideshow, randomize, projectM), the phases (shaders,
    mappings, signals, connections), the continuous beat (take clock). RN V2: nothing reads a beat value from the tracker.
F16 VERIFIED by grep at the pin (`.bpm` read off a snapshot, outside src/analysis, TopBar and the API): the readers of
    the tempo AS A RATE are M:src/render/Renderer.cpp:1722-1733 (a BPM-synced image sequence: fps from bpm, then
    `advanceFrame(dt)`), and ONE shader that scales `u_time` by `u_bpm / 120` (M:src/render/EmbeddedShaders.h:10793; the
    uniform is uploaded at M:CompositorEngine.cpp:1922, M:EffectChain.cpp:372-373, M:ProceduralSource.cpp:197-198). A
    BPM-synced VIDEO's speed line has no tempo term (M:Renderer.cpp:1657-1666: `videoBeats / beatDivision`; RN SF-5). The
    other `.bpm` readers are not motion: the mapping source BPM (M:MappingEngine.cpp:85), the routine "locked" test
    (M:MainComponent.cpp:4281, :6256), the take clock (F17), `u_bpm` uploads.
F17 VERIFIED M:src/recording/RecorderClock.cpp:11-87: the take clock is `totalBeatCount + beatPhase` plus an offset;
    while `snap.bpm <= 0` it is "unmetered": the beat value is frozen, one anchor is written on entry, and on leaving it
    the discontinuity is absorbed with a "lock" anchor (:36-52).
F18 VERIFIED M:src/MainComponent.cpp:36-43, :4846-4850: a quantised fire is queued unless Quantize is Off or the tracker
    is not LOCKED ("an honest immediate trigger (Off) beats a trigger that may never drain"). A queued fire drains on a
    count edge (FB T2 row 1).
F19 VERIFIED M:src/MainComponent.cpp:5764-5817: `applyTempoCommand` posts requests and then CAPTURES the action into a
    take as a "tempo" point (centi-BPM) unless the origin is Replay; a replayed take re-applies them (FB T1 "Take /
    routine replay").
F20 VERIFIED M:src/analysis/FeatureSnapshot.h:193-197: `trackerRequestSeq` at 328, sizeof 384. RN A5 claims offset 332
    (float) and 336 (uint8 `beatShiftState`, bits 0 and 1). Bits 2..7 of that byte are free.
F21 VERIFIED M:src/MainComponent.cpp:4349-4356 (FB Q1): with Link on, every ~30 Hz tick re-applies Link's tempo
    (`applyTempoCommand("link")` -> Manual on + `followExternalTempo`). Link's beat phase is not used; the app never
    sends a tempo to Link. A default build has no Link and the toggle is disabled (M:TopBar.cpp:134-138).
Resolume (FR Q7 with its VERIFICATION)
F22 DOCUMENTED and CONFIRMED: Tap; Resync; "plus" and "minus" ("increasing the BPM slightly by hitting the 'plus' button
    a few times"); Nudge Up / Down = a TEMPORARY tempo change while held; x2 on the BPM bar; Pause; under Link "the Resync
    and Pause buttons for the BPM are disabled". NOT DOCUMENTED: the size of a plus / minus step; what x2 and /2 do to
    the beat phase; a BPM "play" or "stop" (Resolume's page names Pause only); the BPM range (one 2009 staff post: 2 to
    500). The line that put Pause "right next to" Resync is a community user's (VERIFICATION: WRONG label), not staff.
    So "similar to resolume" is settled by HIS list, not by a Resolume page; nothing below claims to copy a Resolume
    behaviour that the sheet does not document.
The record
F23 VERIFIED BD:864-868, BD:879-897 and RN A21: questions 47, 48, 49 and 50 are ANSWERED in the record ("49 default":
    after a Resync the number reads 0). The dispatch lists them open; the record wins, as RN ruled. This lane touches 49
    only through RN A2 (unchanged) and 50 not at all (bindings travel with the saves lane).
F24 VERIFIED boris-clarify-111.md: readings R69..R73 were told to him -- R70 "The text "nudge +12 ms" sits between nudge
    back and nudge forward. "nudge 0 ms" when untouched."; R71 nudge forward = earlier = plus, nudge back = later =
    minus; R72 "/2 and *2 halve and double the tempo. /4, x1 and x4 go."; R73 minus and plus "change the tempo one step
    at a time, and the BPM number can be clicked and typed".

## 3 ITEMS

### NR1 SIGN AND TEXT (61 B)
VERIFIED: RN A8 (one constant, read in three places), RN T-U1 / T-U2 / T-U5 (the old strings), F24 (R70, R71).
Forks. (a) flip the constant and rename the text [CHOSEN]; (b) keep "+ = later" in the engine and negate at the UI.
(b) loses: two sign conventions, one in the show file and one on screen, is exactly the trap A8 exists to prevent.
THE SIGN. `kNudgePlusMeansLater = false` (src/model/BeatNudge.h; RN's "one line"). Plus = the beat comes EARLIER. The
number saved in the show ("beatNudgeMs") keeps the on-screen sign. Nothing else in A1-A7 changes.
THE STRINGS (ASCII only; hyphen-minus; one space each side of the number):
 0 -> "nudge 0 ms";  12 -> "nudge +12 ms";  -7 -> "nudge -7 ms";  500 -> "nudge +500 ms";  -500 -> "nudge -500 ms".
 The sign is printed except at 0 (R70). The text shows the TARGET the moment it changes (PL:269-270 stands).
 Pure functions (src/ui/TopBarModel.h): `juce::String nudgeText(int ms)` REPLACES `offBeatText`;
 `bool parseNudge(const juce::String& typed, int& msOut)` REPLACES `parseOffBeat`: "12" -> 12; "+12" -> 12; "-7" -> -7;
 "nudge -7 ms" -> -7; " 30 ms" -> 30; "900" -> 500; "-900" -> -500; "abc" -> false; "" -> false; "1.9" -> 1.
 `nudgeTextChanged(painted, ms)` keeps its name and rule (Pitfall 59).
WHERE IT SITS (R70): between nudge back (left) and nudge forward (right). Left = back = minus = LATER; right = forward
 = plus = EARLIER (R71).
THE TWO BUTTONS' GLYPHS. Fork: "-" / "+" (RN) against "<" / ">" [CHOSEN]. "-" / "+" loses: BPM minus and BPM plus are
 now two places to the left and carry those glyphs; two "-" "+" pairs within 100 px is a mis-press on stage. "<" / ">"
 are Pitfall 6's own examples. The known cost: play is also ">" (today's glyph, F3). It is 150 px away, inside the
 group "> || []", and the nudge ">" sits against the word "nudge". This is a named question for the critic seats
 (section 5, VG) and for Boris (B-R9), with the fallback pre-ruled: nudge back / forward become "<<" / ">>".
TOOLTIPS (exact; judged as text, RN H-8): "<" = "Move the beat 1 ms later. Hold to keep moving."; ">" = "Move the beat
 1 ms earlier. Hold to keep moving."; the text = "How far the beat is moved from the detected or tapped beat. Click to
 type a number." (A13's, unchanged). With the constant true the words "earlier" and "later" swap (T-U5's rule stands).
THE KEY / PAD TARGETS (S4, PL:369-370): "Beat earlier" / "Beat later" become "Nudge forward" (step +1) / "Nudge back"
 (step -1) -- the row's own names. PL:361's comment "(-1 earlier, +1 later)" reads "(-1 later, +1 earlier)". The
 binding step itself does not depend on the sign ("+" always raises the number: A8).
EVERYTHING IN RN THAT CARRIES THE OLD TEXT OR THE OLD SIGN: table NR5-A. The rule in one sentence: every unit case and
 live row whose nudge D is a LITERAL is run with D NEGATED and every stated sign of "pos' - pos" flips with it; a case
 that already lists both signs is unchanged; the bars are the same numbers. That keeps each case the same physical
 scenario (the same "later" or "earlier") and loosens nothing.
Mutants: M1 (sign flipped) and M12 (no "+") stand. Gate row: LR5 (replaces L11).

### NR2 THE BPM TIMER'S PLAY, PAUSE, STOP (62, 111)
VERIFIED: F3, F4 (what the three buttons do today), F11, F12, F13 (the tracker's clock and its writers), F15, F16
(the readers), F17 (the take clock), F18 (the quantised fire), F19 (take capture), F20 (the free bits), F21 (Link).
IS TODAY'S "STOP" THIS STOP? No. Today's "[]" stops routines; today's ">" and "||" run every clip (F3). He was offered
"Every clip" as 111 B and took A in his own words. The three WIDGETS are re-used for the timer and move to their place
in his row (after the wheel). The old jobs: question 127 (default A: clip play / pause leave the top bar -- each layer's
strip keeps its own, and nothing bound to a key calls them today, F4; "Stop all routines" stays as its own button
OUTSIDE the row, where it is now, left of the Bar readout, text "[]" in the reserved routine cue colour
`AudioDNALookAndFeel::kRoutineCue`, tooltip unchanged -- his 2026-09-26 ruling is on record at M:MainComponent.cpp:
624-626 and nothing he said since withdraws it).
THE THREE STATES (one enum, `BeatRun { Running, Paused, Stopped }`, src/model/BeatTimer.h, new, no juce include):
 PAUSE  the beat is held where it is: all nine published beat fields stay bit for bit; the tempo and the tracker state
        word are still published live. PLAY then lets it run on from the held place (question 111 A's words).
 STOP   the beat is put on the "1" and held: beatPhase 0, beatInBar 0, the downbeat level true, barPhase 0, barCount
        0, phrasePhase 0, resyncBarOrigin = totalBarCount. totalBeatCount does NOT change on the stop: no edge, so
        nothing quantised fires because he pressed stop. PLAY from stop makes that instant beat 1: totalBeatCount + 1
        on that hop (ONE edge with beatInBar 0 and the level true), then the beat runs. Every fire that waited lands
        on his press.
 "The BPM goes to zero": the beat's RATE is zero. The tempo NUMBER is still the tempo it will run at (question 126,
        default A), because minus, plus, /2, x2, Tap and the typed number must still work while it is stopped.
WHERE IT IS DONE. Forks:
 (a) INSIDE THE TRACKER [CHOSEN]: while not Running the tracker's beat clock does not move. Four sites in
     M:BPMTracker.cpp, all on the analysis thread: (1) `updatePhase` returns at once (this covers the free run, the
     wrap and the onset realign: F11); (2) the `scoreBeat()` call is skipped (F12: the bar position cannot move on
     an onset); (3) `applyTempoRequest` applies the tempo but skips its realign; (4) `applyResync` while not Running
     seats the "1" WITHOUT counting a beat and leaves the state Stopped. Everything else in the hop runs: in Auto the
     detector keeps listening, the tempo keeps tracking, the state word keeps changing, onsets and levels keep
     publishing.
 (b) FREEZE THE PUBLISHED SNAPSHOT above a tracker that runs on (a hold in BeatShift). Loses: on play the tracker is
     somewhere else, so the held beat cannot "run again" from its place without a second, real-valued offset between
     tracker and published beat -- the nudge machine twice; and play from stop still needs a write into the tracker.
 (c) PUBLISH bpm = 0. Loses: bpm 0 already means "no tempo" (F5 "---", F17 unmetered, M:Renderer.cpp:1661 and :1722
     fall back to the clip's OWN speed -- the opposite of "nothing moves"), and the row could not show or change the
     tempo while stopped.
 Why (a) is "one stopped beat": every reader reads the published snapshot (F15), the snapshot's beat fields come from
 the tracker through BeatShift, and a tracker that does not move publishes fields that do not move. No reader learns
 a new rule -- EXCEPT a reader that turns the tempo into a SPEED (F16), which never looked at the beat. Those are
 gated by one predicate (below).
THE REQUEST (message thread -> analysis thread; no new mutex, no allocation):
 `void BPMTracker::requestRun(BeatRun to) noexcept;` -- one compare-exchange loop on a new `std::atomic<uint32_t>`
 (the shape of `postTempoRequest`, F13), then `raiseRequestSeq()` AFTER the write (Pitfall 48). The word is a short
 QUEUE, not "latest wins": low 4 bits a count, then 2 bits per command, up to 14. Reason: "stop, play" inside one hop
 must both happen (the stop seats the "1", the play counts it); latest-wins would drop the stop. A 15th command in one
 hop (not reachable by hand) replaces the 14th. The hop takes the whole word with one exchange where it takes the
 tempo request (M:BPMTracker.cpp:79-84) and applies the commands in order. Pack / unpack are pure functions in
 BeatTimer.h (unit-tested without a tracker).
 Transitions. Running + Pause -> Paused. Paused + Play -> Running (nothing else: the clock runs on). any + Stop ->
 Stopped (`applyStop()`: the "1" block above; the phase set to 0 with NO count). Stopped + Play -> Running
 (`startFromOne()`: `++totalBeatCount_`, the "1" block again). Stopped + Pause -> Stopped. Running + Play -> Running.
 Views for the publish step (analysis thread only): `BeatRun runState() const`, `uint32_t appliedStops() const`,
 `uint32_t appliedStarts() const` (counters of stops and of starts-from-stop applied; `appliedResyncs()` is NOT
 changed -- it stays RN A4's).
BEATSHIFT: WHAT THE STOPPED TIMER NEEDS (amends RN A1 / A2; the only re-opening of S1). `Flags` gains `run` (the
 tracker's state after this hop), `stopApplied`, `startApplied` (this hop). Three additions, in A1's step order:
 Step 0 adds: the anchor is also re-taken on a hop with `stopApplied` or `startApplied`.
 Step 2b, THE STOP HOP (`stopApplied`; it takes precedence over step 2 when both a Stop and a Resync were applied):
   the snapshot gets S's own fields (the "1" block) with totalBeatCount' = F.totalBeatCount EXACTLY (not A2's
   [F, F + 1]: a stop is never an edge), totalBarCount' = max(F.totalBarCount, S.totalBarCount), resyncBarOrigin' =
   totalBarCount', the sequence S's, not held. A pending zero request is consumed and Da = 0 (see "the nudge at a
   stop"); without one Da is kept. When either counter differs from S's, `apply` returns them and AnalysisThread calls
   `adoptCounters` -- the SAME one call site (T-N14's lint stands).
 Step 2c, THE HELD LEVEL (`run` is not Running, and this is not the stop hop): Da still glides toward D (step 3), so
   a nudge dialled while the timer is held is settled when it starts; the nine published fields are F's, bit for bit;
   the sequence is S's; the held bit is NOT set (so a Tap or a typed tempo made while stopped is acknowledged at
   once: Pitfall 48 and A3's promise are about a HOLD, and A20's "no hold is unbounded" stays true). In the identity
   state (never engaged, D == 0) step 1 returns first and no byte is written: the tracker's own fields are already
   still.
 A start from stop (`startApplied`) is treated by steps 0 and 6 exactly as a tracker Resync that is not a restart:
   relabelPending is set at S.totalBeatCount. With a nudge of 0 the published beat is the tracker's own and the edge
   is on his press; with an EARLIER nudge the edge is on his press and the phase starts at the shift; with a LATER
   nudge the edge comes that many ms after his press -- which is what a later nudge means.
 Nothing else in A1-A3 changes. `adoptCounters` gains no caller. If a unit case cannot be made green without a rule
 that is not written here, A24 holds: the builder stops.
THE NUDGE AT A STOP (reading R74, to be told to him; his "49 default" and BD:855-857 are the ground): a stop followed
 by play is a fresh start by hand, like Resync, so the nudge reads 0 from the moment of the STOP. Built as A2's two
 message-thread statements in the Stop path (`composition_.beatNudgeMs = kNudgeAfterStop` (0), then
 `requestBeatNudgeZero()`, BEFORE `requestRun(Stopped)`), human origin only. If he says the number should stay: those
 two statements are deleted -- steps 2b and 2c already handle a kept nudge (T-N19). Pause never touches the number.
THE PUBLISHED STATE. Two more bits of RN A5's `beatShiftState` byte (offset 336; F20): bit 2 = Paused, bit 3 =
 Stopped. AnalysisThread ORs them in from `runState()` on every hop, where it already writes the byte. At rest they
 are 0, so RN's golden (bytes 332..383 zero on a never-touched run) stands unchanged. `GET /api/features` gains the
 read-only key "beatTimer": "running" | "paused" | "stopped" from the same coherent snapshot. `bool
 beatRunning(uint8_t state)` (BeatTimer.h) is the ONE predicate every rate reader and the UI use.
THE RATE READERS (F16) -- "nothing moves" for what takes a SPEED from the tempo:
 - A BPM-synced image sequence (M:Renderer.cpp:1718-1734) and a BPM-synced video (M:Renderer.cpp:1657-1666): while
   `beatRunning` is false the clip HOLDS its frame, by the call a paused clip uses (the builder names the line in
   `ClipTransportSync::pushIntent` / the player; INFERRED that a held player shows its last frame without going
   "pending": Pitfall 56 says "a hold is not pending"). The clip's own play / pause setting is not written (his
   answer 72: pause is the clip's). A clip that is NOT in BPM Sync is not touched: "they play just as they were".
 - Harmony constraint (proposed, for the transport lane): RT's `TempoView` gains `bool running` read from this byte;
   the bar lock and the pushed speed hold while it is false. Whichever of the two lanes merges SECOND makes this its
   step 0; the predicate and the bit are this lane's.
 - The ONE shader that scales `u_time` by `u_bpm` (M:EmbeddedShaders.h:10793) is time-driven at a tempo-scaled speed;
   it has no beat position to hold. NOT gated here: said to Boris in B-R4 and listed in section 9. (Gating it needs a
   render-side accumulated time uniform; uploading `u_bpm` = 0 would make it run at its own default speed.)
 - Lint T-G12: outside a named whitelist, no file in src reads `.bpm` off a snapshot into a speed without
   `beatRunning(` in the same function (the whitelist is F16's list of non-motion readers).
IN AUTO. The detector keeps listening (only the four sites are gated). It NEVER restarts the timer by itself: in Auto
 music is always playing, so a stop that un-stops on the next kick is not a stop (runner-up: auto-restart; loses for
 that reason). On play from PAUSE the phase runs on from its held place and the tracker's own next confident onset
 realigns it (F11) -- the beat rejoins the music within a beat, by a rule that already exists. On play from STOP the
 press is beat 1 and the detector then owns the phase again, as after a Resync in Auto today (RN SF-2).
IN MANUAL: as above; nothing realigns except his own gestures. UNDER LINK: the three work as in Manual -- this app
 follows Link's TEMPO only and never sends a beat or a tempo to Link (F21), so the reason Resolume disables Pause and
 Resync under Link (F22) does not exist here.
TAP AND RESYNC WHILE NOT RUNNING (question 128, default A): the stop is sticky. A Tap sets the tempo and moves nothing
 (site 3). A Resync seats the "1" and the state is Stopped -- from Paused too (site 4); it zeroes the nudge as any hand
 Resync (A2). Only play starts the timer. B (they start it) is ONE constant, `kGesturesStartTimer`, read at sites 3
 and 4 (true: a realign or a Resync while held runs `startFromOne()` and the state is Running).
A QUANTISED FIRE WHILE NOT RUNNING (question 129, default A): it waits -- that is what the beat's rule gives with no
 code -- and lands on the edge play makes (from stop: on his press; from pause: on the next beat). A fire with
 Quantize Off acts at once, as always. B (fire at once) is one line beside the "not LOCKED" rule (F18):
 `quantizeModeToForcedSnap` returns Off when `beatRunning` is false, and the two routine sites' `locked` argument
 (M:MainComponent.cpp:4281, :6256) gets the same term.
WHAT A TAKE RECORDS. The three are NOT captured and NOT replayed (reading R75): a replayed routine whose own clock is
 the beat would stop the beat it replays on and never reach its play. So `setBeatTimer` (below) never calls
 `recorderHost_.capture`. While a take is being recorded with the timer held, the take clock treats the snapshot as
 unmetered: one term in M:RecorderClock.cpp:33 (`isUnmetered = bpm <= 0 || !beatRunning(state)`), which gives the
 frozen beat, one "unmetered" anchor and a "lock" anchor on play -- the path that exists (F17). BPM minus / plus, /2,
 x2 and a typed number ARE captured, as the "manual" tempo point they already are (F19).
WHERE THE STATE IS KEPT: in the tracker (the truth) and in the published byte (what everyone reads). NOT in the show,
 not in settings: every launch and every opened show starts Running (a show that opens on a dead beat looks broken,
 and his words do not put it in the show). New and Open do not change it.
THE MESSAGE-THREAD ENTRY: `void MainComponent::setBeatTimer(BeatRun to, Origin origin);` -- the Stop's two nudge
 statements, then `tracker->requestRun(to)`. Callers: the three buttons, the key / pad action (NR4), the debug route.
HOW THE ROW SHOWS IT (a state display, model-driven from the published byte on the 15 Hz timer; Pitfalls 41, 59): the
 button of the CURRENT state is lit with the highlight the old multiplier buttons used (accent cyan at 0.3 alpha,
 M:TopBar.cpp:169-170): play lit = running, pause lit = paused, stop lit = stopped; exactly one is lit. The wheel
 needs nothing: stopped, it shows beat 1 lit at full brightness and still; paused, it is still where it was. No text
 appears and nothing announces the change.
Tests: T-G1..T-G12, T-N17..T-N20, T-R2 (section 5). Mutants MR1..MR12. Gate rows LR1, LR2, LR3.

### NR3 BPM NUMBER, MINUS, PLUS, /2, x2
VERIFIED: F5, F6 (the number and the field today), F7 (the dead buttons), F9, F10 (the fold), F14 (Auto's octave
correction), F21 (Link), F22 (Resolume's step: not documented), F24 (R72, R73).
THE FACTS BEFORE THE CHOICE (F10): wired as tempo requests on today's tracker, x2 would do nothing above 100 BPM, /2
nothing below 120, and neither anything between 100 and 120; minus at 60 would jump to 118 (59 folds up) and plus at
200 would jump to 100.5. In Auto any tempo request is taken and then fought by the detector within about 2 s
(hysteresis), or kept at the wrong octave (F14).
THE FOLD. Forks: (a) a hand-set tempo is CLAMPED into its own range and never folded; the detector and Tap keep the
 fold [CHOSEN]; (b) remove the fold everywhere; (c) keep the fold and make /2 and x2 a multiplier on the published
 beat (the old `bpmMultiplier` idea made live). (b) loses: the fold is the detector's octave guard. (c) loses: R72
 told him they "halve and double the tempo", the number would not show what runs, and it needs a second multiply at
 the publish step.
 - `void BPMTracker::setHandTempo(float bpm) noexcept;` -- a third request kind: one more flag bit in the existing
   request word (`kTempoExact`), never a realign. `applyTempoRequest` clamps an exact request into
   [kHandMinBPM = 30, kHandMaxBPM = 400] and folds every other request as today. 30..400 is the smallest range in
   which ONE halving and ONE doubling of any detector tempo (60..200) works. Both are named constants in BeatTimer.h.
 - Callers of the exact path: the typed number, minus, plus, /2, x2 -- through `applyTempoCommand("manual", bpm)`,
   whose "manual" branch calls `setHandTempo` instead of `followExternalTempo`. Tap (`setManualBPM`), Link, REST
   `/api/set_bpm` and OSC keep `followExternalTempo` / the fold, unchanged (not this lane's; said in section 9).
 - Back to Auto: on the first hop that sees Manual off with the locked tempo outside 60..200, the tracker folds it
   back (one statement in `runPipeline`'s Auto path, analysis thread). So Auto never runs outside the detector's
   range and F14's sticky octave cannot happen.
 - What a tempo outside 60..200 meets downstream: BeatShift's rule is tempo-free (the shift in beats is ms x bpm /
   60000); at 400 BPM a 500 ms nudge is 3.33 beats and A20's hold bound still holds (T-N7b adds the arms). RT's tables
   assume 60 as the slowest tempo (RT V10): at 30 its worst catch-up doubles. That is the transport ruling's to
   restate; it is listed in section 8.
WHAT EACH DOES (pure functions in src/model/BeatTimer.h; base = the published tempo, or 120 when there is none):
 /2    -> base / 2 when that is >= 30; otherwise the button is DISABLED (a greyed state, not a silent clamp: a clamp
          would break "/2 then x2 comes back"). `bool canHalve(float)`.
 x2    -> base x 2 when that is <= 400; otherwise disabled. `bool canDouble(float)`. Halving and doubling a float
          are exact, so /2 then x2 returns the same tempo bit for bit.
 minus -> the next WHOLE number below (127.6 -> 127; 128 -> 127); disabled at 30. plus -> the next whole number above
          (127.6 -> 128; 128 -> 129); disabled at 400. Question 125 (default A: whole numbers; B: steps of 0.1) --
          one function pair, `bpmStepDown(float)` / `bpmStepUp(float)`. Held: the nudge buttons' repeat (400 ms, then
          every 50 ms: A15's two constants).
 /2 and x2 never move the beat's phase or the bar position (a tempo VALUE never realigns: M:MainComponent.cpp:
 5762-5763's rule); the beat simply runs half or twice as fast from where it is.
IN AUTO: any of the five (minus, plus, /2, x2, a typed number) takes the tempo by hand: Manual switches ON (the toggle
 shows it, the state word hides) and the new tempo is applied. Fork: greyed in Auto. Loses: five of his thirteen
 controls would look dead whenever the app listens, and "press x2, it doubles" is what the row promises; REST
 `set_bpm` already forces Manual the same way (FB T1). Told to him as reading R76.
UNDER LINK (toggle on): the five are DISABLED, tooltip "Link sets the tempo" -- Link re-applies its tempo 30 times a
 second (F21), so they would be inert, and no inert control ships. A default build has no Link, so this state exists
 only in a Link build (unit-tested; not captured -- Harmony's decision HR-4).
THE NUMBER SHOWN AND TYPED. One `juce::Label`, editable on a click, REPLACING both the tempo label's old rules (F5)
 and the BPM field (F6), in every mode:
 - shown: `juce::String bpmText(float bpm)` -- no tempo -> "---"; else the tempo rounded to 0.1, printed without the
   decimal when it is whole ("128", "127.6"). It is ALWAYS the published tempo, never the typed text: typing 500
   shows "400". Colour: by the tracker state in Auto (as today); the primary text colour in Manual.
 - typed: the editor opens holding the shown number, all selected. Enter: `bool parseBpm(const juce::String&, float&)`
   -- the first number in the text, decimals allowed, clamped to 30..400; nothing parseable or a value <= 0 leaves the
   tempo unchanged. Esc and a loss of focus cancel. All three hand the keyboard back to MainComponent by A13's call
   (`onBpmEditEnded`, the twin of `onNudgeEditEnded`). This removes RN SF-4 (the old field kept the keyboard) as a
   consequence, not as extra work.
 - Manual ON (the toggle) now takes the tempo SHOWN (or 120 when there is none), not the old field's stale text
   (F6): the field no longer exists.
 - F5's side effect goes with the rewrite: FPS and DSP update in Manual too.
WHAT GOES: the buttons "/4", "x1", "x4" and `handleMultiplierButton`, `onBpmMultiplierChanged`; `Composition::
 bpmMultiplier` (M:Composition.h:142, :229): the key is no longer written; a show file that has it loads and the key
 is ignored (T-C4); M:tests/test_composition.cpp:157 and :194 are replaced by that case. "/2" and "x2" are re-wired
 as above. The label stays "x2" (today's glyph, ASCII; his "*2" is his shorthand -- B-R9 asks; one string).
Tests: T-RB1..T-RB6, T-G9..T-G11, T-C4. Mutants MR7, MR8, MR13..MR15. Gate row LR4.

### NR4 THE ROW ON SCREEN
VERIFIED: F1, F2, F2b, F8; CLAUDE.md "UI Patterns"; Pitfalls 6, 34, 41, 57, 59.
THE ORDER (px; every width is a named constant in TopBarModel.h, one line each to change):
 outside, left:  Audio block 240 (unchanged) | "Stop all routines" 24 + 6 (question 127 A) | "Bar N" 44 + 2
 THE ROW:        wheel 26 + 2 | play 24 + 1, pause 24 + 1, stop 24 + 4 | BPM number 56 + 2 | "-" 20 + 1, "+" 20 + 4 |
                 "<" 20 + 1, the nudge text max(88, measured "nudge -500 ms" + 8) + 1, ">" 20 + 4 | "/2" 26 + 1,
                 "x2" 26 + 4 | Tap 32 + 2 | Resync 50 + 6                                   = 490 px
 outside, right: the state word 60 + 2 (Auto only; its place is kept) | Manual 80 + 2 | Link 50 + 6 | "Quantize:" 55,
                 combo 100 + 6 | then the right block as today.
 Why "Bar N" sits LEFT of the wheel: it is not in his list, so it cannot sit inside the row; and the 15 Hz repaint
 rect is the union of the wheel and the Bar text (F8) -- next to each other the rect stays about 76 px wide; anywhere
 else it would span the row and repaint it 15 times a second (Pitfall 57). The "Tempo" caption painted above the
 number (M:TopBar.cpp:442-449) stays: a caption, not a control. The state word moves right of Resync, beside Manual,
 whose state it is.
THE WIDTH BUDGET (COMPUTED from F1 and the widths above; the pixels are VG-0's and VG's): left of the right block =
 240 + 30 + 46 + 490 + 200 + 161 = 1167; the right block about 522; total about 1689 against an inner width of 1720 at
 his 1728 (F2b): it FITS, with about 31 px to spare (today: 1553 / 1617). At 1280 it does not, as today's does not (F2).
A NARROW WINDOW. Today the bar is cut blindly from the right (F2): at 1280 Master and Master Signal vanish and Outputs
 is clipped with Manual on. Rule: `TopBarFit fitTopBar(int innerWidth, int signalLabelWidth)` (pure, TopBarModel.h).
 The row, the Audio block, the routines stop, "Bar N", Manual, Link, the Quantize combo and the Outputs button are NEVER shed (1050 + 104 px: they
 fit at 1280 with 118 to spare). The rest is shed WHOLE (hidden, never clipped), in this order, until what is left
 fits: DSP, FPS, the "Quantize:" caption, the state word (the number's colour still carries it), Master Signal (label
 + slider), Master (label + slider). COMPUTED: at 1728 nothing is shed; at 1512 DSP, FPS, the caption and the state word (need 185; the first
 three give 161, the four 223); at 1280 all six (need 417). Master and Master Signal stay reachable on the Composition tab
 (M:TopBar.cpp:252-254 calls the fader "a SHORTCUT"). This is the only change outside the tempo cluster and it is
 forced: without it the wider row would push Outputs off a 1280 window.
 Runner-up: leave the blind cut and accept it. Loses: the Outputs button is one of the two homes of the output list
 (CLAUDE.md "Outputs") and would be the first thing lost.
WIDGETS. Buttons are `juce::TextButton`s; the two editable texts are `juce::Label`s (A13's pattern, lossOfFocus
 discards). No slider is added (so no ResettableSlider question arises); no popup menu is added. The nudge "<" ">"
 and BPM "-" "+" repeat while held (A15's constants); the others do not.
TOOLTIPS (exact strings; read from ui_text, judged as text):
 play   "Run the BPM timer. From stop it starts on the 1."
 pause  "Hold the BPM timer where it is. Everything set to BPM waits."
 stop   "Stop the BPM timer on the 1. Everything set to BPM waits until play."
 number "The tempo. Click to type a number."
 "-"    "Tempo down one step. Hold to keep going."     "+"  "Tempo up one step. Hold to keep going."
 "<" ">" and the nudge text: NR1.     "/2" "Half the tempo."     "x2" "Double the tempo."
 Tap and Resync keep today's (M:TopBar.cpp:55, :82). A control disabled at a range end keeps its tooltip; under Link
 the five say "Link sets the tempo". "[]" outside the row keeps "Stop all routines".
KEYS AND PADS. Today: Tap, Resync (F4). By RN's S4: nudge back / forward. NEW, ruled: seven more targets -- "Beat
 play", "Beat pause", "Beat stop", "Tempo -", "Tempo +", "Tempo /2", "Tempo x2" -- as ONE appended action,
 `Binding::Action::TempoRow` (after BeatNudge), with `int targetTempoRowOp` saved as "targetTempoRowOp" (absent -> 0).
 A key or a note fires the op on the press; a release does nothing; a MIDI CC does nothing and cannot be learned
 (`bindingIsLive` gains the action: the same refusal as the nudge, A16). Why: play on the "1" and x2 at a climax are
 things a hand does on a pad, not with a mouse. Runner-up: no keys for the new seven in this lane (Harmony's HR-2 can
 take it: stage S4r is the only thing that goes). The old overlay target "Stop" (routines) is renamed "Stop routines"
 so it cannot be taken for "Beat stop" (one string; HR-3).
TEST-SERVER ROUTES AND KEYS (compiled out otherwise; they drive the buttons' own handlers, never synthetic input):
 `POST /api/debug/tempo_row` {"op": "play" | "pause" | "stop" | "bpm_minus" | "bpm_plus" | "half" | "double" |
 "nudge_back" | "nudge_forward"}; `POST /api/debug/bpm_edit` {"op": "begin" | "type" | "enter" | "escape" |
 "focus_lost", "text"} (the twin of A13's `nudge_edit`); `GET /api/debug/ui_text` gains "tempo_row": {"order": [the
 thirteen names with x and w], "bpm_text", "timer", "lit": the one lit button, "enabled": {minus, plus, half, double},
 "manual", "shed": [what the fit hid], "tooltips": {...}, "bpm_editor_open"} beside A13's "nudge" (whose "tooltip_minus"
 / "tooltip_plus" become "tooltip_back" / "tooltip_forward") and A19's "wheel_beat".
STATES THE VISUAL GATE CAPTURES (window-only captures of the lane's own app; each has a route above or RN's):
 BASELINE (VG-0, before the top bar is touched): V18 today's bar at 1280 and at the default launch size, Manual off
 and on; V19 today's learn overlay. Unchanged from RN, plus V18b: the bar at 1512 wide (the shed order's middle case).
 R1 running, Auto LOCKED, whole tempo, "nudge 0 ms" (default size)   R2 "nudge +12 ms"   R3 "nudge -7 ms"
 R4 "nudge +500 ms"   R5 "nudge -500 ms"   R6 the nudge editor open   R7 the BPM editor open, number selected
 R8 PAUSED mid-bar (pause lit)   R9 STOPPED (stop lit, the wheel on beat 1)   R10 Manual, "127.6"
 R11 Manual 400 ("+" and "x2" greyed)   R12 Manual 30 ("-" and "/2" greyed)   R13 no tempo ("---", SEARCHING)
 R14 1280 wide, Manual on, stopped (all six shed)   R15 1512 wide   R16 the default launch size, Manual on
 R17 the keyboard bind overlay with the nine targets (two nudge, seven row)   R18 the MIDI-learn overlay with them
 R19 a row target selected in learn   R20 the standing learn title (RN V17).
 Dropped as captures: tooltips (text); a pressed button; the Link-on greyed state (no Link in the default build).
 The manifest carries, per state: the model facts (tempo, nudge, timer), every top-bar widget's bounds, each text's
 width against its box, and the shed list.

### NR5 THE CHANGE LIST
NR5-A. WHAT CHANGES IN RN (old -> new). Everything not listed STANDS.
| RN item | old | new |
|---|---|---|
| A8 | `kNudgePlusMeansLater = true`; `offBeatText` | `= false`; `nudgeText`; the constant is read in the same three places |
| A1 step 4 | "-Da x bpm / 60000 beats when plus means later" | unchanged text; with the constant false the shift is +Da x bpm / 60000 |
| A1 / A2 | Flags = { predicted, locked, phraseBars, resyncs } | + run, stopApplied, startApplied; steps 0, 2b, 2c added (NR2) |
| A4 | two views, one write | + `runState()`, `appliedStops()`, `appliedStarts()`, `requestRun`, `setHandTempo`, the four gated sites, the Auto re-fold |
| A5 | beatShiftState bits 0, 1 | + bit 2 Paused, bit 3 Stopped; features key "beatTimer" |
| A13 | "-" / "+" buttons; tooltip_minus / tooltip_plus | "<" / ">"; tooltip_back / tooltip_forward; the text's tooltip unchanged |
| A14 | states V1-V11, V14-V17 | R1-R20 (NR4); V18, V19 stand; V18b added |
| A17 | anchor "nudge" | stands; + T-R2 (held timer = unmetered) |
| T-N1 | "D = +250, plus means later: pos' - pos = -0.5 ... The same two with plusMeansLater = false: the signs swap" | stands as written (it runs both values) |
| T-N2 | "pos' - pos = -D x bpm / 60000"; "pos'(D + 1) - pos'(D) = -bpm / 60000" | "+D x bpm / 60000"; "+bpm / 60000" |
| T-N3 | "within 1.0 ms of D" | "within 1.0 ms of -D" (plus = earlier: the edge comes D ms sooner) |
| T-N7 | D = +500 in all arms; "= -500 x 126 / 60000"; "delta goes from -1.0 to -1.667" | D = -500 in all arms; the same two numbers |
| T-N8 | "at least one arm adopts (-500 at phase 0.2)"; bar-counter arm "D = -40"; M23 "(-500 at 0.2)" | "+500 at phase 0.2"; "D = +40"; "(+500 at 0.2)" |
| T-N8b | "D = +250 kept" | "D = -250 kept" |
| T-N8c | "D = +40" | "D = -40" |
| T-N13 | "D = +500 fully applied while bpm is 0" | "D = -500" |
| T-N16 | "pos' - pos = -(the target of that hop) x 120 / 60000" | "+(the target of that hop) x 120 / 60000" |
| T-N5, T-N5b, T-N6, T-N9, T-N11, T-N12, T-N14, T-N15, T-N8d, T-N10a-d | -- | stand (both signs listed, or no sign) |
| T-U1 | the five "off beat by ..." strings | "nudge 0 ms", "nudge +12 ms", "nudge -7 ms", "nudge +500 ms", "nudge -500 ms" |
| T-U2 | `parseOffBeat`, "off beat by -7 ms" -> -7 | `parseNudge`, "nudge -7 ms" -> -7; the other nine arms unchanged |
| T-U4 | "off beat by +12 ms" with 12 / 13 | "nudge +12 ms" with 12 / 13 |
| T-U5 | "-" = earlier, "+" = later (plus means later) | "<" = "Move the beat 1 ms later. Hold to keep moving."; ">" = "... earlier ..." (constant false); swap rule stands |
| T-B1..T-B6, T-M, T-C1-3, T-R1 | -- | stand; overlay labels "Nudge forward" / "Nudge back" |
| L2 | "measured = -r x 60000 / bpm"; mut-sign "measured near -D" | "measured = r x 60000 / bpm"; mut-sign "measured near -D" (stands) |
| L3a | "nudge +250" | "nudge -250" (the same half beat LATER; the 0.40..0.70 window stands) |
| L3c | "nudge +250" | "nudge -250" (0.10..0.18 stands) |
| L3d | "nudge 0 then +3" | "0 then -3" |
| L4 | "then +500" | "then -500" (0.24..0.28 stands) |
| L1, L6, L7, L8, L9, L10 | -- | stand (L7's steps raise and lower the NUMBER; L9 lists both signs) |
| L11 | "off beat by +12 ms"; "no nudge in ui_text" | REPLACED by LR5 |
| VG | MUSTs on T-U1's text, T-U5's tooltips | REPLACED by the VG row of section 5 |
| S3b | two buttons + an editable text; five buttons hidden | REPLACED by S3m + S3r |
| B1-B13 | "+" later; "off beat by ..." | REPLACED by B-R1..B-R12 where they name a sign, the text or the row; B5, B10, B11, B12, B13 stand |
| question 62's "line that changes" (RN:747-748) | the group moves by one constant | void: the row is his list |
NR5-B. WHAT STANDS UNTOUCHED: G-N0 and the golden (the gate is 0 at rest); A1's steps 1, 3-7 and its guarantees; A2's
restart; A3; A6; A7; A9-A12; A15-A24; S1's and S2's file lists (S1r and S2r ADD to them, in their own contexts); S3a;
S4's mechanism; S5's scope plus the lines of section 4; H-1..H-12.
NR5-C. NEW STAGES: S1r, S2r, S3m, S3r, S4r (section 4).

## 4 STAGES + ORDER
One worktree and branch for the whole nudge lane (RN section 4); no two builders in it at once. A builder builds, runs
unit tests, writes probes and their self-tests; he never runs a live row and never gives a gate verdict. Harmony runs
G-N0, G-N1, every L and LR row, the mutant apps, VG-0 and VG herself.
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| G-N0 | Harmony | as RN | -- | as RN (unchanged) |
| S1 | builder | as RN, with `kNudgePlusMeansLater = false` and NR5-A's literals | this delta ruled | as RN |
| S1r | builder | THE TIMER AND THE HAND TEMPO, unwired. src/model/BeatTimer.h (new, no juce: `BeatRun`, the state bits, `beatRunning`, the command word's pack / unpack, kHandMinBPM / kHandMaxBPM, `canHalve`, `canDouble`, `bpmStepUp`, `bpmStepDown`, kGesturesStartTimer, kNudgeAfterStop); src/analysis/BPMTracker.h/.cpp (`requestRun`, the queue word, the four gated sites, `applyStop`, `startFromOne`, the three views, `setHandTempo` + `kTempoExact`, the Auto re-fold); src/analysis/BeatShift.h/.cpp (the three Flags, steps 0 / 2b / 2c); tests/test_beat_timer.cpp (new: T-G1..T-G11), tests/test_beat_shift.cpp (T-N17..T-N20, T-N7b); CMake lines for the new test only. | S1 green; Harmony's order against the transport lane's S4t (one builder in BPMTracker.cpp at a time: RN H-10) | T-G and the new T-N cases RED on a tree where `requestRun` and `setHandTempo` are stubs that do nothing, then GREEN; MR1-MR11 each RED; the golden still green (the gate is 0 at rest). Proves the timer and the range on a REAL tracker. |
| S2 | builder | as RN | S1, G-N0 | as RN; then Harmony: G-N1, L1-L4, L6, L9, L10 with NR5-A's values |
| S2r | builder | WIRING. src/analysis/AnalysisThread.cpp (the three Flags read from the tracker; bits 2, 3 into the state byte); src/MainComponent.h/.cpp (`setBeatTimer`; the "manual" branch -> `setHandTempo`; the debug callbacks); src/recording/RecorderClock.cpp (one term) + its test (T-R2); src/render/Renderer.cpp (the two BPM-Sync branches hold while `beatRunning` is false -- or, if the transport lane merged first, `TempoView::running`: Harmony says which at dispatch); src/api/ApiServer.h/.cpp ("beatTimer" in features; `POST /api/debug/tempo_row` for play / pause / stop / half / double / bpm_minus / bpm_plus, calling the same MainComponent functions the buttons will); tests/test_rate_reader_lint.cpp (T-G12); .harmony/probe-nudge-row.sh / .py / -selftest.py (LR1-LR4; it sources probe-quit-ours.sh, takes the live lock, quits only its own pid); build scripts for build-mut-gate (MR1 + MR3) and build-mut-fold (MR7 + MR12). | S1r, S2's rows green | Unit green; the probe self-test prints "0 case(s) differ". Then Harmony: G-N1, LR1, LR2, LR3, LR4 (LR4's UI clauses wait for S3r), L10's new lines. Proves the wired app: one stopped beat for every reader; the tempo controls move the real tempo. |
| S3a | builder (short) | as RN | S2r's rows green | as RN |
| VG-0 | capture builder | as RN, plus V18b (1512 wide) | S3a | the manifest: F2's arithmetic against pixels; the size he runs |
| S3m | builder (short) | THE ROW'S MODEL, no widget. src/ui/TopBarModel.h (`nudgeText`, `parseNudge`, `nudgeTextChanged`, `bpmText`, `parseBpm`, the width constants, `rowOrder()`, `fitTopBar`, the tooltip strings); tests/test_topbar_model.cpp (T-U1, T-U2, T-U4, T-U5 as amended; T-RB1..T-RB5). | VG-0 | The cases RED on stubs, then GREEN; MR13-MR15, M12, M13 RED. Proves every string and the layout arithmetic before a pixel moves. |
| S3r | builder | THE ROW. src/ui/TopBar.h/.cpp (the cluster from the routines stop to Quantize in `resized()`; the right block laid by `fitTopBar`; the three timer buttons re-wired and lit from the published byte; the BPM label and its editor; "-" "+"; "<" text ">"; "/2" "x2"; the five-button code, `handleMultiplierButton` and the BPM field removed; `updateBpmDisplay` rewritten; every timer-driven set only when what it would paint differs); src/model/Composition.h (bpmMultiplier: the member, its reset, its save line removed; the load line reads and drops the key) + tests/test_composition.cpp (T-C4); src/MainComponent.cpp (the callback assignments beside :587-628; `onNudgeEditEnded`, `onBpmEditEnded`); src/api/ApiServer (`nudge_edit`, `bpm_edit`, the rest of `tempo_row`, ui_text "tempo_row" and "nudge"); tests: test_topbar_row.cpp (new: T-U6, T-RB6, T-RW1..T-RW4); the three existing top-bar tests kept green (test_master_signal_link.cpp, test_topbar_link_toggle.cpp -- its "Stop all routines" case follows question 127's answer). | S3m | Unit green. Then Harmony: G-N1, LR4 whole, LR5. |
| S4 | builder | as RN, labels per NR1 | S3r | as RN; then Harmony: L7, L8, L11 -> LR5 |
| S4r | builder (short) | THE SEVEN TARGETS. src/binding/Binding.h (`Action::TempoRow` appended after BeatNudge; `targetTempoRowOp`), BindingManager (`bindingIsLive`; toVar / fromVar), the two overlays' target lists, MainComponent (`buildBindableTargets`: seven targets; the old "Stop" label -> "Stop routines"; the `handleBindingAction` case), ApiServer (`binding_action` accepts "tempoRow"), tests/test_binding_tempo_row.cpp (new: T-B7..T-B10). Same worktree, after S4. | S4's rows green; HR-2 | Unit green; MR16, MR17 RED. Then Harmony: LR6. |
| VG | capture builder, five critic seats; Harmony's verdict | R1-R20 against V18, V18b, V19, with the manifest of NR4 | S4r's row green | Row VG. Boris sees nothing before it is green. |
| S5 | builder | DOCS + MANUAL as RN, plus: a second pitfall ("Pitfall NN: the BPM timer is the tracker's own clock held at four sites; a beat reader needs no new rule; a reader that turns the tempo into a SPEED must test `beatRunning`; play from stop is the one place a beat is counted by hand; a stopped timer is never saved"); docs/claude/effects.md "Manual BPM Mode" (the number, the hand range, Manual-on takes the shown tempo); performance-controls.md (the row, the seven targets); integration.md ("beatTimer"); testing-eyes.md (the new debug routes and keys); recording.md (the timer is not captured; a held timer is unmetered; the routines stop's new home); CLAUDE.md's capability line and pitfall index; the manual's tempo-row page. | VG green | Harmony reads the manual against the built app. |
Order: G-N0 -> S1 -> S1r -> S2 -> S2r -> Harmony's rows -> S3a -> VG-0 -> S3m -> S3r -> S4 -> S4r -> Harmony's rows -> VG ->
S5 -> merge by RIG-RULES' merge sequence. S1r may instead follow S2 if Harmony wants the nudge's own rows green first;
it must precede S2r. Reviews: the pinned review after S2r and after S4r. Real-time lens on S1r / S2r: `requestRun` and
`setHandTempo` are one CAS loop each, lock-free, no allocation; the four gated sites add one branch each on the
analysis thread; nothing touches the audio callback; no mutex is added; the render thread reads one more byte of the
snapshot it already has.
What two lanes must not both touch: BPMTracker.cpp (this lane's S1, S1r; the transport lane's S4t); Renderer.cpp's
BPM-Sync branches (this lane's S2r; the transport lane's rebuild) -- one order, Harmony's.

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened; each has its RED arm)
RN section 5 stands with NR5-A's replacements. NEW cases:
UNIT, tests/test_beat_timer.cpp (S1r; a REAL BPMTracker driven hop by hop, 512 samples at 48 kHz, as RN's cases).
 RED arm for the file: `requestRun` and `setHandTempo` stubbed to do nothing.
 T-G1  PAUSE HOLDS, PLAY RUNS ON. Manual 120; Pause posted at tracker phase 0.3 and at 0.8. For 200 hops after the hop
       that applies it: the nine beat fields equal their values on that hop bit for bit; bpm is 120. Then Play: on the
       first hop the phase is the held phase + one hop's advance within 1e-6; the count never falls; 16 beats later the
       bar position has run 0, 1, 2, 3 in order from where it was. MR1 RED.
 T-G2  STOP IS THE 1 AND NOT AN EDGE. Manual 120; Stop at phase 0.2 and 0.7, at beatInBar 2: on the applying hop
       beatPhase == 0.0f, beatInBar == 0, the level true, barPhase == 0.0f, barCount == 0, phrasePhase == 0.0f,
       resyncBarOrigin == totalBarCount, and totalBeatCount equals its value on the hop before. 200 hops later all
       the same. MR2 RED (at 0.7).
 T-G3  PLAY FROM STOP IS THE EDGE. After T-G2: Play. On the applying hop totalBeatCount is exactly + 1, beatInBar 0,
       the level true, beatPhase < 2 hops' advance; the next three count edges show beatInBar 1, 2, 3; totalBarCount
       never falls. MR3 RED.
 T-G4  AUTO: THE DETECTOR LISTENS, THE BEAT DOES NOT MOVE. Auto, downbeat locked, RN T-N5's LATE sequence (a confident
       onset every beat). Pause; 400 hops of onsets: the nine beat fields never change; `rawBPM()` / the confidence
       keep updating; the state stays Paused (nothing restarts it). Play: within one beat of onsets the phase is
       realigned by an onset and the count has risen by 0 or 1 per hop throughout. MR4 RED (beatInBar moves).
 T-G5  TAP WHILE HELD. Stopped; `setManualBPM(140)`: bpm 140 on the next hop; the nine beat fields unchanged; the
       state Stopped. Paused at phase 0.7: the same, the count unchanged. MR5 RED. A second arm with
       kGesturesStartTimer passed as true: the state is Running and the count rose by exactly 1.
 T-G6  RESYNC WHILE HELD. Paused at beatInBar 2, phase 0.7; `requestResync()`: the "1" block of T-G2, the count
       unchanged, the state Stopped, `appliedResyncs()` + 1. Stopped: the same, nothing changes but the counter.
 T-G7  THE QUEUE. Between two hops: Stop then Play -> the next hop ends Running with the count + 1 and beatInBar 0.
       Pause then Play -> Running, fields as if nothing was posted but one hop held. 20 commands in one gap -> no
       crash, the state is the last command's. Pack / unpack round-trip for every length 0..14. MR6 RED.
 T-G8  THE SEQUENCE. After each `requestRun` the posted sequence rises; the hop that applies it latches it (the
       pattern of M:tests/test_tempo_start.cpp:289). MR10 RED (the raise removed).
 T-G9  THE HAND TEMPO IS CLAMPED, NOT FOLDED. Manual: `setHandTempo` 256 -> bpm 256; 500 -> 400; 10 -> 30; 59 -> 59;
       201 -> 201. `followExternalTempo(240)` -> 120 and `setManualBPM(240)` -> 120 (today's fold, unchanged). The
       phase and the bar position on the applying hop continue (no realign). MR7 RED.
 T-G10 BACK TO AUTO. Manual, hand tempo 256, then `setManualMode(false)`: on the first Auto hop bpm is 128; hand
       tempo 32 -> 64. MR8 RED.
 T-G11 PURE FUNCTIONS. `canHalve` is true exactly when bpm / 2 >= 30 (60 -> true, 59.9 -> false, 30 -> false);
       `canDouble` true iff bpm x 2 <= 400; `bpmStepUp` 127.6 -> 128, 128 -> 129, 400 -> 400; `bpmStepDown` 127.6 ->
       127, 128 -> 127, 30 -> 30; (x / 2) x 2 == x bit for bit for x in {60, 127.6, 128, 200}; `beatRunning` of each
       state byte 0..15. MR9 RED (step by 0.5).
UNIT, tests/test_beat_shift.cpp additions (S1r):
 T-N17 THE STOP HOP. Manual 120; D in {+250, -250, +40, -40} applied, and D = 0 never engaged; the tracker's phase at
       the stop in {0.2, 0.7}; with a zero request. On that hop: the "1" block; totalBeatCount' == F's EXACTLY; not
       held; the applied value 0.0; the sequence S's. Adoption is returned exactly when S's count differs from F's;
       the test asserts at least one arm adopts (-250 at 0.2: the published beat was a beat behind) and the never-
       engaged arm writes no byte. MR11 RED (the clamp [F, F + 1]: -250 at 0.2 shows an edge).
 T-N18 THE HELD LEVEL. Paused 100 hops; D changed from 0 to +100 at hop 10: every hop publishes F's nine fields bit
       for bit; the held bit clear; the sequence equals S's on every hop; the applied value reaches 100.0. MR12 RED.
 T-N19 A KEPT NUDGE THROUGH STOP AND PLAY (the B arm of reading R74). D = -250 and +250, no zero request: on the stop
       hop the "1" block with F's count; held hops as T-N18; after Play: count' never falls and rises by 0 or 1 per
       hop; the first not-held hop at or past the start's beat has beatInBar' == 0; from the third beat on pos' - pos
       equals the shift within 2e-5.
 T-N20 PLAY AFTER A PAUSE WITH THE NUDGE MOVED. Paused; D from 0 to -200 and from 0 to +200; Play: any hold lasts at
       most 0.4 beat + 1 hop (200 ms at 120); count' never falls; beatInBar' changes only at a count' edge.
 T-N7b THE RANGE. `setHandTempo` 128 -> 256 and 256 -> 128 at D = -500 (the nudge applied): count' never falls, rises
       by 0 or 1 per hop; any hold is at most |D| x |change in BPM| / 60000 beats + 1 hop (A20: 1.07 beat); after it
       pos' - pos equals the shift at the new tempo within 2e-5.
UNIT, the recorder clock's test (S2r):
 T-R2  bpm 120 throughout; the state byte goes Running -> Stopped for 240 ticks -> Running: exactly one "unmetered"
       anchor on entry, none while held, one "lock" anchor on leaving; `beat` never falls and does not advance while
       held. With the byte 0 throughout the anchor list equals today's. MR18 RED (the term removed: no anchor).
UNIT, tests/test_rate_reader_lint.cpp (S2r):
 T-G12 every file under src that reads `.bpm` from a FeatureSnapshot is either on the whitelist (F16's non-motion
       readers, by path) or contains `beatRunning(`. RED arm: the tree before S2r (Renderer.cpp fails it).
MODEL, tests/test_topbar_model.cpp (S3m):
 T-RB1 `bpmText`: 0 -> "---"; -1 -> "---"; 128 -> "128"; 127.96 -> "128"; 127.6 -> "127.6"; 127.84 -> "127.8"; 400 ->
       "400"; 30 -> "30". MR13 RED (always an integer).
 T-RB2 `parseBpm`: "128" -> 128; "127.6" -> 127.6; " 90 bpm" -> 90; "500" -> 400; "12" -> 30; "0" -> false; "abc" ->
       false; "" -> false. MR14 RED (no clamp).
 T-RB3 `rowOrder()` is exactly: "wheel", "play", "pause", "stop", "bpm", "bpm_minus", "bpm_plus", "nudge_back",
       "nudge_text", "nudge_forward", "half", "double", "tap", "resync" -- his thirteen in his order, with the nudge
       text between back and forward (R70) and nothing else. MR15 RED (tap before half).
 T-RB4 `fitTopBar` with a signal label of 86: inner 1720 -> nothing shed; 1504 -> DSP, FPS, the caption, the state
       word; 1272 -> all six; for every inner width from 1272 to 2200 the kept widths sum to <= the width, the shed
       list is a prefix of the order DSP, FPS, caption, state word, Master Signal, Master, and Outputs and the row
       are never in it.
 T-RB5 the tooltip strings of NR1 and NR4, exactly; every character ASCII.
WIDGET, tests/test_topbar_row.cpp (S3r; a JUCE component is invisible by default: Pitfall 34):
 T-U6  as RN, for the nudge text.   T-RB6 the same three closes for the BPM editor: Enter, Esc and a loss of focus
       each call `onBpmEditEnded` once; only Enter changes the tempo; the label then shows `bpmText` of the PUBLISHED
       tempo, never the typed text.
 T-RW1 at 1728 x 40 and 1280 x 40 the fourteen bounds of `rowOrder()` are in strictly increasing x, none overlaps,
       none is empty; "Bar N" lies left of the wheel and the wheel repaint rect is at most 80 px wide; Manual, Link
       and the Quantize combo lie right of Resync in that order; at 1280 the Outputs button has its full 100 px.
 T-RW2 the lit button follows the published byte: an injected snapshot with bit 2 -> only pause lit; bit 3 -> only
       stop; 0 -> only play. MR19 RED (lit from the last click).
 T-RW3 enabled states follow the published tempo: 400 -> "+" and "x2" disabled; 30 -> "-" and "/2" disabled; 128 ->
       all four enabled; Link on (the test seam of M:tests/test_topbar_link_toggle.cpp) -> the five disabled with
       "Link sets the tempo".
 T-RW4 in Auto a click on "+" calls `onManualBpmChanged(true, bpmStepUp(shown))` once and the Manual toggle is on.
 T-C4  a show file with "bpmMultiplier": 2 loads; a save of it holds no "bpmMultiplier" key. RED arm: main.
BINDINGS, tests/test_binding_tempo_row.cpp (S4r):
 T-B7  a key press on a TempoRow binding returns its op; a release returns none.   T-B8 a note-on / note-off the same.
 T-B9  a CC: nothing; `bindingIsLive` false; `learnMidiCC` refuses and the list is byte-equal. MR16 RED.
 T-B10 toVar / fromVar round-trips ops 0..6; absent -> 0; TempoRow's saved number is BeatNudge's + 1 and every older
       action keeps its number. MR17 RED.
MUTANTS (new): MR1 `updatePhase` not gated. MR2 `applyStop` calls `realignPhaseToZero`. MR3 `startFromOne` does not
 count. MR4 `scoreBeat` not gated. MR5 the Tap's realign not gated. MR6 the command word is latest-wins. MR7 the hand
 tempo folded. MR8 no re-fold on Auto. MR9 a step of 0.5. MR10 `requestRun` does not raise the sequence. MR11 the stop
 hop clamps to [F, F + 1]. MR12 step 2c removed. MR13 `bpmText` always whole. MR14 `parseBpm` unclamped. MR15
 `rowOrder` swaps two. MR16 `bindingIsLive` true for TempoRow on a CC. MR17 TempoRow inserted before BeatNudge. MR18
 the recorder clock's term removed. MR19 the lit button set by the click. (RN's M1-M28 stand.)

GATE ROWS (Harmony; strings exact). The rig is RN's: the lane's test-server build, the real app, the 120 BPM click
file, `open -g`, the live lock, the probe quits only the pid it launched; no Output window, no full-screen capture, no
synthetic input. "Applied" = the first /api/features poll (every 5 ms) whose trackerRequestSeq has reached the
posted value (Pitfall 48).
LR1  PAUSE HOLDS, PLAY RUNS ON. Manual 120. `tempo_row` pause; from the applied poll, 400 polls: the nine beat fields
     identical in all, "beatTimer": "paused", bpm 120. Then play; over the next 8 s the line (probe time, totalBeatCount
     + beatPhase) has a slope of 2.00 beats/s within 1 %, and its value at the applied poll is within 0.06 beat of
     the held value. Three rounds, the pause at pseudo-random times (fixed seed). "PASS  LR1 pause holds and play
     runs on: 3 of 3 rounds, 1200 of 1200 polls held". RED arm: build-mut-gate (MR1): "FAIL  LR1: beat moved while
     paused".
LR2  STOP IS THE 1; PLAY IS THE EDGE; A WAITING FIRE LANDS ON IT. Manual 120, Quantize "Next Downbeat". Note c = the
     last polled totalBeatCount; `tempo_row` stop. BAR, from the applied poll for 400 polls: beatPhase == 0.0,
     beatInBar == 0, barCount == 0, totalBeatCount in {c, c + 1} and constant, "beatTimer": "stopped", and
     `/api/debug/beat_nudge` "ms" == 0 (it was set to +40 before the stop). Fire a clip (`POST /api/trigger_clip`):
     for 1.0 s the layer's playing clip is NOT the new one [question 129 A]. `tempo_row` play: in the applied poll
     totalBeatCount == the held count + 1 and beatInBar == 0; the new clip is the playing one within 150 ms. Five
     rounds. "PASS  LR2 stop is the 1 and play lands the waiting fire: 5 of 5". RED arm: build-mut-gate (MR3):
     "FAIL  LR2: the fire did not land on play".
LR3  NOTHING SET TO BPM MOVES; THE REST DOES. Stopped for 5 s (Manual 120, the click file playing): (a) the nine beat
     fields constant over >= 900 polls; (b) onsetCount rises by >= 8 and rms is above 0 in >= 90 % of polls (the
     analysis listens); (c) a BPM-synced clip's position, read by the route the transport probes use (the builder
     names it; if main has none for an image sequence the clause prints "INFO  LR3c no position route" and is
     reported, not passed), does not change; (d) a Timeline video clip of at least 10 s advances by 5.0 s within 10 %. Then
     play: (c) advances again. "PASS  LR3 one stopped beat: beat 0 moves, BPM clip 0 moves, free clip runs". RED arm:
     build-mut-gate ((a) fails); for (c) the lane at the S2 commit (no clip gate): "FAIL  LR3c".
LR4  THE TEMPO CONTROLS. Manual 120 by `bpm_edit`. In order, each read from `/api/bpm` after it is applied, exact to
     0.01: double -> 240; double -> 240 and ui_text enabled.double false; half -> 120; half -> 60; half -> 30; half
     -> 30 and enabled.half false; `bpm_edit` "127.6" enter -> 127.6 and bpm_text "127.6"; bpm_plus -> 128; bpm_plus
     -> 129; bpm_minus -> 128; `bpm_edit` "500" enter -> 400 and bpm_text "400"; `bpm_edit` "abc" enter -> 400. Then
     Auto on the click file (LOCKED): bpm_plus -> ui_text manual true and bpm == floor(the shown tempo) + 1. And:
     beatPhase + totalBeatCount never falls across the whole row, and no poll shows a step of more than 0.25 beat
     at a double or a half (a tempo value never realigns). "PASS  LR4 tempo controls: 13 of 13 steps". RED arm:
     build-mut-fold (MR7): "FAIL  LR4 step 1: 120, want 240".
LR5  THE TEXTS FOLLOW THE MODEL; BOTH EDITORS GIVE THE KEYS BACK (replaces L11). (a) after every set in L2, L7, L9:
     ui_text nudge.text equals T-U1's string within 200 ms. (b) `nudge_edit`: begin -> editor open; type "12", enter ->
     "ms" 12, "nudge +12 ms", closed, focus_home_count + 1; type "x9", escape -> 12, + 1; focus_lost -> 12, + 1. (c)
     the same three closes through `bpm_edit`. (d) tempo_row.lit is "play", "pause", "stop" within 200 ms of each
     `tempo_row` op; (e) tempo_row.order equals T-RB3's list with strictly increasing x; (f) every tooltip equals
     T-RB5's. "PASS  LR5 texts follow the model: <n> of <n>; editors: 6 of 6 closes hand focus home". RED arm: the
     lane at the S3a commit ("FAIL  LR5: no tempo_row in ui_text").
LR6  THE SEVEN TARGETS. `binding_action` {"action": "tempoRow", "op": k} for k = stop, play, pause, play, double, half,
     bpm_plus, bpm_minus: after each, "beatTimer" and `/api/bpm` are what LR1, LR2 and LR4 state for that op; a
     release and a CC-typed binding change nothing. MIDI learn refuses a CC on a row target and attaches a note (L8's
     shape). "PASS  LR6 the seven targets: 10 of 10". RED arm: the lane at the S4 commit ("FAIL  LR6: 400").
L10  gains one line per new RED arm above.
VG   Five critic seats on R1-R20 against V18, V18b, V19: 0 MUST. Pre-registered MUSTs: the row's order differs from
     T-RB3's; a nudge text not exactly as T-U1; a text clipped in any state; two controls of the row overlapping at
     any captured width; a control of the row missing or cut at 1280; more or fewer than one of play / pause / stop
     lit; a non-ASCII glyph in the row or the learn title; a tooltip that is empty or differs from T-RB5. Named
     questions the seats must answer in words (a SHOULD unless a seat shows a mis-read): can ">" (play) and ">"
     (nudge forward) be told apart at arm's length; can the timer's "[]" and the routines' "[]" be told apart; is the
     greyed "x2" at 400 legible as "cannot", not "broken".
After every live batch: RN's closing line stands (no pid, no window left; no Output window; no full-screen capture;
no synthetic input).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)
RN's B5, B10, B11, B12, B13 stand. The rest are replaced:
B-R1  Music on, the beat locked or tapped. Press ">" (nudge forward) a few times, then "<" -> the flashes and cuts
      come EARLIER, then later, against the music; the text counts "nudge +1 ms", "+2" ...; the tempo number does
      not move. WRONG: forward makes it later; the tempo changes; only some things move.
B-R2  Press pause (||) in the row -> everything that follows the beat freezes where it is: the circle, beat pulses,
      BPM-synced clips, autopilot steps. A clip that just plays at its own speed keeps playing; things that follow
      the SOUND (loudness, hits) keep reacting. Press play -> it all runs on from where it stood. WRONG: a free clip
      freezes; a BPM clip keeps running; the picture jumps when you press play.
B-R3  Press stop ([]) -> the circle sits on beat 1 and stays; the stop button is lit; the tempo number still shows
      the tempo. Wait for a "1" in the music and press play ON it -> beat 1 is exactly your press. WRONG: the circle
      sits on another beat; play starts somewhere else in the bar; the number shows 0 (question 126).
B-R4  One honest limit: a look whose drift SPEED follows the tempo number (not the beat) keeps drifting while
      stopped; it has no beat to stand on. Tell us if you meet one and it bothers you.
B-R5  Stopped, with Quantize on, fire a clip -> it waits; press play -> it starts on that "1" (question 129). With
      Quantize off it fires at once, stopped or not. WRONG: it never starts; it starts a bar late.
B-R6  Stopped, tap a new tempo -> the number changes, the beat stays stopped; Resync -> still stopped, on the "1"
      (question 128). After stop then play the nudge text reads "nudge 0 ms" (as after Resync). Tell us if you want
      the number kept.
B-R7  With the app listening (Manual off) press "x2" -> the number doubles, Manual switches on by itself, the beat
      runs twice as fast from where it was, no jump. "/2" twice from 128 -> 64, then 32; a third "/2" is grey. "+"
      from 127.6 -> 128 -> 129 (question 125). Click the number, type 140, Enter -> 140, and your clip keys work
      again at once. WRONG: x2 at 128 shows 128; the number shows what you typed, not what runs; keys type into
      the box.
B-R8  Put "Beat stop" and "Beat play" on two pads, "Tempo x2" on a third -> they do what the buttons do; a knob does
      not attach to them.
B-R9  At arm's length on your screen: is the row in the order you gave; can you tell the row's play ">" from nudge
      forward ">", and the row's stop from the routines' stop to its left; is "x2" the label you want (you wrote
      "*2"); is "nudge +12 ms" readable where it sits?
B-R10 Make the window narrow -> the row never loses a control; FPS / DSP go first, then the "Quantize:" word, the
      LOCKED word, then the two Master faders (still on the Composition tab). Is that the right order to give up?
B-R11 Hold "+" (tempo) and hold ">" (nudge) with the mouse -> both run steadily and stop at their ends. Is the speed
      right?
B-R12 Record a take while you stop and start the beat -> the take records; replaying it does not stop your beat.

## 7 QUESTIONS FOR BORIS (125..129; each has a default A; nothing waits)
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
       routines" square stays, to the left of the row, in the routines' green so it cannot be taken for the row's stop.
     B All three old buttons stay, to the left of the row.
     C All three go; routines are stopped from their own bands and pads only.
128. The BPM timer is stopped or paused, and you tap a tempo or press Resync.
     A (default) The tempo you tapped is taken, Resync puts it on the "1" -- but it stays stopped until you press play.
     B Tapping or Resync starts it running again.
129. The BPM timer is stopped and Quantize is on. You fire a clip.
     A (default) It waits, and starts when you press play, on that "1".
     B It starts at once.
Readings to tell him (he corrects only what is wrong): R74 after stop then play the nudge reads 0, as after Resync;
pause leaves it. R75 a take does not record the timer's play / pause / stop, and replaying a take never stops your
beat. R76 with the app listening, "-", "+", "/2", "x2" or a typed tempo switch Manual on. R77 the tempo you set by
hand can go from 30 to 400; what the app hears by itself stays between 60 and 200. R78 "/2" and "x2" never move the
"1": the beat just runs half or twice as fast. R79 the "Bar 1..4" text sits just left of the circle; the LOCKED word
sits beside Manual. R80 the nudge buttons read "<" and ">" because "-" and "+" are now the tempo's.
The line that changes for each B: 125 -> `bpmStepUp` / `bpmStepDown`. 126 -> one branch in the label's text source.
127 -> B: two buttons and their two callbacks stay left of the row (+ 50 px); C: one button and one tooltip test go.
128 -> `kGesturesStartTimer`. 129 -> `quantizeModeToForcedSnap` + the two routine sites. R74 -> the two statements in
`setBeatTimer`.

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
RR1 STRONGEST: "The Harmony constraint says the engine stages are not re-opened, and this puts a gate into
    BPMTracker.cpp at four sites, three steps into BeatShift and a second request word. A hold at the publish step
    would touch neither." It loses on his own words as Harmony asked them: pause "holds the beat where it is" and
    play "lets it run again" -- above a tracker that runs on, the beat cannot run again from where it was without a
    real-valued offset between tracker and published beat, carried through A1's guards and bar seating: more
    machinery than four branches, and a second way for the published beat to differ from the tracker's. And play
    from stop must count a beat in the tracker either way (or the published count runs ahead of it for ever). What
    would change this: the real-time review refusing a branch in `updatePhase`, or T-G4 showing the detector's own
    state is disturbed by a held phase (then the alternative is ruled by the architect, not improvised). Cheapest
    refuting test: T-G1 and T-G4 on the real tracker, RED first.
RR2 NOTHING WAS RUN. Every rule here is from reading 185147b. The order inside one hop (request taken, tempo applied,
    `updatePhase`, `scoreBeat`, `updatePhrase`, `applyResync`) decides T-G2, T-G3 and T-G6; a misreading passes this
    paper and fails the app. Refuting test: the T-G cases; A24's stop rule applies.
RR3 PLAY FROM STOP DOES NOT COUNT A BAR. It counts a beat; `totalBarCount` is left alone, as a Resync leaves it (F12).
    A routine queued for "the next bar" therefore starts one bar after the press, as it does after a Resync today.
    INFERRED from FB T2's routine row. If Boris reads that as late (B-R5), the fix is one statement in
    `startFromOne` and a T-G3 clause; it is not built now because it would make play differ from Resync.
RR4 THE HAND RANGE REACHES CODE THAT ASSUMED 60..200. Read: the tracker, BeatShift's rule, the recorder clock, the
    rate readers. NOT read: every shader that takes `u_beatPhase` at 400 BPM (it will strobe at 6.7 Hz -- a
    photosensitivity matter at the user's own hand), the transport ruling's catch-up tables at 30 BPM, `videoBeats`.
    Refuting test: LR4's "never falls" clause and T-N7b; RT restates its own numbers.
RR5 TWO SAME GLYPHS (">" twice, "[]" twice under question 127 A). The critic seats and B-R9 decide; the fallbacks
    ("<<" / ">>"; the routines stop as text) are one string each.
RR6 THE BAR FITS AT 1728 BY ABOUT 31 PX, on an ASSUMED 86 px label. If VG-0's manifest shows less room, the shed order
    starts at his own width and DSP goes first; the widths are constants. Refuting measure: VG-0.
RR7 BPM-SYNCED CLIPS AND THE TRANSPORT LANE. This lane holds them with a predicate at two sites the transport lane
    rewrites. If both lanes build that at once, one is thrown away. Harmony's order (HR-1) is the control.
RR8 A HELD TIMER DURING A TAKE uses the unmetered path, whose documented limit is that events recorded while it is
    held share one beat value (F17). They keep their wall time. Refuting test: T-R2.
RR9 IN AUTO, PLAY FROM PAUSE may show one short beat (the onset realign: 0 or 1 count, as any realign today). It is
    the tracker's existing rule, not a new one. T-G4 pins "0 or 1 per hop".
RR10 "-" AND "+" SWITCH MANUAL ON. A performer who nudges the tempo in Auto loses the listening without having asked.
    It is said in R76 and shown by the toggle; the alternative (grey in Auto) fails "no inert control". B-R7 checks.
HARMONY'S DECISIONS (each has a default):
 HR-1 The clip gate's home: DEFAULT this lane's S2r edits Renderer.cpp's two BPM-Sync branches; if the transport lane
      merges first, S2r instead adds `TempoView::running`. One order for BPMTracker.cpp: S1, S1r, then S4t.
 HR-2 Stage S4r (keys and pads for the seven). DEFAULT built. ALTERNATIVE deferred: the row ships mouse-only for those.
 HR-3 The old overlay target "Stop" is renamed "Stop routines". DEFAULT yes.
 HR-4 The Link-on greyed state is unit-tested, not captured (no Link in the default build). DEFAULT as stated.
 HR-5 No production route and no OSC for the timer or the hand tempo beyond today's `/api/set_bpm` (RN H-1, H-4's
      posture). DEFAULT none.
 HR-6 A second pitfall number for the timer (RN H-12's rule: assigned at merge).
 HR-7 REST `/api/set_bpm`, OSC and Link keep the 60..200 fold (not his row). DEFAULT unchanged; filed as a side note.
SIDE FINDINGS: SF-R1 MAIN: in Manual the FPS and DSP labels stop updating and the number shows typed text (F5).
 SF-R2 MAIN: `GlobalPlayPause` runs the audio file, not clips (F4); the overlay calls it "Play / Pause". SF-R3 the
 dispatch calls 47-50 open; the record has them answered (F23).

## 9 WHAT IS NOT IN THIS LANE
- Anything of RN section 4's "NOT IN THIS LANE" list stands, EXCEPT "re-laying the top bar": the tempo cluster and
  the right block's shed rule ARE now in the lane (NR4); the Audio block and the right block's own widths are not.
- A time-accumulating uniform for the one `u_bpm`-scaled shader (B-R4). A faster or own-layer beat wheel (RN H-5).
- Saving the timer's state in a show or in settings. Recording or replaying the timer in a take. A production or OSC
  route for it (HR-5).
- Changing Tap (it keeps the fold and does not switch Manual on); Tap's shared history; the transport lane's S4t.
- Un-folding REST / OSC / Link tempos (HR-7); sending a tempo or a beat to Link; following Link's phase.
- The clip transport's bar lock, its speed rules and its tables at tempos below 60 (RT's).
- Counting a bar at play from stop (RR3). Fixing RN's SF-1..SF-5 on main other than SF-4, which falls out of NR3.
- The per-layer strips' own play / pause; the audio file's transport; routine pads and bands (only the routines
  stop button's colour and place change, under question 127 A, by the rules of docs/claude/recording.md "Surfaces").
FROM THE STOPPED SYNC-DIAL BRANCHES: nothing new is carried by this delta. RN's list stands -- carried, re-typed:
 from lane/bf2 (740b6d6) the BeatLead parts named in A1 (commits 5a46c07, 4ab00bd), the [golden] case of
 tests/test_analysis_sync_thread.cpp (A9) and `appliedResyncs()` (A4); from lane/bf2-keys (9eab9bd) the key / pad
 work of e15d1d0 and df98f78 (PL:359-385). Dropped with them, as RN and H-17 ruled: the dial's controller, venues,
 routes, OSC, `frameEpoch_`, the fold and slew of the lead. The timer, the hand tempo range and the row have no
 ancestor on either branch (INFERRED from RN's and PL's part lists; the two worktrees were pin-checked, not searched
 for a transport or a tempo range).

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 15:35:36, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-nudge-row.md (status DONE; 15 attacks ruled: 12 ACCEPT, 3 PARTIAL, 0 REJECT; 18 amendments, each
OVERRIDES this delta plan's body). It is a DELTA on ruling-nudge.md: precedence for the beat-nudge lane is now Boris's verbatim
words > the adoption blocks at the end of plan-nudge.md > this adoption > ruling-nudge-row.md > ruling-nudge.md > the plans.
Workflow run wf_2860272e-ece (draft: architect opus high; seats gates 7 attacks / 2 MUST, stage-hands 8 / 2 -- papers whole
(16,389 characters) in attack-nudge-row-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list and decisions, and its section 7 (questions and
readings) in full. NOT read by me: the other sections -- the builders' and reviewers' spec; gate strings only from section 5.
THE RULING'S CAVEAT, kept in view: "Nothing was run: all of it is from reading 185147b"; two widths of the top bar are ASSUMED
until the baseline capture VG-0.
RULED: paused = all nine beat fields held bit for bit; stopped = the "1" published with no count change; play from stop = one
edge; the tempo stays live, never 0 (his "the BPM goes to zero nothing moves" is met by nothing MOVING; what the number shows
is question 126). The timer is stopped INSIDE the tracker (new stage S1r). A hand-set tempo is clamped 30..400 and never
folded; the detector, Tap, REST, OSC and Link keep the 60..200 fold. The three small buttons left of the wheel today (play /
pause every clip, stop all routines) are re-used for the timer (question 127).
HARMONY'S DECISIONS: HR-1..HR-14 at their defaults, with two notes -- HR-1: ONE order in BPMTracker.cpp across lanes: nudge S1,
S1r, then the transport lane's S4t (this settles the nudge adoption's H-10 and the transport adoption's H-T1); HR-6: pitfall
numbers are assigned by me at merge. HR-3 / HR-11 and question 127 name "routines": Boris renamed them "actions" (15:07:44,
BF80) -- the naming lane re-words those strings; no builder invents the new wording.
STAGES of the lane now: G-N0, S1, S1r, S2, S2r, S3a, VG-0, S3m, S3r (replaces S3b), S4, S4r, VG, S5.
Questions 125-129 + readings: boris-clarify-125-129.md (asked at the close; each has a default; nothing waits).
NOT STARTED in this session: nothing of this lane is built.
