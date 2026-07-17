# Lane 7 — Docs / Tests / Build Census (2026-07-16)

Auditor: L7 (read-only deep audit). Repo: `~/projects/RealTimeAudio` (Audio-DNA,
C++20/JUCE/OpenGL). HEAD `9139dd4`. Scope: root `*.md`, `docs/`, `design/`,
`research/INDEX.md` (index only), `tests/`, `src/test/`, `cmake/`, `CMakeLists.txt`,
`.audit/features-gap-fill/`, `CLAUDE_CODE_RTA_HANDOFF.md`, `.harmony/*.md`.
Evidence = file:line or git ref. No files modified outside this dir.

Ground truth this session (from `.audit/renorm-2026-07-16/{build,ctest}.log`):
Release build PASS (exit 0). ctest 113/113 PASS (1.67s).

---

## 1. DOC-ESTATE CENSUS

### 1a. Root `*.md` (13 files)

| Path | Purpose | Last subst. (git %as) | Status |
|---|---|---|---|
| CLAUDE.md | Orchestrator brief + architecture + capability list + effects/sources library | 2026-05-17 (36 commits) | CURRENT-but-ROTTEN — build/run conventions valid; capability counts + source-tree self-contradictory (see §4) |
| ARCHITECTURE_V2.md | "Single source of truth for v2 architecture" (deck/layer/signal system) | 2026-05-17 | CURRENT — architecture SoT; overlaps CLAUDE.md architecture section |
| AUDIO_DNA_DIRECTIVE_FULL.md | "Definitive" design/UX directive (norm v2 spec) | 2026-05-22 | CURRENT — design/UX SoT |
| FEATURE_CONNECTIONS.md | "Canonical" runtime behavior reference (when X → Y, 12 scenarios/~35 decisions) | 2026-05-22 | CURRENT — behavior SoT |
| MENTAL_MODELS.md | Conceptual spine (8 models) | 2026-05-22 | CURRENT — design cluster |
| BORIS_DECISIONS.md | Short-form decision summary; self-declares FEATURE_CONNECTIONS + MENTAL_MODELS canonical | 2026-05-22 | CURRENT — index/summary role |
| MOCKUP_BRIEF.md | v10 mockup design standards + per-mockup template | 2026-05-22 | CURRENT — design-process doc |
| UI_DETAIL_SPECS.md | Companion to DIRECTIVE; gap-fill specs for mockups #5–#8 | 2026-05-22 | CURRENT-DRAFT ("pending Boris review", UI_DETAIL_SPECS.md:9) |
| CONTEXT.md | Domain glossary | 2026-05-24 | CURRENT — but "16-stage" pipeline vs README "14-stage" (§4) |
| README.md | Public prose overview (135 effects, 108 sources, 14-stage) | 2026-05-17 | CURRENT (public) — restates disputed counts |
| PHASE_GUIDE.md | Phase-kickoff workflow; tells Claude to read TASKPLAN_V2.md | 2026-03-23 (23 commits) | STALE — March-era; TASKPLAN_V2.md now archived (dangling ref, PHASE_GUIDE.md:13) |
| LESSONS_LEARNED.md | Verified fixes (3 entries, M1 era) | 2026-03-14 | CURRENT-low-churn — valid but thin; OFF-LIMITS for edits |
| CLAUDE_CODE_RTA_HANDOFF.md | One-shot handoff: "integrate the directive" | UNTRACKED | ORPHAN — task already done (directive integrated as AUDIO_DNA_DIRECTIVE_FULL.md) |

### 1b. `docs/`

| Path | Purpose | Last subst. | Status |
|---|---|---|---|
| docs/FEATURE_INVENTORY.md | User-facing line-by-line inventory (gen 2026-04-19 from 15 slices) | 2026-05-17 | ARCHIVE-CANDIDATE / DUPLICATE-OF design/FEATURE_INVENTORY.md + .harmony/FEATURES.md (both newer) |
| docs/feature_audit/slice_01..15 (15 files, ~7100L) | Raw audit fragments feeding docs/FEATURE_INVENTORY.md | 2026-05-17 | ARCHIVE-CANDIDATE — working artifacts, superseded |
| docs/archive/BUILDLOG.md (5L) | Old build log | 2026-05-17 | ARCHIVE (correctly placed) |
| docs/archive/TASKPLAN_V2.md (494L) | v2 task plan | 2026-05-17 | ARCHIVE (correct) — but PHASE_GUIDE.md still points to it as live |
| docs/superpowers/plans/2026-05-17-directory-reorganization.md (497L) | One-off reorg plan | 2026-05-17 | ARCHIVE-CANDIDATE — completed plan |

