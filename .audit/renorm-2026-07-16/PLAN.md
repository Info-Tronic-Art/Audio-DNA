# Re-normalization + surface accounting — 2026-07-16 (Harmony secondary)

Directive (Boris): top-to-bottom pass, normalize, update docs, full surface/function
accounting, brief works/doesn't-work read. Autonomous; swarm-based.

State: norm v2 = current spec; code drift ~zero (3 meta commits since 4ee10ad).
Phase 1 secrets: PASS. Structure audit: native-cmake, 1 RECOMMENDED gap (.harmony/HANDOFF.md).
Known doc rot: CLAUDE.md effect counts contradict (110/115/135/96); source tree lists
nonexistent src/keyboard/, omits src/core, src/test, src/routing, src/signal, src/sync,
src/binding, src/midi, src/model; duplicate rows (SpectrumDisplay, Knob). No APP-INVENTORY.md.

## Lanes (deep-audit swarm, read-only, output → this dir)
- L1 audio-analysis: src/audio, src/analysis, src/features
- L2 render-effects: src/render, src/effects, shaders/ — settle effect/transition counts
- L3 sources-media: src/sources, src/media — settle procedural source count (64 vs 108)
- L4 control-plane: src/mapping, src/routing, src/signal, src/binding, src/midi, src/osc, src/sync, src/model
- L5 ui-surfaces: src/ui, MainComponent — APP-INVENTORY surface census
- L6 io-api: src/api, src/output, src/recording, src/core, Main.cpp — endpoint census
- L7 docs-tests-build: root *.md, docs/, tests/, src/test, cmake/, .audit/features-gap-fill

## Phase 3 (after swarm): Builder A → .harmony (FEATURES, APP-INVENTORY, FEATURE_MAP,
file_map, state, HANDOFF); Builder B → root docs (CLAUDE.md, FEATURE_INVENTORY, archive).
Gate: validate-features.sh + build/ctest green + Reviewer on diffs + receiver spot-checks.
Background: build.log / ctest.log in this dir.

## Ground truth (2026-07-16)
Release build: PASS (exit 0). ctest: 113/113 PASS (1.67s). Boris directive mid-turn:
autonomous to completion → EOS → features/surfaces account last, before `safe to close.`
