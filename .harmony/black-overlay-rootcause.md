# BLACK OVERLAY — ROOT CAUSE CONFIRMED (2026-08-03b)

**STATUS: root cause CONFIRMED, source-verified end to end and corroborated by a
prediction Boris confirmed after the fact. Not a hypothesis.**

Boris, verbatim: *"the audio dna app is putting a black overlay on all my screens
except for the one's that are full screen."*

---

## THE CHAIN (every link read from source, 2026-08-03)

1. `src/ui/OutputWindow.cpp:337` — `setAlwaysOnTop(true);`
2. JUCE **8.0.4** (fetched, not in-tree: `CMakeLists.txt:24-33` `GIT_TAG 8.0.4`; read at
   `build/_deps/juce-src/`, identical md5 across build/, build-asan/, build-tsan/) maps
   always-on-top to `NSFloatingWindowLevel` —
   `modules/juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm:595-601`.
3. JUCE **never sets an NSWindow Spaces-participation bit.** Its sole `collectionBehavior`
   source is `juce_NSViewComponentPeer_mac.mm:473-489`, returning `FullScreenPrimary` only
   for `(windowHasMaximiseButton | windowIsResizable)`, else bare `fullScreenAuxiliary`.
   `CanJoinAllSpaces` / `Managed` / `Transient` appear **nowhere** in the JUCE tree.
4. OutputWindow's flags are `windowAppearsOnTaskbar | windowHasDropShadow` — not temporary,
   not resizable, 0 title-bar buttons — so it takes the `fullScreenAux`-only branch.
5. Apple `NSWindow.h` (Xcode 16 SDK): *"If neither is specified, the window gets the default
   behavior determined by its window level."* / *"Transient — Floats in spaces, hidden by
   exposé. Default behavior if windowLevel != NSNormalWindowLevel."*

=> Non-normal level + no participation bit => macOS defaults the window to **Transient =
floats in spaces**. It follows the user onto **every desktop Space** as an opaque black
rectangle, and is absent from native-fullscreen Spaces (no `fullScreenAuxiliary`
participation) — which is Boris's sentence word for word.

**BORIS-CONFIRMED PREDICTION (the discriminator).** Levels: floating=3, Dock=20, menu
bar=24, so the chain predicts the menu bar and Dock stay VISIBLE above the black. Asked
before he answered; he confirmed **menu bar + Dock stayed visible**. A black covering those
too would have refuted the chain.

**Why "screens" was a red herring for two sessions:** Boris had only ONE display attached,
and "Displays have separate Spaces" is ON (he checked the UI). "All my screens" meant
**Spaces**, never monitors. Nothing multi-display was ever involved.

## WHY THE OPAQUE BLACK
`OutputWindow.cpp:92-100` — `OpenGLHelpers::clear(black)` then early-return when
`!texMgr_.hasImage()`. With no clip loaded it is a pure black rectangle every frame,
forever. Peer sets the window opaque (`juce_NSViewComponentPeer_mac.mm:258`). Black also
comes from the `DocumentWindow` background (`OutputWindow.cpp:295`) and
`OutputWindow.h:103-106`.

---

## CORRECTION — THE PREVIOUSLY PRESCRIBED FIX IS ALREADY IMPLEMENTED

The prior handoff prescribed: *"use a borderless always-on-top window sized to the target
display instead of native fullscreen, which is what Resolume/MadMapper do."*

`OutputWindow.cpp:328-342` **already does exactly that**, with a comment explicitly
rejecting native fullscreen:
```
auto area = display.totalArea;
// Don't use native macOS fullscreen — it creates a new Space and
// the GL context transition can fail. Instead, cover the display
// with a borderless window and set it always-on-top.
setVisible(true); setBounds(area); setAlwaysOnTop(true); toFront(true);
```
`setFullScreen` / `setKioskModeComponent` appear **nowhere in `src` at HEAD or anywhere in
git history** (`git log -S`, empty). Bounds are one display's `totalArea`, never a union;
only one window ever exists.

**Implementing that recommendation would have changed nothing and preserved the bug —
"always-on-top" IS the defect.** Recorded because a plausible-sounding remedy that is
already in the tree is exactly the kind of inherited instruction this repo keeps shipping.

---

## THREE WAYS OUTPUT OPENS WITHOUT THE OUTPUT MENU (all verified)

