# ATTACK PAPERS -- blind council on plan-bf2-stops.md (s-rta-1003b, 2026-10-03)
Verbatim seat papers, 4 of 4 seats returned. Source: the workflow task text (one JSON array, 36202 characters, closed).
Pretty-printed only (json.dumps indent=2, ensure_ascii=False); no word was changed. Seats were blind to each other.
Plan attacked: .harmony/.reports/s-rta-1003b/plan-bf2-stops.md. Ruling: .harmony/.reports/s-rta-1003b/ruling-bf2-stops.md.

| seat | attacks | ids |
|---|---|---|
| audio-clock | 7 | AU-1 (MUST), AU-2 (SHOULD), AU-3 (SHOULD), AU-4 (SHOULD), AU-5 (NIT), AU-6 (NIT), AU-7 (SHOULD) |
| midi-hands | 7 | MI-1 (SHOULD), MI-2 (SHOULD), MI-3 (SHOULD), MI-4 (SHOULD), MI-5 (SHOULD), MI-6 (SHOULD), MI-7 (NIT) |
| gates | 8 | GA-1 (MUST), GA-2 (MUST), GA-3 (SHOULD), GA-4 (SHOULD), GA-5 (SHOULD), GA-6 (SHOULD), GA-7 (SHOULD), GA-8 (NIT) |
| scope (MINIMALIST) | 7 | SC-1 (SHOULD), SC-2 (SHOULD), SC-3 (SHOULD), SC-4 (NIT), SC-5 (SHOULD), SC-6 (NIT), SC-7 (NIT) |