### 1c. `design/`

| Path | Purpose | Last subst. | Status |
|---|---|---|---|
| design/FEATURE_INVENTORY.md (2491L) | "Authoritative SoT for the design overhaul" — master inventory w/ ghost/UI-state tags | 2026-05-19 | CURRENT — design-overhaul feature SoT; overlaps docs/FEATURE_INVENTORY.md + .harmony/FEATURES.md |
| design/GAP_REPORT.md (154L) | Count-discrepancy audit (Boris doc vs FEATURES.md vs actual code) | 2026-05-21 | CURRENT — records ground-truth counts (effects 135, sources actual 101, params 1379+) |
| design/GHOST_FEATURE_DECISIONS.md (26L) | Ghost-feature include/exclude decisions | 2026-05-19 | CURRENT |
| design/MACRO_CONDITIONS_PLAN.md (585L) | Impl plan: Gate + ScaleRange macro conditions | 2026-05-19 | CURRENT-PLAN (verify implemented before archiving) |
| design/{mockups,references,ux_analyses,.audit}/ | Mockups, competitor UX analyses, design audit leftovers | — | reference subdirs (not deep-censused; .audit = leftover) |

### 1d. `.harmony/` (Harmony brain artifacts — freshness/overlap only)

| Path | Purpose | Last subst. | Tracked | Status |
|---|---|---|---|---|
| .harmony/FEATURES.md (2736L) | Engineering feature registry, norm v2 SHA 4ee10ad, entry points (file:line) | 2026-05-24 | yes | CURRENT — engineering feature SoT; most recent large inventory |
| .harmony/FEATURE_MAP.md (67L) | Features → graph communities | 2026-05-24 | UNTRACKED | CURRENT but uncommitted |
| .harmony/file_map.md (117L) | File index | — | UNTRACKED | CURRENT but uncommitted; minor drift ("108 sources", "35 research docs", OutputWindow listed under both src/output/ + src/ui/) |
| .harmony/gotchas.md (84L) | 12 project gotchas | 2026-06-01 | yes | CURRENT — freshest doc; conftest/test gotchas hold (§2c) |
| .harmony/notebook.md (50L) | Builder notebook (inspection stack) | 2026-05-19 | yes | CURRENT |
| .harmony/state.md (7L) | Norm state (Norm ID 2, SHA 4ee10ad) | 2026-05-24 | yes | CURRENT |

### 1e. `.audit/` prior-audit leftovers

| Path | Purpose | Last subst. | Tracked | Status |
|---|---|---|---|---|
| .audit/features-gap-fill/phase-1/{audio,control,visual}-features.md | Prior gap-fill audit | — | UNTRACKED | ORPHAN — untracked leftover |
| .audit/directive-reconciliation/{CONNECTION_DECISIONS_2026-05-22,DESIGN_PROFILE,FEATURE_MAPPING,HANDOFF}.md | Working notes; CONNECTION_DECISIONS → promoted to FEATURE_CONNECTIONS.md | 2026-05-21/22 | yes | ARCHIVE-CANDIDATE |
| .audit/doc-completeness/phase-{1,2}/summary.md | Prior doc-completeness audit | 2026-05-21 | yes | ARCHIVE-CANDIDATE |

### 1f. `research/INDEX.md` (index only)
- 36 `research/*.md` files on disk (INDEX + 35 docs). INDEX last touched 2026-05-17. CURRENT reference index.
- Count drift: CLAUDE.md:291 "30 research documents"; file_map "35 docs"; actual 35 (excl. INDEX).

### 1g. Doc overlap — who owns what (source-of-truth map)

