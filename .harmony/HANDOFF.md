# Handoff — Audio-DNA (RealTimeAudio)

## NEXT-HARMONY — BIRTH PROMPT & PERSONA

You are Harmony operating in ~/projects/RealTimeAudio (Audio-DNA — C++20/JUCE/OpenGL
live audio-reactive VJ app). The 2026-07-17 triage session dispositioned ALL 34 FLAGGED
rows (Boris-ratified), shipped Waves 0-1 (8 local commits: dead-code purge, UI honesty,
Syphon wired, OSC live 11/11 on UDP 8000, FULL preset round-trip, 5 small fixes), and
specced Wave 2. Session ledger: `.harmony/triage-2026-07-17.md`. Inventory:
`.harmony/APP-INVENTORY.md` (same-wave-update rule). Build:
`cmake --build build --config Release -j`; tests: `ctest --test-dir build` (114/114 at
handoff). Behavioral gates: launch via `open build/AudioDNA_artefacts/Release/Audio-DNA.app`
(NEVER direct exec — hangs; gotchas.md), :7070 binds ~12s.

START HERE — long task, begin at session start: **Undo v1** per
`.harmony/specs/undo-v1-spec.md` (value-copy commands on the existing scaffold; build
order steps 1-9; validate the GL-fence no-deadlock inference in step 1 FIRST). It is the
#1 Committed MUST. Alternative long tasks if Boris redirects: ISF real import
(`.harmony/specs/isf-import-spec.md`, independent) or Session Recorder
(`.harmony/specs/session-recorder-spec.md`, unblocked — Wave 1 landed its capture sites).
Do NOT push to remote (standing rule). Quick Boris asks pending: install
Syphon.framework + rebuild `-DAUDIODNA_BUILD_SYPHON=ON` to verify real publish; 10s UI
eyeball (menus / 3-tab Prefs / new Sources rows).

## PRIMER

- HEAD at close: c30e393 (doc-sync) on 8 unpushed local commits — full list + per-wave
  gate/review verdicts in `.harmony/triage-2026-07-17.md` (Gate results section).
- Counts at close: 135 effects · 108 sources (108 GUI-selectable — 6 added) · 22/22 REST
  functional · 11/11 OSC live (UDP 8000) · persistence COMPLETE (was lossy) · ~45 menu
  items (37 no-ops removed) · Prefs 3 tabs · 114/114 tests.
- Wave-2 specs are source-verified with file:line evidence but lines will drift —
  symbols are authoritative (each spec says so).
- Verifier model: independent Reviewer on source + Harmony runs the behavioral gate
  (build + ctest + `open` app + REST/OSC probes). OSC probe recipe + master-variable
  trap: session log `sessions/2026-07-17-triage-secondary.md`.
- graphify refreshed at close (code-only) — stamps final HEAD.

## WHERE WE ARE IN THE BUILD
<!-- caveman positional status — Boris-facing, skimmable -->
BUILD: Audio-DNA VJ app — FLAGGED-backlog execution: triage done, cleanup + quick wins shipped, three specced feature builds remain.
SHIPPED: all 34 FLAGGED rows dispositioned with Boris; Wave 0 (13 dead items purged, 3 XS fixes, 37 no-op menu items + 5 empty/inert Prefs surfaces removed); Wave 1 (Syphon output wired [flag-gated], OSC live 11/11 UDP 8000, preset serialization COMPLETE incl. transforms/feedback/crossfader/autopilot/playlists, TopBar transport wired, Layer-Clear deck-wipe bug fixed, waveform seqlock, 48k SR guard, tooltips wired); 3 Wave-2 specs (ISF / Undo v1 / Session Recorder); docs synced to reality; 8 local commits, 114/114 tests, every wave gated + independently reviewed.
IN-FLIGHT: none — tree clean except hook-owned graphify churn; all agents closed.
NEXT: Undo v1 (START HERE, long task); then Session Recorder; ISF anytime (independent); Boris: Syphon.framework install + rebuild w/ flag + client eyeball; Boris: 10s UI check (menus/Prefs/Sources); Boris: ratify OSC port 8000 (alt 7000) + transport semantics.
BLOCKERS: none. (Syphon publish VERIFICATION blocked on framework install — wiring itself shipped.)
YOU ARE HERE: post-triage, post-quick-wins — the app's honest surface matches its real capability; remaining work is three well-specced feature builds (undo, session replay, ISF import) plus one deferred design item (model thread-safety).

