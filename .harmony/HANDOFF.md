# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). **Undo v1 is BUILD-COMPLETE, CRASH-HARDENED, and now
LIVE-PROVEN:** the fence lane's app gate CLOSED 2026-07-30 and an autonomous
synthetic-UI drive ran **~130 mutations across every crash-family path under active
GL render with ZERO crashes** (columns ×33 incl. undo/redo replay on the exact
07-28 SIGSEGV scenario, clears, FX vectors, CompNew, deck/layer structure, trigger
merge algebra). ~75% of the manual e2e checklist is PASSED with dated oracle notes;
an **11-item Boris runsheet** sits at the top of `.harmony/undo-v1-manual-e2e.md`
(~15-20 min of his hands: Finder drops, video cases, name-bar gestures, FX scopes,
autopilot confirm, quantize edge, taste calls). Tests 176/176 (`ctest --test-dir
build`). Build: `cmake --build build --config Release -j`. Lane ledger:
`.harmony/undo-v1-ledger.md` — READ the 2026-07-30 entries FIRST.
Launch ONLY via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`.

**START HERE:** (1) `security find-identity -p codesigning -v` — if **Audio-DNA
Dev** exists (Boris's 2-min Keychain step, still pending at close): run cmake
RECONFIGURE (identity resolves at CONFIGURE time — a bare rebuild stays ad-hoc) +
rebuild + relaunch + ONE final Allow click (last TCC prompt ever) + health gate. If
absent, remind Boris (Keychain Access → Certificate Assistant → Create a
Certificate → exactly `Audio-DNA Dev`, Self-Signed Root, Code Signing). (2) When
Boris is present: run the 11-item runsheet sitting — you verify live via the
oracles below. (3) Optional autonomous: crack the cell NAME-BAR geometry (scout
ClipCell.cpp:180-194/:348-355 for exact rects) to close runsheet items 3-4
(drag-move, Clip>Clear) without Boris.

**NEW CAPABILITY (2026-07-30, permanent):** Accessibility is GRANTED to Ghostty —
synthetic UI driving works in ALL future sessions. Driver recipes + flake profile:
`gotchas.md` 2026-07-30 entry (thumbnail=trigger vs name-bar=select; strip-click
layer selection; verify-and-retry mandatory ~15% event drops; window-capture by
CGWindowList id; coords pt=display×0.864, y+38). Oracles: /api/composition (NB:
omits clip effects), Composition-menu undo labels (menu is COMPOSITION — no Edit
menu exists), window captures. Mouse tool: /tmp/adna-mouse.swift (rebuild from
gotchas if wiped).

Boris decision queue (ledger, none started): MilkDropBrowser crash-#2 fix bundled
with default-preset-dir (his ask; .ips 2026-07-28-190701) · mixed-drop
image-discard fix · **Cmd+X cut-to-clear — scout-confirmed NO Cut command exists
at HEAD, net-new build** · marshal the 2 HTTP-thread ApiServer writes (set_param
:399, set_layer_opacity :458 — queue #4, tiny) · wire Comp/Decks browser load/save
(FUTURE-FENCE comment mandatory) or defer to recorder wave · refreshAfterUndoRedo
skip · zero-layer Deck-New intent · inspector-clear UX · Syphon install · B8
removal · NEW candidates: menu enablement not selection-gated (silent no-op class)
· rebuildGrid stale invisible selection · preview animates old deck's clip while
empty deck active (intent?). Next-lane fork after the sitting closes: Session
Recorder (ratified default) vs ISF import (app already has an "Import ISF
Shader..." menu entry) vs visual-design resume (dormant since 05-22).
Standing rules: do NOT push to remote; lane rules in the ledger; conform to
ClipCommands.h / DeckCommands.h / EffectCommands.h / TriggerCommands.h /
UndoService patterns at HEAD (DeckFenceHook lives in ClipCommands.h; every
structural-mutation command carries a fence — new commands MUST too, notebook LAW).

## PRIMER

- HEAD at close: doc-only close commit atop bb076c7/8bd09ba. Local only, no-push
  rule intact. graphify-out/ churn + untracked .audit/ = never stage.
- Counts: 135 effects · 108 sources · **176/176 tests** · fence perf ~2.8ms
  mean/15.6ms max (no perceptible hitch observed across ~130 driven mutations).
- THE LAW (notebook.md 2026-07-28) unchanged: any message-thread mutation of clips
  vectors / composition_.decks / effects vectors runs under
  UndoService::withDeckDetached; commands carry DeckFenceHook.
- App at close: RUNNING healthy (119fps), plasma@L0C0 + perlin@L1C0 staged
  (inactive), history [Drop,Drop]. TCC mic granted for CURRENT cdhash only — next
  rebuild re-prompts ONCE until the cert lands (keychain 0 identities at close).
- Remote surface (scout-mapped, cited in ledger): triggers/deck-switch/BPM/effect
  params/renderer loads drivable; undo + ALL structural mutations UI-only;
  TestServer (--test-mode) is a dead-end BY CONSTRUCTION (no MainComponent
  handles) — do not relaunch for it; it also freezes audio analysis.
- Verifier model unchanged. Accepted v1 limitations unchanged (playing not
  restored; first-trigger auto-play skip; expanded-FX-row collapse + deck-tab
  highlight lag pending refreshAfterUndoRedo ratification) — do NOT re-report.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Undo v1 live verification tail + fence-family in-app proof (Wave-2 MUST #1).
SHIPPED: App gate CLOSED (zero clicks — Allow had landed off-session; health 119fps). Autonomous synthetic-UI drive: ~130 mutations, every crash-family path, ZERO crashes — Column→New SIGSEGV scenario dead across 33 ops incl. replay; ~15 checklist items PASSED with oracle notes; merge algebra + deck triad + layer ops + FX clip-scope + CompNew all verified. API surface fully mapped (4 scout packets). 4 new findings queued + Cmd+X confirmed net-new. Accessibility capability unlocked permanently; driving recipes in gotchas.
IN-FLIGHT: none — drive complete, hands returned to Boris.
NEXT: cert step (Boris, 2 min) → reconfigure+rebuild+final Allow (Harmony) → 11-item runsheet sitting (~15-20 min, Boris hands + Harmony oracles) → decision queue rulings → next-lane fork (Recorder default / ISF / visual design).
BLOCKERS: none autonomous — remaining items need Boris (cert, hands, taste, rulings).
YOU ARE HERE: undo v1 code-complete, crash-hardened, AND live-proven; verification ~75% done; one short Boris sitting from lane closure.

## LOOSE-ENDS LEDGER

Adversarial "what's unfinished / what am I unsure about":
- 11-item runsheet OPEN (checklist top): Finder drops (3 race-lucky re-runs +
  mixed-batch bug), video replace + undo-while-video, name-bar gestures
  (drag-move, Clip>Clear), FX layer/global scopes + delete/bypass/multi-select,
  autopilot confirm, quantize/pendingTriggerColumn edge, ignore-column-trigger
  variant, MilkDrop drops (blocked on preset-dir fix), taste calls, Syphon, cert.
- Autopilot test INCONCLUSIVE — label frozen 15s ✓ but state suggests autopilot
  may never have engaged (no API field proves it ran). Source proof stands.
- Preview animates the OLD deck's clip while empty Deck 2 is active
  (frames-differ verified) — intended composition semantics or display gap?
  Boris intent question; do not "fix" without his ruling.
- Cert NOT created at close (0 identities, 4th check) — per-rebuild TCC prompts
  continue until it lands; reconfigure REQUIRED after creation.
- load_image-black gotcha re-attribution still INFERRED (1-min verify: reset →
  load_image → render_frame → expect non-black; reset wipes effect chain — run
  post-sitting only).
- Crash #2 (MilkDropBrowser::getCuratedPresets null-deref) still UNFIXED — the
  SignalBar arrow click still crashes; warn Boris before he pokes it.
- App left RUNNING with 2 staged clips + [Drop,Drop] history — fine for a
  sitting; Comp→New or relaunch gives a clean slate (relaunch = NO prompt until
  next rebuild).
- Driver gap: synthetic drags cannot initiate clip drag-move (thumbnail press
  triggers) — name-bar drag hypothesis untested; exact rects available in source.
- touched-repos.sh returned 7 machine-wide-dirty candidates again — work repo
  resolved by session evidence (all writes → RealTimeAudio); recurring registry
  ambiguity, other lanes' dirt persists.
- The 3 remaining race-lucky re-runs are Finder-drop gestures — my source-drags
  exercised the same fenced growth paths heavily, but the literal external-drop
  entry (ClipCell filesDropped) remains human-only.

## META-LEARNINGS

- A "manual-only" e2e checklist was ~70% automatable the moment Accessibility
  landed: CGEvent driver + menu AXPress + API/menu-label oracles + verify-and-
  retry. The unlock was ONE 30-second human grant — ask for capability grants
  early, not after exhausting workarounds.
- Synthetic-event delivery drops ~15% of keystrokes/AXPresses — blind-fire
  automation produces FALSE app-bug signals. Verify-and-retry with a
  state+label fingerprint is mandatory; and never fire a blind undo after an
  UNVERIFIED op (over-pops the stack when the op's effect is invisible to the
  oracle — bit us twice).
- When UI automation hits a deterministic no-op wall, a read-only source scout
  adjudicates in minutes what behavioral probing cannot (selection
  preconditions, hit-band geometry, secondary guards) — behavioral evidence +
  source citation together beat either alone.
- Gesture GEOMETRY is load-bearing verification state: thumbnail-vs-name-bar
  (20px) separates trigger from select from drag-move. Record hit-bands in
  gotchas, not just "click the cell".
- Empty-cell operations are state-no-ops by design (state-change guards) — a
  drive on an EMPTY grid proves almost nothing; stock content FIRST, then
  mutate under active render for faithful crash conditions.
- Honest downgrades preserve trust in a mostly-green report: PASS-with-oracle,
  INCONCLUSIVE, and parked-with-recipe are three different verdicts — label
  them; a checklist tick without its oracle is worthless to the next session.

## CHANNEL HARVEST

- Boris turns: overnight Allow click (off-session, inferred from TCC state) ·
  "keep working and verifying till you have taken care of the list" (autonomous
  mandate) · "go with option a" (Accessibility path ratified) · "how do I grant
  accessibility?" (answered; granted in 20s) · close request ("what is left for
  you to do save for next session and run eos"). R2 transcript sweep: NO new
  idea-class statements this session (the Cmd+X + MilkDrop-presets ideas were
  captured 07-28; today only added the scout's no-Cut-at-HEAD fact to the
  existing record's context in the ledger).
- harmony2 writes: ONE .events learning append via log-event (lane-A telemetry,
  sanctioned — ref synthetic-ui-driving); ZERO system files, ZERO memory, ZERO
  .pending → escalation predicate CLEAN → eos-secondary correct.
- Carry-forwards all repo-local: undo-v1-ledger.md (+3 entries incl. queue #4 +
  4 new finding candidates) · undo-v1-manual-e2e.md (15 PASS annotations +
  11-item runsheet) · gotchas.md (+1 recipes entry + load_image rider) ·
  sessions/2026-07-30 log · this HANDOFF.
- Agent traffic: api-surface-scout (Explore) — 4 packets (surface map,
  TestServer ruling, wiring adjudication, recipes), all folded + cited;
  released with thanks.
