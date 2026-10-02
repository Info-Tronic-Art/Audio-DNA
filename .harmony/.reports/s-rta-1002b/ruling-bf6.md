# RULING bf6 -- architect ruling on the blind council (s-rta-1002b, 2026-10-02)

Architect (Fable).
- Plan: .harmony/.reports/s-rta-1002b/plan-bf6.md (NOT edited; this ruling overrides it where they differ).
- Seat papers (verbatim): .harmony/.reports/s-rta-1002b/attack-bf6-papers.md.
- Boris's words: .harmony/boris-feedback-backlog.md (BF6) and .harmony/binding-decisions.md:591-593.
- Code re-read at main fa9604d. Every file this ruling cites differs from the plan's base 5e47d17 only by comment
  edits (ClipInspector.cpp, LayerStrip.cpp, MainComponent.cpp; 4 lines in all), so the plan's line numbers still hold.

**Verdict.** The plan's core stands: wire the existing ClipClock seam; Timeline = a clip-clocked curve; composition
controls do not offer it. There are 18 amendments. 2 attacks are rejected on evidence, and 3 are accepted in part.
Three findings change what Boris would have seen:
- **A flicker the plan missed.** The plan's "a fade hides it" is wrong for layer knobs. Every re-fire of a clip that
  played before would flash a Timeline knob's curve start (AM-6).
- **A silent freeze.** The unconditional legacy upgrade would freeze composition-level knobs (AM-3).
- **A gate that ran nothing.** G1's ctest regex selects 0 tests for 4 of its names (AM-16). Neither seat found this.

## 1. PER-ATTACK RULINGS

State / concurrency seat = S-, gates / tests seat = G-. "ACCEPT -> AM-n" points to an amendment in section 2.

- **S-A1 [MUST] ACCEPT -> AM-1.** The premise holds: plan-bf9b.md is still a skeleton (every section "(pending)").
  The plan required bf9b merged (:274-283) but named no contract and no walk rule.
- **S-A2 [MUST] ACCEPT -> AM-2.** Decision: a parked clip's knob follows its parked playhead.
  - The Clip-tab scrub writes the model playhead directly (ClipInspector.cpp:1247, :1271).
  - A player persists per clip id (Renderer.cpp:1582-1584 retires only on close) and advances only inside syncMedia
    (Renderer.cpp:1831-1836).
  - So the parked playhead is what the clip's timeline bar shows AND where the clip resumes.
- **S-A3 [MUST] ACCEPT -> AM-3.** Part of it is moot: macros are never saved (ConnSerialization's only callers are
  Clip.cpp, Layer.cpp and Composition.h), so no macro-level legacy ramp can exist. The re-scan run during this ruling
  found 0 hits.
- **S-A4 [MUST] ACCEPT (procedure) -> AM-4.** Not adopted: a virtual-playhead DEFAULT. It is an invention with its own
  length setting, built only if Boris picks it (Q1 b).
- **S-A5 [SHOULD] REJECT.** No new cross-thread access to a plain field:
  - The engine runs on the message thread (jassert, ConnectionEngine.cpp:294).
  - inPoint / outPoint are written only on the message thread: ClipInspector.cpp:1263, :1267; Clip.h:315-316
    (replaceContent); Clip.h:355-356 (clear); Clip.cpp:222-228 (fromVar, on the load path). The GL thread only READS
    them (ClipTransportSync.h:56, :69). The attack's "written ... on the GL thread by writeBack" is wrong.
  - mediaType: the same (MainComponent.cpp drop / load handlers; Clip.h:286, :340; Clip.cpp:174).
  - playheadPosition is a RelaxedDouble (Clip.h:238); the trigger tuple is one acquire load (Layer.h:300).
  - probe-tsan-unit.sh never runs this path. G7 stays INFO with corrected wording (AM-17).
- **S-A6 [SHOULD] ACCEPT -> AM-5.** Backward is dead code: `Playback::Backward` is assigned nowhere in src (only
  declared, ParamConnection.h:71), and the picker writes only source / outMin / outMax / inverted
  (UniversalParamControl.cpp:650-659).
- **S-A7 [SHOULD] ACCEPT -> AM-6.** It also exposed an error in plan R2 (see AM-6).
- **S-A8 [SHOULD] ACCEPT -> AM-7.**
- **S-A9 [SHOULD] PARTLY.**
  - ACCEPT: state the evidence (AM-8).
  - REJECT: the index-snapshot test and the saved-id load test. Nothing saves a signal id or registry index (AM-8
    cites). Such a test would pin an order no file depends on, and would break the next lane that adds a signal
    (bf45 near :63-72).
- **S-A10 [NIT] ACCEPT -> AM-9 + AM-14.**
- **G-A1 [MUST] ACCEPT -> AM-4** (same as S-A4).
- **G-A2 [MUST] ACCEPT -> AM-1** (layer tests that do not name the accessor) **+ AM-15** (R15, the deck-switch row).
- **G-A3 [MUST] PARTLY.**
  - ACCEPT: R14 PingPong (AM-15); the not-playing state stated and tested (AM-2).
  - REJECT "scrubbing a non-playing clip will NOT move its knob": the Clip-tab scrub writes the model playhead itself
    (ClipInspector.cpp:1247, :1271), whether the clip is synced or not, and the engine reads the model.
  - REJECT live pause / scrub rows: no pause or seek endpoint exists (ApiServer.cpp:163-338; TestServer.cpp:128-265;
    the OSC clip addresses only trigger or set fit, OscHandler.cpp:59-85). A pause is a constant playhead (TC2). A
    scrub is a model write (TC11).
