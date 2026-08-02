# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). Session 2026-08-02 closed 5 lanes full-tier at 22% ctx
(~16 commits local, NOT pushed; tests 188→189): (1) SYPHON IS REAL — the dormant
May-18 server was a silent no-op (missing compile-time -F on a __has_include gate);
now: SDK vendored via pinned FetchContent (SHA 71351d4b, BSD-3 attributed), build
default ON, REST toggle/status (GET /api/syphon, POST /api/set_syphon), headless
`syphon-check` CLI, behaviorally gated (announce/retire proven live, quit-under-load
clean, zero .ips). (2) FeatureBus CAS → acq_rel (single-reader race closed,
TSan-proven before/after). (3) THREAD-SAFETY DESIGN RATIFIED by blind council →
`.harmony/specs/featurebus-thread-safety-design.md` (R1-R10) — seqlock+value-copy,
pointer handout dies. (4) API hardened: bind 127.0.0.1 default (AUDIODNA_API_BIND
env restores wide), /api/inject_features UNREGISTERED in production, enums clamped.
(5) OutputWindow micro: duplicate cross-thread processFrame DELETED (reviewer FAIL
found a real residual — see KNOWN LIMITATION — adjudicated keep+document; R10 queued).
Full narrative: `.harmony/sessions/2026-08-02-syphon-lane-secondary.md` (entry labels
authoritative over position). Dossiers: scout-syphon-renderer.md · scout-syphon-sdk.md
(carries a shallow-fetch erratum) · scout-outputwindow-glcrash.md.

START HERE:
1. BORIS FEEDBACK FIRST (if any waiting). Two lists may return: (A) the NEW Syphon
   live-check list — frame content in Simple Client/VDMX (non-black, letterbox right,
   120fps smooth), eager-announce taste, runtime-default-OFF taste, loopback veto /
   remote-control question; have him OPEN THE OUTPUT WINDOW during it (exercises the
   unexercised second-GL crash family + the R2 deletion). (B) the 7-item GESTURE
   replay list from 2026-07-30 (verbatim below — STILL OUTSTANDING, never processed).
   PASS → close rows with dated notes; FAIL/odd → receiver-verify on disk BEFORE any
   fix dispatch.
2. **START HERE — long task, begin at session start (if no Boris feedback waiting):
   S2 SEQLOCK CONVERSION** — implement `.harmony/specs/featurebus-thread-safety-design.md`
   R1-R5 + R7 + R9 exactly (council-ratified; do NOT re-litigate the design): seqlock
   with atomic<uint32_t>-array payload, caller-owned value copies, move-only Writer
   handle claimed at the MainComponent.cpp:1568 testMode_ branch, delete
   acquireRead/getLatestRead/hasNewData/kNewFlag, ~16 call sites go one-line, R7
   persistent snapshot parking (fixes the dangling-stack defaultSnap bug), R9 test
   gate (multi-reader TSan case DEMONSTRATED racing on the old protocol first;
   zero suppressions). Full-tier verify; TSan is the behavioral gate.
3. OUTPUTWINDOW ARC (after S2): scout-outputwindow-glcrash.md R1+R3 (per-renderer
   EffectChainGLState — kills the 2-GL-thread uniformLocationCache_ UB crash + the
   programID-collision poison + prevFrame cross-context thrash) + R10 mapping cadence
   (must survive preview auto-detach — closes the KNOWN LIMITATION) + the output
   detach reorder beside MainComponent.cpp:1759 (shutdown-order-law intent). Cheapest
   confirmation the scout named: open the output window under build-tsan with effects
   active.
4. BORIS DECISION QUEUE (all pre-analyzed, deliver ONE per ask): source/MilkDrop
   retrigger-restart scope · column-trigger retrigger parity · Cut/Copy/Paste suite
   (menu enums RESERVED at MenuBarModel.h:76-80) · playlist mode-gate consistency ·
   reset ~5s latency taste · + NEW from this session: eager-announce, runtime
   default/persistence, remote-control+token.
5. REMAINING QUEUED FOLLOW-UPS: ID-based selection remap (Layer has stable uint32_t
   id; CellPos lacks plumbing) · full §3 APP-INVENTORY row pass (TWO dated delta
   blocks now sit in §2: 07-30 + 08-02) · S3 field atomics (design doc, after S2
   survives real use) · empty-string AUDIODNA_API_BIND explicit fallback (nit, next
   time ApiServer.cpp opens).
6. VERIFICATION DOCTRINE (in force, extended this session): FORCED REBUILD before any
   ctest claim · `git commit --only <files>` (graphify-out churn NEVER swept) ·
   subagent idle-without-report → nudge (hit 6/6 this session — budget a nudge per
   agent) · multi-lane shared tree: behavioral gate WAITS until ALL lanes commit
   (chimera-tree rule; reviewers may start early on commits) · exit-proof = pgrep -x
   + port-free (pgrep -f self-matches its own wrapper) · ledger appends anchor on the
   CURRENT FINAL LINE (anchor-reuse scrambles order).

THE 7-ITEM REPLAY LIST (verbatim, from 2026-07-30):
  a. Drag an effect onto a layer's CHANNEL STRIP (single + multi-select) → lands in
     that layer's FX stack, ONE Cmd+Z restores.
  b. Finder-drop 2 videos + 1 image together → all three land, ONE Cmd+Z removes all.
  c. Cmd+X with a cell selected (clears) / with nothing selected (clean no-op).
  d. Click a PLAYING video cell → visibly restarts from in-point.
  e. Header-drag "Energetic (9)" → cell reads "MilkDrop Playlist (9)"; the 3 playlist
     knobs (cycle/timing/blend) now actually change what lands.
  f. Autopilot over a SOURCE cell → advances off it (was frozen forever).
  g. Genre auto-switch to an empty deck → preview goes blank (no ghost clip).