| Domain | Current SoT | Duplicated/overlapping in |
|---|---|---|
| Engineering feature registry (code-level, entry points) | **.harmony/FEATURES.md** | docs/FEATURE_INVENTORY.md, design/FEATURE_INVENTORY.md, CLAUDE.md capability list |
| Design/UX spec | **AUDIO_DNA_DIRECTIVE_FULL.md** (+ MENTAL_MODELS, UI_DETAIL_SPECS, MOCKUP_BRIEF, BORIS_DECISIONS) | ARCHITECTURE_V2 (partial) |
| Runtime behavior (X→Y) | **FEATURE_CONNECTIONS.md** | BORIS_DECISIONS (summary) |
| System architecture | **ARCHITECTURE_V2.md** | CLAUDE.md architecture section |
| Design-overhaul feature inventory | **design/FEATURE_INVENTORY.md** | docs/FEATURE_INVENTORY.md |
| Capability counts (effects/sources/params) | **NONE** — contradictory across CLAUDE.md, README, .harmony/FEATURES.md, design/GAP_REPORT | see §4 |

---

## 2. TEST CENSUS

### 2a. Catch2 unit tests (C++) — 11 targets, 113 tests, all PASS

Run: `cd build && ctest --output-on-failure`. Defined in `tests/CMakeLists.txt`.
No app required. All 11 `tests/test_*.cpp` files have a target; each `catch_discover_tests`.

