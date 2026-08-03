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
