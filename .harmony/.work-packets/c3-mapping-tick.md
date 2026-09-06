# Work packet — OutputWindow arc C3 (W5 + A4 + A6)

STATUS: DRAFT — A4 clause finalises when a4-redteam returns. Do not dispatch until then.
Design: `.harmony/specs/outputwindow-arc-design.md` — RATIFIED by a 3-seat blind council,
chair-adjudicated. DO NOT re-litigate the design. Deviations below are chair amendments
with recorded reasons; treat them as binding.

## Stage protocol (reuse of the C1/C2 protocol that worked)
Builder reports -> Harmony gates -> Harmony commits -> next stage. The builder NEVER
commits and NEVER claims self-verification. Harmony runs the behavioral gate because she
did not build; an independent Reviewer reads the source.

## CORRECTED LINE NUMBERS — every inherited figure had drifted. Verified 2026-08-03.
| Thing | Inherited (WRONG) | Verified at HEAD 82a74c1 |
|---|---|---|
| GL-thread processFrame call | ~232 | **Renderer.cpp:242** |
| GL-thread clearAll | ~1520 | **Renderer.cpp:1538** (last line of initEffectChain, 1495-1539) |
| First-attach guard | ~1489-1490 | **Renderer.cpp:1507-1508** `if (effectChain_.getNumEffects() > 0) return;` |
| initEffectChain entry | — | called from `newOpenGLContextCreated()` at Renderer.cpp:120 (GL thread) |
| OutputWindow stale comment cite | "Renderer.cpp:239" | drifted; rewrite the whole block |
Re-verify before editing. These drift every arc.

## W5 — move the mapping tick to the message thread
- Add ONE named constant `kMappingTickHz = 120`. Rationale (design A5): matches the
  MEASURED attached render rate (119.8fps on the rig; re-measured this session at
  108-118fps steady). The Smoother has NO dt term, so the tick RATE *is* the time
  constant — 60Hz would double the smoothing feel. Alpha untouched. JUCE's ms rounding
  (-> 8ms ~= 125Hz) is accepted; the EMA parity gate asserts empirically.
- Timer member on MainComponent (or a small owned helper), `startTimerHz(kMappingTickHz)`.
- Body is a NAMED SEAM: `tickFeaturePipeline() { auto snap = featureBus_.read(); mappingEngine_.processFrame(snap, effectChain_); }`
  The seam exists so the A1 routing/signal follow-up can join later without re-plumbing.
- DELETE the `mappingEngine_.processFrame` call at Renderer.cpp:242.
- Timer runs UNCONDITIONALLY — no attach gating, no visibility gating. That is the whole
  point: the tick must survive preview detach.
- Do NOT move `routingEngine_.processFrame` (Renderer.cpp:230) or autopilot
  (Renderer.cpp:~237). Out of scope by design A1; recorded residual.

## A4 — the GL-thread clearAll at Renderer.cpp:1538
CHAIR AMENDMENT (2026-08-03). The design says "DELETE after verifying minimal's no-op
proof; if the proof does NOT hold, callAsync marshal instead." An independent architecture
pass FALSIFIED the no-op proof and ALSO rejected the prescribed fallback:
- The proof fails because mappings CAN exist before first GL attach. TestServer is
  constructed and listening inside the MainComponent ctor (MainComponent.cpp:1581-1590,
  test mode only) — before the window is shown and before first attach — so add_mapping
  and preset-load can land first. Our own W7(ii) probe is in exactly this class.
- The guard at 1507-1508 does NOT prevent first-attach execution; it only makes the body
  run once per process. Re-attaches early-return, so clearAll never ran on re-attach.
- callAsync is WORSE than deletion: it preserves the same wipe and ADDS a window where a
  mapping installed between GL attach and the callback is destroyed.
=> REMEDY: DELETE the call. This also retires a live GL-vs-message data race on the
   vectors, and closes the "latent pre-attach preset-mapping wipe" the design's own A4
   line already said deletion would close.
A4 REDTEAM RESULT — **ATTACK FAILS, deletion CONFIRMED.** An adversarial seat, briefed
specifically to break the deletion ruling on the dangerous failure mode, could not:
- Targets ARE index-based (`targetEffectId` = index into EffectChain, MappingTypes.h:127-128),
  so the feared "silently drives the WRONG effect" mode is structurally possible — but
  unreachable here.