- **G-A4 [SHOULD] ACCEPT -> AM-9.** Not adopted: "set the bar after a baseline run on the lane build". That is
  post-hoc, not pre-registration, so the bars are derived from the latency chain instead.
- **G-A5 [SHOULD] ACCEPT -> AM-10**, with a correction the seat missed: Pearson >= 0.98 plus a 4x ratio fails a
  CORRECT build if the compositor blends in linear light.
- **G-A6 [SHOULD] ACCEPT -> AM-11.**
- **G-A7 [SHOULD] PARTLY -> AM-12.**
  - ACCEPT: a numeric V5 bar, the model cross-check, named critics.
  - REJECT: pixel-luma bars on greyed menu text. Antialiased text makes them fragile; the menu-model test (UI3a) is
    the check that cannot be fooled.
- **G-A8 [SHOULD] ACCEPT -> AM-13.**
- **G-A9 [SHOULD] ACCEPT -> AM-3 + AM-16.** The G0 pattern now covers the repo and mounted volumes; run now: 0 hits.
- **G-A10 [SHOULD] REJECT.** Same evidence as S-A9:
  - SignalBar has no persistence (no toVar / fromVar; it reads registry entries live, SignalBar.cpp:44, :194, :216).
  - MacroBank's sourceSignalId is never serialized.
  - Connections save signal names (ParamConnection.h:37-38).
- **G-A11 [NIT] ACCEPT -> AM-14** (TC0).
- **G-A12 [NIT] ACCEPT -> AM-14**, with one correction: UI7 is RED on its seam commit, not a pin.

ACCEPTED (full): S-A1 S-A2 S-A3 S-A4 S-A6 S-A7 S-A8 S-A10 G-A1 G-A2 G-A4 G-A5 G-A6 G-A8 G-A9 G-A11 G-A12.
PARTLY: S-A9 G-A3 G-A7. REJECTED: S-A5 G-A10. Architect-found: AM-16.

## ARCHITECT RULING (s-rta-1002b)

### 2. AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)

**AM-1 -- The bf9b contract, the walk, and layer tests that do not name the accessor (S-A1, G-A2).**
- P0 is replaced. It is the Stage 1 builder's first action, and no bf6 code is written before it passes:
  1. bf9b is merged to main.
  2. The merged tree provides the two items below, and the builder's P0 report names each with file:line.
     - C1, the playing-clip accessor. One message-thread function from a shared layer (or layer row) to
       `const Clip*`:
       - it returns the INCOMING (active) clip of that layer, resolved from ONE `runtime()` load;
       - it returns nullptr when the layer is clear or its cell is empty;
       - the pointer may point into ANY deck box, not only the shown deck.
     - C2, the walk: a way to visit each shared layer exactly once and each clip of every deck box exactly once.
  3. R13-model (AM-6) is run on the merge-base build. It decides whether Item 8 is built.
  - If C1 or C2 is missing or means something else, the builder STOPS and returns to Harmony for an architect
    amendment. The builder never invents an accessor.
- Item 1d is replaced. `ConnectionEngine::tick`:
  - evaluates each shared layer's scalarConns and layerEffects ONCE per tick, with `ModelClipClock{ C1(layer) }`;
  - evaluates each clip's scalarConns, effects and sourceParams ONCE per tick, with `ModelClipClock{ &clip }`,
    wherever the clip's deck box is;
  - keeps nullptr for macros, composition scalars and global effects;
  - never ticks a layer once per deck.
- TC5 / TC6 go through one test helper, `setPlaying(comp, layerRow, deckBox, column)`.
  - The helper calls bf9b's own fire path or setter (whatever C1 reads), so the assertions do not name the accessor.
  - TC5 gains a case: the playing clip is in deck box 1 while the shown deck is 0, and the layer knob follows the
    deck-1 clip.
- New TC5d [PIN; guards C2]. Setup: layer opacity on an Envelope(Beats) (cycleBeats 4) with smoothingMs 100, in a
  composition with 2 deck boxes.
  - Tick 1 at beat 1.0 initializes the smoother at 0.25. Tick 2 runs with dt 0.01 at beat 3.0.
  - Check: the value equals `ConnectionShaper::applySmoothing` applied ONCE (computed in the test), +-1e-6.
  - Once per tick gives about 0.298. A double evaluation gives about 0.341.
- Harmony relays C1, C2 and C3 (AM-6) to the bf9b architect NOW, because plan-bf9b.md is still a skeleton.

**AM-2 -- A parked clip's knob follows its parked playhead (S-A2; part of G-A3).**
- The rule, added to plan section 2 ("How the fix answers each case") and to Item 1's Behaviour:
  - A clip-owned Timeline or Clip Position connection always reads its OWN clip's model playhead, playing or not.
  - A parked clip (not the active or outgoing clip of any layer) shows curve(n(parked playhead)). That is what its
    Clip-tab timeline bar shows, and where it resumes when fired again (AM-6).
  - A never-played clip (playhead 0) shows the curve start.
- Why not NaN for a parked clip:
  - the knob would jump from the hand value to the curve at every fire;
  - dragging a parked clip's Clip-tab timeline would not move its knob (that drag writes the model playhead,
    ClipInspector.cpp:1247, :1271).