## SEAT: audio-clock
```json
{
  "seat": "audio-clock",
  "attacks": [
    {
      "id": "AU-1",
      "target": "P2 hypotheses HB/HC, experiment E1 'hop delay' reading, decision table O2",
      "claim": "RD cannot tell HB (analysis a block late) from HC (stamp a block late). The 'hop delay' instrument is ~0 at D=0 whatever the product does, so O2's first branch ('hop delay +10.7 -> fix serviceOnce') is unreachable and a real 10.7 ms product shift routes to 'stamping side' or is mis-fixed. The witness ring (1024 hops, ~11 s) is also shorter than the 20 s takes.",
      "evidence": "Plan lines 177-178, 183-184, 191-195. AnalysisThread.cpp:127-134 stamps each hop at its drain pass; :150 processes it in the same pass (frontDue(nowMs, 0) is true at D=0, AnalysisDelayLine.h:48-51). So processMs - stampMs is ~0 at D=0 and ~100+epsilon at D=100 under HB or HC alike. SyncWitness.h:98 (stampMs = drain time), :140 ('nothing waits in the SPSC ring before its stamp'), :161 (1024 hops ~11 s). The plan's own F10 (line 58-60) says no hold at D=0, which contradicts HB's predicted +10.7.",
      "severity": "MUST",
      "proposed_change": "Redefine the HB/HC readings in E1 with quantities that differ. Per marker, print stamp minus the FeatureSnapshot::timestamp of the first witness hop whose onsetCount reached it, and the tick wall (take-clock t) minus that hop's processMs. Add SyncIterEntry.ringReadyAfterDrain. Poll the witness every ~5 s during the take. Rewrite the O2 branches on these."
    },
    {
      "id": "AU-2",
      "target": "P2 decision table O1 / O5 and its 0.4 % figure",
      "claim": "O1 as written is satisfied by a run that never shows the state, so it can pass with no evidence. The O5 probability is wrong because the plan's own F12 says the state clusters by launch, so launches (not takes) are the sample. O5 then lets S6 start and re-states R7, with no validity clause that flags a two-valued e'.",
      "evidence": "Plan line 181: 0.71^16 = 0.4 % 'if takes are independent'; lines 160-162 and F12 (66-71) admit clustering. Saved runs: 2 of 4 launches had a shifted take (out-green arm 2; out-red-r7 3 of 3); out-green2 0 of 5, out-live 0 of 3. At launch level P(two launches show nothing) is about 25 %, not 0.4 %. O1 (lines 186-190) requires only 'c0 a multiple of the block in every take, e' one-valued'; c0=0 everywhere with no shifted take meets that literally and also meets O5 (199-201).",
      "severity": "SHOULD",
      "proposed_change": "O1 must require >=1 take with c0 != 0 whose assumed-origin error is shifted and whose e' sits within 64 samples of the unshifted takes. Size the run on launches (e.g. 6 launches x 3 takes). Under O5 keep S6 stopped, or add an R7 validity clause that all take medians on e' agree within 64 samples, else INVALID to the architect."
    },
    {
      "id": "AU-3",
      "target": "P2 'all that F6..F15 allow' (hypothesis list)",
      "claim": "A device-layer hypothesis is missing: the rig opens SEPARATE input and output CoreAudio devices (JUCE's combiner), and the plan never records device restarts, the opened block size or the rate per take. A per-launch or per-take callback phase or restart is the natural carrier of a 'launch-clustered, whole-take' state, and the plan lists no instrument for it.",
      "evidence": "Every saved adna-err.log line 1: input 'MacBook Pro Microphone', output 'MacBook Pro Speakers' (two devices). AudioEngine.cpp:135 comment and pitfalls.md:133 (the combiner). 'Switched to file playback mode' is logged once per launch (first take) in each log. CombinedCallback.h:179 counts device starts (opens_), exposed by GET /api/debug/audio_devices (testing-eyes.md:19). E1 (plan 172-184) reads none of it.",
      "severity": "SHOULD",
      "proposed_change": "Make E1 print per take, from GET /api/debug/audio_devices: opens (delta since the previous take), opened buffer_size and sample_rate. A shifted take that follows an opens increment or a block-size change falls outside HA/HB/HC and gets its own row in the table."
    },
    {
      "id": "AU-4",
      "target": "P4 TQ run and its three-row table",
      "claim": "TQ varies the wrong thing. The known correlate of the LOW regime is a media player, not CPU share: the S3f run was normal at 27.2 %. Four shell busy loops on a multi-core Apple-silicon machine do not reproduce a player's timer-coalescing, QoS or core-placement effects. Row 2 ('loaded normal: CPU share is not the trigger') is then an unsafe inference. The sleep-chain candidate is also unchecked: a crude simulation of the loop in AnalysisThread.cpp:79-96 with per-sleep overshoot o up to 0.45 ms gives overhead equal for D=37 and D=38 (pair slope 1.00 +- 0.03), so it does not explain 37 > 38 by 0.27 ms. INFERRED from my own sim, not run on the app.",
      "evidence": "Plan 233-249; F17 (lines 93-97: normal S3f run at Stremio 27.2 %, bf2-delta.md:593-596). test_analysis_sync_thread.cpp:560 (arms are interleaved 37,38,37,38..., so drift is rightly excluded), :525-531 and :536-538 (the poller busy-yields at USER_INTERACTIVE beside the Priority::high analysis thread, so harness and product contend for the same cores). My sim: scratchpad/sim_au.py.",
      "severity": "SHOULD",
      "proposed_change": "Make the loaded half of TQ the real correlate (the same video player at >20 %), with the shell loops as a separate third arm. Add a table row 'loaded-by-player low, loaded-by-loops normal -> trigger is media/QoS, gate stays quiet-and-no-player'. Add a D sweep (36..40 at idle) to test the D-dependence the candidate predicts. Drop the 'below the dial step' product claim until the mechanism holds."
    },
    {
      "id": "AU-5",
      "target": "P3 ruling 'pre-existing, not a product error' (E3)",
      "claim": "The reading is right (a doubled onset, not a late stamp) but E3 has a likely uninformative outcome. A seed-1 file with one doubled click per ~120 has a ~37 % chance of showing none, which fits content-dependence and also a rare time-dependent mechanism. The row 'B doubles other clicks or none' therefore confirms nothing.",
      "evidence": "F13 (plan 73-78) matches the saved data: in every arm the raw index 70 error is 4224-4736 samples (10048 in the red arm) and index 71 carries the value index 70 should have had, e.g. out-live arm 0 idx 66..72 = 1664,1216,1792,1344,4416,1408,1472. 0.0083 per click gives (1-0.0083)^120 = 0.37 chance of no double. Plan 220-223 treats 'none' as support for content. OnsetDetector.cpp:21 sets minioi 50 ms and :20 turns adaptive whitening on (stateful over the whole run).",
      "severity": "NIT",
      "proposed_change": "Run B with 2-3 seeds, or add the cheap control: replay the SAME seed-0 file from a +0.25 s offset. A content-bound double must still sit on click 69's noise."
    },
    {
      "id": "AU-6",
      "target": "P2 'FOR' evidence for HA (F14, F15, 'which 0.29 a ~3 ms window gives')",
      "claim": "The support for HA is not discriminating, and the 3 ms window is back-fitted from the 0.29 it explains. Both earlier takes read the normal state, which HB and HC predict equally. The window length (arm to play) is never measured, and E1 does not measure it either.",
      "evidence": "Plan 158-162: 'FOR ... two earlier takes show origin 0 and the normal state's numbers (F14)' and '4 of 14 = 0.29 is what a ~3 ms window ... gives (INFERRED)'. F14/F15 (lines 81-87) are normal-state takes (origin 0), where every hypothesis predicts c0 = 0. The window = AudioTap.cpp:146 armed_.store to AudioEngine::play (MainComponent.cpp:5946 -> 5955), with a provisional take.json save inside it (RecorderHost.cpp:278-294).",
      "severity": "NIT",
      "proposed_change": "Drop F14/F15 from HA's FOR list (they support nothing). If the window needs measuring, use a TEST-SERVER-only pair of stamps, but only if O1 is not settled by E1."
    },
    {
      "id": "AU-7",
      "target": "P2 'NOT in this lane' and R7/R7b re-statement on a measured origin",
      "claim": "Subtracting c0 turns the take's own audio into the ruler. That is sound only if c0 is a pure file-to-asset offset. The new validity clause 'c0 a multiple of the device block size' names no source for the block size, and any callback gap inserted by the tap (writeSilenceFrames) moves later clicks, so the 'within 4 samples of c0' clause alone is how an inserted gap is caught. A gap is a product event that would then read as INVALID rather than as a finding.",
      "evidence": "Plan 305-306. AudioTap.cpp (push) inserts gapFrames of silence and a gap marker when the host-time delta exceeds 1.5 blocks, and CombinedCallback.h adds the gap to deliveredSamples_. probe-sync.py:326 and :1098 already read opened buffer_size, but the plan does not say the new clause uses it.",
      "severity": "SHOULD",
      "proposed_change": "State that the block is opened.buffer_size from audio_devices read per arm. Print the take's gap count (take.json gaps) on every R7 arm line, and treat a take with gaps>0 as INFO-flagged, not silently INVALID."
    }
  ],
  "strongest_point": "AU-1: the diagnostic run RD, which gates S6, reads 'hop delay = process time minus drain stamp'. At D=0 the loop drains and processes in one pass, so that number is ~0 under every hypothesis. HB and HC therefore look identical, and the O2 'hop delay +10.7' branch can never be taken. A real one-block product error would be sent to a second instrument run, or fixed in the wrong place, instead of being shown to Boris. The fix is cheap and probe-only: compare each marker's stamp with the rising hop's own sample timestamp, and its tick wall time with that hop's publish time.",
  "citations_rechecked": true
}
```

