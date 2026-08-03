# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). Session 2026-08-03a (secondary, slim) retired the
OutputWindow-arc C3 **blocking gate step** that had stalled two prior sessions, and
finalised the A4 design fork through two independent seats (one adversarial), and then —
after Boris extended the context budget mid-session — BUILT AND SHIPPED C3 ITSELF.
**The OutputWindow arc is COMPLETE.** See the ADDENDUM at the end of this file, which is
authoritative over the C3 sections below (those were written before C3 was built and
describe it as pending — they are preserved for their scope/constraint detail, not their
status). Session commits: `0cc2b5a` (evidence + AX tooling), `799d97d` (handoff),
`c51aff7` (**C3**), `ed0aa91` (addendum), `618b29a`, `e76ca9f` (header comment). NOT pushed.

**UNPUSHED COUNT: do not read it here — RUN IT.** `git rev-list --count origin/main..HEAD`
That number has been wrong in three consecutive handoffs (`~49`, then `90`, then `95`) for
a structural reason: any commit that corrects the figure immediately invalidates it, so a
hardcoded count is guaranteed to rot. It is deliberately not written here. The same rule
applies to every count and line number in this file — the commit SHAs above are stable and
safe to trust; nothing that changes with each commit is.

### READ THIS FIRST — inherited "facts" in this repo have a bad track record
Last session's handoff gave gate-step-1 instructions that could not work as written:
wrong route, missing launch flag, and an oracle that reports the OPPOSITE of the truth.
They were written in good faith by a session that could not execute them, which is exactly
how a plausible-but-untested recipe gets inherited. **Verify every inherited command, line
number, and count against disk before acting on it.** Every line number in the
OutputWindow arc has drifted at least once.

## START HERE

1. **BORIS FEEDBACK FIRST (if any waiting).** Two lists may return:
   (A) the Syphon live-check list from 2026-08-02a (frame content in Simple Client/VDMX,
   eager-announce taste, runtime-default-OFF taste, loopback veto / remote-control question);
   (B) the 7-item GESTURE replay list — **now 5 sessions old and still unprocessed.**
   It kept getting lost because each handoff *referenced* the previous revision instead of
   carrying it. It is carried VERBATIM below so that stops happening.
   PASS → close rows with dated notes; FAIL/odd → receiver-verify on disk BEFORE any fix dispatch.

   **THE 7-ITEM GESTURE REPLAY LIST (verbatim, from 2026-07-30):**
   - a. Drag an effect onto a layer's CHANNEL STRIP (single + multi-select) → lands in
     that layer's FX stack, ONE Cmd+Z restores.
   - b. Finder-drop 2 videos + 1 image together → all three land, ONE Cmd+Z removes all.
   - c. Cmd+X with a cell selected (clears) / with nothing selected (clean no-op).
   - d. Click a PLAYING video cell → visibly restarts from in-point.
   - e. Header-drag "Energetic (9)" → cell reads "MilkDrop Playlist (9)"; the 3 playlist
     knobs (cycle/timing/blend) now actually change what lands.
   - f. Autopilot over a SOURCE cell → advances off it (was frozen forever).
   - g. Genre auto-switch to an empty deck → preview goes blank (no ghost clip).

