# N12-rta report — normalize Phases 1-2 (RealTimeAudio / Audio-DNA) — s158

STATUS: PENDING

## PHASE 1 — SECRETS SCAN

```
SECRETS SCAN: PASS
```

Checked: `git log --all --diff-filter=A -- '*.env' '.env.*'` (no `.env` ever
committed), `*.pem`/`*.key` files on disk (only `.venv/lib/python3.14/site-packages/{certifi,pip/_vendor/certifi}/cacert.pem` —
a public CA bundle shipped by the `certifi` PyPI package inside the gitignored,
untracked `.venv/`; not a secret, not committed), grep for
`sk_live|sk_test|AKIA[0-9A-Z]{16}|password\s*=` across `src/`, `tests/`,
`.github/` (no hits). `.gitignore` excludes `.venv/` and it is confirmed
untracked (`git ls-files .venv | wc -l` = 0). Proceed to Phase 2.

NICE_TO_HAVE: `.gitignore` has no explicit `.env`/`*.key`/`*.pem` lines (only
`.venv/` is excluded, which happens to cover the one `.pem` found). No secret
architecture is evident in this C++/JUCE desktop app (REST/OSC/MIDI control
surfaces, no external API keys seen in `src/`), so this is defensive
hardening, not a live gap.

## PHASE 2 — DISCOVERY + GAP ANALYSIS

DISCOVERY: RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL real-time
audio-reactive VJ app; registered `config/repos.yml:50`, alias `rta`)

EXISTS:
- CLAUDE.md: YES (root, 96025 bytes)
- CONTEXT.md: YES (root, 8178 bytes, domain glossary)
- .harmony/file_map.md: YES / STALE (128 lines; last full re-derivation
  2026-07-16 HEAD 9139dd4; 116 feature-affecting files have changed under it
  since — see (c) below)
- .gitignore: YES — covers `.venv/`, build dirs, JUCE-generated, OS files,
  `.harmony/` + `graphify-out/` (both force-tracked selectively — see (a)),
  harmony transient-artifact hygiene block present (large-file guard install
  marker). Does NOT explicitly list `.env`/`*.key`/`*.pem` (see Phase 1 note).
- Test suite: YES — native Catch2 (`tests/*.cpp`, `tests/CMakeLists.txt`,
  `tests/test_*.cpp` covering preset_manager, undo_commands, bpm_stabilization,
  spectral_features, mapping_engine, routing_engine, autopilot, compositor,
  composition, smoother, ring_buffer, feature_bus, downbeat_detector,
  thumbnail_cache, waveform_snapshot, renderer_source_confinement,
  integration_pipeline) + Python/pytest visual suite (`tests/visual/*.py`:
  test_mapping_tick, test_effects, test_signals, test_render_pipeline,
  test_ax_inspector, conftest, requirements-test.txt). RUNS/PASSES: not
  executed this session (fence is presence-check only, no build/test run per
  dispatch); `state.md`'s last recorded baseline is 113/113 ctest PASS
  (2026-07-16), CI (below) gates every push/PR on 4 platforms.
- Linter config: YES — `.clang-format` (995B) + `.clang-tidy` (572B) at root.
- CI config: YES — `.github/workflows/build.yml`, "Build & Test" on
  push/PR to `main`, 4-platform matrix (macOS ARM64, macOS x86_64, Linux GCC,
  Windows MSVC).
- README: YES (`README.md`, 2054 bytes, dated 2026-07-16).
- Package lock: N/A for this archetype — native-cmake / C++ has no
  package-manager lockfile ecosystem (no npm/poetry/cargo deps found; JUCE +
  aubio/fftw are vendored under `third_party/`).
- Type checking: N/A in the mypy/pyright sense (C++ is statically typed at
  compile time); `.clang-tidy` fills the equivalent static-analysis role and
  is present.

GAPS:

REQUIRED:
- **`.harmony/VALIDATION.md` — MISSING.** Part of the mandated `.harmony/`
  directory set (skill Phase 3 step 1: state.md, notebook.md, file_map.md,
  FEATURES.md, gotchas.md, HANDOFF.md, VALIDATION.md). Create from
  `references/validation-template.md` (CREATE-IF-ABSENT).
- **FEATURES.md §24 "Dual-Mode System (ProgrammingMode — REMOVED Wave 0)"
  missing the REQUIRED `**What it does:**` section** — confirmed by direct
  read of `.harmony/FEATURES.md:2009-2013`: the section opens straight into
  `**Status: REMOVED 2026-07-17 (Wave 0)**` with no `What it does` line.
  This is the ONE real template-validation FAIL once the framework
  misclassification below is corrected (see (c)).

