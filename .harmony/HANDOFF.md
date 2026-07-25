# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). **Undo v1 is BUILD-COMPLETE (steps 1-9)** as of
2026-07-25: this session shipped step 8 triggers (4ee2dac) + step 9 tests/cleanup
(0a1c882), tests 170/170 (`ctest --test-dir build`). Build:
`cmake --build build --config Release -j`. Lane ledger: `.harmony/undo-v1-ledger.md`
— READ FIRST. Launch ONLY via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`.

ENV — SOLVED, one ritual remains: the old "coreaudiod wedge" was an unanswered TCC
MICROPHONE PROMPT (gotchas.md 2026-07-25 — CLI probes can't see dialogs; diagnose
stalls with `screencapture` + image read). The app is AD-HOC signed, so the prompt
RE-FIRES after every rebuild: each first-launch-after-rebuild needs ONE Boris
Allow-click. Never killall coreaudiod / reboot for this.

NO autonomous START-HERE build task remains — everything left is Boris-gated:
(1) **Manual e2e sitting (Boris present, ~15 min):** run
`.harmony/undo-v1-manual-e2e.md` top to bottom (precondition #0 = launch + Allow
click). This clears the live-verification debt for steps 5-9.
(2) **Stable code-signing identity — RATIFIED YES (Boris 2026-07-25), not yet
implemented:** wire a stable codesign identity into CMake for dev builds (small
Boris step: pick/create the identity in Keychain) — kills the per-rebuild TCC
re-prompt class. This is the #1 buildable follow-up, but needs Boris's identity
choice first — do NOT start without him.
(3) Boris decision queue (ledger): refreshAfterUndoRedo pointer/scope-aware skip
ratification (fixes expanded-row collapse + deck-tab highlight); zero-layer
Deck-New intent; inspector-clear-on-undo UX (fold into the sitting).
Optional, low-priority: B8 full ReinspectTarget-path removal (out-of-lane ripple —
inspector_ member + setCollaborators + MainComponent call site).
Standing rules: do NOT push to remote; lane rules live in the ledger; conform to
ClipCommands.h / DeckCommands.h / EffectCommands.h / TriggerCommands.h /
UndoService patterns at HEAD.

## PRIMER

- HEAD at close: EOS chore commit atop 0a1c882 (source HEAD), unpushed local
  history. No-push standing rule intact.
- Counts: 135 effects · 108 sources · 22/22 REST · 11/11 OSC · 170/170 tests
  (158 carry + 8 [trigger] + 4 step-9).
- Undo v1: steps 1-9 DONE (each: independent Reviewer on source + Harmony
  behavioral gate + separate local commit). Trigger doctrine: user-initiated
  local/remote (REST/OSC/MIDI) = undoable through the two shared handlers;
  autonomous (autopilot, GL thread) NEVER creates commands. Same-layer trigger
  runs merge to one history slot; retrigger of active cell pushes nothing.
- Verifier model: Builder → independent Reviewer (source) → Harmony behavioral
  gate (receiver disk-verify, build, own ctest, residue grep) → local commit.
- Live-verification DEBT: steps 5-9 live behavior proven by review trace +
  headless suite only — the manual e2e sitting is the compensating control.
- Accepted v1 limitations (documented, do NOT re-report): playing not restored
  (risk #5); first-ever-trigger → undo → retrigger skips auto-play; merged runs
  retarget the playing slot to the latest column; hasBeenTriggered never rolled
  back; expanded-FX-row collapse + deck-tab highlight lag (pre-existing refresh
  path, follow-up pending ratification).

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Undo v1 — real undo/redo for all structural edits in Audio-DNA (Wave-2 MUST #1).
SHIPPED: steps 8-9 this session — clip/column triggers with same-layer merge (remote-user gestures undoable, autopilot never) + full spec-§7 test remainder and cleanup folds. 2 local commits (4ee2dac, 0a1c882), tests 158→170, both steps independently reviewed + gated. PLUS: the 2-session env mystery SOLVED (TCC mic prompt, not coreaudiod) and stable-signing ratified.
IN-FLIGHT: none — lane build-complete, agents retired, tree committed.
NEXT: Boris sitting — manual e2e checklist (.harmony/undo-v1-manual-e2e.md, one Allow click first); stable-signing CMake wiring (needs Boris identity choice); 3 open Boris decisions in the ledger.
BLOCKERS: none autonomous — all remaining work needs Boris present.
YOU ARE HERE: 9 of 9 build steps done; undo covers every ratified structural edit; only live-app verification + follow-up decisions remain.

## LOOSE-ENDS LEDGER

Adversarial "what's unfinished / what am I unsure about":
- Live-app verification for steps 5-9 still NOT RUN (now UNBLOCKED — TCC solved —
  but the sitting hasn't happened): GL fence under live render, renderer re-point,
  refresh split, trigger/merge feel, all checklist items. Headless 170/170 is
  proven; live behavior remains INFERRED from review traces.
- Stable signing RATIFIED but NOT implemented — until it lands, EVERY rebuild's
  first launch stalls silently unless someone clicks Allow (agents: screencapture
  to see it; schedule app gates for Boris-present windows).
- refreshAfterUndoRedo pointer/scope-aware skip still awaiting Boris ratification
  (whole collapse-on-undo cosmetic class hangs on it).
- Zero-layer Deck-New intent + inspector-clear-on-undo UX still open (Boris queue).
- B8 optional: dead ReinspectTarget path removal (loud hazard comment in place;
  full removal ripples out-of-lane — needs its own small packet if wanted).
- Merge playing-retarget + hasBeenTriggered non-rollback: accepted risk-#5 family,
  documented in TriggerCommands.h + checklist — could surprise a future tester who
  skips the accepted-items section.
- Syphon.framework install + rebuild -DAUDIODNA_BUILD_SYPHON=ON still pending.
- Tree noise left deliberately: graphify-out/ churn (post-COMMIT hook regenerates
  it — permanently dirty by design, never stage into lane commits);
  .audit/features-gap-fill/ untracked (pre-existing, unrelated).
- touched-repos.sh returned 7 dirty candidates at close; work repo resolved by
  today-commit SHA match (precedented, disk-verified) — registry dirt from other
  lanes persists.

## META-LEARNINGS

- screencapture + image-read diagnoses invisible TCC/system dialogs — CLI probes
  (sample/lsof/log) are structurally blind to them; a 2-session env saga fell in
  one screenshot. Universal candidate (also log-event'd mid-session).
- codesign -dv is the first check when a TCC prompt "randomly" recurs — ad-hoc
  signing re-prompts per rebuild (cdhash churn); stable identity is the cure.
- Reviewer-prescribed doc-only fixes can skip the re-review pass (pre-adjudicated;
  gate re-run covers) — used twice at near-zero cost; ONLY for reviewer-verbatim
  comment edits, never code.
- "Already-covered, not duplicating" builder claims get CONTENT-verified (read the
  covering test bodies), not existence-verified (grep the names).
- Per-op spec rows beat global doctrine assumptions: triggers make remote gestures
  undoable (spec row 8) while deck-switch excludes remote call sites (step 6) —
  both correct; check the row, don't extrapolate a rule.
- Boris's "go with recs" ratifies exactly the RECOMMENDED items — the explicitly
  recommended ones (signing YES, sitting agreed), NOT every open queue item;
  unrecommended decisions stay open rather than inheriting approval.

## CHANNEL HARVEST

- Boris turns this session: ONE ("go with recs then eos") + the physical Allow
  click. Ratifications captured in the ledger the same turn (stable signing YES;
  sitting agreed/deferred; env item closed). R2 transcript sweep: zero
  idea-class statements missed → no idea-ledger records owed (no ledger file
  exists; nothing to migrate — gate's ### -prose WARN cannot fire).
- harmony2 writes: one log-event learning row (.events/ transient telemetry,
  kernel D1 protocol — NOT a system/memory write). Zero system files touched →
  escalation predicate CLEAN → eos-secondary is the correct close.
- Carry-forwards all routed repo-local (lane B): undo-v1-ledger.md (ratifications
  + open Boris queue ×3), gotchas.md (TCC root cause + 2 supersession riders),
  undo-v1-manual-e2e.md (the sitting script), APP-INVENTORY.md (3 rows synced to
  steps 1-9), notebook.md (5 meta-learnings), sessions/ log, this HANDOFF.