2. **START HERE — long task, begin at session start (if no Boris feedback waiting):
   OUTPUTWINDOW ARC C3 IMPLEMENTATION.** The blocking gate step is DONE — do not redo it.

   **The work packet is already written: `.harmony/.work-packets/c3-mapping-tick.md`**
   (local-only; `.work-packets/` is gitignored by convention). It contains corrected line
   numbers, the finalised A4 ruling with full reasoning, the A6 ordering constraint, the
   out-of-scope list, and complete test-rig mechanics. **Dispatch a builder against it.**
   Do NOT re-litigate the design (3-seat blind council, chair-adjudicated). Do NOT re-derive
   A4 (settled by two independent seats, one adversarial). If the packet is missing,
   everything in it is reproduced here plus `.harmony/specs/outputwindow-arc-design.md`.

   Scope: **W5** (kMappingTickHz=120 named constant, message-thread juce::Timer as sole
   processFrame caller, `tickFeaturePipeline()` seam, delete the GL-thread call at
   **Renderer.cpp:242**, timer unconditional/no attach gating) + **A4** (**DELETE** the
   clearAll at **Renderer.cpp:1538**; also update the now-misleading comment at
   Renderer.cpp:1537) + **A6** (confinement jasserts in processFrame/addMapping/
   removeMapping/clearAll) + two comment fixes (OutputWindow.cpp KNOWN-RESIDUAL block;
   MappingEngine.cpp:169-196 review minor 1).

   **A6 HARD ORDERING CONSTRAINT:** must land WITH or AFTER W5+A4, never before — the only
   two offenders (Renderer.cpp:242, :1538) are deleted by those changes, so an A6-first
   ordering fires the assert on startup. Unit tests are safe: `tests/test_mapping_engine.cpp`
   has no JUCE init, but the first `MessageManager::getInstance()` sets messageThreadId to
   the calling thread, so the single-threaded test self-identifies as the message thread.

   Then the **full-tier gate**: ctest (baseline 193/193) · probe states 1-4 ALL PASS (the
   "after" half of the fail-first pair) · EMA parity per W7(iii) · TSan drive with 2 GL
   contexts (bar: no NEW finding classes beyond the documented deferred list, design A2) ·
   independent Reviewer on source. The builder NEVER self-verifies; Harmony runs the
   behavioral half because she did not build.

## WHAT LANDED THIS SESSION (commit 0cc2b5a)

**GATE STEP 1 COMPLETE — states 3 and 4 captured FAILING** against verified pre-C3 code
(HEAD 82a74c1; provenance proven: no src/ or tests/ file newer than the binary, both trees
clean at HEAD). Full methodology in `.harmony/ow-freeze-before.log`.
- State 3 (`signalbar`): FAILED — `param Perspective Tilt.tilt_x FROZEN at 0.5 for 500ms
  after rms->1.0 — mapping tick is not running in this attach state`.
- State 4 (`signalbar_output`): FAILED identically **while the output window rendered at
  full rate** (`[OutputRenderer] ... (frame 301)`). This is the decisive case: it isolates
  the freeze to the PREVIEW context (the call at Renderer.cpp:242) and refutes any "some
  other GL thread keeps it alive" explanation.

**`tests/visual/ax_press.py` — the manual operator step is retired.** Drives JUCE buttons
by AX title through the Accessibility C API. This matters because states 3-4 must run
AGAIN after C3 and on every future regression; a permanently manual gate step is a gate
that quietly stops being run. Raises on ambiguity rather than guessing (verified live: `>`
matches 4 buttons → refuses). Glyphs verified from source: SignalBar.cpp:23 grow=`▼`,
:24 shrink=`▲`.

**A4 FORK RESOLVED — design premise falsified, prescribed remedy rejected.** The design
said "delete if the no-op proof holds; if falsified, callAsync marshal instead." The proof
IS falsified: TestServer is constructed and listening inside the MainComponent ctor
(MainComponent.cpp:1581-1590), before window show and before first GL attach, so mappings
CAN exist pre-attach — our own W7(ii) probe is in that class. But callAsync is WORSE: it
preserves the same wipe and adds a window where mappings installed between attach and
callback are destroyed. An adversarial redteam seat, briefed to break the deletion ruling
on its most dangerous failure mode (index-based retargeting), FAILED:
- Targets ARE index-based (`targetEffectId`, MappingTypes.h:127-128) — the feared mode is
  structurally possible but unreachable here.
- `initEffectChain` only populates an EMPTY chain (guard at Renderer.cpp:1507-1508), and
  EffectChain is APPEND-ONLY (EffectChain.h:77-82/136-139) — nothing shifts under an index.