## SEAT: midi-hands
```json
{
  "seat": "midi-hands",
  "attacks": [
    {
      "id": "MI-1",
      "target": "P1 fork (b) + section 7 Q1",
      "claim": "Q1 asks only about a knob and the app cannot tell controls apart, so (b) also kills the control that (d) handled correctly: a CC button (sends 127 on press, 0 on release). The plan's own table says (d) is 'one step per press: right' for it. Boris is non-technical; 'the app has to be told what kind of knob it is' is not a question he can answer. Many pad/button banks send CC, not notes. Q1 never names buttons, so a default 'no' discards buttons without asking.",
      "evidence": "plan-bf2-stops.md:113 (CC button row: (b) 'cannot be bound', (d) 'one step per press: right'); :349-351 (Q1 text 'turn a knob'; 'told what kind of knob it is'); :125-127 (d rejected for the knob/encoder side effects only).",
      "severity": "SHOULD",
      "proposed_change": "Reword Q1 to name his actual hardware: 'Do any of your controls send a CC (a button that sends 127 then 0, or a turning knob) rather than a note? If a button: it will be bindable; a knob will not.' Then, if buttons matter, rule a narrow button-only edge rule or record the exclusion explicitly as a product loss in the Boris page, not as a silent default."
    },
    {
      "id": "MI-2",
      "target": "P1 CHANGE: the kept Relative arm + T4 + R13",
      "claim": "The plan keeps a Relative arm that is unreachable from any screen (prepared file only) and decodes only the 64-offset coding (ticks = value - 64). Its own table says this is wrong for two's-complement/sign-bit encoders (1 -> -63 ms, 127 -> +63 ms, clamp in 8 detents). Unit test T4 and live row R13 (cc relative 67/59/64) assert it as 'the encoder case works', so the gates pass on a decoding that is wrong for common encoders and on a path Boris cannot reach. P6 removes a dead branch because the next reader must re-derive it; P1 keeps one. Also no per-message magnitude cap: 64+30 moves 30 ms in one message.",
      "evidence": "BindingManager.cpp:173-175 (non-Relative -> return false; outputValue = value - 64); plan :111 (two's-complement row: 'wrong way'); :137-138 (arm stays, 'reachable from a prepared file only'); :324 R13 cc relative 67/59/64; :266 (P6 rationale for removing dead code); test_binding_sync_nudge.cpp:75-92 (65/63/67 only).",
      "severity": "SHOULD",
      "proposed_change": "Either delete the Relative Sync arm and ccMode handling for SyncNudge (consistent with P6; 'no CC on Sync' becomes total), or keep it and state in docs/R13 that only 64-offset is decoded, add a test pinning the 1/127 case as a known wrong-way, and cap ticks per message. Do not gate it as 'the encoder case works'."
    },
    {
      "id": "MI-3",
      "target": "P1 CHANGE: fromVar through bindingIsLive",
      "claim": "fromVar refusing non-live entries silently drops a bindings-file entry; fromVar already clears all bindings first and the file is re-saved from memory, so the entry is permanently deleted with no signal. A show/preset file written by the S4-era tree (or hand-edited with an Absolute CC Sync entry) loses it on first load. This is a quiet failure whose only guard is test T4 on the in-memory loader.",
      "evidence": "BindingManager.cpp:260-263 (fromVar clears bindings_ then loads every entry unconditionally); :285 ccMode read; plan :136-137 ('fromVar adds through the same check, so a bindings file's Absolute-CC Sync entry is not loaded'); :140-142 docs only. No saved-back/round-trip test listed in T1-T4.",
      "severity": "SHOULD",
      "proposed_change": "Either load the entry disabled (enabled=false) so it round-trips on save and the dead state is visible in the data, or add a T5: load then toVar, assert the entry survives or is intentionally dropped and say so in the lane report. Prefer keep-disabled over delete."
    },
    {
      "id": "MI-4",
      "target": "P5 / section 6 check 1 / row R13 (key path, held key)",
      "claim": "The plan never rules or gates what happens when a Sync key is HELD. MainComponent::keyPressed has no repeat guard and calls processKeyDown on every call; OS auto-repeat (INFERRED JUCE/macOS behaviour: repeats delivered as keyPressed) would then call syncOffset_->nudge each time, sliding the dial at the OS repeat rate with no indication. Boris's check 1 says 'press each a few times' only; R13 drives a Binding value directly and cannot see repeats. A nudge the operator wants one-per-press, or wants to slide, is decided by accident.",
      "evidence": "MainComponent.cpp:4040 (keyPressed) and :4150 (processKeyDown, no isKeyCurrentlyDown/repeat check; grep -i 'repeat' over src/binding, MainComponent.cpp, BindingOverlay.cpp, MidiLearnOverlay.cpp finds no guard); plan :255-258 (route builds a Binding VALUE, never added to the manager); :340-343 (check 1: 'press each a few times'); K8 :384-386 admits the key path is skipped.",
      "severity": "SHOULD",
      "proposed_change": "Rule it in the plan: repeat is the feature (hold = slide; document and add 'hold a key 2 s' to Boris check 1) or repeat is ignored (latch via isKeyCurrentlyDown for SyncNudge keys). Add a live gate that drives the real key path (processKeyDown N times as a repeat would), not only handleBindingAction."
    },
    {
      "id": "MI-5",
      "target": "P5 route + R13: what the live row actually proves",
      "claim": "R13 is billed as 'the handler moves the dial' but it injects a Binding value straight into handleBindingAction, bypassing exactly the two seams where a wrong control kind matters: processKeyDown/processMidiNoteOn (key and pad velocity normalisation) and processMidiCC's ccMode gate. The only note value tested is 1; a real pad sends velocity/127 (fractional). The reachable defect classes (key repeat, pad velocity, learned CC refused) have no live row except G7's single ML-2 dump.",
      "evidence": "plan :255-258 (route builds Binding VALUE, never added to manager); :323-324 (R13 inputs: key value 1, note value 1, cc 67/59/64); BindingManager.cpp:112-115 (note path hands velocity/127 to the action); Binding.h:103-111 (value > 0 rule).",
      "severity": "SHOULD",
      "proposed_change": "Add R13 cases for note value 0.5 and 0.0079 (velocity 1) -> step, and make the route accept an 'entry' field that routes key/note/cc through the real BindingManager::processKey*/processMidi* with a manager-resident Binding, so the claim covers the seam. Retitle the PASS line if it stays handler-only."
    },
    {
      "id": "MI-6",
      "target": "P1 CHANGE: learn overlay state + G7 ML-2",
      "claim": "The only state Boris can read after turning a knob onto a Sync target is the unchanged waiting title and the 'Last: CC n val=v' line. Nothing distinguishes 'refused' from 'MIDI not reaching the app' except that the title lacks 'or CC'. Worse, ML-2 gates one refused CC but not the second half of Boris check 2 ('a pad or key then attaches as usual') nor that a knob bound elsewhere keeps its binding at the overlay level (only T2 at unit level). A refusal that accidentally clears waitingForMidi_ or selectedTargetIndex_ passes ML-2's 'still reads waiting' only if the dump reads those fields.",
      "evidence": "MidiLearnOverlay.cpp:253 (lastMidiMessage_ = desc set before the target test), :260-266 (existing CC bindings removed before the add), :286-287 (waitingForMidi_/selectedTargetIndex_ cleared after add); plan :139-141 (leave waiting only when learnMidiCC returned true); :338-339 G7 ML-2 text (one CC fed, 'still reads waiting and shows no binding').",
      "severity": "SHOULD",
      "proposed_change": "Extend ML-2: after the refused CC, feed a note through the same entry and assert the Sync target now shows 'Note n'; and with a pre-existing CC 21 binding to another action, assert it still shows 'CC 21' after the refused learn. RED: mutant that removes before the check (the overlay-level twin of T2's mutant)."
    },
    {
      "id": "MI-7",
      "target": "P1 CHANGE: prompt edit + K6",
      "claim": "Differentiating the waiting prompt by target ('Send a MIDI note...' vs '... or CC...') is on-screen text whose only purpose is to tell Boris a CC will be refused. Under his verbatim line 'the only fail message will be a failed save. remove all others' and H-2 (a refusal caption is not built), this is a refusal caption by another name, carried as a standing prompt. The plan itself leaves its judgement to a critic and a fallback.",
      "evidence": "rulings-bf2.md:8-11 (Boris: 'We don't need any text indicating what has happened...'; 'the only fail message will be a failed save'); :19-22 (H-2); plan :129-130, :370-372 (K6 admits the risk, fallback = prompt unchanged).",
      "severity": "NIT",
      "proposed_change": "Make the unchanged prompt the default and the target-specific one the fallback only if Boris asks, or have Harmony rule on H-2's line explicitly before S4b builds it. The silent waiting state already carries the 'Last:' readout."
    }
  ],
  "strongest_point": "P1 (b) removes every CC from Sync targets on the unverified premise that Boris wants only notes and keys, and asks him a question (Q1) that cannot be answered by someone non-technical and omits CC buttons, which the plan's own table shows (d) handled correctly. At the same time it keeps a Relative decoding that is wrong for two's-complement and sign-bit encoders, reachable only from a prepared file, and gates it as working in T4 and R13. The plan cuts the control class that works and keeps the one that does not.",
  "citations_rechecked": true
}
```

