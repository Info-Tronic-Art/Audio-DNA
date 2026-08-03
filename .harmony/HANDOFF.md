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

3. **REVIEW STATUS — COMPLETE.** Independent full-arc source review of C1+C2:
   **PASS, 0 blocking, 3 minor** (verdict persisted at
   `memory/.reports/reviewer-ow-arc-c1c2.verdict.md`). Full-tier gate is therefore
   CLOSED for C1+C2 (my behavioral half: ctest 193/193 ×2 trees independently
   rebuilt, live TSan app drive with 2 GL contexts, live probe states 1-2).
   Notable confirmations: the programID cache-key prefix is REQUIRED (not merely
   safe — ~80 programs per context share uniform names, so a name-only key would
   collide *within* one context); W4 is behavior-identical BY CONSTRUCTION (per-target
   float add order preserved, Smoother tick set identical); the C2 equivalence test's
   "old pipeline" reference was mechanically diffed against pre-C2 source and is
   faithful (sole delta: a dead unused local). C1/C2 do NOT worsen the SignalRegistry
   race. The 3 minors, none blocking:
   - **(carry into C3)** `MappingEngine.cpp:169-196` — a concurrent enabled-flip mid-
     `processFrame` can double-tick one smoother (owner disabled between folding member
     k and the outer loop reaching k). Same pre-existing msg↔GL crossing class, strictly
     better than the old torn-pass zeros, and **unreachable once C3's confinement
     lands** — add one explanatory comment line in C3.
   - `TestServer.cpp:970` — `handleRemoveMapping` skips JSON validation; a malformed
     body removes index 0. Intentional drain semantics, bounds-checked, test-only.
   - `TestServer.cpp:947/978` — callAsync lambdas capture `this`/`renderer_` with no
     shutdown guard. Pre-existing fire-and-forget idiom (ApiServer/OSC do the same);
     window shrunk by `testServer_->stop()` running first in the dtor. Recorded, not
     requested.

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

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA concurrency hardening — making the live-VJ render path crash-free and
race-free while the output window is on a projector (the state that matters on stage).
SHIPPED: S2 seqlock FeatureBus — multi-reader race dead, spurious-scene-change class
gone (cb4d5fa) · OW arc C1 — per-renderer GL state kills the 2-GL-thread map
corruption + the confirmed cross-context program-ID poison (88af683) · OW arc C2 —
single-store processFrame closes a live zero-flash the council found (fcad6d0) ·
ratified design + council record + gate recipes committed (2eaae1b, dd33df2, cd6eb9f).
IN-FLIGHT: none — all lanes wrapped, tree clean, builder + reviewer + 3 council seats
released.
NEXT: OW arc C3 (message-thread mapping tick @120Hz + A4 clearAll ruling + A6 asserts),
gated FIRST on the fail-first freeze capture (needs Boris to expand the SignalBar by
hand — 2 seconds) · then the u_bass band-index question · then A1 routing follow-up.
BLOCKERS: none for building. ONE gate step blocked on a human hand: probe states 3-4
need the preview detached, and the ~15px SignalBar cycler defeated synthetic driving.
YOU ARE HERE: the two crash classes the scout ranked #1 and #3 are CLOSED and reviewed;
the cadence/freeze fix (#2's sibling) is designed, staged, and one session from done.
Nothing pushed — 90 commits deep on local main, still awaiting a review-before-push.

## LOOSE-ENDS LEDGER
- **C3 not built** — designed + ratified + packeted, deliberately deferred (long task,
  must begin at session start). Scope: W5+A4+A6 (W6 already landed in C2).
- **Fail-first freeze capture MISSING** (`.harmony/ow-freeze-before.log` holds states
  1-2 only). BLOCKING for C3's commit under R9 doctrine. Needs a manual SignalBar
  expand; synthetic driving failed (app would not take frontmost focus; ~15px target).
- **A4 unresolved**: whether the GL-thread `clearAll` at Renderer.cpp:~1520 is provably
  a no-op decides delete-vs-callAsync. Builder flagged a `PresetManager::loadPreset`
  lead at MainComponent.cpp:204 that could falsify the no-op proof. UNVERIFIED.
- **Reviewer minor carried into C3**: MappingEngine.cpp:169-196 concurrent enabled-flip
  can double-tick one smoother; unreachable after confinement — wants a comment line.
- **Stale comment**: OutputWindow.cpp KNOWN-RESIDUAL block still describes the freeze as
  live and cites a drifted line number. W5 must rewrite it.
- **Eyes reactivity 3/4 failing** — A/B-proven PRE-EXISTING (fails identically at
  7bb5cc2), so not this work's debt, but UNDIAGNOSED. Strong lead: `u_bass` reads
  `bandEnergies[1]` while the battery injects `[0]`. Which side is wrong is unknown —
  if the shader is, live bass reactivity is mis-banded.
- **Pre-existing SignalRegistry.cpp:154 race** observed twice under TSan
  (`.harmony/ow-c1-signalregistry-race.log`). Not touched by this work. Owned by the A1
  routing/signal follow-up, which is itself gated on an un-done thread audit.
- **Residuals shipped knowingly**: ROUTED params and autopilot still freeze on preview
  detach (only MAPPED params are fixed by C3); P3 scalar-crossing set enlarged, deferred
  to spec Step 3.
- **Unexplained infra event**: my first parallel rebuild pair was externally SIGTERM'd
  (`Terminated: 15`). Serial re-run was clean; cause never identified. Watch for it.
- **90 unpushed commits** and no push authorization — the review-before-push discipline
  is the only thing keeping that stack honest.

## META-LEARNINGS
- **Stage protocol is the right shape for a multi-commit arc**: ratify the design to
  disk FIRST, then one builder runs stages with report → my independent gate → my
  commit → next stage. Every stage is independently green and revertible, and hitting a
  context cap mid-arc produced a clean handoff instead of a half-built tree.
- **Dispatch the independent reviewer BEFORE wrapping, not at arc end.** I nearly closed
  with two committed-but-unreviewed stages because I'd planned the review for after C3.
  Unreviewed commits accumulate silently; the full-tier gate has two halves and mine was
  only one of them.
- **Evidence has timing**: `.ips` crash reports land tens of seconds after death, so my
  immediate "0 crash logs" check was worthless and I had to retract it. Same class: a
  `curl /api/health` can answer from a dying instance — pair liveness claims with
  `pgrep`/window enumeration.
- **Inherited numbers rot.** The "~49 unpushed commits" figure rode through several
  handoffs; the truth was 90. Re-verify inherited claims before repeating them.
- **A council earns its cost when it finds work you didn't ask about**: the pragmatist
  seat surfaced the intermediate-zero publish (a live bug, any cadence) that neither the
  spec nor I had noticed, and it became C2.
- **Race-demonstration tests must mirror production access shape** — a tight memcpy
  reader was TSan-blind where field-by-field consumption caught it.
- **Know when to stop driving the UI.** The SignalBar cycler is a documented flake; I
  spent two attempts, then converted it into a handoff item rather than looping.

## CHANNEL HARVEST
- **Boris ideas captured this session: NONE** — Boris sent no messages (fully autonomous
  drain-mode session), so the R2 transcript sweep for un-flagged idea-class statements
  found nothing to capture. Idea-ledger correctly untouched.
- **Learnings emitted to the harmony2 event log (lane A, co-session secondary)**:
  `tsan-production-faithful-tests`, `stage-protocol-multi-commit-arc`,
  `gate-evidence-timing-traps` — the primary distills these at its close.
- **Repo-local knowledge written**: gate-mechanics gotchas (TSan app recipe, TestServer
  `::1` bind, verify-frontmost-before-click, TSan abort-at-exit vs product crash,
  delayed `.ips`, `git commit --only` can't stage new files) → `.harmony/gotchas.md`;
  builder discoveries → `.harmony/notebook.md`; council record + adjudications →
  `.harmony/specs/outputwindow-arc-design.md`; session narrative →
  `.harmony/sessions/2026-08-02-s2-seqlock-secondary.md`.
- **For Boris, needing his hands or his taste**: see ONLY BORIS CAN CHECK below.
- **No harmony2 SYSTEM files touched** this session (foreign-repo work only) — clean
  secondary, no escalation.

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