RECOMMENDED:
- **`.harmony/binding-decisions.md` — MISSING** (Phase 3 step 3b sibling
  projection of primary binding decisions). Advisory-only per skill doctrine
  (doc-unification-rules.md R4.2 — absence WARNs at boot, never blocks), so
  RECOMMENDED not REQUIRED.
- **Re-run `normalize-check.sh` / re-normalize with `--framework game`, not
  the tool's default `web`.** `bash scripts/normalize-check.sh` (no
  `--framework` flag) hardcodes `FRAMEWORK="${3:-web}"` and reported 3 FAILs
  (2 missing CONTEXTUAL sections "Security"/"API documentation" on §26, 1
  missing "Deployment & Infrastructure"). Re-run directly with
  `--framework game` (the correct classification for a native C++/JUCE
  real-time desktop app — closest of the tool's 4 types: web/game/cli/spa)
  drops this to 264 PASS / 1 FAIL / 1 WARN — the 3 original FAILs were a
  framework-detection artifact, not real gaps (Security/API-doc sections are
  N/A for a local desktop app with no auth surface; "Deployment" correctly
  WARNs N/A for `game`, not FAILs). See (c) for the full comparison. Consider
  whether `normalize-check.sh` should let per-project config
  (`config/repos.yml` `tech:` array) drive `--framework` instead of the `web`
  default, to stop this false-positive recurring on every check.
- 6 gotcha entries in `.harmony/gotchas.md` have schema drift not caught by
  the promotion scan (per the SENT inbox record below) — still open, still
  uncommitted (see DIRTY TREE).

NICE_TO_HAVE:
- Explicit `.env`/`*.key`/`*.pem` lines in `.gitignore` (defensive; no
  current secret surface — see Phase 1).
- `.harmony/UNCLEAN-CLOSE-STAMP-harmony-31879.md` is a system marker dated
  2026-08-30 (session `harmony-31879` closed without a verified EOS) that is
  supposed to be self-consumed (deleted) by `lib/unclean-close-detect.sh` at
  the next boot surfacing it. It is still present on disk 3 days later —
  flagging for awareness, not fixing (Phase 2 is read-only and it is a
  system-owned file, not a normalize-template item).

## MCP RECOMMENDATIONS

Already configured in `.mcp.json` (project root) for the detected C++20/JUCE
stack: `graphify-rta` (architecture/design-intent graph), `codegraph-rta`
(SQLite symbol index, C1/C2), `clangd-rta` (LSP, AUTHORITATIVE for C3
call-graph resolution per `.harmony/knowledge-tools.yml`). No new servers to
recommend — the three that apply to a large multi-file C++ project are all
present and profiled. Note: a SENT inbox record
(`down-2026-08-30-RealTimeAudio-17881102275776627736`) asks which of the
three the workflow actually needs before Harmony removes any as "never
called" — that usage-query is still open and is this repo's call, not
Phase 2's.

## PHASE 2.x — STRUCTURE CLASSIFICATION & LAYOUT AUDIT

```
STRUCTURE-AUDIT  repo=RealTimeAudio  archetype=native-cmake  matched-rule=1 CMakeLists.txt  path=/Users/boriskarpman/projects/RealTimeAudio  (READ-ONLY / advisory)
DEVIATIONS: 0
path | expected-location | actual-location | kind | severity | proposed-action
(none — canonical-clean for native-cmake + universal overlay)
```

Zero structure deviations. `src/` domain subdirs, `tests/`, `docs/` all in
their canonical native-cmake homes; `CLAUDE.md`/`CONTEXT.md`/`README.md` +
the `.harmony/` REQUIRED set (minus VALIDATION.md, flagged above) satisfy the
§1 universal overlay.

## (a) WHAT THE EXISTING .harmony/ ALREADY HAS

77 files on disk under `.harmony/` (55 git-tracked despite the blanket
`.harmony/` gitignore line — force-added individually; 22 untracked/ignored).
Mapped to template homes:

**Normalize-template REQUIRED set — present, git-tracked:**
`state.md` (11 lines, norm v2, last_normalized_sha `9139dd4`, 2026-07-16),
`notebook.md` (437 lines), `file_map.md` (128 lines, STALE — see gap above),
`FEATURES.md` (2723 lines, 26 numbered features + Data Model + Shared
Infrastructure), `gotchas.md` (252 lines, dirty — see DIRTY TREE),
`HANDOFF.md` (1000+ lines, actively maintained with dated sections).
**Missing:** `VALIDATION.md` (gap, above).

**Template-adjacent, present:** `FEATURE_MAP.md` (71 lines, Phase 3 step 8
output, tracked), `knowledge-tools.yml` (Phase 3 step 7c output, tracked,
`last_verified: 2026-07-02`). **Missing (advisory):** `binding-decisions.md`
(Phase 3 step 3b, gap above).

**Project-local machinery to KEEP as-is (not template items, real repo
content — Phase 3 must preserve, never overwrite):**
- `.harmony/sessions/` — 14 dated secondary-session logs (tracked) + 7
  matching `.pre-s144-migration` backups (untracked, see stale leftovers).
- `.harmony/specs/` — 5 design specs (featurebus-thread-safety, isf-import,
  outputwindow-arc, session-recorder, undo-v1), all tracked.
- `.harmony/.work-packets/` — 5 build-ready packets (black-overlay-fix,
  c3-mapping-tick, milkdrop-autoload-fix, output-dismissal-fix-v2,
  preset-retarget-fix), untracked by repo convention (HANDOFF.md states
  `.work-packets/` is gitignored by convention).
- Investigation/root-cause docs (all tracked): `APP-INVENTORY.md`,
  `black-overlay-rootcause.md`, `milkdrop-autoload-rootcause.md`,
  `surface-audit-2026-08-04d.md`, `essentials-plan-2026-08-04d.md`,
  `decisions-2026-08-04c.md`, `commit-b-premise-correction.md`,
  `session-c-new-findings.md`, `gesture-replay-results.md`, `idea-ledger.md`,
  `preset-packet-verification.md`, `triage-2026-07-17.md`,
  `undo-v1-ledger.md`, `undo-v1-manual-e2e.md`, `EOS-COMPLETE.md`, plus 8
  `scout-*.md` recon docs and 3 gate-evidence `.log` files
  (`ow-c1-signalregistry-race.log`, `ow-freeze-before.log`,
  `s2-tsan-{before,after}.log`) and `boris-session-snapshot-1430.json`.
- `.harmony/CHECKPOINT.md` — auto-written, debounced (60s) close-resilience
  snapshot (system machinery, expected to churn; currently dirty, see below).
- `.harmony/inbox.md` — the primary→project down-channel (route-down.sh
  single-writer inbox, C-boundary-ruling s148 §2b); untracked by design
  (project edits only status lines). **5 SENT records currently unflipped**
  (quoted verbatim below).
- `.harmony/.reports/` — this report's directory (created this session).

**Stale leftovers (candidates for archive, not template gaps):** 8
`*.pre-s144-migration` backup files (`.harmony/CHECKPOINT.md.pre-s144-migration`,
`.harmony/HANDOFF.md.pre-s144-migration`, and 6 matching files under
`.harmony/sessions/`) — untracked residue from a prior `s144` migration.
`.harmony/UNCLEAN-CLOSE-STAMP-harmony-31879.md` (see NICE_TO_HAVE gap above).

**SENT inbox records (verbatim id + raw, NOT flipped by this dispatch):**
1. `down-2026-08-30-RealTimeAudio-17881102275772810473` — "RealTimeAudio norm
   drift (HEAVY: 116 feature-affecting files / 121 commits) is FAILing
   Harmony's integration score. Ruled out-of-scope for the primary twice
   (s127/s128) and it recurred; it needs this repo's own session." (matches
   (c) below exactly)