- A dangling index is a SAFE per-tick no-op, re-validated every frame
  (MappingEngine.cpp:161-165, EffectChain.cpp:29-35).
- TestServer resolves targets BY NAME and errors on an empty chain (TestServer.cpp:917-942),
  so it cannot create a mis-targeted mapping pre-attach.
- Deletion additionally removes a GL-thread write to message-thread-owned state, a race
  window, and an ownership violation (TestServer.cpp:950-952 states the ownership rule).
**=> A4 IS FINAL: DELETE.**

**Three inherited errors corrected** (now in `.harmony/gotchas.md`, 25 entries) — see RIG
MECHANICS below for the working commands.

## RIG MECHANICS (verified this session — use verbatim)

```
open --stdout /tmp/adna-out.log --stderr /tmp/adna-err.log \
  build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode
curl -s "http://[::1]:8080/api/health"          # NOT 127.0.0.1, NOT /api/status
# DETACH ORACLE (200 in <0.05s = attached; ~5s then 500 = detached):
curl -s -m 12 -w "%{http_code} %{time_total}" -X POST \
  "http://[::1]:8080/api/render_frame" -d '{"output_path":"/tmp/x.png"}'
# SignalBar drive (no human needed):
.venv/bin/python tests/visual/ax_press.py "▼"    # expand  -> preview DETACHES
.venv/bin/python tests/visual/ax_press.py "▲"    # collapse -> preview REATTACHES
# output window: MENUS work via AppleScript (in-window controls do NOT):
osascript -e 'tell application "System Events" to tell process "Audio-DNA" to click \
  menu item "Fullscreen: 1728x1117 (main)" of menu 1 of menu bar item "Output" of menu bar 1'
# ...and close it via Output > "Disabled" (Cmd+F / Escape get swallowed).
# the 4 probe states:
cd tests/visual && AUDIODNA_NO_SPAWN=1 \
  OW_PROBE_STATE=<preview|preview_output|signalbar|signalbar_output> \
  ../../.venv/bin/python -m pytest test_mapping_tick.py -v -s
```
**Three traps that cost this session time:**
- `--test-mode` is REQUIRED or 8080 never binds — **while 7070 still does**, so a "server
  is up" check against 7070 is a FALSE GREEN.
- TestServer binds `::1` ONLY. `127.0.0.1` returns empty and is indistinguishable from a
  dead server. Health route on 8080 is `/api/health`, NOT `/api/status`.
- **`fps` is an INVALID detach oracle.** Stored only inside `renderOpenGL()`
  (Renderer.cpp:181), so on detach it FREEZES at its last value — measured **106.18 with
  the context provably dead**. Same defect in `/api/status` frameTimeMs. A startup reading
  of ~2.4e-06 is not a detach either: `fpsTimer_` inits to 0.0 and JUCE's counter is
  ms-since-boot, so the first frame computes 1/uptime (1/2.403e-06 = 4.82 days).
- AppleScript CANNOT reach JUCE's nested AX elements (`entire contents` does not recurse
  into AXGroups) — use ax_press.py for in-window controls, osascript for MENUS only. Do
  not conclude "no AX tree" from an empty AppleScript enumeration; that misdiagnosis cost
  a prior session.

## WHERE WE ARE IN THE BUILD
BUILD: Audio-DNA concurrency hardening — making the live-VJ render path crash-free and
race-free while the output window is on a projector (the state that matters on stage).
SHIPPED: S2 seqlock FeatureBus (cb4d5fa) · OW arc C1 per-renderer GL state (88af683) ·
OW arc C2 single-store processFrame (fcad6d0) · ratified design + council record
(2eaae1b, dd33df2, cd6eb9f) · **C3 fail-first evidence + AX tooling (0cc2b5a)**.
IN-FLIGHT: none — all lanes wrapped, agents released.
NEXT: C3 implementation (packet written, unblocked) · then the u_bass band-index question
· then A1 routing follow-up.
BLOCKERS: **none.** The human-hand blocker recorded by the last two handoffs is GONE —
ax_press.py drives it.
YOU ARE HERE: two crash classes closed and reviewed; the cadence/freeze fix is designed,
packeted, evidenced, and one builder dispatch from done. Nothing pushed — 95 commits deep
on local main, still awaiting a review-before-push.

