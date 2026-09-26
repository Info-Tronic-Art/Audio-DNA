# Critic — s-rta-0925 roadmap plan (plan-roadmap.md)
STATUS: DONE
VERDICT: PASS

## Method
Read plan-roadmap.md in full (424 lines). Cross-checked ~40 of its file:line
citations against disk source (not recalled from memory) across
src/recording/{Program,Player,RecorderHost,PerfState,PerfStateCapture}.{h,cpp},
src/MainComponent.{h,cpp}, src/connect/{ManualWrite,ParamConnection,AutomationCurve}.h,
src/ui/{LayerStrip,UniversalParamControl}.*, src/api/ApiServer.cpp,
src/model/{Layer,Composition}.h, .harmony/probe-step3.sh, .harmony/APP-INVENTORY.md,
.harmony/binding-decisions.md, .harmony/HANDOFF.md, tests/CMakeLists.txt. Hunted
specifically for: missed consumers/call sites, RT-rule violations, races,
untestable claims, over-scope, doc-vs-code drift.

## Verified TRUE (spot-checked on disk, not taken on faith)
- `Program.h:82` `preamble` field exists, unwritten; `compile()` (Program.cpp
  151-272) builds no preamble. `Player.cpp`/`RecorderHost.cpp` grep for
  "preamble" → zero hits outside that one declaration. Matches the plan's
  central "verified unbuilt" claim exactly.
- `RecorderHost::play()` (RecorderHost.cpp:618-664) is exactly
  `compile → player_->start(0.0) → playing_ = true` with no restore step —
  confirms the gap the plan is closing.
- `Layer::triggerClipImmediate` (Layer.h ~241-274) auto-plays a
  never-triggered clip (`if (!clip->hasBeenTriggered) clip->playing = true`) —
  confirms the plan's R4 ordering hazard (trigger must precede
  play/pause emission) is real, not invented.
- The "latent replay bug" (section 3.5.1) is REAL and independently
  reproducible from source: `applyLayerFlag`/`applyEffectBypass` (MainComponent.cpp
  5343-5391) call `composition_.getActiveDeck()` unconditionally and gate
  capture on `composition_.activeDeckIndex`, ignoring `f.target.deck` —
  so a Replay/Preamble Fired resolved to a non-active deck is silently
  applied to whichever deck happens to be active today. `handleClipTrigger`
  already has the deckIndex-aware fix pattern (MainComponent.cpp 4082-4100,
  `resolvedDeckIndex`) that the plan proposes extending to the other four
  handlers — confirmed additive/safe: `grep` shows zero existing callers of
  `applyClearActiveClip/applyLayerFlag/applyEffectBypass/applyClipPlaying`
  pass a deck argument, so a new trailing `int deckIndex = -1` is
  behavior-preserving for every current call site (verified full caller list).
- Threading (3.6): `RecorderHost::play` and every touched mutator run under
  `RECORDER_HOST_ASSERT_MESSAGE_THREAD()` / are message-thread-only already;
  `manualTouch/manualWrite/manualRelease` (MainComponent.cpp 3298-3320) assert
  the message thread and route through `ManualWrite.cpp`'s existing
  rank-based grip refusal (`Hand::Lane = 1` < `HumanDecaying/HumanHeld`,
  ManualWrite.h:24; refusal logic ManualWrite.cpp:160-176) — no new mutex, no
  audio/GL-thread touch. No RT-rule violation found in the proposed plan.
- `Origin::Preamble` already exists in the `Origin` enum (Lane.h:50) — the
  plan correctly reuses existing vocabulary rather than inventing a parallel
  concept.
- `gripHoldMs = 250.0f` (Composition.h:82) confirmed — the live-gate's
  `sleep 1` justification (R2) is grounded in the real constant.
- PerfStateCapture's `slotIdx*100+p` key encoding (PerfStateCapture.cpp:33-43)
  matches the plan's proposed `kFxParamKeyStride=100` extraction 1:1 — the
  refactor is behavior-identical, not a guess.