Standing rules: do NOT push (entire local stack, now ~49 commits since last push);
conform to ClipCommands.h/DeckCommands.h/EffectCommands.h/TriggerCommands.h/
UndoService patterns at HEAD; structural-mutation commands carry a fence (notebook
LAW); launch ONLY via `open` (port 7070 binds ~12s; poll /api/health); SIGKILL
disposable instances; SYNTHETIC-CLICK PREFLIGHT (gotchas 11) before any click burst;
NEW gotcha: .harmony/ is gitignored-but-tracked — new knowledge files need one-time
`git add -f` (Harmony owns those commits); TCC mic prompt can re-fire on first launch
after any rebuild (ad-hoc signing) — screencapture-diagnose if port never binds.

## PRIMER
- Design of record: `.harmony/specs/featurebus-thread-safety-design.md` (R1-R10 +
  staging + council record + Boris items).
- Dossiers: scout-syphon-renderer.md · scout-syphon-sdk.md (pin/API/pitfalls +
  erratum) · scout-outputwindow-glcrash.md (top-3 ranked, .ips cross-check).
- Session ledger: sessions/2026-08-02-syphon-lane-secondary.md.
- Syphon quick-drive: enable via menu or `curl -X POST -d '{"enabled":true}'
  localhost:7070/api/set_syphon`; verify announce: `./build/tests/syphon-check
  "Audio-DNA" 1.0` (exit 0 = announced).

## LOOSE ENDS (adversarial — what is NOT done / not sure)
- Boris-only checks NOT run: Syphon frame CONTENT in a real client (gate proves
  announcement, not pixels); output-window live exercise (crash family UNEXERCISED,
  not disproven); test-mode clamp behavior live (structurally verified only).
- 7-item gesture replay list from 07-30: STILL OUTSTANDING, zero feedback processed.
- KNOWN LIMITATION shipped deliberately: SignalBar expanded (preview hidden) while
  output window open → mapping params FREEZE on output until reattach (R10 queued;
  comment in OutputWindow.cpp:111-127 states it).
- Eager-announce: server visible to clients at boot with toggle OFF (taste ruling
  pending; announce-on-enable is a design change if Boris wants it).
- Env-var edge: empty-string AUDIODNA_API_BIND stays loopback only by coinciding
  defaults (reviewer-inferred, not behaviorally tested; explicit fallback queued).
- build-asan cache NOT reconfigured (still pre-flip defaults); build-tsan refreshed
  but does NOT build syphon-check (its negative ctest is absent there).
- Doc drift (pre-existing, flagged not fixed): tests/README.md:85 claims ApiServer.cpp
  ≤900 lines (actual 1017); FFTProcessor _USE_MATH_DEFINES warning.
- `.audit/features-gap-fill/` untracked dir of unknown provenance — untouched,
  unowned; next session should identify or route it.
- touched-repos.sh session-scoping gap (filed as system candidate; REPO_ROOT was
  resolved by session evidence this close).

## META-LEARNINGS (distilled from log-event rows pushed in-session)
- pgrep -f/-if self-matches its own shell wrapper → false STILL-RUNNING verdicts;
  exit-proof = pgrep -x + lsof port check (two sources).
- Multi-lane shared worktree: behavioral gates wait for ALL lanes to commit
  (chimera-tree); reviewers start early on commit diffs.
- Idle-without-report is the NORM (6/6 agents) — budget one nudge per agent.
- Ledger appends: anchor Edit on the current FINAL line; mid-file anchor reuse
  scrambles order and can fuse entries.
- touched-repos.sh lists standing dirt, not session touches — REPO_ROOT disambiguation
  needs session evidence (commits/ledger/handoff origin).

## CHANNEL HARVEST (what left this session, by lane)
- harmony2 event log (lane-A legal, transient telemetry): 5 learning rows
  (gate-probe-pgrep-exact · ledger-append-anchor-reuse · subagent-nudge-norm ·
  multi-lane-gate-sequencing · touched-repos-session-scope).
- Boris idea sweep (R2 backstop): RAN — zero uncaptured idea-class statements (only
  directives: "go with recs work till 40% ctx used then eos"); idea-ledger unchanged.
- Project-local: this handoff + session ledger + APP-INVENTORY 08-02 delta + new
  gotcha (.harmony force-add convention) + design doc R1-R10 + 3 dossiers.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA live VJ app — hardening the concurrency core + making Syphon output real (post-13-item-queue polish phase)
SHIPPED: Syphon output functional end-to-end (vendored SDK, ON by default, REST toggle, verify CLI) · FeatureBus single-reader race fixed (TSan-proven) · thread-safety redesign ratified + committed (R1-R10) · API locked to loopback + injection gated out of production · OutputWindow flicker/race call deleted · 3 recon dossiers · tests 189/189
IN-FLIGHT: none — all 5 lanes closed full-tier, no half-done code
NEXT: S2 seqlock conversion (next session #1, START HERE) · OutputWindow R1/R3+R10 arc · Boris: Syphon eyeball check + 7-item replay + decision queue
BLOCKERS: none for build; Boris-only items (live checks + taste rulings) gate the polish
YOU ARE HERE: core features complete and gated; one ratified concurrency rewrite + one output-window hardening arc from "structurally sound under load", then back to feature polish