- **Cmd+F** — `MainComponent.cpp:2302-2315` -> `openOutputOnDisplay(0)`. Cmd+F is the
  universal Find chord. Highest-probability accidental trigger.
- **Deck load restore** — `MainComponent.cpp:2731-2733`, `deck.outputDisplay > 1` ->
  `setSelectedId(..., sendNotificationSync)` -> `onChange` (`:329-335`) ->
  `openOutputOnDisplay()`. **Any `.deck.json` saved while output was ON re-blackens the
  screen on load**, with no user action. Default is 1/Off (`src/ui/PresetManager.h:58`).
- **TopBar display combo** (`MainComponent.cpp:503-509`) and legacy `displaySelector_`.

RULED OUT as triggers (verified): Syphon (`MainComponent.cpp:4433-4438` flips an atomic
only; Syphon binds the MAIN preview context), key/MIDI bindings (`src/binding/Binding.h:24-45`
has no output action), API routes, autopilot, test-mode. `src/model/Composition.h:103`
`outputDisplay` is serialized but never applied — dead field.

## THE DISMISSAL TRAP (why it felt unkillable)
0 title-bar buttons (`OutputWindow.cpp:297`). Escape works only if the OUTPUT window has
focus (`OutputWindow.cpp:349-358`) or the MAIN window does (`MainComponent.cpp:2253-2261`).
Click any other app and focus leaves Audio-DNA; the opaque window covers the display, so the
main window cannot be clicked to regain focus. **This is a safety defect, not a nicety.**

## HIDE-vs-DESTROY SPLIT
Menu "Disabled" / Cmd+F / combo all DESTROY (`MainComponent.cpp:4393-4395` -> `closeOutput()`
`:2523-2530`). The two IN-WINDOW dismissal paths only HIDE (`OutputWindow.cpp:316-320`,
`:349-358`): GL context stays attached and repainting, MainComponent is never notified, menu
and combo state desync from reality.

## ORDERING BUG
`OutputWindow.cpp:328-342` calls `setVisible(true)` BEFORE `setBounds(area)` — a fresh window
flashes at 0x0.

## NO OS-LEVEL ARTIFACT SURFACE
No `NSWindow`/`CGDisplay`/`CGShield`/kiosk calls in `src`; the only `.mm` is
`src/output/SyphonOutput.mm`. Nothing can outlive the process — consistent with Boris's
report that the overlay is gone once the app quits. **This falsifies the prior handoff's
claim that "the overlay survives a GRACEFUL EXIT ⇒ teardown/ownership defect."** That was an
inference from "no process was running when he reported it," recorded as verified fact.

---

## BORIS RULING (2026-08-03, product call, his)
**"Stay on top, but ONLY on a projector / second display."** Non-main display keeps
always-on-top so nothing covers projector output mid-set; MAIN/laptop display gets a normal
window (normal level -> Managed -> bound to one Space).

## HARDEST OPEN DESIGN PROBLEM — DISPLAY HOT-PLUG
Under that ruling the level depends on WHICH display the window is on. Unplug the projector
mid-set, or plug one in while output is open, or let macOS re-arrange displays, and an
always-on-top window can migrate to the laptop display — **reproducing this exact bug**. The
level must be RE-EVALUATED on display change. A fix without that is a fix that comes back.

## NOT DURABLE — do not "fix" it this way
Overriding `collectionBehavior` directly on the NSWindow does not stick:
`juce_NSViewComponentPeer_mac.mm:1594-1600` `resetWindowPresentation()` re-applies JUCE's
own value.

## PROVENANCE
`build/AudioDNA_artefacts/Release/Audio-DNA.app` mtime Aug 3 12:20 vs HEAD `c51aff7` 12:29 —
the binary Boris ran contains this same code. `src/` working tree clean at time of recon.

---

# RESOLUTION + CORRECTIONS TO THIS FILE (2026-08-03b) — AUTHORITATIVE

## SHIPPED: `5a580c8` fix(output-window): normal window level
`setAlwaysOnTop(true)` deleted from `goFullscreenOnDisplay`; `setBounds` moved before
`setVisible`; comment rewritten to state the window-level mechanism.
Probe harness: `cab003a`.

**Full-tier gate, both halves closed:**
- FAIL-FIRST **measured on the running app**: `kCGWindowLayer == 3` before, **0** after
  (`.harmony/ow-level-fail-first.log`, `tests/visual/test_output_window_level.py`).