- TC11 [RED]. Setup: a Video clip active in no layer, playheadPosition 0.6, opacity on Timeline.
  - Check: twin 0.6. Then set playheadPosition = 0.25 (what the scrub does): the next tick gives 0.25.
  - Main gives 0.
- TC12 [PIN]. Setup: a never-played Video clip, playhead 0, in 0.2, out 0.8. Check: twin == 0.0 (the curve start),
  not NaN.
- R16, the parked-clip probe row, runs inside R13's setup.
  - While column 0 is parked at p of about 0.5 and column 1 plays, column 0's clip.opacity stays within 0.03 of
    n(its parked p) on >= 95% of 100 ms samples over 1 s.
  - Base: FAIL (it reads 0).
- Plan section 8, check 1 now says "the timeline bar in the Clip tab". The layer strip's bar does not seek the player
  (O8).

**AM-3 -- Upgrade legacy Timeline knobs only where a playhead exists; wider G0; Q4 blocks (S-A3, G-A9).**
- Item 3b is replaced:
  - `ConnSerialization::fromVar` gains a trailing parameter `LoadOwner owner = LoadOwner::Other`.
    ConnSerialization.h declares `enum class LoadOwner : uint8_t { Other, ClipOrLayer };`.
  - The exact pre-bf6 Timeline pick is re-clocked to ClipPosition ONLY when owner == ClipOrLayer. Its shape: no
    "envFormat"; clock "beats"; cycleBeats 4 +-0.001; exactly the two points (0,0,linear) and (1,1,linear), +-1e-6.
    Anything else loads as written.
  - `scalarsFromVar` passes ClipOrLayer when ScalarEnum is ClipScalar or LayerScalar, decided at compile time with
    `if constexpr`. All three enums come from ScalarParams.h:26, :27, :38, which ConnSerialization.h already includes,
    so Clip.cpp:313, Layer.cpp:268 and Composition.h:628 are not edited.
  - These sites pass ClipOrLayer (or their equivalents on the bf9b tree): Clip.cpp:207 (source params), Clip.cpp:301
    and :306 (clip effects), Layer.cpp:256 and :261 (layer effects).
  - Composition.h:616 and :621 (global effects) and :628 (composition scalars) keep Other. A composition-level
    legacy Timeline keeps its 4-beat ramp, and a re-save does not rewrite it.
  - Macros need no case: MacroBank is never serialized.
- 3c (Signal "Clip Position" -> Kind::ClipPosition) stays at every level. That signal always read 0 (E7), so nothing
  that moved is lost.
- Why the clip / layer re-clock loses nothing:
  - The old pick was the picker's stopgap meaning of "Timeline". ConnPicker.cpp:90-100 is the only place in src that
    creates a Kind::Envelope (grep).
  - The same 4-beat ramp stays one pick away: BPM Sync > Saw > 4 Beats.
- Item 3's note (plan :422) is replaced by: "A composition-level legacy Timeline keeps its 4-beat ramp."
- New tests:
  - TU6a [PIN]: the legacy JSON in Composition scalar conns and in a global-effect param loads on Beats. With the
    beat clock at beat 2.0 the value is 0.5 (beat / 4).
  - TU6b [RED]: the same JSON in a clip scalar and in a layer-effect param loads as ClipPosition and follows the
    playhead (0.5 -> 0.5).
- G0 is extended (section 4). It was run during this ruling and found 0 hits:
  - 1,808 .json files in the home folders;
  - main's tracked .json files;
  - no external volume mounted.
  - Compositions and decks save only as .json (MainComponent.cpp:2853, :3445; CompDecksBrowser.cpp:284, :300).
- Q4 is asked only if G0 hits. Then it BLOCKS the Stage 1 merge until Boris answers.

**AM-4 -- "Any clip parameter": Q1 is asked now and gates Stage 2's still and source menus (S-A4, G-A1).**
- Plan section 1 gains a coverage line that quotes Boris: "when I set any clip parameter to timeline it should be
  locked to the clips playhead".
  - The reading: any parameter OF a clip (clip and layer level).
  - A still, a source, a camera or an effects-only clip shows no playhead anywhere in the app. The Clip tab draws a
    timeline only for isPlayable() clips (ClipInspector.cpp:485, :641, :1046).
  - So what Timeline means on those clips is Boris's call, and Q1 asks him.
- Q1 goes to Boris NOW, in one message with Q2 and Q3, not on the post-build artifact page.
- Stage 2 is not built until Q1 is answered (G8). Stage 1 is built and merged meanwhile.
- Stage 1's engine publishes NaN (the hand value) for a clip with no playhead. That is the interim under both
  answers: the curve start (F5-C) would hide a clip whose opacity is on Timeline.
- TC7 is split:
  - TC7a [PIN; true under both answers]. An Image clip with a stale playheadPosition 0.7 (opacity on Timeline) and a
    Source clip (one source param on Timeline) never publish curve(0.7): twin != 0.7 +-0.01. The stale field is
    never read.
  - TC7b [RED; interim]. Both twins are NaN, and eff() == the manual value 0.8.
- R9 is labelled interim (it depends on Q1).
- If Q1 = (a): Stage 2 as planned (the items greyed out, "video clips only").
- If Q1 = (b):
  - Stage 2's still / source branch = Offered (enabled).
  - Harmony dispatches a Stage 3 architect spec after the answer. Stage 3 is a virtual clip clock: bars since the
    clip's activation, over a per-clip bar length, holding at the end.
  - Stage 3 rewrites TC7b, R9, UI3a, UI5c and V2.
  - Nothing of Stage 3 is built before the answer.

