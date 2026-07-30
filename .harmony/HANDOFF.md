# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). Session 2026-07-30 PM2 closed a MONSTER day: Boris's
8-item gate came back 7/8 PASS (8/8 after the playlist gesture), then he said "work
on all of them now" — the FULL 13-item queue. Result: 12 of 13 items BUILT, INDEPENDENTLY
REVIEWED, and BEHAVIORALLY GATED in one session (~33 commits, 8 build lanes, tests
182→188, NOT pushed). Combined gate PASS: live API battery + graceful-quit UNDER
concurrent API load → clean exit, ZERO new .ips — the crash family AND the newly-found
quit-hang window are closed. Fresh instance was left running for Boris (120fps).
Read ledger 2026-07-30 PM21–PM55 for the full narrative; dossiers:
scout-playlist-drop.md · scout-autopilot-sources.md (+ prior day's four).

START HERE:
1. BORIS REPLAY FEEDBACK FIRST. Boris has a 7-item GESTURE replay list (below) my
API-scope gate couldn't cover. Process before anything else: PASS → close rows with
dated notes; FAIL/odd → receiver-verify on disk (.ips? /api state?) BEFORE any fix
dispatch. THE 7-ITEM REPLAY LIST (verbatim):
  a. Drag an effect onto a layer's CHANNEL STRIP (single + multi-select) → lands in
     that layer's FX stack, ONE Cmd+Z restores.
  b. Finder-drop 2 videos + 1 image together → all three land, ONE Cmd+Z removes all.
  c. Cmd+X with a cell selected (clears) / with nothing selected (clean no-op).
  d. Click a PLAYING video cell → visibly restarts from in-point.
  e. Header-drag "Energetic (9)" → cell reads "MilkDrop Playlist (9)"; the 3 playlist
     knobs (cycle/timing/blend) now actually change what lands.
  f. Autopilot over a SOURCE cell → advances off it (was frozen forever).
  g. Genre auto-switch to an empty deck → preview goes blank (no ghost clip).
2. **START HERE — long task, begin at session start (if no Boris feedback waiting):
SYPHON LANE** — the ONE deferred item of Boris's 13 ("work on all of them now",
deferred on drain budget 2026-07-30 PM2, Boris informed with override offer, no
override received). Scope: integrate Syphon SDK (FetchContent-pinned per gotcha),
publish composited output as a Syphon server, Renderer hookup. Behavioral proof
needs Boris + a Syphon client — plan the lane so source+review close autonomously
and the live check lands on his list.
3. BORIS DECISION QUEUE (all pre-analyzed, deliver ONE per ask): source/MilkDrop
retrigger-restart scope (their time base is app-init-scoped; Video/ImageSeq restart
shipped) · column-trigger retrigger parity · build Cut/Copy/Paste suite (menu enums
already RESERVED at MenuBarModel.h:76-80) · playlist mode-gate consistency
(header-drag works ANY mode; row multi-select needs Playlist mode) · reset ~5s
latency taste check (pre-existing GL wait).
4. QUEUED FOLLOW-UPS (autonomous-buildable, priority order): FeatureBus TSan race
(test_feature_bus.cpp:143 vs :161, real, pre-existing, TSan-confirmed) ·
renderer-thread-safety design pass (effectChain_ per-field Effect::enabled_/
EffectParam::value GL-reads + the 3 renderer_-via-HTTP endpoints incl. load_image
GL-sleep gotcha) · OutputWindow second-GL-thread crash-family scout (shares
EffectChain by ref; never had scrutiny) · ID-based selection remap (Layer has stable
uint32_t id; CellPos lacks plumbing) · full §3 APP-INVENTORY row pass (dated delta
block sits in §2).
5. VERIFICATION DOCTRINE now in force (from this session's incidents): FORCED
REBUILD before any ctest claim (stale-binary false-green) · `git commit --only
<files>` or pre-verify staging in the shared tree (index-sweep incident) · treat
subagent idle-without-report as auto-nudge trigger (5/5 pattern).

Standing rules: do NOT push (entire ~33-commit day is local); conform to
ClipCommands.h/DeckCommands.h/EffectCommands.h/TriggerCommands.h/UndoService
patterns at HEAD; structural-mutation commands carry a fence (notebook LAW); launch
ONLY via open; SIGKILL disposable instances; SYNTHETIC-CLICK PREFLIGHT (gotchas 11)
before any click burst; scouts/builders need explicit deliver-to-main clauses AND
expect to nudge them anyway.

## PRIMER

- App: Audio-DNA, C++20/JUCE/OpenGL VJ instrument. Port 7070 = production API;
  8080 = TestServer. Release binary: build/AudioDNA_artefacts/Release/Audio-DNA.app
  (Audio-DNA Dev cert, mic TCC inherited — zero prompts on relaunch).
- Sanitizer infra NEW: `ADNA_SANITIZE` (address/undefined/thread) via
  cmake/Sanitizers.cmake; build-asan/ + build-tsan/ pre-configured. Debug builds
  now compile (OutputWindow name-hiding fix). Suite baseline 188 (187 under TSan —
  the known FeatureBus race).
- Thread model hardened this session: composition writes = message thread
  (ApiServer 6 endpoints marshalled; callAsync safety = single-lifecycle invariant,
  documented at first callAsync site) · activeSources_ = GL-thread-confined
  (blocking marshal for rare off-thread callers + isAttached guard) · EffectChain
  container = mutex + idempotent population (per-FIELD sync explicitly deferred —
  comment at effectsMutex_ decl) · teardown order: servers stop → GL detach →
  everything else (~MainComponent).
- Ledger = .harmony/undo-v1-ledger.md (PM21–PM55 this session); authorship note:
  ff19094 contains MISC-authored servers-stop-before-detach reorder (index-sweep,
  disclosed, ledger PM54 is the authorship record).

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA live VJ app — stability + interaction-completeness hardening wave (post-Undo-v1).
SHIPPED: All 13 Boris queue items except Syphon — autopilot-sources fix (+first autopilot tests) · fold-height + stale-selection + structural-flag hardening · 6 API endpoints thread-marshalled · playlist header-drag + real controls · ASan/TSan build infra · source ids, menu gating, preview reconcile, B8 removal · shutdown bundle (detach-first, EffectChain fence, bounds-check) + activeSources_ confinement + quit-hang closed · LayerStrip FX-drop, mixed-drop, Cmd+X, retrigger-restart, genre-switch fix. ~33 reviewed commits, tests 182→188, behavioral gate PASS (API + quit-under-load, zero crashes).
IN-FLIGHT: none (all lanes closed; 2 unconfirmed formalities logged in loose ends).
NEXT: Boris 7-gesture replay → Syphon lane → decision queue (5 rulings) → FeatureBus race + renderer-thread-safety pass.
BLOCKERS: none.
YOU ARE HERE: hardening wave COMPLETE and gated; one feature (Syphon) left in the approved queue; next fork after that = Session Recorder (ratified default) vs ISF import vs visual-design resume.

## LOOSE-ENDS LEDGER

- Boris 7-gesture replay list PENDING (verbatim in birth prompt) — my gate covered
  API+lifecycle scope only; gestures need his hands.
- Infra builder's Debug+ASan APP-target build verification never reported back
  (OutputWindow one-liner committed this close, Release-verified via my gate's
  forced rebuild + 120fps run; the Debug/ASan app-build confirmation is the open
  half — cheap to re-run: `cmake --build build-asan --target AudioDNA`).
- MISC builder's author-confirm on the ff19094 index-sweep content: RESOLVED at
  close (+ its own independent forced-rebuild verification, 188/188) — exact-match
  confirmed line-by-line; ledger PM54 remains the authorship record. NO action
  needed next session.
- FeatureBus TSan race: real, pre-existing, queued (birth prompt item 4).
- Renderer-thread-safety design pass queued (effectChain_ per-field + 3 renderer_
  HTTP endpoints).
- OutputWindow second-GL-thread scout queued (no live gap found, never scrutinized).
- ID-based selection remap queued (selection currently CLEARS on layer reorder —
  correct but lossy).
- reset endpoint ~5s latency: pre-existing GL wait, unexplained in detail — fine
  behaviorally, worth a look if Boris notices it live.
- NOT PUSHED: entire day (~33 source commits + chores) is local-only per standing
  rule — push decision is Boris's.
- Syphon: deferred WITH Boris's knowledge + override offer (none received) — now
  next session's #1 committed MUST.
- Unsure-about: reviewer graded test_renderer_source_confinement an honest
  pattern-simulacrum — real confinement proof needs an app-level HTTP set_preset
  drive (folded into future gate recipes, not yet run under contention).

## META-LEARNINGS

- Adversarial pushback (builder, source-evidence) overturned a reviewer APPROVE;
  reviewer re-derived and self-corrected — the hang window BOTH initially missed
  was real. Culture: verdicts are falsifiable, pushback is the standard.
- Defense-in-depth BEFORE adjudication: with two source-readers disagreeing at
  micro-interleaving granularity and both mitigations cheap, dispatching both made
  the later verdict-flip cost zero schedule.
- Mutex-vs-confinement heuristic (now in code comments): READ FREQUENCY off the
  owning thread decides — rare off-thread calls → confine+marshal; constant
  off-thread reads → narrow container mutex, never across GL calls.
- One-writer-per-file survived 3 boundary collisions via the RELAY pattern (fix
  handed to the file's current owner with full spec + credit) — zero shared-write
  exceptions granted all session.
- stale-binary-false-green + shared-index-commit-sweep + subagent-silent-idle-nudge:
  all three emitted as log-events in-session (lane-A telemetry) for primary distill.
- Sanitizer infra ROI was same-day: two pre-existing bugs surfaced before any
  sanitizer-targeted verification ran.

## CHANNEL HARVEST

- Lane-A log-events emitted in-session (primary distills at its close):
  `stale-binary-false-green` · `shared-index-commit-sweep` ·
  `subagent-silent-idle-nudge` (each a one-liner with the reusable rule).
- Boris-idea sweep (R2 backstop, transcript re-scanned): NO un-captured idea-class
  statements this session — Boris's messages were feedback/approvals/tasking; the
  one observation-class line ("not sure what a playlist is") was captured in-session
  as the playlist discoverability gap (checklist row 9 + PM22, now shipped as the
  header-drag feature + queued mode-gate ruling). Idea-ledger untouched (no
  canonical records needed; no off-canonical prose present).
- System-upgrade candidate routed via events: treat subagent idle-without-report
  as auto-nudge trigger (orchestrator doctrine candidate — primary's call).
- Structural candidates (project-scoped, in birth-prompt item 4): renderer
  thread-safety pass · FeatureBus race · OutputWindow scout · ID-remap.
