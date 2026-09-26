# Completeness Critic — CLAUDE.md split (s-rta-0926)

VERDICT: PASS

## Checks performed (independent re-derivation, not recall)

1. **No-loss line check (re-run independently)**: wrote my own set-membership
   script (`orig` = `git show HEAD:CLAUDE.md`, 1264 lines / 952 non-blank vs
   `CLAUDE.md` + all 12 `docs/claude/*.md` files). Result: 941/952 matched
   verbatim, 11 differences — all 11 individually inspected and confirmed to
   be legitimate, substance-preserving edits already declared by the Builder:
   - Key-capabilities line (14): trimmed but every capability noun-phrase
     (counts, ports, callback wiring) preserved; verified the trimmed
     `docs/claude/history.md:52` copy also survives.
   - 8 cross-reference rewrites (257, 629, 639, 688, 868, 922, 1005, 1187):
     each old inline reference ("see X below", "listed above") rewritten to
     point at the new file (`docs/claude/recording.md`,
     `docs/claude/build-other-platforms.md`, `docs/claude/architecture.md`,
     `docs/claude/pitfalls.md`) and hand-verified each target resolves and
     contains the referenced content.
   - 2 heading-level demotions (526 `### UI Patterns` → `## UI Patterns`,
     1245 `### Updating This Document` → `## Updating This Document`):
     cosmetic only, sections now stand alone in `CLAUDE.md`; content unchanged.
   No unexplained missing line found.

2. **Trigger-table coverage**: every one of the 12 `docs/claude/*.md` files
   has a corresponding row in `CLAUDE.md`'s Trigger Table (lines 282-300)
   naming a concrete, realistic trigger condition (e.g. "Touching
   `FeatureSnapshot`/... struct fields" → architecture.md; "Adding/changing a
   GLSL effect..." → effects.md). Verified each row's target file exists and
   contains matching content.

3. **Pitfalls index**: all 34 pitfalls (verified by grep, numbered 1-34,
   monotonic) present verbatim in `docs/claude/pitfalls.md`, and the
   `Common Pitfalls Index` in `CLAUDE.md` (lines 233-274) has a one-line
   triage entry for every one of the 34, in matching numeric order.

4. **KEEP-list spot check**: Sacred Rules, Shader Rules, Feature Addition
   Rules, Code Quality Rules, UI Patterns, the "kick off phase N" protocol,
   Phase Dependency Map, "Before Any Work", Pitfalls Index, Trigger Table,
   "Updating This Document", Build Essentials (macOS quick-build +
   Prerequisites + Common Build Issues), 4-Thread Model summary, and Latency
   Budget table are all present in `CLAUDE.md` and not truncated (each
   section's full row/bullet count matches the original 1:1 — e.g. Latency
   Budget's 11-row table is intact in full).

5. **No @-import introduced**: `grep -n "^@"` across `CLAUDE.md` and all
   `docs/claude/*.md` returns nothing. Trigger Table explicitly says these
   docs are read on demand, not imported.

6. **Size budget**: `wc -c CLAUDE.md` = 24053 bytes, under the 25000 budget.

7. **External cross-references**: searched every `.md` in the repo
   (excluding `docs/claude/*` and report/session logs, which only mention
   "CLAUDE.md" generically) for references to CLAUDE.md. `PHASE_GUIDE.md`
   and `README.md` reference CLAUDE.md by name only (no section anchors), and
   the two specific claims they lean on — the "kick off phase N" protocol and
   "Step 8" (Post-phase documentation) — both still exist verbatim in
   `CLAUDE.md` (lines 152, 196). No broken anchor found.

## Minor observation (not blocking, SHOULD only)

`### Naming Conventions` (files/classes/methods/shader-uniform naming rules)
moved to `docs/claude/architecture.md` but has no dedicated word in its
Trigger Table row (the row covers "source tree/file layout" broadly, which is
a reasonable but not explicit match). A future split could add "naming
conventions" explicitly to that row's trigger phrase for discoverability.
This does not constitute content loss — the content is present and the
existing trigger row is plausible enough that an agent editing file/class
names would likely open architecture.md anyway.

## Verdict

No MUST-level finding: no content lost, no KEEP-list item missing/truncated,
every moved section has a usable trigger, no @-import, size within budget,
cross-references resolve.
