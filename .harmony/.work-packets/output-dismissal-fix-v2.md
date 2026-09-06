# WORK PACKET — OUTPUT WINDOW DISMISSAL + OUTPUT-STATE TRUTH (v2)

**SUPERSEDES `.harmony/.work-packets/black-overlay-fix.md` entirely.** That packet's Commit A is
already shipped (`5a580c8`) and its Commit B premise is factually wrong. Do not read it for
scope. Evidence for every claim here: `.harmony/commit-b-premise-correction.md`.

**STATUS: NOT BUILD-READY. One design fork must be settled by an architect first — see
section 5.** Do not dispatch a builder against this until that is closed.

---

## 1. DO NOT REBUILD COMMIT A — it is already implemented
`setAlwaysOnTop(true)` is GONE from `goFullscreenOnDisplay`; the rationale comment is at
`OutputWindow.cpp:332-344`; `setBounds` (`:345`) precedes `setVisible` (`:346`). Landed as
`5a580c8`, gated, Boris-confirmed. **Building it again changes nothing.** This exact trap has
now cost this repo two sessions.

## 2. WHAT IS ACTUALLY BROKEN (verified at HEAD `20fc53d`)

### 2a. In-window dismissal only hides — and does not tell MainComponent
- `OutputWindow.cpp:316-320` `closeButtonPressed()` -> `setAlwaysOnTop(false); setVisible(false);`
- `OutputWindow.cpp:358-367` `keyPressed(escape)` -> same two calls, `return true;`

Neither notifies MainComponent. Neither destroys the window object. So the app still believes
output is open while the user sees it gone — that divergence is the defect.

**CORRECTION (2026-08-04): an earlier version of this packet claimed the hidden window leaves a
live GL context repainting forever and burning GPU. THAT WAS FALSE and is retracted.** VERIFIED
in vendored JUCE 8.0.4: `canBeAttached()` requires `isShowingOrMinimised()`
(`juce_OpenGLContext.cpp:1163-1166`), which is false once the component is invisible
(`:1168-1177`), and `componentVisibilityChanged()` calls `detach()` in that case (`:1129-1144`).
**Hiding destroys the GL context. There is no GPU leak and no wasted rendering.** The error was
mine: I verified attach/detach/repaint in our own source and inferred the rest without checking
JUCE's visibility handling. Do not reintroduce this claim, and do not let it into a code comment.

Both `setAlwaysOnTop(false)` calls are now DEAD NO-OPS (nothing sets on-top true post-`5a580c8`).
This fix deletes those bodies anyway — do not file them as a separate defect.

### 2b. Output state has NO single source of truth — TWO desync bugs, opposite directions
Deck save reads the HIDDEN combo `displaySelector_` (`MainComponent.cpp:2640`). Only some paths
write it:

| path | writes `displaySelector_`? | deck saved with output live records |
|---|---|---|
| Cmd+F open (`:2312`) | YES -> 2 | ON (correct; reopens via `:2732-2733`) |
| Main-window Esc close (`:2258`) | YES -> 1 | OFF (correct) |
| Cmd+F close (`:2307`) | YES -> 1 | OFF (correct) |
| **TopBar combo open** (`:503-509`) | **NO** | **OFF — SILENT LOSS** |
| **Output menu open** (`:3770-3775`) | **NO** | **OFF — SILENT LOSS** |
| **In-window Esc / close button** | **NO** | stale, whatever it was |

- **(i) SILENT LOSS — not in the old packet, and the likelier one to bite Boris.** Open output
  the two NORMAL ways (TopBar combo, Output menu), save a deck, and it records output **Off**.
  His performance decks silently forget their output setting.
- **(ii) STALE-ON — what the old packet described.** Reachable ONLY when output was opened by
  Cmd+F then dismissed from inside the window. Deck then reopens output on load.

**SEVERITY OF (ii) HAS DROPPED and the old wording must not be carried forward.** Post-`5a580c8`
a surprise restore opens a normal-level fullscreen window on ONE display. It no longer paints a
black overlay across every Space. Any text saying "re-blackens the screen" is stale.

### 2c. The old packet's stated CAUSE is wrong
It says the combos "still believe output is ON" after dismissal. In fact **the two combos are
never cross-synced at all** — `topDisplay` is written once at construction (`:501`) and never
again; `displaySelector_` is hidden (`:1960`) and written only at
`:2258/:2307/:2312/:2499/:2733`. **They disagree from startup onward, dismissal or not.** A
builder reasoning from the old cause will fix the wrong thing.

## 3. REQUIRED OUTCOMES (what "done" means — not how)
1. An in-window dismissal (Escape or close button) converges on the SAME teardown as every other
   close path. No path may leave a hidden window with an attached, repainting GL context.
2. After ANY open or close, by ANY path, the persisted output state matches reality.
3. A deck saved while output is live reproduces that on load — and one saved while output is off
   does not open it.
4. Existing decks keep loading. Boris has saved decks in both broken states; neither may fail.

## 4. NON-NEGOTIABLE CONSTRAINTS
- **SCREEN-SAFETY LAW applies.** The output window is a real fullscreen window on Boris's actual
  monitor. A builder must NOT launch the app. Harmony runs all behavioral gating.
- Do NOT touch `src/binding/` (BindingManager is out of scope — twice falsified).
- Iron rule for this repo: **re-grep every line number by anchor text.** Every number above was
  verified at `20fc53d` and WILL drift.

## 5. THE DESIGN FORK — architect must settle before build
**What should be the source of truth for "is output on"?**

Today a UI widget (`displaySelector_`, itself hidden) is the de-facto truth, which is what
produces both bugs in section 2b. Candidate approaches:
- **(A) Converge on `closeOutput()`/`openOutputOnDisplay()` as the single mutation point**, and
  have them sync every UI surface. Smallest change; leaves widgets as truth-adjacent state.
- **(B) Make the actual window/renderer state authoritative**, have deck-save query it, and drive
  the combos as pure views. Structurally correct; larger blast radius.
- **(C) A dedicated output-state model** owning the display index + on/off, with widgets and deck
  serialization both bound to it. Cleanest; most work.

Trade-offs to weigh: blast radius against recurrence risk (this class of bug has now appeared
three times in this subsystem); how the choice interacts with the display hot-plug problem and
with combo lists that never refresh after startup (`refreshDisplayList` runs once at
construction); and whether (B)/(C) can be done without destabilising a subsystem that was just
stabilised.

**Also for the architect:** whether the dismissal should DESTROY the window or detach-and-hide.
Destroy is simpler to reason about; detach-and-hide may preserve reopen latency that matters on
stage. This is a live-performance tool — reopen cost is a real consideration, not a micro-opt.

## 6. OPEN QUESTION FOR BORIS (do not assume)
**How does he actually open output on a projector — TopBar combo, Output menu, or Cmd+F?** This
decides which of the two bugs in 2b he has been living with. Asked, not yet answered.