## SEAT: gates
```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "section 5 G7 ML-2 (and P1 chosen fork b)",
      "claim": "ML-2 passes when the CC never reaches the learn overlay. Its pass condition is 'the dump still reads waiting and shows no binding'. That is also what a dead injection route shows. The plan has no positive control, and the 'S5b test route' it relies on is not defined anywhere. The only RED named is 'the S4 tree binds it', which proves nothing about the fixed tree's route.",
      "evidence": "plan-bf2-stops.md:335-337 ('after a CC is fed through the overlay's own MIDI entry (S5b's test route)'; RED: 'the S4 tree binds it'). At 68abc16, git grep -i 'midi' and 'learn' in src/api/ApiServer.cpp return nothing, so no such route exists. The plan's only mention of the route is line 336, and no stage builds it.",
      "severity": "MUST",
      "proposed_change": "Specify and build the MIDI-entry test route in S4b or S5b. Add a control arm to ML-2: the same injected CC, with a non-Sync target selected, must leave waiting and show a binding in the dump. Run it in the same dump session. Only then does 'still waiting' on a Sync target mean the refusal fired. Add a RED: a build whose learnMidiCC always accepts must show the binding."
    },
    {
      "id": "GA-2",
      "target": "P2 experiment E1 'hop delay' and decision table O2",
      "claim": "The hop-delay reading that splits O2 into 'fix in serviceOnce' and 'stamping side' is structurally about 0 at D = 0, so it can never read +10.7 ms. HB's delay would sit before the drain (SPSC ring / resampler), which this measure cannot see. The sub-branch can only ever answer 'unchanged', which silently sends every O2 to the stamping side and never tests HB.",
      "evidence": "plan-bf2-stops.md:176-178 and 191-194 (hop delay = process time minus drain stamp). AnalysisThread.cpp, serviceOnce at 68abc16: delayLine_.commitBack(nowMs, ...) stamps the hop with this pass's nowMs, then processHop(..., stampMs, nowMs) runs in the same pass when D = 0. SyncWitness.h defines stampMs as the drain stamp and processMs as the loop clock. Plan F10 itself says a drained hop is processed in the same pass.",
      "severity": "MUST",
      "proposed_change": "Replace the instrument. Use the witness iters (SyncIterEntry.ringReadyAfterDrain, hopsDrained) and compare each hop's FeatureSnapshot::timestamp with the tap's delivered-sample counter at the same wall time. A hop-delay column from stampMs/processMs must not be the HB discriminator. Add a SELFTEST case showing the O2 sub-branch can fire."
    },
    {
      "id": "GA-3",
      "target": "P2 experiment sizing and outcome O5",
      "claim": "The 0.4% 'no shifted take' chance is miscalculated, and O5 is therefore likely. The p = 0.29 includes the mutant app's 3 of 3 shifted takes. The mutant is a different binary (a RecorderHost.cpp edit that is behaviour-identical at D = 0). On the lane app proper, 1 of 11 takes was shifted. Takes also cluster by launch, so the 16 takes are not independent. O5 then releases S6 and re-states R7 with the cause unconfirmed. That repeats the confounding H-12 forbade.",
      "evidence": "plan-bf2-stops.md:181 (0.71^16 = 0.4 %) and 161-163 (cluster 'weak'). Saved probe-sync.json R7 arms: out-green 1 shifted of 3 (41.333 at the third arm), out-green2 0 of 5, out-live 0 of 3, out-red-r7 (the mutant) 3 of 3 at 41.333. build-mut-r7.sh changes only the lateSeconds line. At p = 1/11, P(none in 16) = 0.91^16, about 22%, and in 32 takes about 5%. With launch latching, higher still.",
      "severity": "SHOULD",
      "proposed_change": "Recompute from the non-mutant takes only. Count launches, not takes, as the independent unit. Make O5 not release S6 on the first pair of launches. Require either a shifted take or N launches (stated up front) before 'not reproducible'. Alternatively, run S6's D9 only after R7 has been re-run on the measured origin."
    },
    {
      "id": "GA-4",
      "target": "section 5 row R13 (CC rows) and P5 route spec",
      "claim": "The CC rows of R13 contradict the handler they claim to drive. The real handler takes ticks (value minus 64) from BindingManager. The route is said to post the raw 'value' to handleBindingAction. Then {'cc', value 67} nudges +67 (to 68), not +3 (to 4), and value 64 nudges +64. Either R13 fails on first run, or the builder makes the route subtract 64. Then the row tests the route's own copy of BindingManager's decode, not the real path, and the 'sign / clamp' claim covers only the key/note rows.",
      "evidence": "MainComponent.cpp:7943-7944 at 68abc16: syncNudgeDeltaMs(binding, value) then nudge. Binding.h:103-110 multiplies the CC value by |step| as ticks. BindingManager.cpp:172-177 hands value - 64 to the action. Plan lines 256-257 ('posts onDebugBindingAction(binding, value)') and 323-324 ('value 67 -> 4; 59 -> -1; 64 -> still -1').",
      "severity": "SHOULD",
      "proposed_change": "State in the plan that the route feeds ticks, or route CC input through BindingManager::processMidiCC with a real Relative binding. Make the row's expected numbers follow from that choice. Add a RED that moves the decode (value - 64 changed) and show R13 catches it."
    },
    {
      "id": "GA-5",
      "target": "P4 TQ run and its outcome table",
      "claim": "TQ does not reproduce the known trigger, and its table leaves a failing gate standing with no way to apply the 'quiet machine only' condition. F17 itself says the low regime is not simply the video player's CPU share (a normal run at 27.2 %). TQ nonetheless proposes four shell busy loops, which on a multi-core Mac will probably read 'loaded normal'. That row ends with 'gate as today; nothing more is built'. The table also has no row for mixed results (loaded low in some runs, idle low with a process above 20 %). The case's own bar fails in the low regime.",
      "evidence": "plan-bf2-stops.md:233-249. timing-run3.log (M0 low run): lines 120-121 give arm 37 at 38.548 and arm 38 at 39.240, so the pair is 0.692, below the 0.7 bar. In the same run, lines 116-117 give 0.758 and lines 118-119 give 0.739. Run5 (normal) gives 0.95-1.00.",
      "severity": "SHOULD",
      "proposed_change": "Make TQ's loaded arm reproduce the earlier low runs: media playback at that CPU share, plus a shell-loop arm. Randomize the idle/loaded order. Add table rows for mixed outcomes. Say what Harmony does when a quiet-machine G1 [timing] run FAILS with the regime present: re-run N times and report the pair values. 'Gate as today' needs a stated action, or the case can fail with no ruling."
    },
    {
      "id": "GA-6",
      "target": "section 5 G1 / S4b scope (pins)",
      "claim": "The added test cases and the added TEST-ONLY route leave pinned inventory counts stale, and the plan lists none of them. S4b adds Catch2 cases (T1-T4 and more), edits test_beat_lead, and adds a TEST-ONLY /api/debug route. The docs the plan names to update are only performance-controls.md:41 and the keys text of APP-INVENTORY.",
      "evidence": ".harmony/APP-INVENTORY.md:31 at 68abc16 pins '1330 unit tests (ctest -N 1330 = 1324 + the 6 cases of test_binding_sync_nudge)' and a route count ('67 registrations ... 25 of them TEST-ONLY /api/debug/*'). docs/claude/testing-eyes.md:24-25 lists the TEST-ONLY routes. The plan's S4b doc list is at plan lines 142-143.",
      "severity": "SHOULD",
      "proposed_change": "Add to the S4b exit: re-measure ctest -N and the registration counts, and update APP-INVENTORY:31 and testing-eyes.md. State the new expected numbers in G1 (1330 + added cases) so a miscount reads FAIL, not 'close enough'."
    },
    {
      "id": "GA-7",
      "target": "section 5 R7 validity clause (c0 multiple of the block)",
      "claim": "The new validity clause cannot reject the failures it exists to flag. 'c0 is a non-negative multiple of the device block size' accepts any N (1, 2, 5 blocks), while the docs sentence the plan proposes says 'up to one block'. The block size is also ambiguous: CLAUDE.md says a 128-sample callback, while every firstSample is a multiple of 512. A large c0 means the take-start window grew (for example after S6's D9 edits the take start), and R7 now subtracts it silently. The only warning is a printed INFO.",
      "evidence": "plan-bf2-stops.md:305-306 and 188-190. CLAUDE.md 'Audio Callback ... every 2.67 ms (128 samples @ 48 kHz)' against plan F11 (firstSample multiples of 512). Plan line 285-288: S6 changes the take start, the same code the window lives in.",
      "severity": "SHOULD",
      "proposed_change": "Bound c0 explicitly (0 or 1 device block as measured in RD, else INVALID). Name the block size as the value measured in RD (512, from the saved firstSample lattice). Make c0 above the bound an INVALID that blocks the gate. Add a SELFTEST arm with c0 = 4 blocks that reads INVALID."
    },
    {
      "id": "GA-8",
      "target": "P1 tests T1-T4 and the re-stated 'Absolute CC = no push' assertions",
      "claim": "After P1 the BindingManager processMidiCC 'return false' guard for an Absolute SyncNudge CC is unreachable, and the re-stated test cannot fail. addBinding refuses the binding, so CHECK_FALSE(processMidiCC(...)) passes on an empty manager. The plan keeps the guard 'as the last guard' but lists no mutant that deletes it. A removal of the guard plus a bypass of bindingIsLive would ship unnoticed.",
      "evidence": "plan-bf2-stops.md:137-138 (guard stays) and 151 (the Absolute assertions re-stated on T2's shape) and 152 (MUTANTS: only bindingIsLive-always-true and remove-before-check). BindingManager.cpp:170-173 at 68abc16 is the guard. tests/test_binding_sync_nudge.cpp 'an Absolute CC bound to SyncNudge fires no action' currently adds the binding through addEncoder, which P1 makes impossible.",
      "severity": "NIT",
      "proposed_change": "Add a mutant: 'delete the Absolute return false' with a test that injects a non-live binding past addBinding (a test-only add path, or a prepared var that bypasses the check). Or delete the dead guard and its claim, so no ungated code remains."
    }
  ],
  "strongest_point": "GA-1 and GA-2 are the two gates that cannot fail where it matters. The ML-2 'still waiting, no binding' bar passes whether or not the injected CC ever reached the learn overlay, and the injection route is not defined or built anywhere. The O2 sub-discriminator 'hop delay +10.7 ms' is stamped and processed on the same nowMs at D = 0, so it reads about 0 by construction. HB (a hop analysed a block late before the drain) can therefore never be separated from the stamping side.",
  "citations_rechecked": true
}
```