| File | Covers | Src deps linked |
|---|---|---|
| test_ring_buffer.cpp | Lock-free SPSC ring buffer | (header-only) |
| test_spectral_features.cpp | Centroid/flux/flatness/rolloff/bands | analysis/SpectralFeatures |
| test_feature_bus.cpp | Triple-buffer atomic swap | features/FeatureBus |
| test_smoother.cpp | EMA + One-Euro filter | (header-only) |
| test_integration_pipeline.cpp | Full analysis pipeline on synthetic audio | analysis/* (11 files) + FeatureBus + JUCE dsp + Aubio |
| test_mapping_engine.cpp | Source→curve→scale→target mapping | mapping/MappingEngine + effects/Effect,EffectChain + render/* |
| test_bpm_stabilization.cpp | BPM lock/stabilization | analysis/BPMTracker + Aubio |
| test_downbeat_detector.cpp | Downbeat detection | analysis/BPMTracker + Aubio |
| test_composition.cpp | Data model + undo | model/Clip,Layer + core/UndoManager |
| test_routing_engine.cpp | Signals + routing engine | signal/* + routing/RoutingEngine + mapping + effects + render |
| test_compositor.cpp | Deck/layer compositing + autopilot | model/Clip,Layer,Autopilot |

### 2b. Eyes visual tests (Python/pytest) — `tests/visual/`, 11 test_*.py, ~491 cases

Run: `cd tests/visual && python -m pytest` (app auto-spawned by conftest). Needs
app built with `-DAUDIODNA_BUILD_TEST_SERVER=ON` for full set (port 8080); ~38
tests work in production mode (port 7070). Files: test_audio_reactivity, test_effects,
test_fractals, test_milkdrop, test_performance, test_range_quality, test_render_pipeline,
test_signals, test_sources, test_time_sweep, test_ax_inspector (19 tests, reads macOS
AX tree, app running any mode). Support: vj_controller.py, vision_check.py (PSNR/SSIM),
ax_inspector.py, scan_all_presets.py, verify_defaults.py, final_default_validation.py.
Docs: tests/visual/{TESTING.md, SHADER_VERIFICATION.md, SIGNAL_TEST_SPEC.md}.

### 2c. src/test/ (TestServer)
- `src/test/TestServer.{h,cpp}` — embedded HTTP test server ("Eyes"), provides
  `inject_features`. Compiled only when `AUDIODNA_BUILD_TEST_SERVER=ON`
  (CMakeLists.txt:336-341). Not in default build.

### 2d. Gotcha check (task item)
- **conftest autouse fixture STILL BLOCKS non-Eyes tests — HOLDS.**
  `tests/visual/conftest.py:77-78` still has `@pytest.fixture(autouse=True) def
  reset_between_tests(app)`. Any new test file in `tests/visual/` that does not
  need the running app must locally override `app` + `reset_between_tests`
  (matches .harmony/gotchas.md 2026-05-18 and tests/README.md:52).

### 2e. Coverage gaps (unit) vs subsystem list
Unit-tested: audio, analysis, features, mapping, model, signal, routing.
**NO dedicated unit test:** api, midi, osc, recording, sources, binding, sync,
output, ui, render (render/effects only compiled *into* mapping/routing tests as
link deps, not behaviorally asserted). These rely entirely on the Eyes harness,
which needs a running app + test-server build. See §4-flag on missing baselines.

---

## 3. BUILD CENSUS

`CMakeLists.txt` (root, 443L) + `tests/CMakeLists.txt` + `cmake/`.

- Toolchain: `cmake_minimum_required(VERSION 3.24)`, C++20 (no extensions),
  langs C/CXX/OBJC/OBJCXX, project `AudioDNA` v0.1.0. `compile_commands.json` on.
- Primary target: `juce_add_gui_app(AudioDNA)` — ~200 source files (CMakeLists.txt:87-333).
- Test targets: 11 executables (§2a) via Catch2 `catch_discover_tests`.

**Options (4 user-facing + 1 derived):**
| Option | Default | Effect |
|---|---|---|
| AUDIODNA_BUILD_LINK | OFF | FetchContent Ableton Link 3.1.2; defines AUDIODNA_HAS_LINK |
| AUDIODNA_BUILD_TEST_SERVER | OFF | Compiles src/test/TestServer; defines AUDIODNA_TEST_SERVER |
| AUDIODNA_BUILD_INSPECTOR | OFF | FetchContent melatonin_inspector cd25631 (KNOWN BROKEN — module header not found) |
| AUDIODNA_BUILD_SYPHON | OFF (APPLE) | find_library(Syphon); defines AUDIODNA_HAS_SYPHON |
| AUDIODNA_USE_CAMERA | derived (APPLE/WIN32=1) | links juce_video, defines AUDIODNA_HAS_CAMERA |

**Dependencies:**
| Dep | How located | Required? |
|---|---|---|
| JUCE 8.0.4 | FetchContent (git) | required |
| cpp-httplib v0.18.3 | FetchContent (git) | required (prod API + test server) |
| Catch2 v3.7.1 | FetchContent (git, tests/) | tests |
| Ableton Link 3.1.2 | FetchContent, gated by AUDIODNA_BUILD_LINK | optional |
| melatonin_inspector cd25631 | FetchContent, gated by AUDIODNA_BUILD_INSPECTOR | optional (broken) |
| Aubio | find_package REQUIRED, cmake/FindAubio.cmake | required |
| FFmpeg | find_package REQUIRED, cmake/FindFFmpeg.cmake | required |
| ProjectM (libprojectM-4) | find_package QUIET, cmake/FindProjectM.cmake | optional |
| Syphon.framework | find_library /Library/Frameworks | optional (macOS) |

- `cmake/`: CompilerWarnings.cmake (`set_project_warnings`), FindAubio, FindFFmpeg,
  FindProjectM. Warnings suppressed in JUCE headers via -Wno-old-style-cast/conversion/sign-conversion.

**CI — `.github/workflows/build.yml` EXISTS** (contradicts renorm PLAN note "native-cmake,
no CI"). 4-platform matrix: macOS-14 ARM64, macOS-15 x86_64, ubuntu-22.04 GCC,
windows-latest MSVC. Triggers on push/PR to main. Builds Release, runs ctest
(`continue-on-error: true` — test failures do NOT fail the job), uploads .app/binary
artifacts (30-day retention). Deps installed per-OS (brew aubio / apt libaubio-dev /
vcpkg aubio). **Gap: CI installs Aubio but NOT FFmpeg dev libs on Linux** (apt list,
build.yml:74-89, omits libavformat/libavcodec/libavutil/libswscale-dev) while
`find_package(FFmpeg REQUIRED)` is mandatory — Linux job cannot configure. Plausible
root cause of the "CI failing" gotcha (.harmony/gotchas.md 2026-05-22, Boris disabled
remote pushes). Flagged, not proven (macOS/Windows runners may ship ffmpeg).

---

## 4. HEALTH READ

**Trustworthy:** design cluster (AUDIO_DNA_DIRECTIVE_FULL, FEATURE_CONNECTIONS,
MENTAL_MODELS, MOCKUP_BRIEF, UI_DETAIL_SPECS, BORIS_DECISIONS — cross-referenced,
coherent, May-22 cohort), ARCHITECTURE_V2, .harmony/FEATURES.md, design/GAP_REPORT.md,
CONTEXT.md, gotchas.md. Build system + unit tests: green and honest (113/113).

**Rotten / stale:** CLAUDE.md capability/count/source-tree sections (§ discrepancies);
PHASE_GUIDE.md; docs/FEATURE_INVENTORY.md + docs/feature_audit/ (superseded);
CLAUDE_CODE_RTA_HANDOFF.md (done); .audit leftovers.

### Top discrepancies
1. **CLAUDE.md effect count self-contradicts:** 110 (CLAUDE.md:7) vs 135 (:11,:395,:1071)
   vs 96 (:242) vs 115 (:397 header). Category table (:399-411) sums to **134**, labeled "115 total".
2. **CLAUDE.md procedural-source count:** 108 (:11) vs 64 (:578). README/file_map=108;
   design/GAP_REPORT actual code=101. No agreed number.
3. **CLAUDE.md source-tree (:183-258)** lists nonexistent `src/keyboard/` (:214) +
   nonexistent `ui/KeyboardPanel` & `ui/KeyEditor` (:257-258); OMITS 9 real dirs:
   src/{core,model,signal,routing,binding,midi,sync,test,sources}.
4. **CLAUDE.md duplicate rows:** SpectrumDisplay (:248 + :252), Knob (:253 + :256).
5. **CLAUDE.md tests/ tree (:278-286)** lists nonexistent `test_onset_detector.cpp`,
   shows only 7 of 11 real test files (omits bpm_stabilization, downbeat_detector,
   composition, compositor, routing_engine).
6. **Pipeline stage count:** README "14-stage" vs CONTEXT.md + file_map "16-stage"
   (CLAUDE.md:11 implies 14). AdvancedAudioAnalyzer described as "stage 14" (CLAUDE.md:1149).
7. **Research doc count:** CLAUDE.md:291 "30" vs file_map "35" vs actual 35 (+INDEX).
8. **Audio effect category** (CLAUDE.md ~:406) lists 4 examples but count=3;
   transitions 15 (CLAUDE.md) vs design/GAP_REPORT "30 actual".

### FLAGGED
1. **CLAUDE_CODE_RTA_HANDOFF.md** — untracked orphan; describes a completed task
   (directive already integrated). Archive/delete.
2. **.audit/features-gap-fill/phase-1/*.md** — 3 untracked leftover files. Orphan.
3. **.harmony/FEATURE_MAP.md + file_map.md** — untracked brain artifacts (uncommitted);
   file_map has minor drift (OutputWindow dir, counts).
4. **No committed golden-reference images for the Eyes harness** — vision_check.py does
   PSNR/SSIM but only `tests/fixtures/test_card.png` exists; baselines are runtime-generated,
   not version-controlled → cross-machine visual regressions undetectable.
5. **PHASE_GUIDE.md** (March-era) instructs reading TASKPLAN_V2.md, now in docs/archive/
   — dangling workflow doc.
6. **docs/FEATURE_INVENTORY.md + docs/feature_audit/ (15 slices, ~7900L total)** —
   superseded by design/FEATURE_INVENTORY.md + .harmony/FEATURES.md; not marked archived.

### Doc-architecture recommendation
- **Keep as SoT:** .harmony/FEATURES.md (engineering features), AUDIO_DNA_DIRECTIVE_FULL.md
  (design/UX), FEATURE_CONNECTIONS.md (behavior), ARCHITECTURE_V2.md (architecture),
  design/FEATURE_INVENTORY.md (design-overhaul inventory), CONTEXT.md (glossary).
- **Create ONE canonical counts table** (proposed APP-INVENTORY.md per renorm PLAN, or a
  pinned section in .harmony/FEATURES.md) owning effects/sources/params/transitions/stages;
  make CLAUDE.md + README *reference* it, not restate numbers. Fix CLAUDE.md source-tree
  + tests-tree from actual `src/` + `tests/`.
- **Archive:** docs/FEATURE_INVENTORY.md + docs/feature_audit/ + docs/superpowers/plans/,
  CLAUDE_CODE_RTA_HANDOFF.md, .audit/{directive-reconciliation,doc-completeness,features-gap-fill}.
- **Update or archive:** PHASE_GUIDE.md (fix TASKPLAN_V2 ref or archive as v1 workflow).
- **Commit** .harmony/{FEATURE_MAP,file_map}.md (currently untracked).
- **Keep design cluster** (MENTAL_MODELS, MOCKUP_BRIEF, UI_DETAIL_SPECS, BORIS_DECISIONS) —
  coherent, cross-referenced; no merge needed.