**AM-5 -- A clip clock ignores the connection's own playback setting (S-A6).**
- Item 1b is replaced by `float clipXform(float pos01)`: clamp to [0,1], with no Playback parameter.
  - For a clip-clocked source, the clip's transport (loop mode, reverse, PingPong) owns direction.
    ConnShape::playback has no effect.
- Item 1c: evaluate() uses clipXform(p) for Envelope(ClipPosition) and for Kind::ClipPosition.
- TC4 is replaced [RED]. FakeClock{1.0} -> 1.0 and FakeClock{0.3} -> 0.3.
  - It runs for an Envelope(ClipPosition) ramp and for Kind::ClipPosition, each with shape.playback = Forward,
    Backward and PingPong. All six cases must give the same value.
  - Main gives 0 for Forward at 1.0.
- TC1 also runs a descending sweep (playhead from .9 down to .1): each value equals the playhead. That is the engine's
  half of reverse play; R8 checks reverse live.
- The rendering.md section and the Pitfall text say this (AM-18).

**AM-6 -- A re-fired clip: a correction to the plan, Item 8, and R13 as a gate (S-A7).**
- Correction to plan R2 (:658-665). "A fade hides it" is wrong for LAYER knobs.
  - F2-A has the layer knob follow the incoming clip from the trigger, so a layer opacity on Timeline dips for the
    whole layer, including the outgoing clip.
  - The flash comes from the most common performance action: re-firing a clip that played before.
  - That clip's player persists (Renderer.cpp:1582-1584) and is not sought on a new activation: MainComponent.cpp
    seeks only for a beat-snap trigger (:4677) and a retrigger (:4699). The player resumes, but the activation tail
    has already written the in point (Layer.h:539).
- C3, the bf9b contract (bf9b is the preferred owner): at every activation that does not seek the player, the model
  playhead equals the position the player will play from.
- Item 8 belongs to Stage 1. It is built ONLY IF P0's R13-model run on the merge base shows >= 1 in-point sample.
  - Where: in the message-thread fire path. Today that is MainComponent::handleClipTrigger, inside the
    `if (auto* clip = layer->getClipAt(t.after.activeClipColumn))` block (MainComponent.cpp:4670-4723), as one more
    `else if` after the beat-snap branch (:4677) and the retrigger branch (:4699). On the merged tree it is bf9b's
    equivalent.
  - When: a transition that NEWLY activated `column` (`!wasRetrigger && t.after.activeClipColumn == column`), so not a
    retrigger and not a queued trigger. The clip is playable, and its player (`renderer.getVideoPlayer(clip->id)`) or
    sequence (`renderer.getImageSequence(clip->id)`) already exists.
  - What: `clip->playheadPosition = player->getPlayheadPosition();` (for a sequence, `seq->getPlayheadPosition()`), in
    the same message-thread call as the trigger.
  - Behaviour is unchanged: a re-fired clip still resumes, as the picture does today. The model just stops reporting
    the in point while the player is somewhere else.
  - Not covered (INFO): activations on the GL thread. These are autopilot advances and quantized triggers, fired by
    Autopilot.cpp:75 -> Layer::processPendingTrigger. The same frame's write-back corrects them, because the
    autopilot runs before compositing (Renderer.cpp:544-560, before syncMedia :1831-1836).
  - Files: + src/MainComponent.cpp (only if Item 8 is built).
- R13 becomes a GATE row (section 4).
- New TC14 [RED]. Setup: layer opacity on Timeline, manual 0.8.
  - Active = a Video clip at playhead 0.6: the value is 0.6.
  - Set the tuple to active = an Image clip: the next tick's eff() == 0.8 exactly, with no value in between.
- Note to bf9b: if bf9b makes a re-fire RESTART the clip, that is a visible change it must clear with Boris. It must
  then seek the player at activation, and Item 8 is not built.

**AM-7 -- Copies, undo, the new format's round trip, and a stack-only clock (S-A8).**
- TU5 [RED]. A NEW-format Timeline knob goes through Clip::toVar -> fromVar -> toVar.
  - Check: "envFormat": 2 and clock "clipPosition" survive, and the second toVar equals the first (it is never
    re-upgraded).
  - Main writes no envFormat.
- TC13 [RED]. Copy a clip that has a Timeline opacity and a Timeline effect param into another cell. Use the model's
  duplicate path if the merged tree has one (Pitfall 36); otherwise copy-construct.
  - Set the original's playhead to 0.7 and the copy's to 0.2: each twin follows its own clip.
  - Then copy-assign the effects vector, which is what an effect-stack undo restore does (ParamConnection.h:156-166),
    and tick again: the twins still follow.
  - Main gives 0.
- ConnectionEngine.h gets a comment: a ModelClipClock is built inside tick() for one tick and is never stored.

**AM-8 -- The retired signal: evidence only, no new tests (part of S-A9).**
- Plan :433 is replaced by: "VERIFIED: nothing saves a signal id or registry index.
  - Connections save the signal NAME (ParamConnection.h:37-38).
  - MacroBank::Macro::sourceSignalId (MacroBank.h:37) has no serializer; it is written only at MacroPanel.cpp:147
    and :155.
  - SignalBar.cpp has no toVar / fromVar.
  - After the removal, signals the user added (SignalRegistry::addSignal :88-99) sit one index lower. That is
    runtime-only, and every menu is rebuilt when it opens."

