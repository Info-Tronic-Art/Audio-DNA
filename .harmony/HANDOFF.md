# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). Session 2026-08-02b (secondary, slim) closed 2 lanes
full-tier at ~33% ctx — 5 commits local, NOT pushed (tests 189→193). **Branch is
`main`, ahead of `origin/main` by 90 commits — VERIFIED via `git branch -avv` this
session; earlier handoffs quoted ~49, which was stale. Re-verify, never inherit.**

(1) **S2 SEQLOCK — SHIPPED** (`cb4d5fa`). FeatureBus is now a seqlock with
caller-owned VALUE copies: `alignas(64) atomic<uint64_t> seq_` (odd=publishing) +
`atomic<uint32_t>[80]` payload, writer-private staging, bounded retry (4 attempts,
in-attempt odd-seq spin) → per-reader-THREAD last-good fallback. `acquireRead/
getLatestRead/hasNewData/kNewFlag` DELETED (grep-zero). Move-only `FeatureBus::Writer`
claimed at the `testMode_` branch; double-claim = deterministic invalid handle
(jassert'd). R7: Compositor/EffectChain snapshots became owned VALUES (dangling
defaultSnap escapes gone). R9 evidence in-tree: `.harmony/s2-tsan-before.log` (7 races
+ torn coherence on the OLD protocol) / `s2-tsan-after.log` (clean). Reviewer PASS,
0 blockers, all latitude mechanisms ratified.

(2) **OUTPUTWINDOW ARC — C1+C2 SHIPPED, C3 IS YOUR START-HERE.** Council-ratified
design: `.harmony/specs/outputwindow-arc-design.md` (U1-U5, A1-A6, W1-W7, ship order).
- `88af683` **C1** = per-renderer `EffectChainGLState` (uniformLocationCache_ +
  prevFrame quartet out of shared EffectChain — kills the 2-GL-thread map UB AND the
  program-ID location poison), render() takes (GLState&, const FeatureSnapshot&) with
  shared `latestSnapshot_` DELETED (each renderer reads the bus itself), output detach
  added to the shutdown law. **Scout R1's INFERRED ID collision is now VERIFIED**:
  passthrough=2 in BOTH contexts, hue_shift 26 vs 41, vignette 89 vs 104.
- `fcad6d0` **C2** = single-store `processFrame` (the old reset→accumulate→clamp
  3-pass published intermediate ZEROS that the second GL thread uploaded live — a
  pre-existing zero-flash found by the pragmatist council seat) + W6 test-mode
  mapping routes (`/api/add_mapping`, `/api/remove_mapping`, 8080 ONLY; 7070 = 404
  verified live). Equivalence: 300-tick lockstep vs a verbatim old-pipeline copy,
  EXACT float equality, 1262 assertions.
- `2eaae1b` = knowledge layer (design spec, council record, gate-mechanics gotchas,
  evidence logs, session record).

Full narrative: `.harmony/sessions/2026-08-02-s2-seqlock-secondary.md` (entry labels
authoritative over position).

START HERE:

1. **BORIS FEEDBACK FIRST (if any waiting).** Two lists may return: (A) the Syphon
   live-check list from 2026-08-02a (frame content in Simple Client/VDMX, eager-announce
   taste, runtime-default-OFF taste, loopback veto / remote-control question);
   (B) the 7-item GESTURE replay list from 2026-07-30 (verbatim in the previous handoff
   revision — STILL OUTSTANDING, never processed, now 4 sessions old). PASS → close rows
   with dated notes; FAIL/odd → receiver-verify on disk BEFORE any fix dispatch.
   **NEW for Boris this session** — see "ONLY BORIS CAN CHECK" below.

2. **START HERE — long task, begin at session start (if no Boris feedback waiting):
   OUTPUTWINDOW ARC C3** — the cadence change, per `outputwindow-arc-design.md`
   W5+A4+A6+W6-follow-through. Do NOT re-litigate the design (3-seat blind council,
   chair-adjudicated).
   **GATE STEP 1 (BLOCKING, do this FIRST): produce the fail-first freeze capture.**
   `.harmony/ow-freeze-before.log` currently holds ONLY probe states 1-2 (both PASS).
   States 3-4 (preview DETACHED via SignalBar expand, output closed / open) must be
   captured FAILING before C3 lands — R9 doctrine. I could not produce it: the
   SignalBar cycler is the documented ~15px synthetic-click flake AND the app would
   not take frontmost focus (a coordinate click landed in the terminal; AX button
   enumeration returned empty). **Cheapest path: ask Boris to expand the SignalBar
   by hand (2 seconds), then run the two commands** (they are in the C2 builder report
   and the probe file header):
     cd tests/visual && AUDIODNA_NO_SPAWN=1 OW_PROBE_STATE=signalbar \
       ../../.venv/bin/python -m pytest test_mapping_tick.py -v -s
     (then reopen output via menu Output>"Fullscreen: ... (main)" and repeat with
      OW_PROBE_STATE=signalbar_output)
   Detach oracle: `/api/status` fps collapses when the preview GL context detaches —
   use it to PROVE the state before trusting a FAIL.
   Then implement W5 (kMappingTickHz=120 named constant — matches the measured
   119.8fps rig; the Smoother has NO dt term so rate IS the time constant, 60 would
   double the smoothing feel), the `tickFeaturePipeline()` seam, deletion of the
   GL-thread processFrame call, A4 (verify minimal's no-op proof for the GL `clearAll`
   at Renderer.cpp:~1520 — builder flagged a PresetManager::loadPreset lead at
   MainComponent.cpp:204 that could falsify it; if falsified → callAsync marshal),
   A6 confinement jasserts. Also: W5 must REWRITE the KNOWN-RESIDUAL comment block in
   `src/ui/OutputWindow.cpp` (it still documents the freeze as live, and its
   "Renderer.cpp:239" citation has drifted) — the comment is the last place the retired
   limitation would survive as a lie. Builder `ow-builder` executed C1+C2 flawlessly
   under a stage protocol (report → Harmony gate → Harmony commit → next stage) —
   reuse it.

