# WORK PACKET — Preset silent-retarget fix (designed 2026-08-03b, fable max effort)

STATUS: build-ready, NOT dispatched. Serialized BEHIND the black-overlay fix — both
packets touch MainComponent.cpp, so two concurrent builders would collide.
All line numbers verified at design time and DRIFT-PRONE: re-locate by anchor text.

## THREE INHERITED PREMISES ADJUDICATED FALSE

- **P-FALSE-1 — BindingManager does NOT have the same bug.** The handoff claimed it did and
  said it "should ride the same fix." `Binding::targetEffectIndex` (Binding.h:85) is consumed
  only by ToggleEffectBypass, which indexes `clip->effects` — the per-clip EffectSlot list
  (MainComponent.cpp:5099-5104), NOT the 135-effect global chain. Clip slots are user-ordered
  and carry effectName strings (Clip.h:48-56); library re-grouping cannot shift them. Surface
  is also DORMANT: buildBindableTargets (MainComponent.cpp:4733-4828) never creates a
  ToggleEffectBypass target and always passes effectIndex 0. => BindingManager is OUT OF SCOPE.
- **P-FALSE-2 — display names have NOT "drifted" from shader keys.** `git log -S` per name:
  divergent label/key pairs were AUTHORED divergent in one commit (2f4f638, cb742e6, 07485fd)
  and never renamed. Zero rename history. The rename risk is INFERRED (labels are UX-mutable
  by nature), not evidenced. Weakens but does not flip D1 — see below.
- **P-FALSE-3 — severity is ~52 effects, not ~100.** Category counts over all 135 defs against
  categoryOrder (Renderer.cpp:1518-1521): 3d 9, warp 27, color 31, glitch 15, pattern 19,
  animation 6, blend 5, blur 10, time 6, composite 3, audio 4. The 2 glitch adds shift
  pattern..audio (~52) by +2; the pattern add shifts animation..audio (34) by +1 more. Still
  fatal (silent-wrong), but the honest number is ~52 of 135 shifted by 2-3 positions.

## CORE BUG (re-verified at design time)
savePreset writes raw ints "targetEffect"/"targetParam" (PresetManager.cpp:113-114) and
"version",1 (:76, never read anywhere); loadPreset restores raw (:212-213) while the effect
restore just above resolves BY NAME (:162-179). MappingEngine skips out-of-range silently
(MappingEngine.cpp:176-180); EffectChain::getEffect bounds-checks to nullptr
(EffectChain.cpp:29-35). Mapping struct has NO name fields (MappingTypes.h:123-140).

## RULINGS
- **D1 KEY — dual-key, component-wise.** Save 4 fields/mapping: targetEffectKey
  (=getShaderName(), primary) + targetEffectName (=getName(), fallback); targetParamKey
  (=uniformName, primary) + targetParamName (=name, fallback). Resolve effect by shaderName
  else displayName; then param by uniformName else name. Keep writing the old ints.
  Loser: displayName-only (the prior design). DECISION IS CLOSE ON EVIDENCE (P-FALSE-2 removed
  the historical case) — decided on cost asymmetry: dual-key costs ~4 JSON fields and ~20 lines
  and removes an entire failure class; being wrong about "labels never change" reruns this bug.
- **D2 ISF DEDUPE — at registerDynamic only.** Returns bool; reject duplicate def name OR
  shaderName, reject intra-def duplicate param/uniform names; caller alerts. registerEffect
  (private, built-ins) stays bare — built-ins guarded by test T6 instead.
  Loser: per-def stable UUIDs (speculative for a surface that cannot reach the chain).
- **D3 SCOPE — PresetManager commit ONLY.** No BindingManager (P-FALSE-1), no SessionRecorder
  (dead code), no Route (runtime-only, no serialization), no Mapping-struct change.
- **D4 VERSION — write "version":2; dispatch PER-MAPPING on FIELD PRESENCE, not file version.**
  Deck files embed fx objects (PresetManager.cpp:358-366) that can be v1 while fresh presets
  are v2; hand-edited files exist. Version read for diagnostics/forward-tolerance only.
  Backward compat VERIFIED not merely apparent: v1 loader reads only known properties
  (:211-220), JUCE DynamicObject carries unknown properties inertly, "version" is never read,
  and the ints we keep writing are the live indices at save time.
- **D5 UNRESOLVABLE — DROP + log + counts** via a new optional LoadStats out-param. Premise
  re-verified: Mapping has no field to carry an unresolved name, so keep-disabled retains a
  live-wrong index one click from silently driving the wrong param — worse than the disease.
  Boris loses the mapping's source/curve/range/smoothing for targets that no longer exist.
- **D6 MIGRATION — NO automatic remap of v1 presets.** Files don't record library version;
  mtime-keyed remap tables are a guess dressed as a fix and recreate silent-wrong under a
  "migrated" banner. Salvageable without action: v1 mappings targeting 3d/warp/color (chain
  positions 0-66) never shifted and load correctly. Loader flags legacy files; explicit Load
  shows a one-time warning to verify and re-save (re-saving upgrades permanently).

