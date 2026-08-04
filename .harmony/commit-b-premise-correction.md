# COMMIT B — PREMISE CORRECTION (2026-08-04, session c)

**Status: Harmony-verified on disk, not relayed.** Every line below was confirmed by
direct grep/read of the working tree at HEAD (`20fc53d`) by Harmony after a recon agent
flagged the contradiction. Recon findings that Harmony did NOT independently re-verify are
marked [RECON-ONLY].

## 1. The packet's Commit A is ALREADY IMPLEMENTED — do not rebuild it
`setAlwaysOnTop(true)` is gone from `goFullscreenOnDisplay`; the rationale comment sits at
`OutputWindow.cpp:332-344`; `setBounds` (:345) precedes `setVisible` (:346). This landed as
`5a580c8`. **Rebuilding Commit A is a no-op.** This is the second time in two sessions that
this packet's prescribed work was found already done — the packet was written before the fix
and never revised.

## 2. Commit B is genuinely NOT started — VERIFIED
`grep -rn "requestClose\|onCloseRequest\|handleOutputDismissed" src/` returns **nothing**.
Both dismissal bodies are unchanged and confirmed verbatim:
- `OutputWindow.cpp:316-320` `closeButtonPressed()` -> `setAlwaysOnTop(false); setVisible(false);`
- `OutputWindow.cpp:358-367` `keyPressed(escape)` -> `setAlwaysOnTop(false); setVisible(false); return true;`

Neither notifies MainComponent. Neither destroys. **Both `setAlwaysOnTop(false)` calls are now
DEAD NO-OPS** — post-`5a580c8` nothing ever sets on-top true. Commit B deletes these bodies
anyway, so the no-ops are cosmetic, not a second defect.

## 3. THE HANDOFF'S STATED PREMISE IS WRONG — corrected here
Handoff/birth-prompt claim: *"a deck saved after an in-window Esc records output ON and
re-blackens the screen on load."*

**True on ONE path only, and the severity framing is obsolete.** Deck save reads the HIDDEN
combo `displaySelector_` (`MainComponent.cpp:2640`), and only some paths write it:

| How output was opened/closed | writes `displaySelector_`? | what a saved deck records |
|---|---|---|
| Cmd+F open (`:2312`) | YES -> 2 | ON — correct, and reopens on load (`:2732-2733`) |
| Main-window Esc close (`:2258`) | YES -> 1 | OFF — correct |
| Cmd+F close (`:2307`) | YES -> 1 | OFF — correct |
| **TopBar combo open** (`:503-509`) | **NO** | **OFF — SILENT LOSS** |
| **Output menu open** (`:3770-3775`) | **NO** | **OFF — SILENT LOSS** |
| **In-window Esc / close button** | **NO** | stale — whatever it was before |

So there are **TWO** desync bugs, not one:
- **(i) Silent loss (NEW, not in the packet, and the likelier one to bite Boris).** Opening
  output via the TopBar combo or the Output menu — the two NORMAL ways — leaves
  `displaySelector_` at 1, so a deck saved with output live records **Off**. His performance
  decks silently forget their output setting.
- **(ii) Stale-ON (what the handoff described).** Only reachable when output was opened by
  **Cmd+F** and then dismissed from inside the window. Then `displaySelector_` stays 2 and the
  deck reopens output on load.

**Severity of (ii) has DROPPED since the handoff was written.** Post-`5a580c8` the output
window is normal level, so a surprise restore opens a fullscreen window on one display — it no
longer paints a black overlay across every Space. The words "re-blackens the screen" are stale;
do not carry them forward.

## 4. The packet's stated REASON for the combo fix is also wrong
Packet says the combos "still believe output is ON" after dismissal. In fact **the two combos
are never cross-synced at all**, dismissal or not — `topDisplay` is written only once at
construction (`:501`) and never again; `displaySelector_` is hidden (`:1960`) and written only
at `:2258/:2307/:2312/:2499/:2733`. They disagree from startup onward. Commit B's prescription
happens to help, but a builder reasoning from the packet's stated cause will fix the wrong thing.

## 5. What is actually load-bearing about Commit B

### RETRACTION — the "live GL context" claim below was MINE and it is FALSE
**I originally wrote: "a live GL context repainting forever behind a hidden window ... the GPU
keeps rendering frames nobody sees." That is WRONG. Retracted 2026-08-04 after an architect
challenged it and I verified against the vendored JUCE 8.0.4 source.**

What JUCE actually does (VERIFIED, read directly at
`build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.cpp`):
- `:1163-1166` `canBeAttached(comp)` returns `... && isShowingOrMinimised(comp)`
- `:1168-1177` `isShowingOrMinimised` returns **false** immediately when `! c.isVisible()`
- `:1129-1144` `componentVisibilityChanged()` -> `if (canBeAttached(comp)) {...} else { detach(); }`

So `setVisible(false)` **destroys the GL context**. Hiding wastes no GPU. There is no leak.

**How I got it wrong, recorded because the mechanism matters more than the fact:** I verified the
three things visible in OUR source — attach in the ctor (`OutputWindow.cpp:308`), detach only in
the dtor (`:313`), `setContinuousRepainting(true)` never revoked (`:24`) — and then INFERRED that
hiding therefore leaves it running. I never checked what JUCE does on a visibility change. The
inference was sound-looking and wrong, and I recorded it as VERIFIED in two documents. This is
the same class as last session's "a fresh window flashes at 0x0" — an inference of mine that
reached a code comment. **Verifying the parts of a chain you can see does not verify the chain.**

Corroboration that this path was already known-good in our own code: `OutputWindow.cpp:56-63`
re-queues `lastImageFile_` "after context recreation" — hide/show context churn is a path the app
already handles deliberately.

### What IS actually load-bearing
- **Deck output state silently wrong in BOTH directions** (section 3). This is the whole defect,
  and it is entirely real.
- A hidden-but-alive window object with `outputWindow_` non-null, so the app's notion of
  "output is open" diverges from what the user sees. State, not GPU.

## 6. Gaps — named, NOT filled
- [RECON-ONLY] "no test references OutputWindow" and the ctest 193/193 baseline — unchecked.
- [RECON-ONLY] shutdown path `:1786-1806` — only `:1789` spot-checked.
- The packet's JUCE-internals citations were not re-verified; they are not load-bearing for B.
- Whether Boris actually opens output via combo/menu vs Cmd+F in practice — **unknown, and it
  decides which of the two bugs matters more.** Ask him; do not assume.
