# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). **Undo v1 is BUILD-COMPLETE and now CRASH-HARDENED:**
this session shipped the **GL-fence family fix (8bd09ba)** — a reproduced Column→New
SIGSEGV (pre-existing UAF: message-thread model mutation under the lock-free GL
render thread) was diagnosed to disassembly level and fenced across the ENTIRE swept
family (columns/clears/clip-clear/drop-growth/effects vectors/replace-content/
kCompNew/undo-redo replay + 2 never-fenced drop paths). Tests 176/176
(`ctest --test-dir build`). Build: `cmake --build build --config Release -j`.
Lane ledger: `.harmony/undo-v1-ledger.md` — READ FIRST (2026-07-28/29 entries).
Launch ONLY via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`.

**START HERE — the parked app gate (needs Boris, 1 click):** the fenced build was
left RUNNING with its TCC mic prompt UNANSWERED on screen. (1) Boris clicks
**Allow** (if the app was quit: relaunch → prompt re-fires → Allow). (2) Verify
`curl -s http://127.0.0.1:7070/api/health` → ok/ready. That closes the fence lane's
app gate. THEN two Boris steps in order: (a) **create the codesign cert** —
Keychain Access → Certificate Assistant → Create a Certificate → name exactly
`Audio-DNA Dev`, Self-Signed Root, Code Signing (keychain had ZERO identities all
session); then **cmake reconfigure** (identity resolves at CONFIGURE time — a bare
rebuild will NOT pick it up) + rebuild + one final Allow = last TCC prompt ever.
(b) **Resume the manual e2e sitting** — `.harmony/undo-v1-manual-e2e.md`: the HOLD
list (columns/clears/clip-clear/effects) DISSOLVES once the app gate passes; ALSO
re-run the 4 race-lucky drop passes (multi-video edge, multi-FX far, drag-move far,
multi-SOURCE). ~10 of ~40 items are done.

ENV ritual unchanged until the cert lands: ad-hoc signing → TCC prompt re-fires per
rebuild; ONE Allow click each first-launch. CLI probes can't see the dialog —
diagnose stalls with `screencapture` + image read (gotchas.md). Never
killall coreaudiod / reboot.

Boris decision queue (ledger, none started): MilkDropBrowser crash-#2 fix bundled
with default-preset-dir (his ask; .ips 2026-07-28-190701) · mixed-drop
image-discard fix (ClipCell external path) · Cmd+X cut-to-clear (idea-ledger) ·
wire Comp/Decks browser load/save (UNWIRED no-ops at HEAD — FUTURE-FENCE comment
mandatory when wiring) or defer to recorder wave · refreshAfterUndoRedo skip ·
zero-layer Deck-New intent · inspector-clear UX · Syphon install · B8 removal.
Next-lane fork after the sitting closes: Session Recorder (ratified default) vs
ISF import vs visual-design resume (v10 row study partB, V20 recommended, no
ratified winner, dormant since 05-22).
Standing rules: do NOT push to remote; lane rules in the ledger; conform to
ClipCommands.h / DeckCommands.h / EffectCommands.h / TriggerCommands.h /
UndoService patterns at HEAD (NOTE: DeckFenceHook now lives in ClipCommands.h;
every structural-mutation command carries a fence — new commands MUST too, see
notebook LAW).

## PRIMER

- HEAD at close: EOS chore atop **8bd09ba** (fence lane, 16 files +777/−246),
  atop 649a809/0a1c882. Local only, no-push rule intact. graphify-out/ churn =
  permanent post-commit-hook noise, never stage.
- Counts: 135 effects · 108 sources · 22/22 REST · **176/176 tests**
  (170 undo-v1 carry + 6 fence-invocation).
- THE LAW (notebook.md 2026-07-28): renderOpenGL runs with NO lock; ANY
  message-thread mutation of layer clips vectors, composition_.decks, or
  clip/layer effects vectors MUST run under UndoService::withDeckDetached
  (RAII-safe, fenceActive_ reentrancy jassert; ONE fence per user gesture,
  sequential never nested). Commands carry DeckFenceHook; headless tests pass
  noopFence(). SetClipCmd/SwapClipsCmd exemption is DELETED — they are fenced.
- Verifier model unchanged: Builder → independent Reviewer (source) → Harmony
  behavioral gate (receiver disk-verify, build, own ctest) → local commit.
  This lane's review arc caught 2 MAJORs + ruled 2 scope expansions across 3
  rounds — treat multi-round adversarial review as the norm for thread-safety
  work.
- Fence perf envelope: ~2.8ms mean/15.6ms max per fence; N-child composite undo
  (10-cell drop ≈ 30-150ms) ACCEPTED — watch for perceptible hitch during the
  sitting; composite-level batch-fence only if it actually stutters.