**AM-9 -- Sampling and bars for the event rows (G-A4, S-A10).**
- Steady rows (R1, R2, R6, R7, R8, R14, R15, R16): 100 ms cadence; >= 95% of samples within 0.03 (unchanged).
- Event rows: R3 and R4 at 20 ms, R13 at 10 ms. Every sample records its wall time.
- The bars come from the latency chain, about 60 ms in a typical case:
  - REST callAsync to the message thread, up to about 17 ms;
  - one GL write-back, up to 16.7 ms;
  - one engine tick, up to 8.3 ms;
  - the /api/composition read, up to about 20 ms.
  No bar is set from a run on the lane build.
- R3 bar: within 250 ms after the POST returns, some sample has p <= in + 0.05 and clip.opacity <= 0.06. Run 3
  retriggers; all 3 must pass.
- R4 bar: take the first sample with p < 0.1 that follows a sample with p >= 0.85. Within 100 ms of it, some sample
  has opacity <= 0.13. After that, >= 95% of samples are within 0.03 for 1 s.

**AM-10 -- R10, the picture row (G-A5).**
- The fixture clip runs at speed 0.25, so one pass of grey4 takes 16 s.
- Take >= 12 render_frame captures across one pass.
  - Bracket each capture with an /api/composition read just before and just after it.
  - n_mid = n((p_before + p_after) / 2).
  - Discard and retake a capture whose bracket spans more than 0.05 of n.
- Bars:
  - Spearman(mean decoded luma, n_mid) >= 0.95. A rank bar holds whether the compositor blends in gamma or linear
    light.
  - The mean luma of captures with n_mid >= 0.8 is at least 2x the mean luma of captures with n_mid <= 0.2. The
    plan's 4x would fail a correct build that blends in linear light: (0.9/0.1)^(1/2.2) is about 2.7.
  - A NaN Spearman (constant luma) is a FAIL.
- Base: FAIL (opacity 0 renders black).

**AM-11 -- Hand values that are not zero (G-A6).** Every NaN-twin assertion sets the manual value to 0.8 and asserts
eff() == 0.8 (not 0, and not the curve value). This applies to TC5 (cleared layer), TC7b, TC8b and TC14.

**AM-12 -- Visual gate bars (part of G-A7).**
- V5. The TEMP `sweep` hook (or the probe) takes V5a when n(p) is in [0.10, 0.25] and V5b when n(p) is in
  [0.60, 0.75], in the same pass (p rises between the two reads).
  - Bar: on the decoded PNGs, the Opacity slider thumb's centroid moves >= 30% of its track length toward the high
    end.
  - Bar: /api/composition clip.opacity at the two captures differs by >= 0.35.
- Model cross-check: each menu capture (V1-V4) is checked against the UI-test model for the same setup (UI1, UI3,
  UI4, UI2): is the item present, is it enabled, and what is its text. Any disagreement is a MUST.
- Named critics: visual-design, UX, graphic-design, logic, interaction-logic. Bar: no open MUST.

**AM-13 -- One rule function, and no fallback (G-A8).**
- Add to src/connect/ConnPicker.h / .cpp (headless, already unit-tested):
  `enum class PlayheadSources : uint8_t { Offered, NoPlayhead, NotOffered };`
  `enum class ParamLevel : uint8_t { Composition, Layer, Clip };`
  `PlayheadSources playheadSourcesFor(ParamLevel level, bool clipIsPlayable);`
- What it returns:
  - Composition -> NotOffered.
  - Layer -> Offered.
  - Clip -> Offered if the clip is playable. Otherwise NoPlayhead (Q1 = a) or Offered (Q1 = b).
- Item 5a's enum moves into ConnPicker.h.
- Every provider calls playheadSourcesFor; no inspector restates the rule. The providers are:
  - the ClipInspector, LayerInspector and CompositionInspector providers;
  - EffectStackView's scope default (Global -> Composition, Layer -> Layer, Clip -> Clip).
- TP3 [RED on its seam commit, where the function returns Offered]: the full table (3 levels x playable or not).
- The named fallback (plan :488-489) is deleted, and UI2 links CompositionInspector.
  - Its include closure is LayerInspector's (MacroPanel, EffectStackView, UniversalParamControl), plus
    PerTypeAutopilotLayout.h and CanvasSizeCombo.h.
  - LayerInspector already links headless (tests/CMakeLists.txt:2202-2212).
  - Add whatever sources the link needs.

**AM-14 -- The null-clock pin, and explicit RED and PIN lists (G-A11, G-A12, S-A10).**
- TC0 [RED; in test_timeline_clock.cpp]: evaluate(Kind::ClipPosition, nullptr) and evaluate(Envelope(ClipPosition),
  nullptr) both return NaN. It lands in the same commit that rewrites test_connection.cpp:1014-1018 to FakeClock{0.0}.
- RED set. Each test fails at an assertion on the merge base, or on its seam commit where marked:
  - engine: TC0, TC1, TC2, TC3, TC4, TC5, TC6, TC7b, TC8b, TC9, TC11, TC13, TC14, TE1;
  - picker: TP1, TP2, TP3 (seam);
  - load and registry: TU1, TU3, TU4, TU5, TU6b, TR1;
  - menus: UI2, UI3a, UI5a, UI5c, UI7 (seam).
  - UI7 moves from the plan's pins to RED: on the seam commit the provider is ignored, so the Image clip's items stay
    enabled.
