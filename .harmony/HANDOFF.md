# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). Session 2026-07-30 PM closed with a LANDMARK day: Boris
ran the 11-item sitting, every finding was root-caused same-day, and **4 fix lanes +
the TRUE crash-#2 fix are ALL source-closed** — 7 source commits (9229f87 MilkDrop
presets+guards · a718572+20fe75d clear-path bundle · 9c316e6 composition FX drop
target · 8c746ab+9b74c7d FilesBrowser perf · 76594fd GL-detach UAF fix), every one
independently reviewed (APPROVE), tests 176→**182** green. NOT pushed (lane rule).
Crash #2 was proven (disassembly-level) a deterministic UAF: preview-panel hide →
sync GL detach → activeSources_.clear() → preset manager died under the browser's
interior pointer. Fixed by letting sources survive context close (lazy GL re-init).
Full dossiers: `.harmony/scout-milkdrop-uaf.md`, `scout-sitting-triage.md`,
`scout-shutdown-sigbus.md`, `scout-namebar-geometry.md`. Ledger PM2-PM20 entries =
the session narrative. Read ledger 2026-07-30 PM14-PM20 FIRST.

START HERE:
1. **BORIS FEEDBACK FIRST.** Boris has an 8-item test list (below) and said he will
   report feedback after this boot. Process it before anything else: PASS → close
   the matching checklist/ledger rows with dated notes; FAIL/odd → receiver-verify
   on disk (.ips? /api state? capture) BEFORE any fix dispatch. The 8 items map to:
   crash-#2 replay (UAF fix) · X-clear output-stop (A1) · multi-layer isolation
   (A1 edge) · Clip>Clear empty+autopilot-skip+undo (A2) · composition panel FX
   drop + one-undo-entry (B) · Files browser speed (C) · item-3 drag-move undo
   (runsheet leftover) · item-9 MilkDrop drops (runsheet).
   THE 8-ITEM TEST LIST (verbatim, as given to Boris at close):
   1. Crash-#2 replay: Browser→MilkDrop (presets listed?) → SignalBar expand arrow
      → collapse → MilkDrop again: presets still there, no crash.
   2. X-clear: trigger a clip (shader/MilkDrop best) → X on that layer strip →
      output visual STOPS.
   3. Isolation: clips playing on 2 layers → X-clear ONE → other keeps rendering.
   4. Clip>Clear: name-bar select a staged cell → Clip>Clear → autopilot ON never
      lands on that cell → Cmd+Z restores the clip exactly.
   5. Composition FX drop: drag an effect anywhere onto the Composition inspector
      panel → lands in global stack, "Undo Add Effect" in Composition menu;
      multi-select drop = ONE undo entry.
   6. Files browser: open Desktop → instant list, thumbnails fill in after; List
      toggle instant.
   7. Drag-move a clip by its NAME BAR to an empty column → Cmd+Z → both cells
      restore (the still-open item-3 yes/no).
   8. MilkDrop drops: single preset into a cell + a playlist drop → Cmd+Z each
      (old runsheet item 9).
2. The instance running at close (PID 91888) PREDATES 9b74c7d (teardown-internal
   hardening only). First relaunch picks up everything:
   `open build/AudioDNA_artefacts/Release/Audio-DNA.app` — ZERO TCC prompts (the
   Audio-DNA Dev cert inherited the mic grant; proven this session).
3. Synthetic driving is ALLOWED but MUST follow gotchas rule (11) SYNTHETIC-CLICK
   PREFLIGHT: fresh HID idle + frontmost-app==Audio-DNA checked in the SAME command
   as every click burst; abort otherwise (a click landed in Boris's Firefox this
   session — disclosed, doctrine written). Driver recipes: gotchas 2026-07-30 entry
   + addenda (7)-(11). Mouse tool /tmp/adna-mouse.swift (rebuild from gotchas if
   wiped). API port is 7070.
4. Anything Boris's feedback does NOT cover from the scripted gate plan (ledger
   PM8/PM18): UAF arrow replay, X-clear stop, Clip>Clear emptiness via /api (empty
   cell = ABSENT from clips[], blank stub = present with name ""), comp FX drop,
   FilesBrowser timing, item-3 drag-move self-close.