## LOOSE-ENDS LEDGER
- **C3 implementation + full-tier gate** — packet written, not started. #1 priority.
- **A1 routing/signal follow-up** — extract RoutingEngine + SignalRegistry evaluation onto
  the same tick, gated on a SignalRegistry thread audit. Until then ROUTED params still
  freeze on preview detach (and autopilot pauses — pre-existing). Pre-existing race
  captured: `.harmony/ow-c1-signalregistry-race.log` (SignalRegistry.cpp:154 evaluateAll).
- **NEW — PRESET/BINDING SILENT RETARGET. Assessed 2026-08-03: LATENT but probably ALREADY
  BITING. High priority; Boris-visible.** `loadPreset` restores RAW saved effect indices
  (PresetManager.cpp:212-213, saved at :113) while the EFFECT restore right above it already
  name-matches (:162-179) — that asymmetry is the bug.
  **Why "append-only is safe" does NOT apply here:** the chain is built by CATEGORY GROUPING
  over a fixed 11-category order (Renderer.cpp:1518-1534), so appending to the end of
  EffectLibrary.cpp lands MID-CHAIN. The 3 newest defs (EffectLibrary.cpp:794-811) are
  pattern/glitch — categories 4th-5th of 11 — so ~100 downstream effects shifted when they
  landed. **Presets saved before those additions already mis-target today** if their mappings
  pointed past the insertion point.
  **Fails SILENT, not safe:** out-of-range would no-op (MappingEngine.cpp:161-165,
  EffectChain.cpp:29-35), but with 135 effects and shifts of 1-3 positions a stale index is
  almost always still IN range — driving the WRONG effect's param. Silent-wrong dominates.
  **No guard exists:** savePreset writes "version",1 (PresetManager.cpp:76); loadPreset never
  reads it. No count check, no validation, no migration.
  **SAME BUG IN BindingManager** (Binding.h:85 `int targetEffectIndex`, raw save/load at
  BindingManager.cpp:238/282) — key bindings mis-target identically. Should ride the same fix.
  Also unaudited: SessionRecorder.cpp:212 writes a version field; unknown whether it
  serialises effect indices.
  **FIX (designed, ONE COMMIT, PresetManager.cpp only):** additionally save
  `targetEffectName` (unique across the chain — verified, 135 effects, no duplicate names)
  and `targetParamName`; KEEP writing the old int fields so new presets stay loadable by old
  builds. On load, resolve by name into the existing index fields (MappingTypes.h stays
  index-based — no engine change, no hot-path change). Name absent → fall back to today's
  raw-index path, and the next save upgrades the file. Name present but not found → DROP the
  mapping and log (Mapping has no field to carry an unresolved name, so "keep disabled" would
  retain a bogus index that re-enabling silently mis-drives). Bump version to 2 and START
  READING it. Unavoidably lossy: an OLD preset saved against a DIFFERENT effect list cannot
  be re-keyed — the name was never written.
  **Before relying on it:** param-name uniqueness within an effect was spot-checked, not
  exhaustively scanned — add a save-time assert or do a one-off check across all 135 defs.