## NEW FINDINGS
- **N2 — ISF effects CANNOT enter the render chain today.** Renderer::initEffectChain's
  categoryOrder lacks "isf", it runs once behind a re-population guard (Renderer.cpp:1511)
  before any import, and import never compiles the shader (MainComponent.cpp:3090-3093 is a
  comment + log). So ISF aliasing is FORWARD-HARDENING for presets, not a current corruption
  path. It IS real today for CLIP SLOTS, which resolve by name (EffectStackView.cpp:336).
- **N4 — ADJACENT SAME-CLASS BUG:** loadPreset restores effect param VALUES BY POSITION
  (:186-194) even though param names are saved (:95). A mid-list param insertion silently
  shifts values. Cheap fix, rides this commit (severable).
- **SessionRecorder audit:** ParameterChange events serialize index-shaped targetId/paramIndex
  (:177-181, load :242-246), but recordParameterChange and advancePlayback have ZERO callers.
  Latent schema hazard in dead code. No change; must adopt this key scheme when wired.
- **N1 mechanical audit of 135 defs:** 0 duplicate display names, 0 duplicate shaderNames, 333
  params, 0 intra-effect duplicate param names or uniformNames (uniformNames unique globally).

## WORK PACKET (one commit, dependency order)
- **W1** PresetManager.h: add `struct LoadStats { int mappingsTotal, resolvedByKey,
  resolvedByName, legacyIndex, dropped; bool legacyFile; juce::StringArray droppedDescriptions; }`;
  loadPreset/loadDeck take `LoadStats* stats = nullptr`.
- **W2** savePreset (:68-130): version->2; per effect add "shader"=getShaderName(); per mapping
  write the 4 key fields resolved off the live chain (const_cast idiom exists at :82); ALWAYS
  keep writing the two ints (:113-114).
- **W3** loadPreset (:134-227): read version (log if >2, proceed); effect restore matches
  shaderName then display name; param restore resolves by NAME with positional fallback (fixes
  N4); mapping restore replaces the raw assignment at :212-213 with key resolution, DROP+log on
  failure, and does NOT fall back to ints when key fields are present; v1 mappings validate ints
  against the chain (in-range -> load + legacyIndex++, OOR -> drop+log).
- **W4** loadDeck (:316-378): thread stats through the embedded loadPreset call (:365).
- **W5** MainComponent surfaces: explicit loadPreset wrapper (:2187-2214) shows async
  AlertWindow (idiom at :3095) on drops, else legacy-file info alert; loadSlotPreset (:2566-2581)
  and slot lambda (:204-215) append " (check mappings)" to fileLabel_ — NO modal on performance
  surfaces.
- **W6** ISF dedupe (independent): EffectLibrary.h:53 registerDynamic -> bool, out-of-line body
  rejecting duplicate name/shaderName and intra-def duplicate params; MainComponent.cpp:3088
  alerts on rejection.

## TEST PLAN — new target test_preset_manager (model on test_mapping_engine, tests/CMakeLists.txt:108-139)
- **T1 FAIL-FIRST:** chain [Alpha,Beta,Gamma], map->Gamma/p0, save; load into
  [Alpha,Beta,Inserted,Gamma]; assert mapping targets Gamma. TODAY loads raw 2 -> Inserted -> FAILS.
- T2 display name changed, shaderName kept -> resolves by key.
- T3 shaderName changed, display name kept -> resolves by name fallback.
- T4 target absent -> 0 mappings, stats.dropped==1, description non-empty.
- T5 hand-written v1 JSON: in-range -> loads + legacyFile; OOR -> dropped.
- **T6** premise gate over all built-ins: unique names, unique shaderNames, per-def unique param
  + uniform names. Passes today; permanently guards the resolution invariant.
- T7 registerDynamic: duplicate name -> false; duplicate shaderName -> false; clean -> true.
- **T8 second fail-first:** param values land BY NAME when params are reordered (fails against
  today's positional restore).
- T9 back-compat shape: ints still present and equal to live indices; version==2.
- Baseline ctest 193/193 must stay green; saveDeck/loadDeck round-trip covers the embed path.

## RISK REGISTER
- **R1** Old v1 presets still silently mis-target via the legacy fallback — unavoidable,
  accepted residual. Detection: legacy warning + Boris's verify-and-re-save pass.
- **R2** A retired shaderName reused later for a different visual re-targets old presets
  plausibly-wrong through the PRIMARY key. No mechanical detection; mitigate with a
  "never reuse shader keys" comment + decision-log note.
- **R3** Clip slots remain display-name-keyed (Clip.cpp:78/229, EffectStackView.cpp:336) — a
  future label rename stays safe for mappings but breaks clip slots. Asymmetry could
  false-reassure.
- R4 MainComponent async-lambda re-entrancy — keep alerts async (existing idiom).
- R5 loadDeck stats-threading vs the temp-file embed path — covered by the round-trip case.
- R6 message-thread jassert in addMapping if MessageManager isn't initialized first in the new
  test — proven pattern in test_mapping_engine.
- R7 SessionRecorder later wired on its index-shaped schema — comment recommended in-commit.

## ONLY BORIS CAN DECIDE
- Whether his pre-fix presets are worth a manual verify-and-re-save pass vs discarding
  (~52 later-category targets are the suspect set; no honest automatic migration exists).
- Alert ergonomics: modal on explicit Load only (default) vs also slot/deck vs never-modal.
- Whether display-label polish is on the roadmap — if yes, schedule R3 (clip slots).
- Whether ISF import matters enough to actually wire (today decorative for the render chain).