- PIN set (green on the base by design): TC5d, TC7a, TC8a (the plan's TC8 beats pin), TC10, TC12, TU2, TU6a, UI1,
  UI3b, UI4, UI5b, UI6.
- Every TEST_CASE name starts with its ID ("TC1 clip sweep ...") so G1 can check that it exists.

**AM-15 -- New probe rows (G-A2, part of G-A3).**
- R14, PingPong. Variant: loopMode PingPong, out 1.0.
  - Bar: through one forward leg and the return leg, |opacity - n(p)| <= 0.03 on >= 95% of samples.
  - Bar: Spearman(opacity, time) >= +0.9 on the forward leg and <= -0.9 on the return leg.
  - Base: FAIL.
- R15, deck switch. Layer opacity and clip opacity on Timeline; the clip is playing.
  - At p of about 0.3, POST /api/switch_deck (ApiServer.cpp:205) to another deck box, with no trigger.
  - Bar: for 2 s, both values stay within 0.03 of n(p) of the SAME clip on >= 95% of samples, and that layer's
    playing clip does not change.
  - Base: FAIL.
- R16, parked clip: see AM-2.
- There are no live pause or scrub rows, because no pause or seek endpoint exists (section 1, G-A3).

**AM-16 -- G1 ran almost nothing (architect-found; neither seat raised it).**
- The plan's `ctest -R "timeline|conn_picker|connection|..."` matches Catch2 TEST_CASE names, not target names.
  tests/CMakeLists.txt registers them with catch_discover_tests and no TEST_PREFIX.
- On the current build (1,114 tests), that regex selects 0 tests for conn_picker, effect_stack_binding,
  right_click_reset and timeline, and 12 for connection.
- It is replaced by running the test binaries directly (section 4, G1).
- G0's repo grep reads main's tree (`git grep ... main`), so bf6's own committed fixture (.harmony/probe-timeline.json)
  cannot set it off.

**AM-17 -- New wording for G7 (S-A5 rejected).** G7 stays INFO, with this text: "No new cross-thread access to a
non-atomic field.
- The engine reads playheadPosition (a RelaxedDouble written by the GL thread) and the trigger tuple (one acquire
  load).
- inPoint, outPoint and mediaType are written only on the message thread, and the engine reads them on the message
  thread.
- probe-tsan-unit.sh does not run this path, so it is not required."

**AM-18 -- Docs, files, observations, and Boris's checks.**
- The docs/claude/rendering.md section (plan section 6) also states:
  - parked clips follow their parked playhead (AM-2);
  - a clip clock ignores the connection's playback setting (AM-5);
  - the legacy upgrade applies at clip and layer level only (AM-3);
  - the model playhead must equal the player's at every activation (AM-6);
  - the still / source rule, per Q1.
- The Pitfall text below replaces plan :625-633:
  "NN. **Timeline / Clip Position read ONE clip clock -- never nullptr at a clip or layer site, never frac() at the
  end**: `clipTimelinePosition(clip)` = (playheadPosition - inPoint) / (outPoint - inPoint), clamped to [0,1]; NaN when
  the clip has no playhead (`!isPlayable()`). `ConnectionEngine::tick` hands every clip-owned connection its OWN clip's
  `ModelClipClock` (playing or parked), each shared layer -- evaluated ONCE per tick -- its playing clip (the INCOMING
  one during a crossfade, Pitfall 35), and composition / macro / global-effect ones none; a NaN clock publishes nothing
  (the knob shows its hand value). `clipXform` clamps and ignores the connection's own playback (the clip's transport
  owns direction): `frac(1.0) == 0` would snap a OneShot clip parked at its end back to the curve start. The playhead
  is GL-written (RelaxedDouble): `.load()` it on the message thread; a `ModelClipClock` lives for one tick. The model
  playhead must equal the player's at every activation, or each re-fire flashes the curve start. Guards:
  tests/test_timeline_clock.cpp, tests/test_timeline_picker.cpp; live .harmony/probe-timeline.sh."
- The CLAUDE.md index line stays as planned (<= 150 bytes; Harmony assigns NN).
- Plan section 10 (shared files) adds:
  - src/connect/ConnSerialization.h (LoadOwner);
  - src/model/Clip.cpp :207, :301, :306 and src/model/Layer.cpp :256, :261 (one argument each);
  - src/connect/ConnPicker.h / .cpp (playheadSourcesFor);
  - src/MainComponent.cpp (Item 8, if it is built).
  bf6 builds after bf9b merges, so none of these is edited at the same time as bf9b.
- Plan section 11 adds two observations:
  - O8 (pre-existing): LayerStrip's transport scrub writes the model playhead without seeking the player
    (LayerStrip.cpp:978-988).
    - The next write-back undoes it, so the scrub never moves the picture.
    - After bf6, a Timeline knob would blink to the scrub point for about one tick.
    - Same class as R2. Recommended as a follow-up: seek the player as the Clip tab does (MainComponent.cpp:1515-1528).
  - O9 (pre-existing): "stop" writes playhead = in point without seeking the player (MainComponent.cpp:6410). Same
    class.
