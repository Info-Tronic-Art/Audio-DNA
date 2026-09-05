<!-- Sibling projection, NOT part of CONTEXT.md — see .claude/skills/normalize/references/context-and-gotchas.md
     "binding-decisions.md Projection" and harmony-references/doc-unification-rules.md R1.1/R4.2.
     ADVISORY only: a stale stamp WARNs at boot, never blocks (R4.1/R4.3). Re-normalize to refresh. -->
synced-at-normalize: 2026-09-04

# Binding Decisions — RealTimeAudio

Currently-binding decisions relevant to this project, projected from Harmony_Main's primary
memory (R1.1 classes) plus this repo's own equally-binding local rulings. This is a PROJECTION —
no rule below was authored here; each is cited to its source of record, and anything ambiguous is
flagged as such rather than resolved.

## SYSTEM-level (from Harmony_Main memory — binds by R1.1)

- **Session role (primary/secondary) is a pure function of boot, fixed for the session's life; no
  in-session secondary→primary promotion exists.** A fresh boot is required to acquire a primary
  seat. Citation: `memory/DECISION_LOG.md`, heading "2026-06-13 — Remove secondary→primary
  in-session promotion [PERMANENT]".
- **Operating model: the primary works only on Harmony itself; secondaries/tertiaries (sessions
  like this one, in a specific repo) are for that repo's own work and code-building — "the fruit
  of Harmony."** Citation: `memory/DECISION_LOG.md`, heading "2026-08-22 (s128) — The primary's
  job, net-negative sessions, and retirement as a duty [PERMANENT]".
- **The party that builds is never the party that verifies: when a Builder finishes, Harmony runs
  the behavioral gate herself and dispatches an independent Reviewer to read the source.** Tester
  is optional (browser-heavy QA only). Citation: `memory/PREFERENCES.md`, heading "2026-06-17 —
  \"You build, I test\" — SUPERSEDED 2026-08-22 (s128)" (current rule stated there), corroborated
  by the bullet "Parallel Tester dispatch — SUPERSEDED 2026-08-21 (s126)" in the same file's
  Confirmed Preferences list. Reaffirmed project-locally — see PROJECT-level section below.
- **Autonomous operation is the permanent session mode: work the backlog with no human in the
  loop, and any friction that impedes unattended operation is fixed in the system on sight, never
  merely worked around.** Where a rule requires Boris, an autonomous path must be defined rather
  than waited on. Citation: `memory/DECISION_LOG.md`, heading "2026-08-18 — DIRECTIVE [PERMANENT]:
  AUTONOMOUS OPERATION IS THE PERMANENT SESSION MODE".
  - Same entry, load-bearing corollary on code changes: when code is added, code it replaces must
    be fully removed (not left alongside it), and a deeper pass must remove code that became
    orphaned as a *consequence* of the change even if it never appeared in the diff. STATUS on
    that corollary is logged as NOT-IMPLEMENTED (as a system-wide enforced gate) — treat as
    standing intent, not a proven mechanical gate.
- **A builds-never-verifies method (parallel generation / serial integration, Reviewer-on-source +
  Harmony-on-behavior, disk-verified fences, one-change-per-commit) was ruled worth making durable
  system-wide.** Citation: `memory/DECISION_LOG.md`, heading "2026-08-13 — [BORIS] DIRECTIVE:
  integrate the s93 working method into how Harmony works" (full method at
  `memory/specs/s93-parallel-verified-build-method.md`). AMBIGUOUS: this entry is a directive, not
  tagged `[PERMANENT]`, and its cited integration targets are system-doc placements inside
  Harmony_Main (kernel, execution-protocol, etc.) — whether it *itself* still binds a foreign
  project session by R1.1's "system/cross-project scope" disjunct, versus only its already-adopted
  descendant (the builds-never-verifies rule above, which IS carried in PREFERENCES.md), is a
  judgment call this projection does not resolve.