2. `down-2026-08-30-RealTimeAudio-1788110227577472721` — "6 gotcha entries in
   this repo's .harmony/gotchas.md have schema drift invisible to the
   promotion scan. Harmony owns the schema and the scan; the entries are this
   repo's file content."
3. `down-2026-08-30-RealTimeAudio-17881102275776627736` — "USAGE QUERY (reply
   via scripts/idea-capture.sh): which of graphify-rta / codegraph-rta /
   clangd-rta does your workflow actually need? ... Removal of clangd-rta is
   DEFERRED pending your answer."
4. `down-2026-08-30-RealTimeAudio-17881102275778519984` — "Re-normalize
   flagged due since 08-10 by the s147 EOD (kill-date 2026-08-01; all
   registered projects flagged). Verify against this repo and run the
   normalize skill in your own session." (this dispatch is that response)
5. `down-2026-08-30-RealTimeAudio-17881102275780412232` — "Commit this
   repo's uncommitted .harmony/ edits (pointer rewrites + s147 promotion
   markers, plus this inbox file)." (matches the CHECKPOINT.md/gotchas.md
   dirty diffs below)

## (b) DIRTY TREE

`git status --short | wc -l` = **268** files. By status code: 259 `D`
(deleted), 8 `M` (modified), 1 `??` (untracked). By top-level path: 265 under
`graphify-out/`, 2 under `.harmony/`, 1 under `.audit/`.