## SEAT: scope (MINIMALIST)
```json
{
  "seat": "scope (MINIMALIST)",
  "attacks": [
    {
      "id": "SC-1",
      "target": "P7 stage table, S6 row 'needs S4b'; S4b 'may start the moment RD has run'",
      "claim": "The order adds two waits the facts do not need. S6 is made to wait for S4b (keys, route, BeatLead removal); and S4b waits for RD although only its R7 re-statement depends on RD.",
      "evidence": "ruling-bf2-delta.md:516 gives S6 'needs M0, S3f', not S4. Plan line 285 adds S4b and justifies it only by 'S6 then builds on the clean file' (line 267, P6). Plan line 288: S4b 'must not rebuild build-lane while RD runs', which is a build-dir collision, not a data dependency. P1/P5/P6 touch src/binding, ui/MidiLearnOverlay, ApiServer, MainComponent, BeatLead; none touch the take start (RecorderHost). Other build dirs already exist (build-mut-r7, build-tsan in the scratchpad bf2-M0 logs).",
      "severity": "SHOULD",
      "proposed_change": "Split S4b: S4b-a (P1, P5) starts now in its own build dir, parallel with D0 and RD. S4b-r7 (R7/R7b on the measured origin) is the only part gated by RD. S6 needs only RD not-O2 (as in ruling-bf2-delta:516), not S4b. Lost: nothing; the stage count stays, the wall-clock chain shortens by one full stage."
    },
    {
      "id": "SC-2",
      "target": "P6 dead BeatLead branch (remove in S4b)",
      "claim": "Removal is a larger change than the defect: six symbols deleted across BeatLead.{h,cpp}, AnalysisThread.cpp:457 and 5 test sites, plus a new mutant, G3(b), G2 and a real-time review of analysis-thread code, to fix a comment-sized problem.",
      "evidence": "BeatLead.cpp:195-199 already says in a comment 'the branch decides nothing since D5' and 'originX == barX... so = barX_ and += absorbed agree'. The invariant does hold: originX_ and barX_ change only at :191 and :195/:198 (grep at 68abc16). Plan line 262-270 lists the full removal; K9/K10 (lines 377-380) admit the cost and that S4b may need splitting.",
      "severity": "SHOULD",
      "proposed_change": "Leave the branch, tighten the existing comment to say 'dead since D5; both arms equal', and file the removal for a lane that already edits BeatLead. If removed anyway, make it its own S4c so it cannot delay the keys fix or S6. Lost: the next reader still sees one dead line; gained: no analysis-thread diff, no review round, no mutant."
    },
    {
      "id": "SC-3",
      "target": "P1 CHANGE (bindingIsLive, learnMidiCC, addBinding/fromVar refusal, prompt edit)",
      "claim": "The keys MUST has a three-line fix; the plan builds a predicate, a new manager method, load-time refusal, a changed prompt string and a new G7 text bar.",
      "evidence": "The defect is the learn block, MidiLearnOverlay.cpp:244-289: it removes every binding on the CC (:255-262) and then adds one that BindingManager.cpp:168-177 will never fire for SyncNudge. A guard 'if target.action == SyncNudge: ignore the CC and stay waiting' placed before :255 closes it with nothing removed. Plan lines 131-143 add addBinding/fromVar refusal (a prepared Absolute-CC file is harmless: :173 already returns false) and a new on-screen string (line 140), which Boris's 'no text' words (rulings-bf2.md:8) make a liability and which needs G7 ML-2 text assertions (plan 335-337).",
      "severity": "SHOULD",
      "proposed_change": "Overlay-only guard (optionally one pure bindingIsLive for a unit pin of 'existing CC binding is not stripped'), prompt text unchanged, G7 ML-2 asserts only 'still waiting, no binding on the target, the other CC binding intact'. Lost: load-time refusal of a hand-edited file (benign) and the prompt hint (the 'Last:' readout is untouched)."
    },
    {
      "id": "SC-4",
      "target": "P5 route + R13 (13 steps, two mutants)",
      "claim": "R13 re-proves in 13 steps what unit tests and R1a already prove (clamp at +-500, tick arithmetic); the only unproven thing is the two-line case calling the nudge with the right sign.",
      "evidence": "MainComponent.cpp:7939-7944: the case is 'if deltaMs != 0 -> syncOffset_->nudge(deltaMs)'. syncNudgeDeltaMs is pure and unit-tested (plan F3); nudge/clamp is reached by REST at :2323 which existing rows drive. Plan lines 321-328: 13 steps incl. 499/-499 clamps, Relative CC 67/59/64 (a path the plan P1 makes reachable only from a prepared file), plus a second mutant 'nudge call removed'.",
      "severity": "NIT",
      "proposed_change": "Keep the TEST-SERVER route (it is the only way to reach the real handler) but cut R13 to: key +1 press, release no move, key -1, one Relative CC, and ONE mutant (sign flip). Lost: nothing a different row does not already cover."
    },
    {
      "id": "SC-5",
      "target": "D0 / RD / E3 (P3 diagnosis) and the hop-delay INFO",
      "claim": "Part of the diagnosis cannot change any decision: E3 (seed-1 file, PROBESYNC_R7_WAV_B, two extra 60 s takes) and the hop-delay field. P3 is already ruled 'R7's rule handles it, filed, not built'; the hop delay only matters inside O2, where the plan itself ends in 'another instrument-and-run' for one branch.",
      "evidence": "Plan lines 224-229 (P3 ruling: no change to R7, 'filed for an analysis lane, not built here'); the only E3 fork that acts is 'doubles near 34.5 s again -> architect' (line 222-223). Plan lines 191-195: the hop-delay split chooses between an AnalysisThread fix and a further instrument run, so it is needed only on O2.",
      "severity": "SHOULD",
      "proposed_change": "D0 = E1 (c0 per take, spread, e and e') + the doubled-click INFO already in R7's rule. Drop seed-1 file, WAV_B knob and the second 60 s take pair; build the hop-delay readout only if RD reads O2. Lost: the content-vs-time proof of P3 (no gate depends on it)."
    },
    {
      "id": "SC-6",
      "target": "TQ [timing] 5 idle + 5 loaded, P4",
      "claim": "TQ duplicates G1's quiet [timing] x3 and its loaded half answers a question whose answer is already ruled 'case stays a gate, quiet only, bar untouched'.",
      "evidence": "Plan lines 246-248 rule the gate before TQ runs; the only TQ outcome that changes anything is 'idle LOW once' (line 245), and G1's own quiet x3 (plan 290-291) would show that identically. Loaded half (4 busy loops) yields only 'filed, not built' (line 249). rulings-bf2.md:35-39 (H-8) bars touching the bar and asks for the cause only as an architect question.",
      "severity": "NIT",
      "proposed_change": "Run the idle half as G1's quiet x3 (it has to run anyway); skip the 5 loaded runs and the 4-busy-loop setup, noting 'regime INFERRED, not triggered by CPU share proven'. Lost: the load-trigger evidence."
    },
    {
      "id": "SC-7",
      "target": "P2 / P7: RD gates S6 and S4b-r7",
      "claim": "MUST NOT CUT: E1's c0 measured in the take's own audio, run BEFORE R7 is re-stated and BEFORE S6 touches the take start. This is what separates a probe artefact from a product 11 ms shift.",
      "evidence": "Plan F6/F7 (lines 41-51) are INFERRED only; F8 shows the probe assumes file frame 0 = asset frame 0. rulings-bf2.md:58-60 (H-12): S6's D9 edits the take start (RecorderHost::startDue, plan F10 line 60) and a diagnosis must not be confounded; R7's 21-arm run not run until understood. Without E1 the O1 re-statement (lines 296-306) could hide a real fault (K1, line 353-358).",
      "severity": "NIT",
      "proposed_change": "Keep as planned (D0 -> RD -> S6) and note it as the single non-negotiable; every other cut above is conditional on it. Lost if cut: the ability to tell O1 from O2, so R7 could be quietly re-defined around a product defect."
    }
  ],
  "strongest_point": "SC-1: the plan adds a dependency that the spec chain never had (S6 needs S4b, ruling-bf2-delta.md:516 says M0+S3f only) and holds the independent keys/handler work behind RD for a build-dir collision, so the serial chain D0, RD, S4b, S6, S5a, S5b is longer than the facts require. The one thing that must NOT be cut is E1: measuring c0 in the take's own audio before R7 is re-stated and before S6 touches the take start.",
  "citations_rechecked": true
}
```

STATUS: DONE