- **Model/compute tier convention for dispatched work (fleet-level): reviewer and researcher roles
  run on the lighter tier; the main/decision-making loop runs on the stronger tier.** Citation:
  `memory/DECISION_LOG.md`, heading "2026-05-29 — Opus 4.8 fleet switch + re-baseline (GO)
  [PERMANENT]". AMBIGUOUS/WEAK: this entry governs Harmony's own fleet pins, not RealTimeAudio's
  dispatch directly — included because a RealTimeAudio session that dispatches its own
  builder/reviewer agents may be expected to follow the same reviewer-stays-lighter-tier shape; not
  a confirmed requirement for this repo. A companion RULING E ("councils that decide get the
  strong tier") at `memory/DECISION_LOG.md` "2026-08-15 — [BORIS-DELEGATED] s100 council rulings"
  is explicitly logged STATUS: NOT-IMPLEMENTED — do not treat it as binding.
- **This repo's own project map is stale past its re-normalize kill-date and is a named, live
  risk: do not trust `.harmony/FEATURES.md` / surface maps here without re-verifying against
  disk.** Citation: `memory/KNOWN_RISKS.md`, heading "Re-normalize backlog — all 7 registered
  projects past the kill-date" (RealTimeAudio named explicitly: "HEAVY, 116 feature-affecting
  files, 121 commits"; re-seen unchanged through 2026-09-03). NOTE ON SOURCE: this is a
  KNOWN_RISKS entry, not a `memory/DECISION_LOG.md` entry — included because R1.1 (which this
  projection is scoped to) names "a KNOWN_RISKS entry that NAMES the project" as its own, separate
  binding class, and this one names RealTimeAudio directly.
- **Down-channel / project-boundary convention: the Harmony primary is the sole writer that
  appends new records into a project's `.harmony/inbox.md`; the project session only flips
  existing records' status lines (single-writer-per-direction).** Citation for the DECISION_LOG
  mention: `memory/DECISION_LOG.md`, heading "2026-09-02 - [HARMONY] s158 RULINGS (the drain and
  two Boris project tasks; Fable main loop)" — bullet "Boris mid-session tasks," which states
  Phase 3 of normalize "stays Boris-gated in each secondary" (RealTimeAudio named) and that
  down-channel records were routed to all four projects that session. The finer single-writer
  mechanics are stated in this repo's own `.harmony/inbox.md` header comment (citing
  `C-boundary-ruling.md` s148 §2b), which is a spec file, not a DECISION_LOG entry — flagged here
  since step 3's source was DECISION_LOG.md and this detail lives one hop away from it.
  AMBIGUOUS: whether "Phase 3 stays Boris-gated" constrains THIS write (an in-flight Phase 3
  normalize projection) is not resolved by this projection — it is stated as read, not
  interpreted.

**Checked for and NOT found in `memory/DECISION_LOG.md` or `memory/PREFERENCES.md` (two
independent grep patterns each, so treat as a real absence, not a missed search):** any
system-level rule about screen-safety / owner-attended gates (this class exists only as a
PROJECT-level rule below — see SCREEN-SAFETY LAW), and any system-level rule assigning a specific
compute tier to a *secondary session's own* dispatches (only the general fleet-pin convention
above exists, plus one NOT-IMPLEMENTED proposal noted above).

## PROJECT-level (this repo's own rulings — equally binding, local scope)

- **SCREEN-SAFETY LAW — mandatory, every session, no exceptions.** Audio-DNA's output window is a
  real fullscreen window on Boris's actual monitors, not a headless test artifact. Never end a
  session with it open; never `pkill`/SIGKILL the app while it is open (close the window first,
  quit gracefully); verify the SCREEN itself (`screencapture -x` + read the image), not just the
  process table, before declaring a session safe to close; state the app/window state explicitly
  in the handoff; minimise unattended fullscreen drive in automated gates; a Boris-reported screen
  artifact outranks whatever lane is running. Citation: `.harmony/HANDOFF.md`, heading
  "SCREEN-SAFETY LAW — MANDATORY, EVERY SESSION, NO EXCEPTIONS".
  - Owner-attended-gate corollary, from the latest (2026-08-04d) session: **output-window
    ("L-OUT") gates are OWNER-ATTENDED ONLY** — ask Boris to drive the output test himself; never
    let an automated gate open fullscreen unattended. Citation: `.harmony/HANDOFF.md`, section
    "SESSION 2026-08-04d (secondary, slim) — THE SURFACE MAP + THE PLAN. AUTHORITATIVE OVER ALL
    ABOVE.", subsection "NEXT SESSION — START HERE", item 5.
