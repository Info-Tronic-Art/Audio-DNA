# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). The 2026-07-22 session shipped Undo v1 steps 4-7
(4 local commits: 6d2def4 composites + column-growth gap closed, 7f87094 layer ops
GL-fenced, 316a2bf deck ops + latent renderer re-point fix, d90e953 effect stacks
3-scope performEdit). Tests: 158/158 (`ctest --test-dir build`). Build:
`cmake --build build --config Release -j`. Lane ledger (step status, decision log,
Boris decision queue, manual e2e checklist): `.harmony/undo-v1-ledger.md` — READ
FIRST. Behavioral gates: launch ONLY via
`open build/AudioDNA_artefacts/Release/Audio-DNA.app` (NEVER direct exec).

ENV BLOCKER FIRST: coreaudiod is WEDGED (gotchas.md 2026-07-22) — every app launch
stalls in CoreAudioInternal::start; repeated pkill -9 remedy cycles WORSEN it. Boris
must clear it (`sudo killall coreaudiod` or reboot) before ANY app-level check runs.
If still wedged at your boot: single-attempt recheck only, no retry loops.

START HERE — long task, begin at session start: Undo v1 step 8 per
`.harmony/specs/undo-v1-spec.md` §6 (TriggerClipCmd/TriggerColumnCmd + canMergeWith
merge rules; REST flows through the same handlers automatically). It is the #1
Committed MUST. Step 9 (tests + manual e2e) follows. Ledger carries binding lane
rules: two-pattern rule (non-idempotent structural = command-owns-the-mutation;
idempotent field write = mutate-then-push); guarded-jassert defense on
caller-discipline invariants; no test constructs in violation of an asserted ctor
invariant; per-command stale-coordinate tests; NEVER create commands on the GL path
(autopilot triggers from GL thread — spec §1 consequence 3 is LOAD-BEARING for
step 8). Established patterns at HEAD: ClipCommands.h / DeckCommands.h /
EffectCommands.h + EffectScope.h, UndoService, pushCommands/refreshAfterUndoRedo —
conform, don't fork. Do NOT push to remote (standing rule). Quick Boris asks
pending: see BORIS DECISION QUEUE in the ledger (refresh-skip follow-up ratification,
zero-layer Deck-New intent, coreaudiod env clear, inspector-clear UX) + Syphon
install/rebuild + the 10s UI eyeball incl. all manual e2e additions (steps 4-7 items
now queued behind the env blocker).

## PRIMER

- HEAD at close: EOS chore commit(s) atop d90e953 (source HEAD), on unpushed local
  history (prior triage + Undo v1 steps 1-7). No-push standing rule intact.
- Counts: 135 effects · 108 sources · 22/22 REST · 11/11 OSC · 158/158 tests
  (130 carry + 8 [column/clear] + 7 [layer] + 8 [deck] + 5 [effect]).
- Undo v1 state: steps 1-7 DONE (each: independent Reviewer on source + Harmony
  behavioral gate + separate local commit). Steps 8-9 queued; deferred at ~38% gauge
  per LONG-TASK DEFERRAL. Step-8 hazard: autopilot triggers from the GL thread —
  commands wrap ONLY user entry points (handleClipTrigger/handleColumnTrigger),
  NEVER Layer::triggerClip.
- Verifier model: Builder → independent Reviewer (source) → Harmony behavioral gate
  (receiver disk-verify, build, own ctest, residue grep, app health) → local commit.