## LOOSE-ENDS LEDGER

1. Syphon REAL publish unverified — framework absent on this machine, build flag OFF by
   default. Needs: install Syphon.framework → rebuild `-DAUDIODNA_BUILD_SYPHON=ON` →
   verify in a Syphon client → decide flag-default-ON. Y-orientation (flipped:NO) only
   verifiable live.
2. Boris UI eyeball pending: rebuilt menus, 3-tab Prefs, 6 new Sources rows (incl. new
   MilkDrop category header) — I could not drive native menus (no assistive access).
3. OSC port 8000 hardcoded, no prefs UI — Boris may prefer 7000 (Resolume-style).
   One-line change if so.
4. TopBar transport semantics chosen by Harmony, unratified: active-deck all-layers;
   Stop = pause + rewind to in-point.
5. MilkDrop playlist POSITION stays runtime-only (deliberate; Boris may override →
   trivial addition to Wave 1-C's serialization).
6. Model thread-safety design DEFERRED (one family): Clip::playing plain bool
   cross-thread + dual mapping-engine write-order + lock-free model reads — needs a
   design session, not patches. Undo-spec risk #1 and reviewer LOW finding both cite it.
7. Undo spec's GL-fence no-deadlock claim is INFERRED from JUCE semantics — must be
   validated empirically in Undo build step 1 before `withDeckDetached` is trusted.
8. ISF v1 acceptance target (≥60% of a 30-50 isf.video corpus sample compiling) is
   unmeasured until built; corpus sampling is part of ISF step 7.
9. Hidden-surface decisions still open (interact with mapping design): TimingWindow's 3
   empty tabs, EffectsRackPanel+MappingEditor (only full mapping-edit UI, unreachable),
   AudioReadoutPanel+SpectrumDisplay.
10. No-push rule still active — 8 commits local-only; CI breakage unconfirmed either way
    (pre-existing loose end).
11. SR guard is WARN-only; true sample-rate independence remains deferred (48k hardcode
    still real for non-48k devices).
12. graphify-out working-tree churn is hook-owned and deliberately uncommitted.

## META-LEARNINGS

- Launch-context trap: direct binary exec of the JUCE app from an agent shell hangs
  pre-UI (alive, windowless, socketless) and mimics a broken REST server — `open` (LaunchServices)
  is mandatory for behavioral gates. Cost ~20 min of false-negative diagnosis.
- Prescribe the STRESS TEST, not the concurrency idiom: the packet-suggested
  double-buffer failed the builder's own torn-read test (reader lapping); the seqlock it
  shipped is provably correct. The test requirement produced the right fix, the idiom
  suggestion nearly produced the wrong one.
- Probe the exact variable the write path touches: /api/status masterLevel ≠ composition
  masterOpacity — first OSC master probe false-alarmed; traced in one grep.
- Lane-contention design: combining B+D into one builder (shared MainComponent.cpp +
  build dir) eliminated the races that parallel lanes would have hit; disjoint-file
  parallelism (A∥C) worked flawlessly. Partition by files, not by features.
- Shared-doc consolidation protocol: builders leave shared-file edits uncommitted + flag
  → dedicated doc-sync lane reconciles (caught the 113-vs-114 drift) and commits once.
- Force-add hygiene: force-added files get their own commit + explicit mention (reviewer
  MEDIUM finding on W1-A) — packet template updated expectation.

## CHANNEL HARVEST

- Lane: FOREIGN-REPO secondary (lane B) — zero harmony2 writes this session (verified at
  close: all harmony2 dirty files belong to other sessions). No `.pending` entries.
- Boris idea sweep (R2 backstop): all Boris messages were task directives + 5 product
  decisions (4 triage forks + plan approval) — captured in `.harmony/triage-2026-07-17.md`
  at decision time; no un-captured idea-class statements → no idea-ledger records.
- Learnings routed repo-local per lane B: 2 new gotchas (launch-via-open; defaultFBO
  restore for post-render GPU steps) in `.harmony/gotchas.md`; 6 meta-learnings in
  `.harmony/notebook.md` (2026-07-17 block); session log in `sessions/`.
- For the next PRIMARY (via this handoff, slight-mention only): the
  "combine-contending-lanes" + "shared-doc consolidation" + "stress-test-not-idiom"
  patterns are universal candidates for promotion beyond this repo.