3. **REVIEW STATUS — READ BEFORE PUSHING ANYTHING.** An independent full-arc source
   review of C1+C2 was dispatched at the end of this session; its verdict may not have
   landed before close. CHECK for it and fold it in before C3 builds further. If no
   verdict is recorded below this line, RE-DISPATCH the review (packet shape is in the
   session record) — C1+C2 have my behavioral gates (ctest 193/193 ×2 trees
   independently rebuilt, live TSan app drive with 2 GL contexts, live probe states
   1-2) but the source-review half of the full-tier gate is unconfirmed.

4. **BORIS DECISION QUEUE** (all pre-analyzed, deliver ONE per ask): source/MilkDrop
   retrigger-restart scope · column-trigger retrigger parity · Cut/Copy/Paste suite
   (menu enums RESERVED at MenuBarModel.h:76-80) · playlist mode-gate consistency ·
   reset ~5s latency taste · eager-announce · runtime default/persistence ·
   remote-control+token.

5. **QUEUED FOLLOW-UPS** (unchanged unless noted): ID-based selection remap · full §3
   APP-INVENTORY row pass (TWO dated delta blocks in §2: 07-30 + 08-02) · S3 field
   atomics (after S2 survives real use) · empty-string AUDIODNA_API_BIND fallback (nit)
   · **NEW: A1 routing/signal follow-up** — extract RoutingEngine + SignalRegistry
   evaluation onto the same tick as mapping, gated on a SignalRegistry thread audit;
   until then ROUTED params still freeze on preview detach. A pre-existing race is
   already captured: `.harmony/ow-c1-signalregistry-race.log` (SignalRegistry.cpp:154
   `evaluateAll`, seen twice under TSan) · **NEW: Eyes reactivity triage** — 3 of 4
   reactivity tests fail identically at HEAD *and* at 7bb5cc2 (A/B-proven pre-existing,
   NOT S2/arc debt). Reviewer found a concrete lead: `u_bass` uploads
   `bandEnergies[1]` (ProceduralSource.cpp:161, CompositorEngine.cpp:1410) while the
   battery injects `bandEnergies[0]`. **If the shader index is the wrong side, live
   bass reactivity is mis-banded on the rig** — worth an early look.

## ONLY BORIS CAN CHECK (this session's additions)
- **Mapping response FEEL after C3** (kMappingTickHz=120 was chosen to preserve the
  measured incumbent rate; only his eye/ear confirms the smoothing feel is unchanged).
- **SignalBar-expanded freeze demo** (2-second manual layout change unblocks the
  blocking C3 gate step above — no gate can substitute).
- Whether ROUTED-param and autopilot freezing on preview detach (both still open,
  recorded residuals) is acceptable to ship with, or should jump the queue.

## STANDING RULES (unchanged)
Do NOT push (90 commits ahead of origin/main — verified 2026-08-03). Conform to ClipCommands.h/DeckCommands.h/
EffectCommands.h/TriggerCommands.h/UndoService patterns at HEAD; structural-mutation
commands carry a fence (notebook LAW); launch ONLY via `open` (7070 binds ~12s in
Release, ~2s warm; poll /api/health); SIGKILL disposable instances; SYNTHETIC-CLICK
PREFLIGHT (gotchas 11) before any click burst; `.harmony/` is gitignored-but-tracked —
new knowledge files need one-time `git add -f`; TCC mic prompt can re-fire after a
rebuild. **NEW gate mechanics (gotchas, 2026-08-03):** build-tsan is configured
TEST_SERVER=OFF (TSan app runs are production-mode, mic-driven); open the output
window via menu AXPress Output>"Fullscreen: ... (main)" and close via Output>
"Disabled" (Cmd+F/Escape get swallowed); VERIFY frontmost before any coordinate click;
a TSan build that reported races ABORTS at exit (SIGABRT + .ips) — that is the
sanitizer, not a product crash; `.ips` files land with a DELAY so an immediate
post-quit check proves nothing; TestServer binds `::1` (probe via `localhost`);
e2e client class is `VJAppController`; `git commit --only` cannot stage NEW files
(`git add` them first).