- Plan section 8 (Boris's checks):
  - check 1 says "the timeline bar in the Clip tab";
  - new check 6: "Fire a clip that played earlier: its Timeline knob carries on from where that clip left off, with
    no flicker. Try it once with Quantize on."

### 3. FINAL BUILD STAGES

| Stage | Items | Depends on | Ships alone? |
|---|---|---|---|
| P0 | C1 / C2 check (AM-1); R13-model on the merge base, which decides Item 8 (AM-6) | bf9b merged | -- |
| 1 engine + data + witness | 1 (with AM-1, AM-2, AM-5, AM-7), 2, 3 (with AM-3), 4, 6 (with AM-9, AM-10, AM-15), 8 (AM-6, conditional) | P0 passed | Mergeable alone after G0-G4; not shown to Boris |
| 2 menus + docs | 5 (with AM-13), 7 (with AM-18), TEMP | Stage 1 merged + Q1 answered (G8) | Ships to Boris with Stage 1, after G5 |
| 3 (contingent) | virtual clip clock | Q1 = (b); architect spec after the answer | -- |

Commit order, Stage 1:
1. Tests only: test_timeline_clock.cpp and its target. Every RED test must compile on the merge base; RED fails and
   PIN passes are shown on this commit (G2).
2. Item 1, plus TC0 and the rewrite of test_connection.cpp:1014-1018.
3. Items 2, 3 and 4.
4. Item 8, if P0 calls for it.
5. Item 6, the probe (run on the base and on the lane).

Commit order, Stage 2:
1. Seam commit: playheadSourcesFor returns Offered, and the provider member exists but is ignored. TP3 and the UI RED
   tests fail here.
2. Implementation.
3. Docs.
4. TEMP hook (reverted before the merge).

### 4. FINAL GATE LIST (pre-registered; Harmony copies gate strings ONLY from this list)

**G9 -- P0 (before any bf6 code).**
- What: bf9b is merged. The P0 report names C1 and C2 with file:line, gives the R13-model count on the merge base
  (in-point samples across 10 re-fires), and therefore says whether Item 8 is built.
- Bar: C1 and C2 exist with the meanings in AM-1. Otherwise STOP and return to Harmony.

**G0 -- Read-only, just before merging Stage 1.**
- Commands:
  - `find ~/Library/AudioDNA ~/Library/Audio-DNA ~/Documents ~/Desktop -maxdepth 4 -name '*.json' -size -20M -print0 2>/dev/null | xargs -0 grep -l -E '"envelope"|"clipPosition"|"Clip Position"' 2>/dev/null`
  - `git -C /Users/boriskarpman/projects/RealTimeAudio grep -l -E '"envelope"|"clipPosition"|"Clip Position"' main -- '*.json'`
  - `ls /Volumes`. If any volume other than "Macintosh HD" is mounted, run the first command on it as well.
- Bar: no file listed -> proceed.
- If a file is listed, open it. If it holds a pre-bf6 Timeline, a clipPosition connection or a "Clip Position"
  signal connection, ask Q4 and do not merge until Boris answers.

**G1 -- Unit tests, on the lane build.**
- Run every binary:
  `cd <lane build>/tests && for t in test_timeline_clock test_timeline_picker test_conn_picker test_connection test_effect_stack_binding test_right_click_reset test_param_control_routine_cue test_clip_inspector_paint_key; do ./$t >/dev/null 2>&1 || echo "FAIL $t"; done`
  - In Stage 1, leave test_timeline_picker out of the loop.
  - Bar: no FAIL line.
- Check the test lists with `./<binary> --list-tests`. Bar: each binary lists every ID this ruling gives it:
  - test_timeline_clock: TC0-TC14 (including TC5d, TC7a, TC7b, TC8a, TC8b), TE1, TU1-TU6 (including TU6a, TU6b), TR1;
  - test_timeline_picker: UI1-UI7;
  - test_conn_picker: TP1-TP3.
- Run `ctest --output-on-failure` on the lane build and on the merge-base build. Bar: the tests that fail on the lane
  are a subset of the tests that fail on the base.
- On the merge commit:
  - `git grep -n ADNA_BF6_SHOT` -> 0 lines;
  - `git grep -n ClipPositionSignal -- src` -> 0 lines;
  - `git grep -n '"Clip Position"' -- src/signal` -> 0 lines.

**G2 -- RED proof.**
- The builder's report shows the failing assertion output on the merge base for every test in the AM-14 RED set. For
  TP3, the UI tests and UI7, it shows the output on their seam commit. Every PIN passes on both.
- Harmony spot-checks TC1, TU1, TC7b and TC8b by building test_timeline_clock against the merge-base sources.

**G3 -- Live probe (one launch, test mode, under the lock).**
- Lane: `TIMELINE_APP=<lane app> .harmony/probe-timeline.sh <out>`.
  - Bar: every row from R1 to R16 PASSES. R12 is reported under G4, and R13 has the bar below.
- Merge base: the same command with the merge-base app.
  - Bar: R1-R11 and R14-R16 FAIL, and R12 PASSES.
- Where each row's bar comes from:
  - the plan as written: R1, R2, R5-R9, R11, R12;
  - AM-9: R3, R4;
  - AM-10: R10;
  - AM-15: R14, R15;
  - AM-2: R16.
- **R13 (a GATE row):**
  - Setup: layer 0 with column 0 = grey4 and column 1 = grey6; clip opacity and layer opacity are both on Timeline.
    Play column 0 to p of about 0.5, fire column 1, wait 1 s, then re-fire column 0 (immediate).
  - Repeat 10 times: 5 cuts (transition 0) and 5 with a 1 s transition. Sample every 10 ms for 200 ms after each
    re-fire POST returns.
  - Bar on the lane: across all 10 re-fires, 0 samples that meet any of these:
    - column 0's playheadPosition <= its inPoint + 0.02;
    - its clip.opacity <= 0.10;
    - on a cut, layer.opacity <= 0.10.
  - On the merge base (P0), count only the playheadPosition condition.
    - >= 1 such sample: Item 8 is built.
    - 0: Item 8 is not built, and R13 becomes a pin.
  - Report the cadence actually achieved. If it is slower than 20 ms, run 20 re-fires instead of 10.

**G4 -- Rig hygiene:** row R12.

**G5 -- VISUAL (after Stage 2).**
- Capture by Quartz window id only; decode the PNGs.
  - V1-V4 as in the plan; V2 follows Q1's answer.
  - V5 per AM-12.
  - V6 = R10's captures.
- Run the model cross-check (AM-12).
- Named critics: visual-design, UX, graphic-design, logic, interaction-logic. Bar: no open MUST.
- Then the artifact page for Boris: the captures and the section 8 checks. Q1-Q3 were already asked.

**G6 -- INFO:** tick cost (as in the plan).

**G7 -- INFO:** TSan (AM-17 text).

**G8 -- Process.**
- Q1 is answered before Stage 2's build starts.
- If G0 led to Q4, Q4 is answered before the Stage 1 merge.

### 5. FINAL QUESTIONS FOR BORIS (Q1-Q3 go out NOW, in one message)

- **Q1 -- Pictures and sources.** This BLOCKS only Stage 2's still / source menus. Recommended answer: (a).
  "A still picture or a generated source (like MilkDrop) has no play position: the Clip tab shows no timeline bar for
  it. If you set one of its knobs to Timeline, should:
  (a) Timeline be greyed out there with 'video clips only', and the knob stays where you set it [recommended], or
  (b) the knob sweep once from low to high over a number of bars you set, starting when the clip starts?"
- **Q2 -- Two menu entries.** Non-blocking. Default: keep both.
  "After this fix, 'Clip Position' and 'Timeline' do the same thing: the knob goes from its low end to its high end as
  the clip plays. Once you can draw the Timeline curve, Timeline follows your drawing and Clip Position stays a
  straight line. Keep both [default], or remove Clip Position?"
- **Q3 -- Master Signal.** Non-blocking. Default: (a).
  "Should the Master Signal fader also turn down knobs that follow a clip's Timeline? (a) Yes, Timeline counts as a
  signal [default, how it works today]. (b) No, a Timeline knob always follows its clip fully."
- **Q4 -- Old shows.** Ask ONLY if G0 finds a file; then it BLOCKS the Stage 1 merge. Proposed answer: yes.
  "A show you saved before this fix has knobs set to Timeline. From now on, those on clips and layers will follow
  their clip like new ones, and any on the Composition tab keep their old 4-beat sweep. OK?"

### 6. RISKS OF THIS RULING

- **RK1 -- bf9b's shape.** If C1 or C2 is missing, bf6 stops at P0 and needs an amendment. Mitigation: relay the
  contract now. Check: G9.
- **RK2 -- Quantized and autopilot re-fires (INFERRED, not measured).**
  - Item 8 covers immediate fires only. The GL path relies on the frame order (Renderer.cpp:544-560 runs before
    :1831-1836).
  - Test mode has no locked beat clock, so the probe cannot fire a quantized trigger.
  - Residual: a 1-frame flash if an engine tick lands inside that window within one frame and the next tick is late.
  - Check: Boris's check 6, with Quantize on.
- **RK3 -- Q1 delays what Boris sees.** Mitigation: Q1 is asked now, with a recommended answer. bf6 cannot start before
  bf9b merges anyway.
- **RK4 -- Parked clips show a curve value, not the hand value.** If Boris reads a parked clip's knob as wrong, the
  rule is one line in clipTimelinePosition, plus TC11 and TC12.
- **RK5 -- R13 is probabilistic (INFERRED).** With a 16.7 ms frame and 10 ms sampling, the chance of missing a real
  flash across 10 re-fires is about 1e-5. The chance grows at a slower cadence, hence the G3 rule (20 re-fires if
  slower than 20 ms).
- **Strongest counterargument to this ruling.** "It grows a 7-item lane into one with an extra MainComponent item, a
  serializer parameter and about 30 tests. That is over-building for one broken control."
  It loses, because each addition is one of these:
  - a visible defect bf6 would otherwise ship: a flicker on every re-fire of a Timeline knob;
  - a silent behaviour change bf6 would otherwise make: composition-level legacy knobs freezing;
  - a gate that would otherwise pass without running anything: G1.
  What was cut: the virtual-playhead default, registry id / index tests, a TSan gate, live pause and scrub rows, and
  pixel-luma bars on greyed menu text.

### 7. NOTES FOR HARMONY

- Relay C1, C2 and C3 (AM-1, AM-6) to bf9b's architect NOW.
- Q1-Q3 go out now, in one message (G8).
- Learning, for Harmony to log. It was not logged here, because a project-repo boot has no log-event fence:
  - In this repo, `ctest -R` matches Catch2 TEST_CASE names, not target names (catch_discover_tests without
    TEST_PREFIX).
  - A gate regex on target names can select 0 tests and pass without running anything.
  - Run the binaries by path and check `--list-tests` instead.

STATUS: DONE