BORIS DECISION QUEUE (none started; all pre-diagnosed): shutdown-crash fix bundle
(scout-scoped: detach-GL-first 1-liner + EffectChain fence + :472 bounds-check;
verify via ASan variant — repo has NO sanitizer wiring) · (11) activeSources_
unordered_map 3-thread no-mutex UB · (D) retrigger-restart design ruling (click on
playing cell = emergent no-op today; restart needs player seekTo) · LayerStrip as
FX-drop-target ruling (Boris tried it; NOT-WIRED at HEAD) · (8) fold-height
MIRROR-INDEX bug (DeckView.cpp:274 — DIAGNOSED NOT FIXED; folded layers skew all
cell Y) · mixed-drop image-discard · Cmd+X cut-to-clear (net-new; no Cut at HEAD) ·
HTTP-thread marshal (#4, ApiServer :399/:458) · (5) menu enablement not
selection-gated · (6) rebuildGrid stale invisible selection · (7) preview animates
old deck's clip on empty active deck · Syphon install · B8 removal.
Taste calls: deliver ONE per ask (Boris directive — never batch).
NEXT-LANE FORK after the gate closes: Session Recorder (ratified default) vs ISF
import vs visual-design resume (dormant 05-22).

Standing rules: do NOT push; conform to ClipCommands.h/DeckCommands.h/
EffectCommands.h/TriggerCommands.h/UndoService patterns at HEAD; every
structural-mutation command carries a fence (notebook LAW); launch ONLY via `open`;
SIGKILL disposable instances (gotchas 10); Explore scouts need an explicit
"SendMessage to main" line in their contracts or they idle silently.

## PRIMER

- Build: `cmake --build build --config Release -j` · Tests: `ctest --test-dir build`
  (182/182) · Launch: `open build/AudioDNA_artefacts/Release/Audio-DNA.app`
- Oracles: /api/health + /api/composition on **port 7070** (NB: omits clip
  effects; empty cell = absent from clips[]) · Composition-menu dynamic undo labels
  (menu is COMPOSITION — no Edit menu) · window captures via CGWindowList id +
  `screencapture -o -x -l<id>` (window coords ×0.864, y+38) or full-screen
  `screencapture -x` (coords ×0.864, NO +38)
- Cert: `Audio-DNA Dev` self-signed, trusted (user-domain), resolves via
  AUDIODNA_CODESIGN_IDENTITY at configure time; TCC grant persists across rebuilds
- Key docs: undo-v1-ledger.md (lane narrative) · undo-v1-manual-e2e.md (checklist
  + sitting results) · gotchas.md (driver doctrine (1)-(11)) · 4 scout dossiers ·
  APP-INVENTORY.md (reconciled 2026-07-30)

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Undo v1 lane TAIL — sitting-driven fix wave; app hardening before the next-lane fork (Session Recorder default).
SHIPPED: MilkDrop presets auto-load + crash guards · clear-path bundle (X-clear stops output, Clip>Clear truly empties, autopilot skip, pointer resets) · composition panel-wide FX drops (one undo entry incl. multi-select) · FilesBrowser instant open + async cached thumbnails · crash-#2 TRUE fix (GL-detach UAF, disassembly-proven) · cert trust fixed → zero TCC prompts forever · tests 176→182 · 4 scout dossiers + gotchas doctrine (7)-(11).
IN-FLIGHT: Boris's 8-item test list (his hands; feedback lands after next boot) · behavioral gate items not covered by his feedback.
NEXT: process Boris feedback → close/triage per item · relaunch onto 9b74c7d binary · then Boris picks from the pre-diagnosed queue (shutdown-crash bundle recommended first) · then the next-lane fork.
BLOCKERS: none hard — behavioral proof pends Boris feedback or a machine-free window.
YOU ARE HERE: all known bugs from the sitting are FIXED at source and reviewed; the lane closes when the behavioral gate (Boris's list + scripted remainder) confirms live behavior.

## LOOSE-ENDS LEDGER

LOOSE-ENDS (adversarial — what is NOT done / what I'm unsure about):
1. **NO fix has been behaviorally proven live.** All 5 fix groups are source-closed
   + reviewed only; the combined gate never ran (machine occupied, then EOS).
   Boris's 8-item list + feedback is the primary closure path; scripted remainder
   in ledger PM8/PM18.
2. Item-3 yes/no (drag-move → Cmd+Z restored both cells?) still unanswered from the
   sitting; self-closeable synthetically.
3. Item-5 residue untested behaviorally: delete-FX-row undo, FX-bypass undo;
   multi-select-one-entry is source-proven only.
4. Item-9 (MilkDrop single + playlist drop + Cmd+Z) unblocked but untested.
5. Running instance at close predates 9b74c7d (teardown hardening not live until
   next relaunch).
6. Fold-height mirror bug (queue 8) diagnosed NOT fixed — folded layers corrupt all
   cell Y geometry for users AND synthetic drivers (drive guard: unfold first).
7. Stale-active anomaly: A2 fixed the CLEAR path; the column-REMOVAL remap variant
   observed at 12:33 is unverified post-fix — re-observe after Boris feedback.
8. Shutdown SIGBUS (queue 10): mechanism proven, WRITER unidentified; repo has no
   ASan wiring; fix bundle awaits Boris nod.
9. activeSources_ 3-thread no-mutex (queue 11) — UB class, unfixed, distinct from
   the UAF fix.
10. Lane-doctrine ambiguity for the PRIMARY to reconcile: a slim-door (~/Harmony)
    secondary working a foreign repo used log-event (kernel Write-Immediately
    allows "secondary → log-event append"; eos-secondary lane-B forbids harmony2
    event-log writes). 2 learning rows were pushed this session (single-item-checks,
    cert-trust recipe). Kernel and skill disagree for this hybrid case.
11. Pre-existing dirt left alone: untracked .audit/ dir; FileListContent::hitTest
    hides base-class overload warning (pre-dates lane C).
12. My 13:02 SignalBar probe as the 13:25 arming event is INFERRED (code path
    proven; instance unobservable) — labeled as such everywhere.

## META-LEARNINGS

- Perceptual/taste checks: ONE item per Boris ask, never batched (his directive,
  rejected a 6-item batch) — division-of-judgement per-unit routing.
- Green gate ≠ proof at the right LAYER: /api/composition (model) passed clears
  while the RENDERER kept playing — behavioral gates must watch the layer the user
  sees (captures/frame-diff), not just state oracles.
- A "fixed" crash symbol recurring is EVIDENCE: the 13:25 recurrence on the guarded
  binary killed the empty-state theory and forced the disassembly-level root cause.
- SYNTHETIC-CLICK PREFLIGHT (gotchas 11): fresh-HID + frontmost-app in the SAME
  command; stale idle checks (even 2 min) are worthless; HID-idle alone fails while
  the user READS.
- SIGKILL disposable instances (gotchas 10): graceful quit traverses a
  corruption-discovery teardown; SIGKILL = no crash noise, no misleading .ips.
- Explore scouts idle WITHOUT transmitting — contracts must carry an explicit
  "SendMessage to main when done" (cost 2 nudge round-trips before adopted).
- Self-signed cert trust: diagnose with `find-identity` WITHOUT -v (shows reason
  codes); fix user-domain `add-trusted-cert -p codeSign`, no sudo, no GUI.
- R3 warm fix-loops (builder+reviewer pairs) closed 3 review findings same-hour at
  near-zero spin-up cost — keep builders/reviewers warm through their lane's close.

## CHANNEL HARVEST

- Lane-A telemetry: 2 log-event learning rows pushed in-session
  (boris-single-item-checks · selfsigned-cert-trust-recipe) — primary distills at
  its close; flagged the lane ambiguity in loose-end 10 for doctrine reconcile.
- Idea-ledger: NO new records this session (R2 sweep found no uncaptured
  Boris-ideas; his two process directives were captured in-session to ledger +
  log-event). Ledger remains canonical (--- IDEA --- only, no ### prose).
- Carry-forwards to primary: loose-end 10 (lane doctrine); the
  boris-session-snapshot-1430.json pattern (snapshot user state before instance
  swap) as a reusable secondary practice.
- .pending channel: NOT used (no harmony2-memory carry-forwards warranted).
