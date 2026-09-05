# WORK PACKET — L1 · Media-leak family (`Clip > Clear` strands a decoder, permanently)

**TASK:** Make `Clip > Clear` and its sibling vacate commands actually release the media they orphan.
**DEPARTMENT:** engineering · **PROJECT:** Audio-DNA (`~/projects/RealTimeAudio`)
**TIER:** full — touches the undo/command system and GL-thread resource lifetime.
**SOURCE OF TRUTH:** `.harmony/essentials-plan-2026-08-04d.md` § `### L1 — Media-leak family`
(Fable-architect authored, chaired and attacked by Harmony). **Do not re-plan it.**

> **CONFIRM YOUR REPO FIRST.** `~/projects/RealTimeAudio copy` is a STALE DUPLICATE
> (HEAD `f128bdc`, Jul 11). HEAD here must descend from `7d3a203`.

---

## THE DEFECT (established — do not re-derive, but DO re-verify the anchors)
`Renderer::closeMediaForClip` is **2 hits repo-wide** (declaration + definition) with **zero call
sites**, and it holds the ONLY two `.erase()` calls on the media maps. `Deck::clearCell` never
touches the Renderer. `s_nextClipId` is monotonic with **no reuse**.

⇒ every Clear on a video or image-sequence strands an FFmpeg decoder + a GL texture + a decode
thread **for the life of the process**. Unbounded, and it accumulates in exact proportion to how
much clip churn a set produces.

**Re-verify the zero-call-sites claim yourself, with at least two different grep patterns, and
quote both.** A negative from a grep is only as strong as its pattern — a prior session in this
repo asserted a symbol did not exist because it searched `lockResolution` when the symbol was
`setLockedResolution`.

## THE THREE TRAPS — this is why "just call the existing function" is wrong
The naive "it's one call" sizing was wrong in three independent ways. All three are load-bearing.

**(a) DOUBLE-APPLY.** The handler pre-mutates, and then `UndoManager::perform` re-applies. By
execute time the cell is **already empty**, so any dispose keyed off live cell state fires
**never**. ⇒ **Dispose MUST key off the command's `before_` / `after_` snapshots, not off the
live model.** Read `UndoManager`'s perform path before writing a line of this.

**(b) GL-THREAD DESTROY.** `~VideoPlayer` calls `glDeleteTextures` with **no context current**.
⇒ needs a **mutex-guarded retire list drained on the GL thread**. There is an existing precedent
for exactly this pattern in `Renderer.cpp` near the source-disposal code — **find it by anchor
text and follow it.** Do not invent a second disposal mechanism alongside it.

**(c) FAMILY COVERAGE.** `ClearLayerClipsCmd` and `RemoveColumnCmd` vacate cells identically.
A fix that only covers `Clip > Clear` is a half-fix that will gate green and still leak.

## WHAT IS ALREADY SAFE — do not "fix" it
**Undo is already handled.** `ClipMediaHook` was built for exactly this case. Read it, confirm it,
and say in your report what it already guarantees so the gate does not re-litigate it.

## THE RISK YOU MUST GUARD
Disposing still-referenced media. The guard is a **liveness scan** plus the **unique-clip-id
invariant** (no clipboard exists, 6 minting sites, atomic swap).
**This invariant is FUTURE-FRAGILE and you must leave a load-bearing comment saying so: adding a
clipboard feature would break the liveness scan.** A comment is the only thing that will carry
this to whoever adds copy/paste.

## HARD CONSTRAINTS
- **DO NOT BUILD, DO NOT RUN ctest, DO NOT LAUNCH THE APP.** Harmony compiles and runs the
  behavioral gate herself — the party that builds never verifies. Because you cannot compile,
  re-read your own diff line by line before reporting.
- **DO NOT run any git write command.** Harmony integrates and commits serially.
- **SCREEN-SAFETY LAW** (`.harmony/HANDOFF.md`): never open the output window, never `pkill` the app.
- **Line numbers drift constantly — RE-GREP BY ANCHOR TEXT.**
- **Check whether this is already implemented** before building it. This repo has burned a session
  on re-implementing a fix that was already in the tree.

## OUT OF SCOPE
Composition persistence (L3 — depends on this lane's mechanism, but is a separate lane), the
output window, anything in `src/ui/`.

## FAIL-FIRST (Harmony captures this, not you — but tell her what to look for)
`lsof` on the app shows the media file handle **still open after Clear** today; after the fix it
must be **closed**, including across the full cycle **clear → undo (plays again) → redo (closed)**.
If you believe a cheaper or more reliable oracle exists, name it and say why.

## SUCCESS CRITERIA
1. Handle closed after Clear; still correct across clear→undo→redo.
2. All three vacate paths covered (`Clip > Clear`, `ClearLayerClipsCmd`, `RemoveColumnCmd`).
3. No GL call on a thread with no context current.
4. ctest 203/203 (re-run, never inherited). Release build: 0 errors, no new warnings.

## REPORT FORMAT
`STATUS` · `FILES CHANGED` · `WAS IT ALREADY IMPLEMENTED?` · `THE ZERO-CALL-SITES RE-VERIFICATION`
(both grep patterns + output) · `HOW I KEYED DISPOSE OFF SNAPSHOTS` (trap a) ·
`THE RETIRE-LIST MECHANISM AND WHICH PRECEDENT I FOLLOWED` (trap b) ·
`FAMILY COVERAGE — the three paths and how each is handled` (trap c) ·
`WHAT ClipMediaHook ALREADY GUARANTEES` · `WHERE I PUT THE FUTURE-FRAGILE COMMENT` ·
`WHAT I COULD NOT VERIFY` (exhaustive — you could not compile) · `RISKS I AM HANDING TO THE GATE`