- Independent ctest **193/193**, run by Harmony — not the builder's claim.
- Forced Release rebuild: 0 errors, **0 new warnings**, measured by compiling both versions
  as standalone TUs rather than asserted; binary sha256 confirmed relinked.
- Independent source review: **PASS, 0 blocking, 3 minors** (M1 folded in pre-commit).
- **Boris confirmed the symptom is gone**: "when I go through the other apps open, the black
  screen only covers audio dna."

**What his original words actually meant** (worth recording — it misled two sessions):
"all my screens except the ones that are full screen" was never about monitors, and not
really about Spaces either. He was describing **his open apps** — the floating window sat
above every normal window and could not reach native-fullscreen apps. Reading "screens" as
displays sent two sessions hunting a multi-display bug that never existed.

## CORRECTION 1 — THE BORIS RULING IN THIS FILE IS WITHDRAWN
The section above recording **"Stay on top, but ONLY on a projector / second display"** is
**SUPERSEDED**. Boris withdrew it the same session, after the architect showed always-on-top
buys ~nothing on a projector (with separate Spaces ON the projector holds its own Space with
nothing else on it, so floating and normal are indistinguishable there). **FINAL RULING:
drop always-on-top UNCONDITIONALLY. No conditional, no toggle.** This is what shipped.
Consequently the **"HARDEST OPEN DESIGN PROBLEM — DISPLAY HOT-PLUG"** section above is MOOT:
with no on-top state anywhere, there is nothing to re-evaluate on display change.

## CORRECTION 2 — "ORDERING BUG … flashes at 0x0" IS FALSE
A fresh OutputWindow is **never 0x0**. `DocumentWindow`'s ctor calls
`setResizeLimits(128, 128, …)` (`juce_DocumentWindow.cpp:69`) which ends in
`setBoundsConstrained` (`juce_ResizableWindow.cpp:311`), and `checkBounds` clamps
unconditionally when not stretching (`juce_ComponentBoundsConstrainer.cpp:189,194`).
**The floor is 128x128.** And the real consequence is worse than a flash: `canBeAttached`
(`juce_OpenGLContext.cpp:1163-1176`) binds on VISIBILITY, and size is always satisfied
because of that same floor — so showing first **created the GL context at 128x128 and then
resized it**. Bounds-first attaches once, already at display size.
**This false claim propagated from this file into a code comment before review caught it.**
It was written here as an inference and read downstream as fact.

## CORRECTION 3 — "NOT DURABLE — do not fix it this way" IS WRONG
The claim above that overriding `collectionBehavior` natively would not stick is FALSE for
this app: `resetWindowPresentation` (`juce_NSViewComponentPeer_mac.mm:1592-1601`) has exactly
two callers (`windowDidExitFullScreen:` `:2786-2790`, kiosk-disable `:2997`) and **neither is
reachable here** — the app never enters native fullscreen or kiosk. A native override WOULD
persist. Not needed for the shipped fix; relevant only if an on-top toggle is ever added.

## STILL OPEN (own tickets, NOT regressions from `5a580c8`)
1. **"Fullscreen" does not actually cover the display.** JUCE's peer calls AppKit's
   `constrainFrameRect:toScreen:` super FIRST (`juce_NSViewComponentPeer_mac.mm:2756-2763`),
   clamping below the menu bar. Measured: output at `x=0 y=38 1728x1117` on a 1117-tall
   display — **the bottom ~38px hangs off-screen**. Projector output has been getting cropped.
   Pre-existing, identical at floating level.
2. **COMMIT B — dismissal paths still only HIDE.** `OutputWindow.cpp` `closeButtonPressed`
   and the Escape branch leave a hidden window with a live continuously-repainting GL context,
   and both display combos still believe output is ON — so a deck saved after an in-window Esc
   records output ON and re-blackens on load. Packeted in
   `.harmony/.work-packets/black-overlay-fix.md`. **Re-grep line numbers: the dismissal
   no-ops moved 318/353 -> 318/363 across this session's comment edits.**
3. **Display combo lists never refresh after startup** (`refreshDisplayList` runs once at
   construction) — plug/unplug mid-session and the combos are wrong while the MENU stays
   correct, so they silently disagree.
4. **M2 (cosmetic):** `toFront(true)` is redundant — `TopLevelWindow::visibilityChanged`
   already calls it on show.