- Accepted v1 limitations unchanged (playing not restored; first-trigger
  auto-play skip; expanded-FX-row collapse + deck-tab highlight lag pending the
  refreshAfterUndoRedo ratification) — do NOT re-report during the sitting.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Undo v1 live verification + GL-thread crash-hardening (Wave-2 MUST #1 tail).
SHIPPED: GL-fence family fix committed (8bd09ba, 16 files, tests 170→176) — the reproduced Column→New crash class is dead across every swept path incl. undo/redo replay; fence internals RAII-hardened; CMake stable-signing wiring ready (ad-hoc fallback active). Plus: 2 crashes root-caused (1 fixed, 1 queued), mixed-drop bug + unwired Comp/Decks browser buttons discovered, 10 manual-checklist items passed, 2 Boris feature ideas captured.
IN-FLIGHT: app gate PARKED — fenced build running with the TCC mic dialog unanswered on screen; one Allow click + health check closes it.
NEXT: Allow click → app gate closes → cert creation + reconfigure/rebuild (last TCC prompt ever) → resume sitting with HOLD dissolved + 4 re-runs → Boris decision queue (9 items) → next-lane fork (Recorder / ISF / visual design).
BLOCKERS: none autonomous — every remaining item needs Boris (click, cert, decisions, sitting).
YOU ARE HERE: undo v1 code-complete AND crash-hardened; live verification ~25% done, paused mid-sitting on one Allow click.

## LOOSE-ENDS LEDGER

Adversarial "what's unfinished / what am I unsure about":
- App gate NOT closed: the fenced build has never answered health (TCC dialog
  unanswered ~45min at close; watchers expired). If the app was quit overnight,
  relaunch re-fires the prompt — expected, not a regression.
- Fenced build has ZERO live-app proof yet — 176/176 headless + review traces
  only. The first post-Allow sitting minutes are the real gate (esp. Column→New,
  clears, effects edits under live render — the exact paths that crashed/were
  exposed).
- Cert never created (3 keychain checks, 0 identities). Until it lands: ad-hoc
  → per-rebuild prompts. After creating it: **cmake reconfigure is REQUIRED**
  (configure-time `security find-identity` check) — a bare rebuild keeps ad-hoc.
- Sitting ~25% done; HOLD dissolves only AFTER the app gate; 4 earlier PASSes
  were race-lucky (rode then-unfenced paths) and need re-runs.
- Crash #2 (MilkDropBrowser::getCuratedPresets null-deref on layout with empty
  preset state) is UNFIXED — clicking the SignalBar arrow still crashes until
  the queued mini-lane lands. Warn Boris before he pokes that button.
- Composition → New is now fenced but UNTESTED live.
- N-fence composite-undo hitch (worst ~150ms on big drops) unmeasured in-app.
- The mixed-drop image-discard bug is diagnosed but UNFIXED (silent data loss
  UX: hover highlight promises acceptance, then drops the PNG).
- EffectFenceHook/DeckFenceHook are byte-identical twin typedefs (UI vs core) —
  accepted by review; drift would be a compile error, but a future reader may
  be confused. Cross-referenced in comments.
- touched-repos.sh returned 7 machine-wide-dirty candidates at close; work repo
  resolved by session-commit evidence (8bd09ba), not the script — recurring
  ambiguity, registry dirt from other lanes persists.
- Visual-design track untouched: v10 layer-row study partB (V20
  HARMONY-RECOMMENDED) awaits a ratified winner; dormant since 2026-05-22.

## META-LEARNINGS

- Multi-round adversarial review is the correct shape for thread-safety work:
  every single round's independent sweep found REAL new gaps (exception-safety
  in the fix's own new code; kCompNew; occupied-cell replay; 2 never-fenced live
  paths). "Prescription executed" never equals "family closed" — the family is
  closed when an independent sweep finds nothing.
- A spec's "status-quo risk profile" is a falsifiable hypothesis, not a
  settled fact — one live crash falsified undo-v1's column/cell-write exemption
  and produced a LAW. Treat every accepted-risk row as awaiting evidence.
- Parse the .ips faulting stack BEFORE dispatching any agent: 2 crashes in one
  sitting → 2 unrelated subsystems (GL UAF vs UI null-deref); the stack read
  kept them from being conflated into one lane.
- Disassembly-grade crash triage (binary-UUID match + register decode) converts
  "probably a race" into a provable mechanism cheaply — the scout pattern
  (read-only Explore + the stack + a falsifiable hypothesis) is reusable.
- Reviewer-prescribed verbatim comment edits via a retirement-deferred builder
  (no re-review, gate re-run covers) — reaffirmed twice; still comment-only,
  never code.
- Builder judgment calls (typedef relocation) flagged-with-rationale + reviewer
  confirm beat packet-literalism; the include-direction argument was correct.
- Boris-present sittings surface non-target bugs at high rate (2 crashes, 1
  data-loss bug, 2 unwired features, 2 feature ideas in ~30 min of clicking) —
  budget triage capacity into any live sitting; async scouts keep the sitting
  moving while diagnosis runs.

## CHANNEL HARVEST

- Boris turns: lane pick ("Sitting, then cert"), sitting feedback (items 1-12,
  2 crash reports, 3 how-do-I questions), 2 feature ideas → CAPTURED as
  canonical `--- IDEA ---` records in `.harmony/idea-ledger.md` (Cmd+X
  cut-to-clear; MilkDrop presets loaded by default), family-fix ratification
  (AskUserQuestion), visual-design status question (answered: v10 partB, no
  ratified winner), "that's all my feedback" + remaining-list request, close.
  R2 transcript sweep: no other idea-class statements found.
- harmony2 writes: ZERO (no system, no memory, no .pending, no events) →
  escalation predicate CLEAN → eos-secondary correct.
- Carry-forwards all repo-local (lane B): undo-v1-ledger.md (+11 entries) ·
  undo-v1-manual-e2e.md (ticks/findings/HOLD lifecycle) · notebook.md (LAW +
  scope riders + 2 bug entries) · APP-INVENTORY.md (Comp/Decks row → partial)
  · idea-ledger.md (2 canonical records) · sessions/ log · this HANDOFF.