- **Eyes reactivity 3/4 failing — DIAGNOSED 2026-08-03. The rig is FINE; the cause is a
  product gap.** The `u_bass` mis-banding worry is CLOSED: `bandEnergies[1]` IS the canonical
  Bass band (60-250Hz, SpectralFeatures.cpp:18-23; corroborated by SignalRegistry.cpp:21,
  the UI meter labels at AudioReadoutPanel.h:68 / SpectrumDisplay.h:34, and
  MappingEngine.cpp:64-70). All three shader upload sites agree (ProceduralSource.cpp:161/164/167,
  CompositorEngine.cpp:1410-1412, EffectChain.cpp:325/327/329). **No live bass mis-banding.**
  TWO REAL DEFECTS instead:
  1. **Test bug (trivial):** `tests/visual/test_audio_reactivity.py:33` injects index 0 = Sub
     when simulating "Bass". Should be index 1. Fixing this ALONE will not make the test pass.
  2. **PRODUCT GAP (the substantive one, needs a Boris ruling):** several shaders DECLARE audio
     uniforms and never USE them — GLSL strips unused uniforms, the location lookup returns -1,
     nothing uploads, nothing moves. `u_beatPhase` is declared in 3 shaders
     (EmbeddedShaders.h:2824, :6172, :3042) and used in ZERO. `u_bass` is consumed only in
     sourceAudioWaveform (:3062, :3089), declared-unused in sourceMandelbrot (:2823). The
     ripple / hueShift / chromaticAberration effects declare NO `u_rms` at all (:222, :246, :551).
     Net: some visuals advertised as audio-responsive are not consuming audio.
     Why this diagnosis is trustworthy: it predicts the pass/fail split exactly — `u_rms` IS
     genuinely used in all 6 sources, and the RMS test is the one that passes.
     **Boris call:** which sources/effects SHOULD be audio-reactive, and how strongly. That is
     a product/taste decision, not a technical one. Do not "fix" it by wiring every uniform in.
- **S3 field atomics** — after S2 survives real use.
- ID-based selection remap · full §3 APP-INVENTORY row pass (TWO dated delta blocks in §2:
  07-30 + 08-02) · empty-string AUDIODNA_API_BIND fallback (nit).
- **C1/C2 review minors, not blocking:** TestServer.cpp:970 handleRemoveMapping skips JSON
  validation (intentional drain semantics, bounds-checked, test-only) · TestServer.cpp:947/978
  callAsync lambdas capture this/renderer_ with no shutdown guard (pre-existing idiom).
- **Residuals shipped knowingly**: ROUTED params and autopilot still freeze on preview
  detach (C3 fixes MAPPED params only); P3 scalar-crossing set enlarged, deferred to spec Step 3.
- **Unexplained infra event** (2026-08-02b): a parallel rebuild pair was externally
  SIGTERM'd (`Terminated: 15`); serial re-run clean, cause never identified. Watch for it.
- **Working tree carries `graphify-out/` churn + untracked `.audit/features-gap-fill/`** —
  not from this lane, left untouched deliberately.
- **95 unpushed commits**, no push authorization. Review-before-push is the only thing
  keeping that stack honest.

## BORIS DECISION QUEUE (all pre-analyzed, deliver ONE per ask)
source/MilkDrop retrigger-restart scope · column-trigger retrigger parity · Cut/Copy/Paste
suite (menu enums RESERVED at MenuBarModel.h:76-80) · playlist mode-gate consistency ·
reset ~5s latency taste · eager-announce · runtime default/persistence · remote-control+token.

## ONLY BORIS CAN CHECK
- **Rig-feel taste check of mapping response after C3.** kMappingTickHz=120 was chosen
  specifically to PRESERVE the incumbent feel: the Smoother has no dt term, so the tick
  RATE *is* the time constant, and 60Hz would double the smoothing. A gate can prove the
  param tracks; it cannot tell him whether it feels the same under his hands. Confirm only.
- **The 7-item gesture replay list** above — needs his hands; no gate covers gestures.
- **Syphon live-check list** from 2026-08-02a.
- Whether ROUTED-param and autopilot freezing on preview detach (both recorded residuals)
  is acceptable to ship with, or should jump the queue.
- NOTE: the "SignalBar-expanded freeze demo" previously on this list is **CLOSED** — it no
  longer needs his hands.

## META-LEARNINGS
- **Prove an oracle fires POSITIVE in the known-good state before trusting any negative
  from it.** The fps oracle reports a healthy ~106fps for a provably dead context; trusting
  it would have sent this session chasing a phantom click failure, which is plausibly what
  happened to the last one.