- initEffectChain only ever populates an EMPTY chain (the 1507-1508 guard), and EffectChain
  is APPEND-ONLY ("effects_ only ever grows (no removal API)", EffectChain.h:77-82/136-139).
  Nothing can shift beneath an existing index. Population order is deterministic per build
  (Renderer.cpp:1514-1517 over EffectLibrary.cpp:836-845).
- A dangling index is a SAFE PER-TICK NO-OP, re-validated every frame: getEffect returns
  nullptr out-of-range under mutex and processFrame `continue`s (MappingEngine.cpp:161-165,
  EffectChain.cpp:29-35). Never a crash, never a wild write.
- TestServer resolves targets BY NAME against the live chain and errors out on an empty
  chain (TestServer.cpp:917-942), so it CANNOT create a mis-targeted mapping pre-attach.
  UI paths take indices from the live chain and guard non-null (MainComponent.cpp:2972-2990).
- The ONE real retarget hazard — PresetManager restoring RAW saved indices
  (PresetManager.cpp:212, saved at :113) from a stale cross-build preset — is exposed
  IDENTICALLY with or without the clearAll (a post-attach load of the same preset hits it
  too). The hazard lives in PresetManager, not at Renderer.cpp:1538. Recorded as a separate
  follow-up; NOT in C3 scope.
- BONUS, strengthens deletion: today's clearAll is a GL-thread write to state the codebase's
  own comments declare message-thread-owned (TestServer.cpp:950-952). It can destroy an
  already-ACKed install, and the unsynchronized clear races a concurrent push_back.
  Deletion removes a wipe, a race window, and an ownership violation in one line.
=> A4 IS FINAL: DELETE. No further adjudication needed.

ADDITIONAL REQUIREMENT from the redteam: the comment at **Renderer.cpp:1537** ("No demo
effects or mappings") must be updated or removed with the deletion, or it becomes
misleading — same class as the OutputWindow stale-comment problem below.

## A6 — confinement jasserts
`jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());` in
`processFrame`, `addMapping`, `removeMapping`, `clearAll`.
**HARD ORDERING CONSTRAINT:** A6 must land WITH or AFTER W5+A4 — never before. The only
two offenders at HEAD are Renderer.cpp:242 and Renderer.cpp:1538, both deleted by W5/A4;
an A6-first ordering fires the assert on startup.
Unit tests are safe: `tests/test_mapping_engine.cpp` has no JUCE init, but the first
`MessageManager::getInstance()` constructs the singleton with `messageThreadId` = the
calling thread, so the single-threaded test self-identifies as the message thread.

## Comment work (do not skip — this is where retired limitations survive as lies)
1. `src/ui/OutputWindow.cpp` KNOWN-RESIDUAL block: it still documents the preview-detach
   freeze as a LIVE limitation and cites a drifted line. REWRITE it to describe the
   post-C3 reality (message-thread tick; mapping survives detach) and to name the
   remaining residuals honestly: routed params and autopilot still freeze on preview
   detach until the A1 follow-up.
2. `MappingEngine.cpp:169-196`: add ONE explanatory comment line for review minor 1 — a
   concurrent enabled-flip mid-processFrame can double-tick one smoother; unreachable
   once message-thread confinement lands.

## Out of scope — do NOT touch
Routing/signal extraction (A1 follow-up, gated on a SignalRegistry thread audit);
autopilot; `EffectParam::value` staying a plain float (design A2, 2:1 ruling);
TestServer.cpp:970 JSON validation and TestServer.cpp:947/978 callAsync shutdown guards
(recorded minors, explicitly NOT requested).

## Gate (Harmony runs it, not the builder)
ctest full suite (baseline 193/193) · probe states 1-4 ALL PASS post-W5 (the "after" half
of the fail-first pair) · EMA parity per W7(iii) · TSan app drive with 2 GL contexts, bar
= no NEW finding classes beyond the documented deferred list (design A2).

## Test-rig mechanics the builder will need
Launch: `open --stdout /tmp/o.log --stderr /tmp/e.log build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode`
(without `--test-mode`, 8080 never binds while 7070 still does = false green).
Probe `http://[::1]:8080` — NEVER 127.0.0.1 (returns empty, looks dead). Health route is
`/api/health`, not `/api/status`.
Detach oracle is `POST /api/render_frame` (200 fast = attached; ~5s then 500 = detached).
The `fps` field is NOT a valid oracle — it freezes at its last value on detach.