- **Boris's binding scope for this build arc: get it working, then upgrade — ESSENTIALS ONLY
  (every main surface does what it presents itself as doing; no upgrades, no polish; anything that
  improves an already-working surface is parked).** Citation:
  `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", preceding line "BORIS'S SCOPE,
  BINDING."
- **Output = Strategy A: native output must show real deck content** (rejected alternative:
  declaring Syphon the supported output path — that fixes the headline surface by redefining it
  away). Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", item 1 under
  "SETTLED"; reaffirmed at `.harmony/HANDOFF.md` section "SESSION 2026-08-04d…", heading
  "BORIS'S RULINGS THIS SESSION", bullet "Output = Strategy A."
- **Display targeting: KEEP the surface, simplify the UI only** — do not delete it (it is the
  app's only path to a projector). Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading
  "RULINGS", item 2 under "SETTLED"; reaffirmed at `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS
  THIS SESSION", bullet "Display targeting: KEEP, simplify the UI only."
- **Legacy v1 row-1 controls + the 10 preset slots: DELETE.** Citation:
  `.harmony/essentials-plan-2026-08-04d.md`, heading "RULINGS", item 3 under "SETTLED"; reaffirmed
  at `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS THIS SESSION", bullet "Legacy v1 row-1
  controls + 10 preset slots: DELETE."
- **The Honesty batch (hide/remove Record tab, Timing placeholder, Comp-Inspector dead blocks,
  dead param-source trio) and the Rack (`EffectsRackPanel`) deletion are OPEN, not yet Boris-ruled**
  — both are recommended yes/DELETE by Harmony and the architect, but the plan and the handoff both
  mark them explicitly STILL OPEN. Citation: `.harmony/essentials-plan-2026-08-04d.md`, heading
  "RULINGS", section "OPEN"; `.harmony/HANDOFF.md`, heading "BORIS'S RULINGS THIS SESSION", bullet
  "STILL OPEN: honesty batch · the rack." AMBIGUOUS BY DESIGN — cited as open, not resolved here.
- **The party that builds never verifies** is independently reaffirmed at the project level (not
  only inherited from system PREFERENCES.md above). Citation: `.harmony/HANDOFF.md`, section
  ">>> BIRTH PROMPT FOR NEXT SESSION (paste this) — supersedes every earlier one in this file",
  "METHOD NOTES THAT KEEP PAYING", bullet "Delegate recon; run the behavioral gate yourself. The
  party that builds NEVER verifies."
- **Repo-local gotcha overriding a general habit: re-grep by anchor text, never trust a cited line
  number — line numbers in this repo drift constantly, evidenced repeatedly this arc.** Citation:
  `.harmony/HANDOFF.md`, section ">>> BIRTH PROMPT FOR NEXT SESSION…", paragraph beginning
  "INHERITED FACTS IN THIS REPO HAVE A BAD TRACK RECORD."
- **A negative from a grep is only as strong as its pattern** — a wrong pattern produced a false
  "no resolution lock exists anywhere" claim this arc (the symbol was `setLockedResolution`, the
  grep looked for `lockResolution`). Citation: `.harmony/HANDOFF.md`, section "SESSION
  2026-08-04d…", heading "CORRECTIONS — including one of Harmony's own", item 1.
- **`.harmony/` is gitignored with many files force-tracked; `git add` prints an "ignored" WARNING
  and still stages, which breaks `&&` chains — use `;` instead.** Citation: `.harmony/HANDOFF.md`,
  section ">>> BIRTH PROMPT FOR NEXT SESSION…", RIG paragraph (final bullet before the stale-clone
  warning).
- **`~/projects/RealTimeAudio copy` is a stale duplicate repo (HEAD `f128bdc`, Jul 11) — confirm
  you are in the real one; HEAD should descend from `7d3a203`.** Citation: `.harmony/HANDOFF.md`,
  same RIG paragraph, final sentence.

## Not found / explicitly out of scope for this projection

- No system-level DECISION_LOG or PREFERENCES entry assigns RealTimeAudio (or foreign-repo
  secondaries generally) a specific model/compute tier for their own dispatched work — see the
  weak/ambiguous fleet-pin citation above; do not infer a firm rule beyond what is cited.
- No system-level "project gotchas override system defaults" rule text was found in
  `memory/DECISION_LOG.md` under that framing (checked via "project.local", "override.*default",
  "local gotcha", "repo-local.*wins" and "project convention" — no hits). The closest system
  mechanism is R1.2 in `harmony-references/doc-unification-rules.md` ("a secondary READS
  [advisory classes] for context but is not constrained by them") plus the project's own
  gotchas file taking precedence in practice for this-repo-specific facts (e.g. the line-number
  and force-tracked-`.harmony/` items above) — stated as observed practice, not as a cited
  system rule.