- **A handoff written by a session that could not execute its own instructions is a
  hypothesis, not a recipe.** Three of six inherited facts here were wrong.
- **Carry lists VERBATIM; never write "see the previous handoff revision."** That single
  habit is why the gesture list survived 5 sessions unprocessed.
- **When a ratified design has a conditional fallback, the fallback deserves the same
  scrutiny as the main path.** A4's "if falsified, marshal" would have preserved a real bug
  while looking like faithful compliance with the council.
- **Red-team the ruling you like.** The A4 deletion looked obviously right after one seat;
  the adversarial seat confirmed it but ALSO surfaced the PresetManager retarget hazard,
  which no one had asked about.
- **Automate the gate step rather than spending the human**, when the step must run more
  than once. Boris's 5 seconds were available; the tooling was still worth it because
  states 3-4 must run again after C3 and on every regression.
- Idle-without-report from a subagent is an auto-nudge trigger (pattern held: 3/3 agents).
- Carried from 2026-08-02b: stage protocol (ratify to disk → builder stages → independent
  gate → commit) is the right shape for a multi-commit arc · dispatch the reviewer BEFORE
  wrapping, not at arc end · evidence has timing (`.ips` land late; `/api/health` answers
  from a dying instance — pair liveness claims with `pgrep`) · race-demonstration tests
  must mirror production access shape.

## CHANNEL HARVEST
- **Boris ideas captured this session: NONE of idea-class.** Boris gave one standing
  directive (model routing: fable for planning/architecture, opus for other work — he
  switches the main session, Harmony sets subagent models per dispatch) — applied
  immediately, both architecture seats ran on fable.
- **Repo-local knowledge written**: 3 new gotchas (fps-oracle-freezes, `--test-mode`
  requirement + `::1` bind + `/api/health` route, AppleScript-cannot-reach-JUCE-AX) →
  `.harmony/gotchas.md`; fail-first methodology + evidence → `.harmony/ow-freeze-before.log`;
  C3 work packet → `.harmony/.work-packets/c3-mapping-tick.md` (local-only).
- **No harmony2 SYSTEM files touched** — clean secondary, no escalation.

## STANDING RULES (unchanged)
Do NOT push (95 commits ahead of origin/main — verified 2026-08-03). Conform to
ClipCommands.h/DeckCommands.h/EffectCommands.h/TriggerCommands.h/UndoService patterns at
HEAD; structural-mutation commands carry a fence (notebook LAW); launch ONLY via `open`,
never direct binary exec (7070 binds ~12s in Release, ~2s warm); SIGKILL disposable
instances; scouts/builders need explicit deliver-to-main clauses; FORCED REBUILD before any
ctest claim (stale-binary false-green); `git commit --only` cannot stage NEW files.
**Gate mechanics:** build-tsan is configured TEST_SERVER=OFF, so TSan app runs are
PRODUCTION-mode (no 8080, mic-driven) — reconfigure the cache if injection is needed; a
TSan build that reported races ABORTS at exit (SIGABRT + `.ips`) — that is the sanitizer,
not a product crash; `.ips` files land with a DELAY so an immediate post-quit check proves
nothing; e2e client class is `VJAppController`; TCC mic prompt can re-fire after a rebuild
(ad-hoc signing changes the cdhash) — schedule app-level gates when Boris is present.
**`.harmony/` is gitignored but 44 files are force-tracked** — `git add` prints an
"ignored" WARNING and still stages tracked files; do not read that as failure (it breaks
`&&` chains, which cost this session two commit attempts). New knowledge files need a
one-time `git add -f`.

---

# ADDENDUM — C3 SHIPPED (same session, 2026-08-03a, after the budget extension)

**`c51aff7` feat(mapping): message-thread mapping tick @120Hz (OW arc C3: W5+A4+A6).**
The OutputWindow arc is COMPLETE: C1 `88af683` · C2 `fcad6d0` · C3 `c51aff7`.
Ahead of origin/main by **97**. Nothing pushed.

