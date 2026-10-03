# Blind council seat papers -- plan-bf9b-merge.md (s-rta-1003)

Verbatim record of the four seat papers (4 of 4 seats returned), pretty-printed, one
section per seat. Written by the Architect BEFORE ruling. Nothing below is edited,
re-ordered or summarised; the ruling is `.harmony/.reports/s-rta-1003/ruling-bf9b-merge.md`.

Plan attacked: `.harmony/.reports/s-rta-1003/plan-bf9b-merge.md`. Lane pin: `a7491d4`.

---

## Seat 1 -- memory-safety

```json
{
  "seat": "memory-safety",
  "attacks": [
    {
      "id": "ME-1",
      "target": "P1 / plan :9-10 (\"every holder of a pointer into a Layer or a Clip\"), F7 :43 (\"No other member holds one\"), P6(c) :314-318 (\"NOT a lasting dangling pointer\")",
      "claim": "The Clip inspector is a live holder the hook does not forget or re-point. The stack-shrink hook re-points only the Layer inspector. Direct Layer > Remove Layer (not undo, so refreshAfterUndoRedo never runs) frees the erased ClipRow's clip buffer while ClipInspector::clip_ still points into it. Every 30 Hz tick then reads freed memory. This is pre-existing (main has the same handler shape), but the plan says the class is closed and P6(c) says the Clip pointer is not dangling. The 8-step live ASan gate cannot reach it (no REST Remove Layer driver, plan :184-185), and A3 covers only the Layer inspector. ClipCell::clip_ (ClipCell.h:95) is a second holder the F7 list omits.",
      "evidence": "Pin a7491d4: MainComponent.cpp:707 (selecting a cell calls inspectClip(clip,...)); :6731 case C::kLayerRemove -> pushCommands -> RemoveLayerCmd -> Composition.h:519-526 eraseLayer -> d.rows.erase destroys the last ClipRow and frees its clips buffer. The only hook is MainComponent.cpp:1800 -> repointLayerInspector :5304 (Layer inspector only). The handler then only calls rebuildGrid (DeckView.cpp:114-262, which does not touch the inspector). MainComponent.cpp:4196 -> InspectorPanel.cpp:170-177 tickModulation runs unconditionally -> ClipInspector.cpp:900-905 reads clip_ (and effectStackView_.tickModulation reads effects_). Pre-existing: main:src/MainComponent.cpp:6853-6869 (same shape). Plan :158-161: A3 is the Layer inspector, A6 is Undo of Load Deck only. Kinship: kLayerClearClips row->clips.clear() at MainComponent.cpp:6774 also destroys the inspected Clip with no inspector re-point.",
      "severity": "MUST",
      "proposed_change": "Make the hook body forget and re-point BOTH inspectors: add forgetClip() then repointClipInspector(...) (the pure function P1.4 already builds) to the onLayerStackMoved body, and give lint (i) a `repointClipInspector(` token. Add an asan case A3b: select a cell in the LAST layer (Clip inspector bound), run RemoveLayerCmd through the real fence and hook, then tickModulation + refresh; RED at the pre-fix head. Correct F7 (add ClipInspector::clip_ and ClipCell::clip_) and P6(c). File Clear Layer/Deck Clips and column-growth resizes as the same class (not hook-reachable), or add a repointClipInspector after those handlers."
    },
    {
      "id": "ME-2",
      "target": "P1.5/P1.6 grip rule 6 + test A8 + MU4 + P3 lint (i)",
      "claim": "Rule 6 (release every human Held rank-3 grip on a stack move) is placed in the MainComponent hook body, but only repointLayerInspector / repointClipInspector are moved into the headless pure functions. A8 therefore either tests a test-local copy of the rule or has no code to drive. This is exactly the P3 defect the plan set out to cure (a headless case installing its own lambda). Lint (i) pins only `forgetLayers(` and `repointLayerInspector(`, so deleting the rule-6 lines from the hook leaves A8 and the lint green. \"RED without rule 6 in any build\" cannot be true of the shipped wiring, and MU4 names no file in which the mutation bites.",
      "evidence": "Plan :127-131 (InspectorRepoint.h holds only the two re-point functions); :132 (\"the hook body becomes: deckView_->forgetLayers(); the grip rule (6); repointLayerInspector()\"); :143-145 (the rule is described as hook code); :163 (A8); :167 (MU4); :248-249 (lint (i) tokens). The hook is a lambda in MainComponent.cpp:1800, which no headless test can run; the existing headless case installs its own lambda (plan :241 \"FINDING TRUE\").",
      "severity": "MUST",
      "proposed_change": "Move rule 6 into a pure function (e.g. releaseHumanHolds(Composition&) in InspectorRepoint.h) that the hook calls. A8 drives that function. Lint (i) also requires `releaseHumanHolds(` in the hook statement, in order after forgetLayers( and before repointLayerInspector(. MU4 deletes that call line in MainComponent.cpp and must turn the lint RED."
    },
    {
      "id": "ME-3",
      "target": "P1.6 grip rule, human Held bullet (plan :141-146) vs F7 holder list",
      "claim": "Rule 6 releases only layer scalarConns, yet F7 itself lists the layer EffectStackView row controls (fx.paramConns[p], fx.dryWetConn) as holders. forgetLayer() drops those rows' conn_ (setEffects(nullptr) -> rebuildRows forgets first), so a human slider drag in progress on a layer-effect row never gets its onDragEnd release (it is guarded by `if (conn_)`). Held rank 3 \"never expires on its own\", so that effect parameter stays frozen against every modulation source until the user touches that slider again. This is the stuck-grip class the plan names, left open for the effect rows.",
      "evidence": "UniversalParamControl.cpp:27-28 (onDragStart gripHeld / onDragEnd `if (conn_) conn_->release`); EffectStackView.cpp:369 and :424 (rows bound to fx.dryWetConn / fx.paramConns); EffectStackView.cpp:256-280 (rebuildRows forgets conn_ first); ParamConnection.h:107-108 (\"Held ... never expires on its own\"); plan :141-146 (rule covers \"every shared layer's scalarConns\"); plan :38-42 (F7 lists the effect rows). The grip stays on the moved or kept layer's layerEffects slot because a move preserves it (ParamConnection.h move = default).",
      "severity": "SHOULD",
      "proposed_change": "Extend rule 6 to every ParamConnection a forgotten widget could have been holding: each shared layer's layerEffects[*].paramConns and dryWetConn, with the same Held && rank==3 predicate. Add A8b for a held effect-row slider. Alternatively have forgetLayer() release Held rank-3 grips on the connections it is about to forget, BEFORE dropping them (memory is still valid at that moment only for the removeDeck-style pre-mutation case, so the hook version is the safe one)."
    },
    {
      "id": "ME-4",
      "target": "P1.6 first bullet (plan :137-140, \"never stuck\")",
      "claim": "\"A routine hand's grip travels WITH the layer ... the routine lets go by ControlPath key ... so it is never stuck\" is false whenever the layer's INDEX changes. The key is positional and is re-resolved at release time. Move Layer Up/Down (MoveLayerCmd -> Composition::moveLayer) keeps size and data() unchanged, so onLayerStackMoved does not fire. The Lane grip travels with the moved Layer to a new index while the routine's release resolves the OLD index, which is now another layer. The moved layer keeps a Held Lane grip that nothing releases (no routine stop on a layer move). The plan also asserts the property holds on every stack move, but the hook cannot see this class.",
      "evidence": "ControlPath.h:67 (\"POSITIONAL fields only\"); MainComponent.cpp:4100-4105 manualRelease -> resolveControl(composition_, ..., p) per call; Program.cpp:49-56 resolveLayer by key.layer index first; Composition.h:537-557 moveLayer = erase + insert (size and data() unchanged); UndoService.cpp:62-70 hook condition `data() != stackBefore || size != stackSizeBefore`; routineEngine_.stopAll()/stopOnLayer call sites (MainComponent.cpp:634, :737, :2424, :3018, :6163, :7762) include none in the kLayerMoveUp/Down handlers (:6815-6850); ParamConnection.h:107-108 (Held never expires).",
      "severity": "SHOULD",
      "proposed_change": "Do not claim \"never stuck\". State the scope: a grip travels with the layer, and a routine releases correctly only while the layer's index is unchanged. For an index-changing edit (MoveLayer, a mid-stack Remove Layer undo), either stop routines whose footprint includes the moved layers (RoutineEngine::stopOnLayer for from and to) in the MoveLayer handler, or FILE it with a test A9 (routine grip on layer 2, Move Layer Up, routine stop, assert no connection holds a grip). Remove the sentence from the Boris-facing and gate text until then."
    },
    {
      "id": "ME-5",
      "target": "FIX-1 PROVES (plan :412-413) and A3",
      "claim": "The PROVES clause requires A3 (Remove Layer of the inspected last layer) to be RED on ASAN-PRE, but it cannot be. Erasing the last element of Composition::layers destroys it in place and the vector's storage stays allocated. The dangling targets are scalarConns/scalarLive, a std::array INSIDE the Layer, so there is no freed heap block for ASan to flag. The layerEffects rows are forgotten by setEffects before they are read. The functional asserts (inspector empty, then re-pointed after undo) already pass at the pre-fix head. So A3 has no tooth for P1 at the pre-fix head, and the \"RED where it can be\" hedge hides which cases those are. (INFERRED: Apple libc++ vector container annotations are not assumed on; the builder should confirm on the RED run.)",
      "evidence": "Composition.h:519-523 (layers.erase of the last element); Layer.h:302 (`std::array<ParamConnection,...> scalarConns` inline in Layer); plan :38-40 (F7 itself says the storage is INSIDE the Layer); plan :159 (A3), :412-413 (PROVES lists A3 as RED-able); EffectStackView.cpp:256-280 (rows forget before they are read).",
      "severity": "SHOULD",
      "proposed_change": "Name the cases that must go RED under ASan at the pre-fix head: A1, A2, A5-twin, A6, A7 and the new A3b (ME-1, whose erased ClipRow buffer is heap-freed). Mark A3/A4 as functional regression guards, not memory teeth. Add a capacity assertion to A1/A7 (layers.capacity() < the 5 rows needed) so the realloc that frees the old buffer is guaranteed rather than depending on the vector's growth history."
    },
    {
      "id": "ME-6",
      "target": "A2-live RED arm (plan :187-188, :459-461, \"expected RED at L2\")",
      "claim": "The pre-registered RED location is probably wrong, and the RED arm cannot discriminate the Load Deck fix. The lever (ADNA_INSPECT_LAYER) binds the Layer inspector at startup. The plan's own F5 says refreshUiAfterModelSwap's post-swap setLayer(nullptr) reads freed memory. L1 \"load a 3-layer show\" (and L8) go through that swap path. ASAN-PRE would then abort at L1, before L2's Load Deck path is ever run, so the reviewer's actual trace (insertLayer realloc under the hook) is never shown RED at the live level. The literal \"RED at step L2\" check could then fail or be hand-waved.",
      "evidence": "MainComponent.cpp:2333-2344 (lever, runs once startup is done); MainComponent.cpp:2955-2959 / plan :30-32 (F5: setLayer(nullptr) after the swap reads freed memory; same line on main at :3038); plan :181-184 (L1 loads a show, L2 load_deck, L8 load_composition); plan :187-188 and :459-461 (\"RED at step L2\").",
      "severity": "NIT",
      "proposed_change": "Pre-register \"first RED step is L1 or L2, the report names which\". Add a PRE variant whose 3-layer show is supplied at launch (no model swap before L2), so L2 is the first step to fire. Likewise add an MU5 live twin. Otherwise A2-live's RED arm proves only F5, not the Load Deck path."
    },
    {
      "id": "ME-7",
      "target": "A2-live steps L5/L6 (plan :183-184) vs R9 (plan :573-574)",
      "claim": "The ASan live script has no VALID clause on the step that is supposed to drive P2's undo path. If the REST fire in L6 is an Undo step (R9 admits this is only INFERRED), /api/debug/undo undoes the fire, not the Duplicate Deck. The AddDeck/InsertDeck undo-with-retire never runs under ASan, and the script still prints GREEN (8 steps). Only the probe-boxes row k9d is protected by the \"numDecks dropped by 1 after ONE undo\" clause. The ASan row is the only gate that exercises the retire/restore memory moves (retiredDecks_ push_back and reap) through the real chain.",
      "evidence": "Plan :183-184 (L6: fire, switch, /api/debug/undo, no validity check); plan :231-236 (k9d has the numDecks VALID clause) and :573-574 (R9: \"INFERRED from both reviews\"); ApiServer.cpp:2162-2168 (the undo is marshalled with callAsync and answered at once, so a wrong-target undo is silent).",
      "severity": "SHOULD",
      "proposed_change": "Copy k9d's VALID clause into L3/L6: read numDecks and layer activeClip via GET before and after the undo. If numDecks did not drop by one, print INVALID and exit non-zero (never GREEN). Add L6b: redo, and assert the restored deck's id and retiredDeckCount==0, so the restoreRetiredDeck path also runs under ASan."
    }
  ],
  "strongest_point": "ME-1: the plan claims one forget-then-re-point rule for every holder of a pointer into a Layer or a Clip, and calls the Clip pointer not dangling (P6(c)). But direct Layer > Remove Layer heap-frees the erased ClipRow's buffer. The hook re-points only the Layer inspector, and InspectorPanel::tickModulation reads ClipInspector::clip_ unconditionally every tick. Nothing in the plan or its 8-step ASan live script reaches this path. The same flaw exists on main, so the merge does not make it worse, but the plan would certify the class as closed while it is still reachable.",
  "citations_rechecked": true
}
```

---

## Seat 2 -- gates

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "P1 live call-chain gate A2-live (plan :171-189, :459-462), MU3 (:166), lever fix #7 (:150-151)",
      "severity": "MUST",
      "claim": "The live ASan row cannot bite after its own step L1. L1 loads a show, which is a model swap. The swap unbinds the Layer inspector and sets the selected layer row to -1. Steps L2-L8 therefore run with no bound Layer inspector, so the P1 read never happens. MU3 (delete the wiring line) would leave the live row GREEN, although the plan says it turns RED at L2. The pre-registered 'RED at step L2' on ASAN-PRE is also misplaced. With the lever's startup binding of layer 1 still in place, the RED comes at L1, from F5's setLayer(nullptr) after the swap. At L2 the pre-fix hook would resolve 'no row' and rebind nothing. The row is therefore either RED at the wrong step, or vacuous.",
      "evidence": "Plan :179-185 puts 'L1 load a 3-layer show' before 'L2 load_deck'. Plan :150 adds selectLayer to the lever once at startup. src/MainComponent.cpp:2946-2959 (a7491d4) calls getLayerInspector().setLayer(nullptr) after the swap, at :2958. :2983 calls deckView_->selectLayer(-1), and DeckView.cpp:475-480 sets selectedLayerIndex_ = -1. The lever at MainComponent.cpp:2333-2343 runs once, in callAsync. MainComponent.cpp:5304-5320 resolves the layer from getSelectedLayerIndex() and returns nullptr when it is -1. Plan :186-189 and :460-461 pre-register 'RED at step L2'. Composition::initDefault (Composition.h:200-216) already gives 3 layers, so L1 is not needed to create a stack. INFERRED: that L1 uses /api/load_composition; the plan names no other loader.",
      "proposed_change": "Drop the L1 load and start from the default 3-layer show, which the lever binds at startup. Or add a test-server route that re-binds the inspector and call it after every swap. Add a VALID precondition before each step: read the bound layer from /api/debug/ui_text and print INVALID, never GREEN, when the inspector is unbound. Re-pre-register the ASAN-PRE RED location after dry-running the order. Make MU3 on the ASan app a required recorded RED for this row, not a 'second tooth'."
    },
    {
      "id": "GA-2",
      "target": "P1 item 5 hook body, rule 6, tests A7/A8, MU2/MU4, lint B4h(i) (plan :127-135, :143-145, :163-166, :248-251)",
      "severity": "MUST",
      "claim": "The plan re-creates the P3 defect one level down. The hook body is forgetLayers(), then the grip rule, then repointLayerInspector(). It lives in a MainComponent lambda, and only two pure functions are extracted: repointLayerInspector and repointClipInspector. A7 ('hook body') and A8 (grip released after a stack move) can only drive a test-local copy of that body. MU2 and MU4 would mutate the copy, so the mutants cannot bite production. Lint (i) pins only the substrings forgetLayers( and repointLayerInspector( inside the statement. It does not pin the grip-rule call or the order. Deleting the grip-rule line passes the lint and every test. Rule 6 is also narrower than the holders in F7. It releases only layer scalarConns. A human Held grip on an effect row's fx.paramConns / dryWetConn would stay stuck, because that row is forgotten and its mouse-up release goes through the nulled conn_.",
      "evidence": "Plan :127-131 lists only the two pure functions. Plan :132 describes the hook body as a MainComponent.cpp edit. Plan :163-164 (A8) and :161-162 (A7) drive 'a stack move, hook body'. Plan :248-251 lint (i). Plan :143-145 says 'for every shared layer's scalarConns'. Plan :41-42 (F7) lists fx.paramConns and dryWetConn as holders. UniversalParamControl.cpp:28 shows onDragEnd releasing through conn_, and UniversalParamControl.h:146-151 shows forgetConnection nulling conn_. The reviewers' SHOULD-1 (gates-r2 :1, live-r2 SHOULD-1) is exactly the gap that the lambda at tests/test_show_model.cpp:1476-1481 copies the body.",
      "proposed_change": "Put the whole hook body in one named function in InspectorRepoint.h, for example onLayerStackMoved(DeckView&, LayerInspector&, Composition&). That function contains forgetLayers, a named endHumanHolds(Composition&) and the repoint, in that order. The MainComponent lambda becomes a one-line call. Lint (i) must require that call, and A7/A8 must call the same function. Extend endHumanHolds to every Held rank-3 grip on the layer's scalarConns, layerEffects paramConns and dryWetConn. Add one assertion per kind."
    },
    {
      "id": "GA-3",
      "target": "B3b ASan unit gate and its RED arm (plan :168-170, :455-457; P1 TESTS :152-167)",
      "severity": "SHOULD",
      "claim": "Three weaknesses. (a) The RED arm 'the same script on the M3 head's tree (>= 1 report)' cannot run. That tree has no [asan] label, no A cases and no InspectorRepoint.h. The script would fail closed with 'no [asan] case' (as the tsan script does at exit 3), which is not an ASan report. The RED evidence reduces to raw lines the builder pastes. (b) The tsan precedent pins the sanitizer environment and FAIL_REGULAR_EXPRESSION in the CTest properties, so the caller's shell cannot hide a report. The asan plan relies on the script exporting ASAN_OPTIONS and on a grep of --output-on-failure output, which prints nothing for a passing case. (c) 'Each case labelled asan' cannot be done per case. catch_discover_tests applies PROPERTIES to a whole binary. The pinned EXPECTED_ASAN_CASES would then be the total count of test_show_model plus test_layer_strip_source_deck. That contradicts 'add T6g to the label'.",
      "evidence": "probe-tsan-unit.sh:55-61 (fail-closed exit 3). tests/CMakeLists.txt:3230-3240 (ADNA_TSAN_TEST_PROPERTIES with ENVIRONMENT and FAIL_REGULAR_EXPRESSION, and the comment explaining why). tests/CMakeLists.txt:3274 and :3318 (catch_discover_tests with PROPERTIES on a dedicated binary). Plan :168-170 and :455-457 (the B3b RED arm 'the same script').",
      "proposed_change": "Define ADNA_ASAN_TEST_PROPERTIES like the tsan set, with LABELS asan, ENVIRONMENT ASAN_OPTIONS=abort_on_error=0:halt_on_error=1 and FAIL_REGULAR_EXPRESSION 'ERROR: AddressSanitizer'. Use a dedicated asan target, or catch_discover_tests with a TEST_SPEC on an [asan] tag. State the RED arm as 'a scratch tree at the M3 head with only the label and the existing bf9b fix case added, run through the same script, >= 1 report', and give it its own pre-registered line."
    },
    {
      "id": "GA-4",
      "target": "P1 tests A1-A8 and 'RED where it can be' claim (plan :155-167, :412-413)",
      "severity": "SHOULD",
      "claim": "A memory test that does not prove the storage was freed cannot fail under ASan. Only A1 and A2 are tied to a reallocation, and even there no precondition assertion is listed. A3 (Remove Layer of the last layer) erases an element, which frees nothing: the Layer is destroyed in place inside a live buffer. A4 (Add Layer) may fit in spare capacity. The plan nonetheless lists A3 among the cases that are 'RED on ASAN-PRE where they can be', and the functional assertions it names (inspector empty, re-pointed) already hold pre-fix. A5-A7 are new-API cases. Their pre-fix 'RED' is a compile error, not a memory report.",
      "evidence": "Plan :412-413 ('every A case RED on ASAN-PRE's tree where it can be (A1, A2, A3, A5-twin, A6, A7)'). Plan :159. src/model/Composition.h:518-524 eraseLayer is layers.erase(...). Composition::initDefault uses push_back at :203-212, so capacity is only a growth artefact. The reviewer's case asserts '&layers[1] != before' (state-r2 finding 1), but the plan carries that assertion into no A case.",
      "proposed_change": "Every A case starts with a REQUIRE that layers.data() (and, for A6, the Deck's row buffer) moved, or that the deck object died. If it did not, the case is INVALID, not green. Drop A3 and A4 from the 'RED under ASan' list, or give them a fixture that forces a reallocation. Say in the plan which A cases are ASan-RED and which are functional-only."
    },
    {
      "id": "GA-5",
      "target": "P2 probe row k9d_undo_load_playing and risk R9 (plan :231-237, :573-575); P2 'INFERRED reach' (:194)",
      "severity": "SHOULD",
      "claim": "The k9d driver rests on a premise the code contradicts. The plan assumes a REST fire pushes no Undo step. A REST or OSC trigger_clip calls handleClipTrigger with the default Origin::Human and pushes a TriggerClipCmd whenever the layer's runtime changes. The first Cmd+Z / debug undo after the fire would therefore undo the trigger, and numDecks would not drop. The row's VALID clause would fire, so k9d FAILs on merge day instead of being GREEN. The stated reach of P2 ('REST / OSC / a routine / a queued or autopilot fire') is also wrong for REST and OSC.",
      "evidence": "src/MainComponent.cpp:1877 (apiServer_->onTriggerClip = handleClipTrigger(layer, column)). src/MainComponent.h:542 (Origin origin = Origin::Human). src/MainComponent.cpp:4873-4879 (push under Origin::Human when the runtime changed). OSC at :2209-2210 takes the same path. Plan :194 and :573-575 (R9, 'INFERRED from both reviews').",
      "proposed_change": "Fix the driver now. Fire through a path that is not an Undo step: a take or routine replay (Origin::Replay, as K7 does), the autopilot, or a bar-snapped queue that the GL thread performs. Pre-register that driver and its VALID clause. Reword P2's reach to 'a routine, the autopilot, a queued fire', and drop REST/OSC from it."
    },
    {
      "id": "GA-6",
      "target": "B6 PERF bar and its fallback (plan :476-481, R12 :580-582)",
      "severity": "SHOULD",
      "claim": "The BAR cannot see the change it guards, and its tolerance is undefined. frame_time_ms is an EMA (alpha 0.1) of CPU submission time between renderStart and renderEnd. It excludes the autopilot and playlist walk (which run before renderStart), the capture and recorder work, and GPU execution. The lane collapsed per-deck work into one pass and one autopilot, so most of the changed cost lies outside the measured window. '3 x pooled SD' does not say whether it uses the SD of EMA samples within a run or the SD of run means. If it is within-run SD, the tolerance is wider than any plausible 20-deck regression. No injected regression proves that the new interleaved driver can fail. On a busy machine the row prints BLOCKED and the fallback is 'the lane's reading', which the plan itself labels INFERRED.",
      "evidence": "src/render/Renderer.cpp:509-520 (show autopilot, before renderStart). :685 (renderStart). :822-834 (frameMs measured from renderStart to renderEnd; EMA at :833-834). Renderer.cpp:298-299 comment: peak_frame_time_ms misses work before renderStart. Renderer.h:437-441 and ApiServer.cpp:1438 expose peak_callback_ms, which includes it. Renderer.h:450-452 gives gpu_time_ms. Plan :476-481 names only frame_time_ms. Plan :580-582 gives the fallback.",
      "proposed_change": "Sample the callback cost (a mean of peak_callback_ms over fixed short windows) and gpu_time_ms as well as frame_time_ms. Define the SD precisely (SD of the per-run means over >= 5 runs). Give the driver a sensitivity proof: an injected 2 ms busy loop on a scratch copy must make (ii) FAIL. State now that 'B6 BLOCKED' needs a recorded waiver from Harmony or Boris, with no 'covered by reading' wording."
    },
    {
      "id": "GA-7",
      "target": "P4 'covered by' ruling and the K verdict string (plan :257-284, :463-468)",
      "severity": "SHOULD",
      "claim": "A BLOCKED bar is being dressed as covered. (a) The pre-registered line `PROBE-BOXES BLOCKED 1 (...)` carries only a count. The shell prints ${NBLOCKED} and no row names. Any other row going BLOCKED (a missing fixture, or the new k9d / k5_queue_tempo_feed) prints the identical line. The 'BLOCKED set == {k5_queue_link_on}' check is then a manual step with no gate string. (b) The 'covered' claim has no failing arm. k5_queue_tempo_feed is expected RED on MAIN0 only because the switch cancels the queue, which is the same RED as k5_queue_link_off. No mutant shows the row would go RED if the tempo path disturbed a queue. B4i is an identifier-absence lint, and it ignores applyTempoCommand and BPMTracker, where the 'link' action does its work. (c) rows are listed by hand in main(). gates-r2 NIT 4 already shows a new row silently not run when it is missing from that list. Nothing pins the number of rows executed.",
      "evidence": "probe-boxes.sh:83-86 (NBLOCKED is a count only). probe-boxes.py:1518-1535 (the rows list in main(); the k1b_duplicate omission is gates-r2 NIT 4). Plan :272-274 (the tempo_feed expected RED), :275-278 (B4i), :279-282. MainComponent.cpp:5665-5703 (action 'link' sets manual mode and calls followExternalTempo). I VERIFIED that REST set_bpm sends 'link' (MainComponent.cpp:1885-1888), so the plan's INFERRED F13 is now VERIFIED.",
      "proposed_change": "Make the verdict line list the identities: `PROBE-BOXES BLOCKED 1 [k5_queue_link_on]`. Pin the exact list of `--- <row>` headers expected in py.log, or the PASS count, as part of the gate. Add a mutant, MU-tempo: make applyTempoCommand('link') cancel the pending queue on a scratch copy, and require k5_queue_tempo_feed to go RED. Record the K5 Link-on line as 'NOT RUN (BLOCKED), accepted by <ruler>', not as 'covered'."
    },
    {
      "id": "GA-8",
      "target": "P6 tests G1 and M-c (plan :327-334, :482-484); K8 as the gate (:338)",
      "severity": "SHOULD",
      "claim": "'Every LayerStrip and every deck tab is the same object' is checked by comparing raw pointer vectors. After a destroy-and-recreate rebuild the allocator can reuse the same addresses, so the identity check can pass vacuously. The plan's 'RED today' relies on addresses differing. K8 is listed as a gate for P6 but its bar is peak_frame_time_ms <= 50 ms, which cannot see a strip rebuild or an unlit header. Only G1/G2 and B7 state 10 can.",
      "evidence": "tests/test_layer_strip_source_deck.cpp:504 and :515 (`CHECK(now == before)` over vectors from collect(), dynamic_cast<T*> pointers). Plan :328-330 (G1: 'the same object (RED today)'). Plan :338 and :331-334 name K8 as a P6 gate with 'bar unchanged'. ruling-bf9b K8b (`peak_frame_time_ms <= 50 ms at every switch`). Renderer.cpp:298-299 notes peak_frame_time_ms is blind to much of the work.",
      "proposed_change": "Track identity with juce::Component::SafePointer or with a per-instance construction serial (a static counter in the test fixture or the strip). Assert that every old SafePointer is still non-null after the walk. Drop K8 from the P6 gate line, or add a K8 sub-check on a model-visible signal. Keep the false-GREEN trap (the 8-column fixture) as an explicit mutant: MU11 must turn G1 RED."
    },
    {
      "id": "GA-9",
      "target": "P5 'ACCEPT' clauses (plan :286-305)",
      "severity": "SHOULD",
      "claim": "The clauses cited as deciding do not say what is cited, and the conditions are not gated. Item 3 cites 'plan S2.3 (:260): a deck holds no layers and no tuple' and says the surviving half is 4.B's row ':1607 InsertDeckCmd ... untouched'. In plan-bf9b.md the only matching sentence is at :269 ('there is no tuple to clear any more'). Line :260 is the end of S2.2. The :1607 row (plan-bf9b.md:464) is a test_undo_commands.cpp row, not test_composition. Item 6 cites S2.9 (:330) for 'PerfStateCapture captures the shared layers once ...'. That sentence is at :340 (:330 is the heading). Items 1 and 2 are conditional ('if not, FIX-2 adds it', 'builder greps'), but the only gate is 'none new; the review round's checklist names the six hunks'.",
      "evidence": "plan-bf9b.md:269 ('compload::duplicateDeck: rows copied ... there is no tuple to clear any more'). plan-bf9b.md:463-464 (test_undo_commands.cpp:2840 and ':1607 InsertDeckCmd' rows) against :473 (test_composition.cpp:1322 imagePaths, a different row). plan-bf9b.md:340 (the PerfStateCapture sentence). Plan-merge :291-304 (the six ACCEPT items). The lane report's 'S2b deviations' item 3 (bf9b.md:942-956 at the pin) lists the six removed assertions.",
      "proposed_change": "Quote the deciding sentence with its correct line in each ACCEPT. For item 3, separate the layer-id half (clause: Composition owns the layers, plan-bf9b :442) from the queued-trigger half (:269), and drop the :1607 reference. Turn the conditions into FIX-2 PROVES lines with a named test or grep result each: the positive stopOnLayer twin, and the recording.md corner-note grep. Make the review round list the six hunks by test name and file."
    },
    {
      "id": "GA-10",
      "target": "Section 5 rows with no pre-registered string, and RED arms that cannot fail (plan :444-447, :439-443, :459-462, :469-481)",
      "severity": "SHOULD",
      "claim": "The claim that 'Harmony has ONE pre-registered gate list' is not true for several rows. (a) H1's strings are deferred to 'M3's report', and R11 says that if M3 measures something else, M3's report wins. (b) G-1 is 'grep of the run list for by-name quits == 0 hits' with the pattern left to 'M2's sweep'. probe-quit-ours.sh still quits by name through osascript at line 38, retained by H-3. A literal grep therefore hits it, or the pattern is silently loosened. (c) 'MAIN0: RED on the rows that had a RED STAGE_P arm', with any difference 'LISTED, never re-thresholded', means a row expected RED that is GREEN on MAIN0 is logged and not gated. That row's test cannot fail. (d) A2-live and B6 may print BLOCKED, and 'Harmony rules' or 'Harmony decides whether the merge waits'. Only K5 has the 'never GREEN' cap. (e) B2's count equation uses lists the builder writes ('both lists in the lane report'). Nothing pins the retired set to the four named badge cases.",
      "evidence": "Plan :469-474 (H1: 'its exact strings are M3's'), :579 (R11), :446-447 (G-1), :441-443 and :467-468 (MAIN0 'LISTED'), :461-462 and :480-481 (BLOCKED to Harmony), :449-451 (B2). .harmony/probe-quit-ours.sh:38 (`osascript -e 'tell application \"Audio-DNA\" to quit'`). rulings-bf9b-mergein.md H-3 (NO CHANGE for that behaviour) and H-5.",
      "proposed_change": "Pre-register the H1 PASS line now. Name the G-1 grep pattern and its single exempted file. State that a MAIN0 row expected RED but GREEN means the row is INVALID and goes to Harmony as a stop, not a note. Give A2-live and B6 an explicit BLOCKED line plus a recorded waiver rule. Pin B2's retired set to exactly {M-b, M-d geometry, M-f, badge click}, with every other removal a gate failure."
    }
  ],
  "strongest_point": "GA-1. The live ASan call-chain row is the gate meant to catch a holder nobody listed and to prove the wiring. It cannot do either as ordered. Step L1 loads a show, and that model swap leaves the Layer inspector forgotten and the selected row at -1 (MainComponent.cpp:2958, :2983). L2-L8 therefore run with nothing bound to read. MU3 (delete the wiring line) stays GREEN live, which contradicts the plan's own tooth. The ASAN-PRE 'RED at L2' string is pre-registered at the wrong step. The fix is cheap: start from the default 3-layer show, or add a re-bind route, and add a bound-inspector VALID precondition before each step.",
  "citations_rechecked": true
}
```

---

## Seat 3 -- live-show

```json
{
  "seat": "live-show",
  "attacks": [
    {
      "id": "LI-1",
      "target": "P7 'A removed deck's still-playing clip now looks like any playing clip ... no replacement mark is proposed' (plan :363-366); B7 live state (3) and (10) (:370-372, :375-376); Boris step 8.5 (:521-524)",
      "claim": "The plan's only remaining trace of a removed deck's playing clip is the Undo-Remove hint, and the plan misdescribes it. It does not last 'until the next structural change': it dies after 10 s. On the plan's own 20-deck bf9b-check show it is never drawn at all. After that a layer plays a clip from a deck that exists nowhere (no lit cell, no dot, no tab, no hint, no badge) and Boris has nothing to look at. Step 8.5 ('the button at the end of the tab row reads Undo Remove ...') cannot be performed on that show.",
      "evidence": "(a) DeckView.h:190 kUndoHintMs = 10000; DeckView.cpp:704-706 hides it after that delay. (b) DeckTabRow.h:29-30: plus.x = min(n*(w+2), rowWidth-24) and the hint is drawn only if rowWidth - hintW >= plus.x + 24 + 8; DeckView.cpp:99-102 then calls hideUndoHint() silently. When the tabs fill the row, w = floor((rowWidth-24-2n)/n) gives rowWidth - plus.x in [24, 24+n). The hint (text ~50 chars at 14 pt, ~400 px) therefore needs n > ~400, or tabs at the full 100 px (rowWidth >= n*102 + 32 + hintW). With 19 decks left after the removal that is a DeckView at least ~2350 px wide. DeckView::resized uses getLocalBounds() (DeckView.cpp:57-59), i.e. the full window width. (c) make-bf9b-check.py:14,24: Boris's show has 20 decks. (d) The lane's live capture of state (3) was a 4-deck fixture: bf9b.md at a7491d4 line 1326 'numDecks 3'. (e) showUndoHint has one caller, the Remove Deck path: MainComponent.cpp:3885. P2 adds a second route to a retired deck (Undo of Add/Load/Duplicate Deck) and that route shows no hint at all. (f) rebuildGrid calls hideUndoHint (DeckView.cpp:117) and has 31 call sites in MainComponent.cpp, so the hint is also lost on any ordinary edit.",
      "severity": "MUST",
      "proposed_change": "1) Correct the plan text (10 s, or earlier on any rebuildGrid or lack of room) and drop the argument 'so no replacement mark is proposed'. 2) Run B7 states (3) and (10) on the 20-deck show at the narrowest window the critics use; the hint must be visible there or the gate prints BLOCKED. 3) Give a removed or undone deck's still-playing layer a trace that survives 10 s and 20 decks. Cheapest: keep the layer names in the hint/file label until the layer is replaced or cleared, and let the hint truncate instead of hiding. Alternative: ask Boris as Q-D with default 'file label only'. This does not put a deck back on the strip, so it stays inside BF14. 4) Rewrite 8.5 so it can be performed: expect the strip unchanged and Cmd+Z bringing Deck 3 back, with the hint only if it fits."
    },
    {
      "id": "LI-2",
      "target": "Section 7 step 8.12 (plan :527-530)",
      "claim": "The step cannot be performed as written. The app has no 'File' menu. The chooser does not open in 'the same folder' as the show, and the deck file has no stated extension.",
      "evidence": "MenuBarModel.cpp:7-9 menu names are 'Audio-DNA','Composition','Deck','Layer',...; Load Deck... is under Deck (MenuBarModel.cpp:86). MainComponent.cpp:3656-3662: the chooser opens at CompDecksBrowser::getDecksDir() with filter '*.json'. The plan says 'File -> Load Deck and pick the deck file five-rows in the same folder' and defines the file only as 'a deck file five-rows' (:421, :527-528).",
      "severity": "SHOULD",
      "proposed_change": "Reword: 'Deck menu -> Load Deck... (or the + button at the end of the tab row)'. Name the exact file (five-rows.json). Either have FIX-3 write it into getDecksDir() or tell Boris to navigate to the bf9b-check folder. Add: a new Deck tab appears, the Layer tab keeps its layer. Note the strip highlight may be gone (see LI-4)."
    },
    {
      "id": "LI-3",
      "target": "P6 / section 7 step 8.2 ('Deck 6 is wider ... must not make the strips blink or jump', plan :517-519) and the P6 probe line (:335-336)",
      "claim": "As a hands-on check this cannot fail. On the unfixed build rebuildGrid runs inside one message-thread turn (showDeck rebuilds and returns), so no frame shows a blink or jump. The things the rebuild really loses (selected-layer highlight, undo hint, an open rename box, a held fader) are not what the step tells him to look at. 'Deck 6' is also ambiguous: the plan's own fixtures count from 0 (G1 'deck 5', B7 (1) 'deck 1's clip ... deck 2's tab') while Boris's tabs are 1-based names.",
      "evidence": "DeckView.cpp:359-365 (rebuild then return, no intermediate paint). The strip highlight is set only by DeckView::selectLayer (DeckView.cpp:475-479) and is never re-applied in rebuildGrid (DeckView.cpp:114-262; selectedLayerIndex_ is set only at :477). The plan says so itself at :58-60 / :310-312. make-bf9b-check.py:148 names decks 'Deck N' (1-based). Plan :335 'deck 6 eight columns' vs :328 'deck 5 has 8 columns'.",
      "severity": "SHOULD",
      "proposed_change": "Rewrite 8.2: 'Click the Layer 2 strip (it gets a highlight). Click the tab named Deck 7 (the wide one), then back. Expect: the highlight is still on Layer 2 and nothing else on the strips changed.' Fix the deck by tab NAME in make-bf9b-check.py and in the page. State in the plan that the 0-based test index and the 1-based tab name differ."
    },
    {
      "id": "LI-4",
      "target": "P6 G2 / F6-B (only the column header is cured on a full rebuildGrid) and P1 hook re-point",
      "claim": "Every other rebuildGrid still silently drops the selected-layer highlight while the inspector keeps showing that layer. The plan fixes only the lit header. The P1 hook makes it more visible: after Load Deck or Add Layer the Layer tab is re-pointed to the DeckView's selected row (repointLayerInspector), the strips are new, and no strip is highlighted. A stuck human fader grip is also left behind whenever a strip dies mid-drag outside the stack-move hook.",
      "evidence": "DeckView.cpp:114-262 has no call to setSelected/selectLayer; selectedLayerIndex_ survives (DeckView.h:139,199). MainComponent.cpp:6716-6728 (Add Layer): rebuildGrid() with no selectLayer. The Move Layer cases do re-apply it by hand (rebuildGrid(); selectLayer(...) at the kLayerMoveUp/Down sites). That shows the convention is per-caller and easy to miss. LayerStrip.cpp:446-451: the fader's gripHeld() is released only on drag-end through layer_; ParamConnection.h:108 'Held ... never expires on its own'. Plan rule 6 (:142-145) releases human holds only inside the layer-stack hook, not on the other rebuilds.",
      "severity": "SHOULD",
      "proposed_change": "Add one line to rebuildGrid: re-apply selectedLayerIndex_ to the new strips. Pin it next to G2 (same fixture, 'after a full rebuildGrid the highlight is on the selected layer'). Add to 8.12 'Layer tab still shows your layer AND that strip is highlighted'. Optionally have a strip being destroyed release its own Held rank-3 grip while layer_ is still valid. It must not do this in the forget path."
    },
    {
      "id": "LI-5",
      "target": "P7 'M-e tab dots: unchanged' (plan :355) and B7 states (1)/(7); Boris step 8.2 ('a small dot (a clip from that deck is playing)')",
      "claim": "The dot is 'some layer's ref names this deck', not 'a clip from this deck is playing'. It can light when nothing plays: a ref into an emptied or short cell, or a bypassed/muted/hidden layer. The lane already carved this exception out for the badge but not for the dot. After this change the dot is the only per-deck cue left, so a false dot is now the whole story. M-e proves only the ref predicate.",
      "evidence": "Composition.h:608-617 deckIsPlaying checks activeClipColumn >= 0 && activeDeckId == id (or a running fade's previous). It has no clipAt() or layer-state test, whereas the badge required Composition::playing(i).clip to exist (bf9b.md at a7491d4 line 1334 vs line 1336). Layer has bypassed/muted/visible/solo (make-bf9b-check.py LAYER_TEMPLATE). Test test_layer_strip_source_deck.cpp:372-411 covers only fire / fade-end / clear. INFERRED: the exact UI path that leaves a ref on an emptied cell was not traced.",
      "severity": "SHOULD",
      "proposed_change": "Decide the dot's meaning and pin it: at minimum require the referenced clip to exist (same carve-out as the old badge). Add a case 'ref into an emptied cell -> no dot'. Put 'bypassed layer: dot or not?' to Boris as a defaulted question (default: dot only when the clip is actually drawn). Add the case to M-e."
    },
    {
      "id": "LI-6",
      "target": "P2 / Q-B (plan :538-540) and R9",
      "claim": "Under the default, Cmd+Z of Add/Load/Duplicate Deck can leave a layer playing from a deck that now has no tab and no hint (see LI-1). Q-B asks only about the same-row case. It does not tell Boris that in the wider-deck case (T6h) a clip playing in an added layer stops and the layer disappears, so his answer does not cover what the code does. Q-B is also phrased around 'a routine or the autopilot starts one of its clips', which he cannot easily reproduce, so he will answer without seeing it.",
      "evidence": "Plan :196-216 and :223 (T6h: the clip in ADDED layer 4 is erased, retired 0). MainComponent.cpp:3885 is the only showUndoHint caller (Remove Deck), so the P2 retire path shows no cue. Plan :531-533 'NOT on the page'.",
      "severity": "SHOULD",
      "proposed_change": "Add to Q-B: 'If the loaded deck added new layers, those layers go away with it.' Show a one-line cue when an Undo retires a playing deck (reuse the file label). Include one P2 live state in B7 (state 9 already exists) with the cue visible."
    },
    {
      "id": "LI-7",
      "target": "P6 F6-B rebuildCells + G1 (plan :323-333)",
      "claim": "rebuildCells keeps the selection prune, so switching to a narrower deck silently drops the multi-cell selection that Delete/copy/paste and binding actions act on, and switching back does not restore it. G1 asserts strips, tabs, hint and highlight but nothing about cell selection, the one thing the new switch path still changes.",
      "evidence": "DeckView.cpp:252-257 erases selectedCells_ entries with column >= numCols. MainComponent.cpp:6924, 6945, 6990, 7090-7097 act on getSelectedCells(). Plan :323-326 lists the selection prune as part of rebuildCells.",
      "severity": "NIT",
      "proposed_change": "Add one G1 assertion that a selection inside the common columns survives 0 -> wide -> 0 and 0 -> narrow. Document the dropped out-of-range cells as intentional. Or keep them and restore on return."
    },
    {
      "id": "LI-8",
      "target": "P1 forgetLayers (plan :123-126, :565-567, R6) and rule 6",
      "claim": "The plan's safety for forgetLayers rests on 'every caller rebuilds the grid', which it admits it did not re-read (R6: 'INFERRED'). A missed caller shows a blank strip column on stage, the worst visible failure in this lane. Rule 6 also lets go of a mouse-held fader mid-drag (R5): the slider then moves without a grip, so automation can fight his hand.",
      "evidence": "Plan :565-567 and :563-564. The hook fires on any fenced edit that moves the layer vector (UndoService.cpp:61-70); 13+ makeDeckFence sites in MainComponent.cpp (e.g. :6726, :6836, :6859, :6887, :6911, :3308). The two I read (Add Layer :6728, Move Layer) do rebuild, so this is not shown to be wrong. Only unread callers remain. A7 is a unit test of the strip, not of the callers.",
      "severity": "NIT",
      "proposed_change": "Add one text lint in the B4h family: every pushCommands site whose command can change the layer count/pointer (AddLayer, RemoveLayer, InsertDeck, AddDeck, Undo/Redo) is followed in the same function by rebuildGrid or refreshAfterUndoRedo. List the sites in the lane report. Make forgetLayers set a 'stale' flag that the next refresh() turns into a rebuildGrid, so a miss self-heals instead of staying blank."
    }
  ],
  "strongest_point": "LI-1. The plan removes the badge and then says a removed deck's still-playing clip needs no replacement mark because the Undo-Remove hint covers it \"until the next structural change\". The hint is a 10-second timer (DeckView.h:190, DeckView.cpp:704-706). On Boris's own 20-deck show it is silently never drawn: the layout hides it unless the tabs are at full width, and with 19 tabs that needs a DeckView about 2350 px wide (DeckTabRow.h:29-30, DeckView.cpp:99-102). The lane only ever captured it on a 4-deck fixture (bf9b.md line 1326). So step 8.5, state (3) and state (10) cannot show what the plan says they show. After 10 seconds a layer plays a clip from a deck that exists nowhere, with no trace on screen.",
  "citations_rechecked": true
}
```

---

## Seat 4 -- scope

```json
{
  "seat": "scope",
  "attacks": [
    {
      "id": "SC-1",
      "target": "P1 fork choice F1-C (forgetLayer/forgetClip/InspectorRepoint.h/hook edits at 3 MainComponent sites) vs runner-up F1-B",
      "claim": "The plan rejects the smallest class-wide fix (make LayerInspector::setLayer and ClipInspector::setClip forget their 7 / 6 scalar-control connections FIRST) on grounds its own grip rule contradicts, and in exchange ships ~6 new public methods, a new header, 3 MainComponent edits in the file four lanes rebase on, and a fix that by the plan's own admission can miss a site (R2). F1-B fixes the finding, the model-swap site and the Clip inspector in two edits inside two UI files, and covers holders nobody listed.",
      "evidence": "Plan :110-113 rejects F1-B because 'a Decaying touch would no longer be ended at once'; but plan :139-140 says for the stack move 'A Decaying touch travels too and expires by itself' and :137-139 that a routine grip is released by ControlPath key, never by a widget. A human Held grip cannot exist at an ordinary select (mouse is down elsewhere). Precedent in the codebase: ClipInspector::buildSourceParamControls forgets on EVERY setClip (ClipInspector.cpp:837, called at :795) and EffectStackView::rebuildRows forgets on every rebuild (EffectStackView.cpp:258-271); Pitfall 33 (docs/claude/pitfalls.md:75) already mandates forgetConnection 'FIRST ... clip re-point'. The bug is exactly setLayer calling syncFromLayer (LayerInspector.cpp:734) then bindScalarControls->bindConnection (UniversalParamControl.cpp:126-128) on the OLD conn_. Plan R2 :554-555 itself names F1-B as the 'three-line' fallback if one more site turns up.",
      "severity": "SHOULD",
      "proposed_change": "Adopt F1-B: first statement of LayerInspector::setLayer / ClipInspector::setClip = forgetConnection() on the scalar controls (leave the effect stack, already safe). Drop InspectorRepoint.h, public forgetLayer/forgetClip, A5/A6/A7 and MU1/MU2/MU5 as separate artifacts; A1/A2 call setLayer directly after a forced storage move under ASan. Keep only the single wiring lint for MainComponent.cpp:1800. Lost: a Decaying/Held touch on the old layer is not ended at once by an ordinary re-select (it self-expires; a programmatic select mid-drag could leave a Held grip, rare, pre-existing class). Acceptable on merge day; F1-C stays the follow-up if ASan finds anything."
    },
    {
      "id": "SC-2",
      "target": "P1 items 3 and 6: LayerStrip::forgetLayer / DeckView::forgetLayers in the hook, plus the 'grip rule' (release every human Held grip on a stack move), A7, A8, MU2, MU4",
      "claim": "Both are additions no review finding asked for, and they trade a dormant hazard for a live one. Strips are never read in the gap (every caller rebuilds the grid synchronously), so nulling them buys nothing and risks blank strips; the grip rule exists only because forgetLayers disables the strip's own mouse-up release, and it adds a new write to ParamConnection grip that the plan admits is not TSan-proven.",
      "evidence": "gates-r2 :8: 'Grid rebuild follows each caller synchronously ... so LayerStrip's Layer* (LayerStrip.h:166) is not read in the gap.' Plan R6 :565-567 admits 'forgetLayers leaves blank strips if some caller does not rebuild ... I did not re-read all of them (INFERRED)'. Strip release goes through layer_ (LayerStrip.cpp:447-450 `if (layer_) ...release`), so nulling layer_ is what strands a drag grip; a rebuildGrid already destroys strips with no release (DeckView.cpp:114-125) today, so a stuck grip mid-drag is a pre-existing class, not this finding. Plan R5 :561-564: 'not TSAN-proven'.",
      "severity": "SHOULD",
      "proposed_change": "Cut LayerStrip::forgetLayer, DeckView::forgetLayers, rule 6, A7, A8, MU2, MU4. The hook body stays 'repointLayerInspector()' (+ the SC-1 fix). Lost: a stack move during a mouse-held strip-fader drag may leave opacity gripped until next touch/undo (exotic: needs REST/Cmd+Z mid-drag); file it. Acceptable because the live ASan row (if kept) would catch a strip read, and a blank-strip regression would be a new visible defect."
    },
    {
      "id": "SC-3",
      "target": "P2: F2-A (retire on Undo of Add/Load/Duplicate Deck, incl. wide-deck redo) vs F2-C / F2-B",
      "claim": "P2 is the largest new logic in the plan for a case the plan itself says Boris cannot reach by hand, and both reviewers explicitly allowed 'document it'. The wide-deck Load redo (snapshot rows put back into a restored deck) is the one piece the plan's own R1 concedes is the weak point.",
      "evidence": "Plan :531-533: 'Every way Boris fires a clip by hand is itself an Undo step ... the case needs a routine, the autopilot or a queued fire.' Plan R1 :550-553: 'the counterargument is right about ONE piece: P2's wide-deck redo' and names F2-C as fallback. state-r2 finding 2 (:65-66): 'or state the exception in performance-controls.md and pin it in T6'; live-r2 NIT-2 (:21): 'Decide: retire-like behaviour, or document it'. Cost list: T6f-T6j, MU6-MU9, B4f re-pin (:229-230), probe row k9d with its own RED arm, redo/dispose ordering (steps 1-5 :207-215).",
      "severity": "SHOULD",
      "proposed_change": "Ship F2-C now: retireOrEraseDeck in AddDeckCmd::undo and in InsertDeckCmd::undo only when the command added no layers (Add Deck, Duplicate Deck); Load Deck of a wider deck keeps today's erase and is a documented, T6-pinned exception. Cuts T6g/T6h/T6i, MU7-MU9, the wide redo path and likely k9d (T6f/T6j cover it). Lost: Cmd+Z of a wide Load Deck blanks a layer playing a non-human-fired clip from that deck. Acceptable: unreachable by hand, redo restores it, Q-B (:538-540) already routes the question to Boris; file the wide case."
    },
    {
      "id": "SC-4",
      "target": "P6 (b): rebuildCells split of DeckView::rebuildGrid, G1, MU11, 8-column deck in make-bf9b-check.py and the Boris page",
      "claim": "The finding is a NIT in a hot ui-lane file; the plan answers it with a refactor (new rebuildCells, selection prune/layout/bands re-plumbed) that four rebasing lanes must merge around. The visible bug (a) is cured by a one-line predicate, (b) only matters on a switch between decks of different widths and costs an undo hint / open rename box.",
      "evidence": "live-r2 NIT-1 (:20): 'Cheap fix: resize cell rows/column triggers in place, or call refresh() after rebuildGrid()'. DeckView::showDeck (DeckView.cpp:349-365) already routes to rebuildGrid only when shape differs; rebuildGrid ends with selection prune + resized (DeckView.cpp:238-262) which the split must reproduce. Plan R7 :568-569 concedes 'splits a function other paths rely on'. Header unlit cause: setupColumnTriggers creates every header unlit (DeckView.cpp:427-450) while refresh() colours them (:296-303).",
      "severity": "SHOULD",
      "proposed_change": "Keep only the header predicate in setupColumnTriggers (G2, MU12), or F6-A (refresh() after rebuildGrid in showDeck). File (b). Lost: on a width-changing switch strips/tabs are recreated (hintUndo hidden, rename box/drag lost) - invisible otherwise; note the B7 interaction-logic critic may answer 'no' for state 10, so reword that question to 'cells, header and tab highlight change; picture unchanged'. Needs Harmony to accept the bar wording change."
    },
    {
      "id": "SC-5",
      "target": "P4: k5_queue_tempo_feed row + lint B4i with pinned MainComponent.cpp hit count + MU10",
      "claim": "A new live row and a count-pinned lint for a code path that is not compiled into the app Boris runs; the pinned count in MainComponent.cpp is a conflict/false-RED generator for the lanes that will touch tempo/sync next.",
      "evidence": "MainComponent.cpp:4244-4245: 'never enabled in a default build -- LinkSync::isAvailable() is false'; plan F13 :69-70 AUDIODNA_BUILD_LINK OFF. The row's driver is REST set_bpm, which 'turns manual mode on' (MainComponent.cpp:5655-5659), a state k5_queue_link_off does not necessarily share. B4i pins 'the only MainComponent.cpp hits are the toggle and the tempo feed (count pinned at the merged head)' (plan :275-278) while Boris's 2026-10-03 feedback orders sync saved with the composition and moved to the top bar (binding-decisions.md 'Sync' bullet), i.e. the next lanes edit exactly those lines.",
      "severity": "SHOULD",
      "proposed_change": "P4 = 'ruled covered by T5 + k5_queue_link_off, printed BLOCKED'. Drop k5_queue_tempo_feed and MU10. If a lint is wanted keep ONLY the zero-hit assertion over src/model, src/render, src/core, DeckView.cpp, and the three handler bodies; remove the MainComponent.cpp count pin. Lost: no executable check that a repeated tempo command does not disturb a queued trigger; acceptable since the Link tick is a no-op in the shipped build and T5 drives the beat path."
    },
    {
      "id": "SC-6",
      "target": "Section 6 docs: new Pitfall entry + CLAUDE.md index line; omitted stale badge text",
      "claim": "The new pitfall duplicates Pitfall 33, claims a number that collides with the reserved/lane numbering and spends CLAUDE.md bytes four lanes also need, while the plan misses the doc lines that actually become false.",
      "evidence": "Pitfall 33 (docs/claude/pitfalls.md:75) already says call forgetConnection() FIRST on 'row delete, rebuild, clip re-point' because bindConnection dereferences the old connection. Plan :500-506: 'ONE new entry (... 67 is the lane's, 68 is reserved for bf2)' plus ~180 B of CLAUDE.md; CLAUDE.md is 24,264 B at the pin vs cap 25,000 (main 23,962 B), shared with the other lanes' index lines. Stale and unlisted in F12 / section 6: docs/claude/pitfalls.md:139 (the lane's own pitfall: 'every entry: tab, strip badge, REST ...') and the tests/CMakeLists.txt:3453 comment; B4j greps src/ only.",
      "severity": "SHOULD",
      "proposed_change": "Append one sentence to Pitfall 33 ('also the Layer/Clip inspector scalar controls: forget before setLayer/setClip re-points after a storage move'); no new number, no CLAUDE.md edit. Add pitfalls.md:139 and tests/CMakeLists.txt:3453 to the P7 doc edits and widen B4j to docs/claude/*.md and .harmony/APP-INVENTORY.md. Lost: a separate triage line in CLAUDE.md for the lesson; acceptable since 33 is already the triage entry."
    },
    {
      "id": "SC-7",
      "target": "Section 5 B8 POST-MERGE: full re-run of B1, B2, B3, B3b, the K batch and H1 on main",
      "claim": "The plan applies the 'tree identical -> cite the FH result' shortcut to A2-live only, while the K batch, the longest live gate, is re-run unconditionally on a tree that is byte-identical to FH when the merge-in was done this session.",
      "evidence": "Plan :487-489: 'B1, B2, B3, B3b; then the K batch on the merged build ..., H1, and A2-live once on an ASan build of main UNLESS `git diff --stat FH main -- src tests cmake CMakeLists.txt` is empty, in which case ASAN-FH's run stands'. Ruling H-1 (rulings-bf9b-mergein.md:1-3): main is merged INTO the lane so Harmony's final merge is conflict-free; main since 5abdf01 has moved by docs commits (c6fbc14 is docs-only per git log).",
      "severity": "SHOULD",
      "proposed_change": "Make the whole of B8 conditional on that one diff-stat: empty -> B1 build of the merged tree + ctest only (or cite FH results with the diff attached), K/H1/ASan re-runs only for a non-empty diff in src/tests/cmake/CMakeLists/shaders. Lost: a re-run that would only catch a merge-commit mistake; the diff-stat catches the same thing. Needs Harmony because ruling-bf9b B8 (:~607) says K1-K10 'once more'."
    },
    {
      "id": "SC-8",
      "target": "P1 LIVE CALL-CHAIN GATE (A2-live): two ASan app builds, ASAN-PRE RED arm, MU3 on the ASan app, new lever line, probe-asan-live.sh",
      "claim": "Most of the plan's wall-clock risk sits in a gate whose own feasibility is ASSUMED (R3) and whose RED arm needs a second full ASan app build of the pre-fix head, to prove what B3b's unit RED arm (same reviewer trace) already proves. The stage order also front-loads two full-app ASan builds before any edit.",
      "evidence": "Plan :171-178 'ASSUMED ... an ASan AudioDNA links and starts in --test-mode ... no macOS crash dialog'; 'two ASan app builds (ASSUMED 10-25 min each at -j3)'; R3 :556-559 falls back to BLOCKED; FIX-1 :406-411 'FIRST, before any edit: configure build-asan, build AudioDNA + test_show_model + test_layer_strip_source_deck at the M3 head'. The RED reproduction already exists: state-r2 :44-52 (heap-use-after-free READ at UniversalParamControl.cpp:349 <- LayerInspector.cpp:734 <- UndoService.cpp:78). The lever edit (item 7) changes MainComponent.cpp:2330-2344 only to serve this gate. Whether L2 reproduces on ASAN-PRE also depends on the vector actually reallocating (3->5 layers; a spare capacity would pass silently) - not established.",
      "severity": "SHOULD",
      "proposed_change": "Keep B3b (unit ASan) as the blocking memory gate. Run A2-live on ASAN-FH ONLY, as a non-blocking best-effort row (BLOCKED allowed, merge not held); drop ASAN-PRE and MU3-on-app; build the ASan app in the background during FIX-2/3 instead of as step one of FIX-1 (ASAN-PRE can be rebuilt from `git archive` of the M3 head if ever needed). Lost: a live proof the row can fail and discovery of an unlisted holder before merge; acceptable because SC-1's F1-B removes the 'unlisted holder' risk class at its root. Harmony set the 'real call chain' constraint, so this needs its explicit OK."
    },
    {
      "id": "SC-9",
      "target": "MUST-NOT-CUT: P1 unit-level proof (A1 forced storage move through the production inspector + B3b probe-asan-unit.sh with a RED arm at the M3 head) and the P7 badge removal",
      "claim": "Whatever else is cut, keep (i) one ASan-instrumented test that moves Composition::layers under a bound LayerInspector and ticks it, RED on the M3 head and GREEN after the fix, and (ii) Boris's BF14 removal. They are the only items that are either the sole memory-safety gate or a binding user instruction.",
      "evidence": "state-r2 :44-52 reproduces the freed-memory read only under ASan; the Release gates (1157 tests) pass with the wiring deleted (gates-r2 :21). Boris verbatim: 'The layer strip does not need to show the deck a clip is playing from.' (binding-decisions.md 2026-10-03; plan F18 :92-93).",
      "severity": "NIT",
      "proposed_change": "Protect A1/A2 + B3b + RED arm and the badge deletion + B4j from any cut above; everything else in this list is deferrable."
    }
  ],
  "strongest_point": "SC-1: the plan picks the wide fix (F1-C: new public forget methods on three widgets, a new InspectorRepoint.h, DeckView::forgetLayers, three MainComponent edits) over the two-edit class-wide fix (F1-B: setLayer/setClip forget first) by arguing a grip-behaviour change its own rule shows is harmless, against the project's own precedent (forget on every rebuild, Pitfall 33), while admitting F1-C may miss a site (R2) and naming F1-B as the fallback. Taking F1-B first removes P3's need for pure re-point functions, A5-A7, MU1/2/5, and the strip/grip-rule additions (SC-2) that carry the plan's only new user-visible risk.",
  "citations_rechecked": true
}
```