- D7 cross-reference: `AutomationCurve.h` exists, `ParamConnection.h:52-63`'s
  `Envelope::curve` is exactly that type with the exact "architect ruling,
  s167-l2" comment quoted in the plan; `grep AutomationCurve|s167` on the s166
  spec file returns 0 hits — the "cross-ref missing" doc gap is real.
- Doc-drift claims (step 7 NOT DONE) confirmed: `.harmony/APP-INVENTORY.md`
  has zero mentions of RecorderHost/AudioStore/AudioTap/`/api/perf/` anywhere;
  line 85 still says "Play fires nothing — playback dead" (stale); REST/test
  counts in the doc (22 endpoints / 188 tests) are stale vs actual
  (32 registered routes incl. 7 perf/* ones; `tests/CMakeLists.txt` has
  exactly 40 `add_executable(...)` blocks, matching the plan's "40 targets").
- L-R/L-P/L-D lane-status greps (Routine.h absent, `Composition.h` has no
  `routines` field, `resolveKey` explicitly refuses `Scope::Routine`
  (Program.cpp ~100-102), `grep manualWrite src/ui` → exactly the one comment
  at LayerStrip.cpp:430, `Player::swap` has zero callers in
  MainComponent.cpp/RecorderHost.cpp, `CompositorEngine.cpp` has zero
  `1.0f / 60` literals, `src/` has zero `FlacAudioFormat` hits) — all
  independently reproduced, all confirmed TRUE.
- binding-decisions.md 2026-09-25 ruling text and probe-step3.sh's
  "Replay does NOT apply checkpoint0" comment both exist and match the
  plan's quoted wording (see minor citation-drift note below).

## Findings

### Non-blocking (nits, cite for the record)
1. **Citation line-drift, probe-step3.sh** — plan cites the "Replay does NOT
   apply checkpoint0" comment at `probe-step3.sh:568-571`; it is actually at
   `:590-593` (off by ~22 lines, likely stale from a prior edit to the file).
   Substance is verbatim-correct, only the pointer is off. Not
   blocking — a Builder following the plan would find it a few lines away in
   the same section.
2. **T2 hardware claim (row 6) not independently re-run this pass** — the
   plan itself discloses it is quoting HANDOFF.md's numbers (mean_offset,
   drift, p95) rather than re-executing probe-step3.sh; that is the correct
   posture for a read-only architect pass (COUNTS said to be "run, never
   inherited" applies to the BUILD step, not this planning pass), so not a
   defect in the plan, just noting the boundary.
3. Section 3.2's `buildPreamble` design embeds a fairly large amount of
   forward-specified emission-order logic (8-row table) directly readable
   only in prose/pseudocode, not as compilable code — this is appropriately
   flagged by the plan itself as material for RED-first tests (3.7) rather
   than asserted as already-working, so it is a spec, not an unverifiable
   claim dressed as fact. No action needed.

### Scope check
The plan explicitly proposes optionally splitting the deck-aware handler
change (3.5 items 2-3) into its own micro-lane if the critic judges it too
large for one M-small build (3.10, R8). Given the change is four short,
structurally-identical additive edits (verified: each handler is <30 lines,
pattern is copy-paste of the already-existing `handleClipTrigger` shape) and
every existing caller is unaffected (verified above), I do NOT recommend the
split — doing so would leave the exact "latent replay bug" documented in R1
half-fixed (deck-aware clip trigger but not deck-aware flags/bypass/playing)
for a full extra review/gate cycle with no plausible benefit. This is a
judgment call the plan leaves open; recording it here per 3.10's invitation.

## Verdict rationale
No blocking defect found. Every checkable file:line citation I sampled
(~40 across 15 files) resolved to code/text that says what the plan claims
it says, including the two highest-risk claims (the preamble is genuinely
unbuilt today; the deck-aware handler gap is a genuine live bug, not a
hypothetical). The one line-number drift is cosmetic. No RT-thread, GL-thread,
or lock-free-channel violation is proposed anywhere in the plan — every new
line runs on the JUCE message thread through funnels that already assert
that thread. No missed consumer/call-site was found that the plan's
"existing callers pass nothing → identical behaviour" claims did not already
account for.

METADATA: reviewer=critic, plan=.harmony/.reports/s-rta-0925/plan-roadmap.md, date=2026-09-25