**Full-tier gate — BOTH halves closed:**
- Behavioral (Harmony, who did not build): forced-rebuild Release, 0 errors, no new
  warnings · **independent ctest 193/193** (her own run, not the builder's claim) ·
  **probe states 1-4 ALL PASS**. States 3-4 flipped from the fail-first capture WHILE the
  detach oracle still reported the preview context dead (HTTP 500 after 5.0s) — the states
  did not get easier. Builder additionally ran Debug/ASan with jasserts live: 1478
  assertions, none fired.
- Independent source review: **PASS, 0 blocking, 1 minor.** The reviewer specifically
  verified the timer-lifetime risk: member order is analysisThread_ → previewPanel_ →
  mappingTickTimer_, so the timer is DESTROYED FIRST, and juce::Timer::~Timer()
  unconditionally stops itself. No window to fire against half-destroyed members.

## CARRIED MINOR — do this first, it is one line
**`src/mapping/MappingEngine.h:21`** still reads *"Called on the render thread each frame."*
That is now FALSE — the sole caller is the message-thread MappingTickTimer
(MainComponent.cpp:2420). Deliberately NOT in `c51aff7`: the fix was dispatched but had not
landed when the session hit its context budget, and only gate-verified files were committed.
A builder was asked to rewrite it AND to scan the rest of that header for other
threading claims C3 made false. **Check whether an uncommitted edit to that header is
sitting in the working tree before redoing it.** This is the exact "stale comment survives
as a lie" pattern this arc exists to treat — it should not survive another session.

## GATE GAPS — NOT run, recorded honestly. Close these before calling the arc done.
1. **W7(iii) EMA parity** — attached-state step-response time constant vs the pre-change
   measurement. NOT MEASURED. Risk: kMappingTickHz=120 was chosen to match the measured
   ~119.8fps rate precisely because the Smoother has no dt term, so the tick rate IS the
   time constant; JUCE rounds 120Hz to 8ms (~125Hz). If that shift matters, every mapped
   param's smoothing feel moves slightly. **Boris's rig-feel check is the real arbiter.**
2. **TSan 2-context app drive** (design A2 bar: no NEW finding classes). NOT RUN —
   build-tsan needs a full rebuild and is configured TEST_SERVER=OFF (production mode).
   Risk LOW by construction: C3 strictly REMOVES cross-thread access (deletes a GL-thread
   processFrame call and a GL-thread clearAll, adds confinement asserts) so it should
   REDUCE findings. But "should" is not "measured."

## NEXT PRIORITIES (revised)
1. The carried minor above (one line).
2. Close the two gate gaps.
3. **Preset/binding silent retarget** — design is written in the ledger above; ONE commit,
   PresetManager.cpp only. Possibly already biting Boris's saved presets. Verify param-name
   uniqueness across all 135 effects first.
4. Boris rulings: audio-reactivity scope · the 7-item gesture list (now 5 sessions old).
5. A1 routing/signal follow-up — and note: if the ROUTED-param/autopilot freeze is what
   Boris actually notices on stage, A1 should jump this queue.

## SESSION NOTE
Two agents' findings this session were things nobody asked for and both matter more than
the task that surfaced them: the audio-reactivity product gap (shaders declaring uniforms
they never consume) and the preset/binding silent retarget. Red-teaming a ruling you
already like keeps paying — the A4 redteam confirmed the ruling AND found the preset bug.

### CARRIED MINOR — CLOSED (same session, commit below)
The MappingEngine.h:21 stale threading comment was fixed and committed after all. The
builder's report landed moments after the addendum was written. Comment-only, one file, no
rebuild needed (nothing non-comment changed, so the gate above stands unmodified). It also
scanned the rest of that header and correctly LEFT two other "frame" mentions alone
(MappingEngine.h:8 and :43) — those describe what one call does, not which thread calls it,
so C3 did not make them false. **Nothing carried. The arc is fully closed.**
