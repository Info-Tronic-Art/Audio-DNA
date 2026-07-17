# Handoff to Claude Code: Integrate Audio-DNA Design Directive

## Context

I've completed an extensive design session and produced a comprehensive design directive document: `AUDIO_DNA_DIRECTIVE_FULL.md`.

This document is now the **source of truth** for the Audio-DNA VJ application's UI/UX architecture, signal system, Hit system, visual design language, and layout structure. It supersedes any conflicting decisions in the existing codebase, mockups, or other documentation.

Your job is to **integrate this directive into the repo as the authoritative reference, then reconcile what already exists against it.**

---

## Step 1: Read the directive

Start by reading `AUDIO_DNA_DIRECTIVE_FULL.md` end-to-end before doing anything else. Don't skim. Pay particular attention to:

- **Part 1 (Signal System)** — the three-tier model, triangle gateway, vertical signal columns, the universal accent color
- **Part 2 (Hit System)** — the new Hit primitive (replaces older "Q" or "click" concepts), three-category payload (Signals/Clips/Macros), pill visual representation
- **Part 4 (Visual Design)** — color system (final accent is `#00d9ff` cyan/blue), typography, hard visual rules (no circular knobs, no border-radius, etc.)
- **Part 9 (Resolved Decisions Log)** — quick scan of every locked-in decision

---

## Step 2: Inventory what's in the repo

Survey the codebase:

1. List all top-level directories and identify what each contains
2. Find any existing design documentation (other .md files, README, design notes)
3. Locate the implementation code (whatever exists of the actual app — components, stylesheets, etc.)
4. Note: HTML mockups in `/Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/` are **historical design explorations** that informed the directive — these are references, NOT implementation. Do not modify them unless I explicitly ask.

Produce a brief inventory report before making changes.

---

## Step 3: Identify conflicts with the directive

Compare what exists against the directive. Specifically look for:

### Color conflicts
The directive's final accent is `#00d9ff` (cyan/blue). Earlier iterations used:
- `#ff4500` (orange) — RETIRED
- `#5fdba7` (mint/Resolume green) — REPLACED by blue

Find and flag any references to these old accent colors in CSS, design tokens, or code.

### Naming conflicts
The directive uses "Hit" for timed cues. Earlier explorations may have used:
- "Q" or "Qs"
- "Click" or "click track"
- "Cue" (note: "cue point" is still valid for clip-internal markers — distinct from "Hit")

Find and flag any usage of these older terms in code, comments, or docs.

### Control conflicts
The directive mandates **zero circular knobs** — every knob is a vertical signal column. Find any circular knob implementations and flag them.

### Architectural conflicts
Look for any code or docs that contradict:
- The three-tier signal model (Source → Signal → Macro/Signal Group)
- The three-scope composition (Global / Layer / Clip)
- The Hit capture workflow (single method: snapshot composition state at playhead, then filter)
- The diff-based override semantics (Hits are partial state changes, not replacements)

Produce a conflict report listing every discrepancy with file paths and line numbers.

---

## Step 4: Propose a reconciliation plan

Before changing code, give me a structured plan:

1. **Direct fixes** — things that are clearly outdated and should just be updated (e.g., color tokens, naming)
2. **Architectural changes** — bigger restructurings that align code with the directive (e.g., replacing knob components with signal columns)
3. **Ambiguous items** — places where the directive doesn't fully cover what exists, or where multiple interpretations are valid — flag these for me to decide

Don't make sweeping changes without my sign-off on the plan.

---

## Step 5: Execute approved changes

Once I approve the plan, execute it. Work in focused commits, one logical change per commit, with clear messages referencing the directive section number (e.g., "Update accent to #00d9ff per directive 4.1").

---

## Constraints

- **Do not modify mockup HTML files** in `/Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/`. They are historical design references that inform the directive — they should not be "fixed" to match it. Their purpose is to illustrate the visual quality target.
- **Preserve git history** — make changes through commits, not by rewriting files wholesale where possible.
- **When the directive is ambiguous, ask me.** Do not guess. The directive's Part 10 (Open Questions) lists known ambiguities — additional ones may surface during integration.
- **Treat the directive as immutable in this session.** If you think the directive itself needs revision, raise it with me — do not modify the directive file unless I explicitly ask.

---

## Deliverables

1. **Inventory report** — what's in the repo
2. **Conflict report** — what contradicts the directive (file paths + line numbers)
3. **Reconciliation plan** — proposed changes in priority order
4. **Execution** — once approved, the actual code changes
5. **Final summary** — what was changed, what remains, any open items

Begin with Step 1.
