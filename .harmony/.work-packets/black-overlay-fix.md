# WORK PACKET — Black-overlay fix (designed 2026-08-03b, fable max effort)

Root cause + evidence: `.harmony/black-overlay-rootcause.md` (committed `e75436e`).
Line numbers verified at design time and DRIFT-PRONE — builder must re-locate by ANCHOR
TEXT (`setAlwaysOnTop`, `goFullscreenOnDisplay`, `closeOutput`), never by raw offset.

## BORIS RULINGS (2026-08-03b) — these SUPERSEDE the earlier conditional ruling
1. **Drop always-on-top OUTRIGHT. No conditional, no toggle.** (His earlier "on-top only on
   a projector" ruling was revisited and withdrawn once the architect showed on-top buys
   ~nothing there — see WHY below.)
2. **Leave Cmd+F as-is.** Post-fix it is no longer a trap.
3. **Build both lanes** — overlay first, then preset (both touch MainComponent.cpp).

## WHY UNCONDITIONAL BEATS THE CONDITIONAL (the argument that changed the ruling)
With "Displays have separate Spaces" ON (Boris's verified config):
- **Laptop-only (his actual case):** on-top means the output covers the UI on its own display
  AND follows every Space. Pure harm.
- **Laptop+projector, output on projector (stage):** the projector display has its own Space
  with nothing else on it, so a Transient float is **indistinguishable from Managed**. On-top
  buys nothing EXCEPT protection against a window deliberately dragged onto the projector.
  Notifications/menu bar/Dock float above floating level anyway (floating=3 < Dock=20 <
  menu bar=24), so it never protected against those.
- **Conditional cost:** forks behavior across every dock/undock event and is untestable
  without a projector attached. Complexity for a benefit that cannot be demonstrated.
**Accepted failure mode (named, not hidden):** a window deliberately placed on the output
display can cover output. Recovery is one display-combo re-select (`toFront`).

## D2 — CAN JUCE DO "FLOATING BUT SPACE-BOUND"? Honest answer: not in pure JUCE.
VERIFIED: every collectionBehavior write in JUCE 8.0.4 goes through `setCollectionBehaviour`
(call sites `juce_NSViewComponentPeer_mac.mm:280`, `:497`, `:1600`) and can only produce
`FullScreenPrimary` or bare `fullScreenAux`. No Managed/CanJoinAllSpaces/Transient anywhere.
**CORRECTION to the recon's claim that a native override "isn't durable":** it WOULD be
durable in this app — `resetWindowPresentation` (`:1592-1601`) has exactly two callers
(`windowDidExitFullScreen:` `:2786-2790`, kiosk-disable `:2997`) and NEITHER is reachable
here, since the app never enters native fullscreen or kiosk. Not needed for the fix; it is
the right implementation for a future opt-in toggle's ON state, and would need ~5 min of
human verification because the "explicit bits beat the level-derived default" claim is
INFERRED from NSWindow.h, not observed.
Also corrected: JUCE's flag test at `:481` compares `== (maximise|resizable)` — BOTH
required, stricter than the recon's "OR" phrasing. Conclusion unchanged.

---

## COMMIT A — "OutputWindow: normal window level (Space-bound); set bounds before show"
Single file: `src/ui/OutputWindow.cpp`, function `goFullscreenOnDisplay` (~:328-342).
1. **DELETE** `setAlwaysOnTop(true);` (~:337).
2. **REORDER** `setBounds(area);` to come BEFORE `setVisible(true);` (currently setVisible
   ~:335, setBounds ~:336) — fixes GL attaching at the 128x128 constrainer floor then resizing (NOT a 0x0 flash — that claim was false). Rides commit A because A
   rewrites exactly these lines; leaving a known-wrong order while editing the adjacent line
   would be deliberate bug preservation.
3. Keep `toFront(true);` and the trailing `outputComponent_.setBounds(getLocalBounds());`.
4. **REPLACE the comment (~:332-334)** with the real rationale: borderless cover at NORMAL
   level; `setAlwaysOnTop` => NSFloatingWindowLevel with no Spaces-participation bit =>
   macOS Transient default => floats across every Space as an opaque black overlay; normal
   level => Managed => pinned to its Space; native fullscreen still avoided (new Space + GL
   transition risk). The existing comment is correct about fullscreen and silent about the
   thing that actually bit — that silence is what let this survive.
`setAlwaysOnTop(false)` at ~:318 and ~:353 become no-ops; leave them, commit B deletes those
bodies anyway.

## COMMIT B — "OutputWindow: all dismissal paths destroy and resync UI state"
Rides because it is the SAME defect surface and the safety half of the escape hatch: today
the two in-window dismissals leave a hidden window with a live continuously-repainting GL
context, and both display combos still believe output is ON — so a deck saved after an
in-window Esc records output ON and **re-blackens the screen on load**.
- `src/ui/OutputWindow.h`: public `std::function<void()> onCloseRequest;` + private
  `void requestClose();`.
- `src/ui/OutputWindow.cpp`: `closeButtonPressed` (~:316-320) body becomes `requestClose();`;
  `keyPressed` Escape branch (~:349-358) becomes `requestClose(); return true;`.
  New: `void OutputWindow::requestClose() { setVisible(false); if (onCloseRequest)
  juce::MessageManager::callAsync(onCloseRequest); }`
  **Keep the rationale in a comment:** hide first for instant dismissal; destruction is
  DEFERRED via callAsync because the callback destroys THIS object — a synchronous destroy
  inside its own keyPressed/closeButtonPressed frame is use-after-free when the event
  unwinds. callAsync copies the std::function, so it is safe after `this` dies.
- `src/MainComponent.cpp` `openOutputOnDisplay` (~:2502-2521): wire
  `outputWindow_->onCloseRequest` with a `juce::Component::SafePointer<MainComponent>` guard;
  after `goFullscreenOnDisplay`, sync BOTH combos with `setSelectedId(displayIndex + 2,
  juce::dontSendNotification)` (both use id = index+2; TopBar accessor verified `TopBar.h:41`).
- New `MainComponent::handleOutputDismissed()` — `if (outputWindow_ &&
  !outputWindow_->isVisible()) closeOutput();`. The visibility guard makes a stale queued
  close a NO-OP if the user re-opened between Esc and the async hop.
- `closeOutput` (~:2523-2530): after the reset, converge both combos to id 1
  (`dontSendNotification`) unconditionally.
- DELETE now-redundant per-caller resyncs (~:2258 Esc path, ~:2307 and ~:2312 Cmd+F paths).
- Shutdown path (~:1786-1806) unaffected — SafePointer nulls on MainComponent destruction.
  Menu items carry no checked state (`MenuBarModel.cpp:127,142`), so combos are the only
  state to resync.
- Deck-load path (~:2731-2733) and menu open path (~:3770-3774): INTENTIONALLY UNCHANGED per
  Boris's "leave Cmd+F as-is" and the decision not to smuggle a behavior change into a bugfix.

## NOT IN SCOPE (deliberate)
- Opaque black when no clip is loaded (`OutputWindow.cpp:92-100`) — that is the renderer's
  current contract; changing it (watermark/test pattern) is user-visible design. Follow-up.
- On-top toggle, Cmd+F rebind, deck-restore semantics — all product follow-ups, Boris-gated.

---

## TEST PLAN
**Safety rule for every step that shows the output window:** it is a real full-display black
window. Open only when Boris expects it, close on EVERY exit path, hard timeout so a wedged
run cannot strand it.
1. **FAIL-FIRST (automatable):** `tests/visual/test_output_window_level.py` — launch, open
   output, find the window named "Audio-DNA Output" via
   `Quartz.CGWindowListCopyWindowInfo`, assert `kCGWindowLayer == 0`. **Today reads 3 => the
   test FAILS. That failure is the deliverable.** Passes post-A. Tests the LEVEL, which is
   the mechanism; Spaces-following itself is not machine-checkable.
2. **Commit B probe:** open output, send Esc to the OUTPUT window, assert (a) the stderr
   heartbeat `[OutputRenderer] No image loaded yet` (~:96-98, ~every 60 frames with no clip)
   CEASES within ~3s — proves the GL context was destroyed, fails today because Esc only
   hides; (b) via AX the display combo reads "Output: Off".
3. **ctest baseline 193/193 stays green** — no test references OutputWindow (verified), and
   neither commit touches tested logic.
4. **IRREDUCIBLY HUMAN (Boris, ~3 min, laptop-only):** open output; **switch Spaces — output
   must NOT follow** (the original bug); Mission Control shows it as a normal window on its
   Space; click the black window, press Esc — gone, combo reads Off; Cmd+F reopens
   (regression-checks the stale-close guard); load a deck saved with output ON — opens
   Space-bound and dismissable. Multi-display check deferred until a projector is attached.
5. **Rapid-toggle race:** Esc then re-select a display within <1s — window must stay OPEN
   (stale queued close no-ops via the isVisible guard).

## RISK REGISTER (ranked)
1. **INFERRED premise:** normal level => Managed => Space-bound on this macOS version. Basis:
   Apple's defaults language + the bug itself confirming the level->participation model in the
   floating direction. P(wrong) low; consequence = fix doesn't fix. **Detection: Boris's
   Space-switch check, first thing verified.**
2. Async-destroy racing a re-open — mitigated by the isVisible guard; residual risk if a
   future path shows the window without going through openOutputOnDisplay (today `:2520` is
   the only call site). Detection: test 5 + a grep audit of goFullscreenOnDisplay callers.
3. Stage regression (accepted, named): output coverable by a window placed on its display.
   Recovery: re-select display in combo. Revisit if it bites in practice.
4. Combo resync with dontSendNotification firing onChange — standard JUCE suppression, relied
   on elsewhere in this file. P very low. Detection: test 2's combo assert + test 5.
5. **Line drift** — re-grep anchors, never trust the numbers above.
6. The 0x0-flash reorder assumes setBounds-before-peer is plain bookkeeping picked up at peer
   creation (INFERRED, standard JUCE). Consequence if wrong: cosmetic. Detection: Boris sees
   no top-left flash.

## GATE POSTURE
Full-tier both commits: independent Reviewer on source + Harmony runs the behavioral gate
herself (she did not build). Builder NEVER self-verifies. Forced rebuild before any ctest
claim (stale-binary false-green is a documented trap here).

---

# SALVAGE FROM ARCH-OVERLAY v2 (designed to the WITHDRAWN conditional ruling)

v2 was designed against Boris's "on-top only on a projector" ruling and landed AFTER he
withdrew it for unconditional drop. **The v2 design is NOT to be built.** These findings
survive it and are independently useful.

## 1. EXISTING BUG, unrelated to the overlay fix: the display combo lists are HOT-PLUG STALE
`refreshDisplayList` runs ONLY at construction (`MainComponent.cpp:328`), and the TopBar
combo build loop (`:486-501`) likewise runs once and is never refreshed. **Plug or unplug a
display while the app is running and both combos are wrong.** The MENU is already fresh
(MenuBarModel rebuilds per open, `MenuBarModel.cpp:131-143`), so the menu and the combos
disagree after any display change. Worth fixing on its own merits; extract
`refreshTopBarDisplayList()` and call both on display-config change.

## 2. VERIFIED hot-plug notification chain (if an on-top toggle is ever added)
Complete, read end-to-end from JUCE 8.0.4 this session:
`CGDisplayRegisterReconfigurationCallback` (registered `juce_Windowing_mac.mm:406-429`, wired
`:474-475`) -> `Displays::refresh()` (`juce_Displays.cpp:197-210`) which fires ONLY if the
display array actually changed (operator== over all fields, `:211-225`) -> `peer->
handleScreenSizeChange()` on every live peer -> `ComponentPeer::handleScreenSizeChange()`
(`juce_ComponentPeer.cpp:403-407`) -> `component.parentSizeChanged()`.
**Overriding `OutputWindow::parentSizeChanged()` is therefore a clean display-change-only
hook with zero false triggers** — `ResizableWindow::parentSizeChanged`
(`juce_ResizableWindow.cpp:502-506`) no-ops when there is no parent component, and for a
parentless top-level window `handleScreenSizeChange` is its ONLY caller (verified by grep).
**Re-entrancy rule:** it fires INSIDE refresh()'s peer loop — any handler MUST defer window
mutation via `MessageManager::callAsync`.

## 3. `juce::Displays::Display` has NO persistent ID
Verified against the full field list (`juce_Displays.h:52-110`). Any display-identity compare
must use `totalArea` equality. Also: on macOS `isMain` derives from NSScreen ordering
(`juce_Windowing_mac.mm:441-444`), so `displays[0]` IS the primary — meaning Cmd+F's
`openOutputOnDisplay(0)` targets the primary display. Useful if display predicates ever return.

## 4. Why the conditional was the more fragile design (recorded, since the call was close)
Under the conditional, hot-plug re-evaluation is NOT optional — unplugging a projector
mid-set migrates a still-floating window onto the laptop and RESURRECTS the reported bug.
That turns a 3-line fix into three commits plus an intent-tracking state machine
(`intendedOutputDisplay_`), async coalescing of reconfiguration bursts, and a
stale-dismissal guard. Boris's unconditional ruling removes that entire surface: with no
on-top state anywhere, there is nothing to re-evaluate on hot-plug. Recorded because the
simpler design won on more than simplicity — it deleted a whole class of race.