Grouped, with a one-line recommendation each:

| Group | Count | What it is | Recommendation |
|---|---|---|---|
| `graphify-out/cache/ast/*.json` (deleted) | 259 | AST cache from an on-disk graph regen (`GRAPH_REPORT.md` dated 2026-08-04) whose cache dir is no longer emitted/kept by the current regen run | Clean (matches `graphify-out/` regen churn — this whole dir is committed selectively despite `graphify-out/` being in `.gitignore`, i.e. it was force-tracked historically; the deletions are the regen tool cleaning its own cache). Boris/next session: `git rm` the cache tree in one commit, or stop force-tracking `graphify-out/cache/` going forward. |
| `graphify-out/graph.html` (deleted) + `.graphify_analysis.json`, `.graphify_labels.json`, `.graphify_root`, `GRAPH_REPORT.md`, `graph.json`, `manifest.json` (modified, 6 files) | 7 | A real, newer graphify regen (2026-08-04, 281 files / ~433,729 words per `GRAPH_REPORT.md`) sitting uncommitted on top of the last committed graph (2026-07-16 state per `FEATURE_MAP.md`) | Commit — this is fresh, wanted regen output, not noise; folds into the graphify-out cache cleanup above as one `chore(harmony): regen graph` commit. |
| `.harmony/CHECKPOINT.md` (modified) | 1 | Auto-written close-resilience snapshot, now stamped session `harmony-31879` 2026-08-30 vs the previously-committed `harmony-89245` 2026-07-29 | Commit — it's supposed to churn every close; the committed copy is just behind by one close cycle. |
| `.harmony/gotchas.md` (modified) | 1 | 3 entries flipped Scope `repo`→`universal` + Promoted `no`→`yes → <skill> (2026-08-30)` — this IS SENT record #5's "s147 promotion markers" | Commit — these are real, dated promotion decisions already made; leaving them uncommitted risks losing them (SENT record #5 already asks for exactly this). |
| `.audit/features-gap-fill/` (untracked, 3 files: `phase-1/{visual,audio,control}-features.md`) | 1 dir | A prior features-gap-fill audit's Phase-1 output, sitting outside `.harmony/` at `.audit/` | Flag to Boris: commit if it's a kept artifact, or clean if superseded — outside this dispatch's read of the repo's history to judge which. |

None of the above was staged, committed, or cleaned by this dispatch (fence
is read-only + no git-write). This report file
(`.harmony/.reports/N12-rta.report.md`) does not appear in
`git status --short` at all — `.harmony/.reports/` sits inside the
gitignored `.harmony/` tree, so the write is invisible to git status by
design; the 268-file dirty count above is identical before and after this
report was written.

## (c) DRIFT CLASSIFICATION — `normalize-check.sh --classify`