- App-level verification DEBT: steps 5-7 live behavior (GL fence wiring, renderer
  re-point, UI refresh split) proven only by review trace, NOT live — all manual
  e2e items queued behind the coreaudiod env fix.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Undo v1 — real undo/redo for all structural edits in Audio-DNA (Wave-2 MUST #1).
SHIPPED: steps 4-7 this session — composites + column ops (growth gap closed); layer ops GL-fenced; deck ops (+ latent renderer re-point bug FIXED); effect stacks across clip/layer/global scopes. 4 local commits, tests 130→158, every step independently reviewed + gated.
IN-FLIGHT: none — lane wrapped clean at the drain off-ramp.
NEXT: step 8 trigger commands + merge (START HERE next session); step 9 tests/manual-e2e + cleanup fold-ins; Boris decision queue ×4 in the ledger.
BLOCKERS: coreaudiod WEDGED — all app-level/manual checks queued behind `sudo killall coreaudiod` or reboot (Boris-level; gotcha captured).
YOU ARE HERE: 7 of 9 build steps done; undo covers every structural edit except triggers; live-app verification debt queued behind one env fix.

## LOOSE-ENDS LEDGER

Adversarial "what's unfinished / what am I unsure about":
- ALL app-level verification for steps 5-7 is UNPROVEN LIVE (coreaudiod wedge): GL
  fence wiring in-app (layer/deck ops under live render), step-7 UI refresh split,
  renderer re-point after deck add/remove, every manual e2e item. Headless model
  behavior is proven (158/158); live behavior is INFERRED from review traces only.
- Step-7 refresh design accepts: expanded FX rows collapse on ANY undo (pre-existing
  unconditional refreshAfterUndoRedo). Fix = tracked follow-up (pointer/scope-aware
  skip) — NEEDS BORIS RATIFICATION; also fixes deck-tab-highlight staleness class.
- Zero-layer raw Deck-New + id=0 deck collisions: pre-existing HEAD weirdness,
  faithfully wrapped — is it intended? (Boris queue.)
- Undo with no cell selected clears clip inspector (carried from s. 2026-07-19).
- Step-9 fold-ins owed: RemoveLayerCmd own stale-coord test; SyncScope::DeckStructure
  dead code; EffectCommands.h:98 stale inline comment; dead 2-arg reinspect path
  (scope-None trap if revived); inert no-op-command history entries (unreachable
  today).
- Syphon.framework install + rebuild -DAUDIODNA_BUILD_SYPHON=ON still pending
  (needs build dir + working app).
- Uncommitted tree noise left deliberately: graphify-out/ churn (regenerates),
  .audit/features-gap-fill/ untracked (pre-existing, unrelated).
- touched-repos.sh returned 7 dirty candidates at close; work repo resolved by
  today-commit SHA match (precedented) — registry dirt from other lanes persists.

## META-LEARNINGS

- Pre-declared budget riders (disposition rule written at DISPATCH, not at finding
  time) removed all rationalization pressure when the s7 MAJOR landed. Keep doing.
- Remedy-(b) doc-truth is the right disposition for "comments overclaim vs
  pre-existing mechanism gap" MAJORs; invite the reviewer to overrule the
  disposition explicitly — undo-s7-review's independent UNBLOCK (citing s6
  precedent) is worth more than a self-approved one.
- Rotate reviewers at ~2 full packets like builders; fresh eyes killed a false
  "headless-silent test violation" precedent claim in minutes.
- Env wedges that worsen with remedy cycles get single-attempt rechecks + hard
  stop; retry loops actively degraded coreaudiod (4-attempt, sample-verified).
- Write-immediately ledger discipline made this EOS nearly free — every decision,
  gate result, and carry-forward was already on disk at close time.

## CHANNEL HARVEST

- Boris turns this session: ZERO (autonomous drain from boot handoff) → no
  idea-class statements owed; R2 sweep of transcript confirms none missed; no
  idea-ledger records written (none exist to migrate — no ledger file present).
- harmony2 writes: ZERO (clean lane-B secondary; no system files, no memory, no
  .pending). Escalation predicate CLEAN → eos-secondary is the correct close.
- Carry-forwards: all routed repo-local — undo-v1-ledger.md (decision log, Boris
  queue, e2e checklist), gotchas.md (coreaudiod wedge entry), notebook.md (4
  meta-learnings), APP-INVENTORY.md (undo rows → steps 1-7, same-wave rule),
  this HANDOFF (birth prompt + loose ends).