```
FEATURE_AFFECTING
  222 changed files — 106 non-feature, rest feature-affecting
  commits: 121
```
i.e. **116 feature-affecting files across 121 commits** since the last norm
(`state.md`: norm v2, `last_normalized_sha: 9139dd4`, 2026-07-16) — this
matches the packet's "Tracked issue: HEAVY drift" and SENT record #1 exactly.
`deciding files` (full list in tool output, 116 total) span nearly every
`src/` subsystem: `analysis/`, `api/`, `audio/`, `core/` (undo/command
system — new since last norm), `effects/`, `features/`, `mapping/`, `media/`,
`model/`, `output/` (Syphon/NDI/Spout — new), `render/`, `signal/`, `test/`,
`ui/` (11 UI files touched), plus `cmake/Sanitizers.cmake` and 4 `shaders/`
files. This is a full-tier "code changed" drift (not doc-only), so a Phase
2.5 deep-audit re-trace of the touched features is the right next step
(Boris's call in Phase 2 gate, not run by this dispatch).

Note the discrepancy between the two check modes on this same repo:
`normalize-check.sh` (no `--classify`) prints "70 source commits since
normalize" (its own separate staleness heuristic) while `--classify` counts
121 total commits / 222 changed files with the 116/106 feature split — they
measure different things (a quick staleness trip-wire vs. the full changed-file
classification) and are not in conflict, but only `--classify`'s number
matches the packet's cited "116 files/121 commits," so that is the number to
carry forward.

## BASELINE — `normalize-check.sh` (verbatim, tool default `--framework web`)

```
════════════════════════════════════════
  Normalize Check: RealTimeAudio
════════════════════════════════════════

Project norm:  v2 (unknown)
Spec norm:     v2
Last SHA:      9139dd4

Norm version matches. Checking staleness...
  70 source commits since normalize — features may be stale

RECOMMENDATION: Run incremental re-normalize (stale features only)

Running template validation...
  FAIL: [26. Application UI Framework] missing CONTEXTUAL section: Security (should be present or N/A)
  FAIL: [26. Application UI Framework] missing CONTEXTUAL section: API documentation (should be present or N/A)
  FAIL: missing Deployment & Infrastructure section


STATUS: NEEDS FIXES (template validation failed)
```

Exit code: 1.

## CORRECTED — `tests/validate-features.sh RealTimeAudio --framework game` (verbatim)

```
[... 264 PASS lines omitted, full per-feature table available on request ...]
  FAIL: 24. Dual-Mode System (ProgrammingMode — REMOVED Wave 0) (9 pass, 1 missing)

Global sections:
  PASS: Data Model
  PASS: Shared Infrastructure
  WARN: Deployment & Infrastructure (N/A for game)
  PASS: Section classification markers (26 found)
  PASS: Header metadata
  PASS: Norm version in header

═══════════════════════════
  TOTAL: PASS=264  FAIL=1  WARN=1

ISSUES:
  FAIL: [24. Dual-Mode System (ProgrammingMode — REMOVED Wave 0)] missing REQUIRED section: What it does
```

Exit code: 0 (script only fails hard on zero-features; 1 FAIL is reported,
not gate-failing at the shell level). **Real gap count once framework is
correctly classified: 1 REQUIRED fix (§24 missing "What it does"), 0 real
CONTEXTUAL-section gaps, Deployment correctly WARNs N/A for a desktop app.**

## GIT / IDENTITY

- HEAD: `9ce7fb3ecee18a51bdf0fe1751169e9c5d9cdaf5` (short `9ce7fb3`), branch
  `main`, committed 2026-08-04.
- `config/repos.yml:50` registration: path `~/projects/RealTimeAudio`, alias
  `rta`, tech `[cpp, cmake, juce, opengl, glsl, aubio, ffmpeg, catch2]` — all
  confirmed present on disk (CMakeLists.txt, JUCE via vendored/build config,
  OpenGL/GLSL shaders, aubio/fftw under `third_party/`, Catch2 tests).

## BORIS PICKS

REQUIRED:
- [ ] Create `.harmony/VALIDATION.md` from `references/validation-template.md`
- [ ] Fix FEATURES.md §24 — add the missing `**What it does:**` line
      (`.harmony/FEATURES.md:2009`, before the `**Status: REMOVED...**` line)

RECOMMENDED:
- [ ] Create `.harmony/binding-decisions.md` (Phase 3 step 3b projection)
- [ ] Fix `normalize-check.sh`'s `--framework` default (currently hardcoded
      `web`) so this repo's checks stop reporting 3 false-positive FAILs —
      either pass `--framework game` explicitly going forward, or wire
      `config/repos.yml`'s `tech:` array into the default
- [ ] Resolve the 6 schema-drift gotcha entries (SENT record #2) and commit
      the 3 already-edited Scope/Promoted flips in `.harmony/gotchas.md`
      (SENT record #5)
- [ ] Commit the graphify regen (7 modified/deleted top-level files) and
      decide on the 259-file `graphify-out/cache/ast/` deletion (stop
      force-tracking the cache, or restore it)
- [ ] Commit `.harmony/CHECKPOINT.md`'s latest snapshot (SENT record #5)
- [ ] Run Phase 2.5 deep-audit re-trace given the HEAVY drift (116
      feature-affecting files / 121 commits, SENT record #1) — this Phase-2
      dispatch does not run it
- [ ] Reply to the MCP usage-query (SENT record #3): which of
      graphify-rta/codegraph-rta/clangd-rta does the workflow need

NICE_TO_HAVE:
- [ ] Add explicit `.env`/`*.key`/`*.pem` lines to `.gitignore`
- [ ] Decide fate of `.audit/features-gap-fill/` (untracked, 3 files) — keep
      or clean
- [ ] Archive the 8 `*.pre-s144-migration` backup files under `.harmony/`
- [ ] Check why `.harmony/UNCLEAN-CLOSE-STAMP-harmony-31879.md` (2026-08-30)
      hasn't been consumed at a subsequent boot yet

STATUS: DONE
