# plan-opacity — Fix plan: Video twin removal, top-right master fader link, right-click reset

Session: s-rta-0925 · Author: Architect (Fable) · Date: 2026-09-25
Source diagnosis: `.harmony/.reports/s-rta-0925/diag-opacity.md` (all root causes VERIFIED there; this plan re-read
every touched site). Ruling: `.harmony/binding-decisions.md:523-534`.
Line numbers = worktree at HEAD `e501b00` (2026-09-25 19:00). Every edit carries a code anchor — if lines drift,
re-grep the anchor. Apply a file's edits bottom-up so earlier anchors stay valid.
Labels: VERIFIED = read in source this session at the cited line; INFERRED = follows from cited code, not executed;
ASSUMED = not checked, named as such.

---

QUESTION: Minimal, buildable fix for (1) remove the Composition-inspector Video "Opacity" twin (keep "Master"),
(2) link the top-right master fader to the Composition "Master" knob (one value, both follow each other, every
external writer lands on both), (3) right-click reset does nothing on Composition-tab sliders — with RED-first
tests, exact files/functions, a scope fence, and risks.

APPROACH: Three lanes, ONE Builder, strictly sequential C → A → B (C's header change is used by B; A and C both
edit `CompositionInspector.cpp`).
- Lane C `rclick` — `ResettableSlider` gains a nested-child right-click relay, a `resetToDefault()` entry point and an
  `onResetToDefault` hook; `UniversalParamControl` arms its inner slider in its ctor and routes the hook to
  `conn_->gripTouch`; `CompositionInspector::bindScalarControls` arms every scalar knob from `compScalarDefs()`.
  6 edits in 3 files + 1 new headless test target.
- Lane A `twin` — delete the whole Video section of the Composition inspector (9 deletions, 2 files) + 1 doc line.
  No test (pure UI removal); gate = build + visual.
- Lane B `link` — the TopBar fader becomes a second widget-grip view of `CompScalar::Opacity` (exactly the
  `LayerStrip::opacitySlider_` pattern, `src/ui/LayerStrip.cpp:414-436` VERIFIED) with a timer read-back mirroring
  `CompositionInspector::syncFromComposition` (`src/ui/CompositionInspector.cpp:537-549` VERIFIED);
  `Renderer::masterLevel_`, its second multiply, and the hidden legacy v1 slider are deleted; the three JSON readers
  keep their keys but report the model. 7 files + 1 new headless test target.
Deferred (explicit, see SCOPE): production-REST master writer (diag §3 step 5), LayerInspector twin, output-window
dimming, Master Signal slider (`diag-mastersignal.md`).

TRADEOFFS CONSIDERED (design of the link itself is settled in diag TRADEOFFS A-D; these are the plan-level ones):
- Headless JUCE-widget tests (chosen) vs live-gate only. No ctest target in this repo constructs a `juce::Component`
  today (grep of `tests/*.cpp` for `ScopedJuceInitialiser|juce_gui_basics|juce::Component`: zero hits, VERIFIED),
  but `test_mapping_engine` already links `juce::juce_opengl` (`tests/CMakeLists.txt:196-200` VERIFIED), which
  pulls `juce_gui_basics` transitively — the CMake side is proven. The runtime side (constructing widgets under
  `juce::ScopedJuceInitialiser_GUI`, `juce_events/messages/juce_Initialisation.h:81` VERIFIED) is INFERRED from
  JUCE's own console unit-test runner doing the same; a 30-minute fallback is named in RISKS. Chosen because the
  contract being fixed (fader → model, grip on drag, reset fires the grip) regressed silently once already
  (`Renderer.cpp:714-718`'s own comment: masterOpacity was render-dead all session) and the two closures are tiny.
- Self-register the slider as its own nested mouse listener (rejected) vs a separate relay object (chosen).
  `Component::internalMouseDown` calls `mouseDown(me)` directly (`juce_Component.cpp:2214`) and THEN dispatches to
  the listener list (`:2221`), whose first pass calls every listener registered on the target component itself
  (`:158-160`) — a self-registered slider would run `mouseDown` twice for own-area clicks. A relay object that
  ignores `e.eventComponent == &owner` cannot double-fire. VERIFIED.
- Relay resets on ANY child (rejected) vs on non-Button children only (chosen). `juce::Button::mouseUp` fires the
  click on any mouse button (`juce_Button.cpp:484-502`, no popup-menu check, VERIFIED), so a right-click on the
  IncDec "+" would reset AND step. The IncDec text box is a `Label` subclass (`juce_Slider.cpp:601,1358`;
  `LookAndFeel_V2::SliderLabelComp` `:1603`) whose `mouseUp` already ignores popup-menu clicks
  (`juce_Label.cpp:350-359`) — the safe relay target. The buttons are `TextButton`s (`:1598-1601`).
- Delete vs keep `TopBar::getMasterLevelSlider()` (diag said delete). KEPT, return type narrowed to
  `ResettableSlider&` — it is the test seam; its only caller (`MainComponent.cpp:629`) is deleted in B3.
- Legacy v1 deck preset `masterVideoLevel`: apply on load (diag step 4) vs drop both lines (chosen). `masterOpacity`
  is already persisted with the composition (`Composition.h:214/:331`); letting a v1 deck preset overwrite it on load
  would be a second, older persistence source. Both `MainComponent.cpp:3606` and `:3702` are deleted; the
  `PresetManager.h:87` field stays at its 1.0 default so the file format is unchanged. Harmony may flip this.

---

## 0. Order of work, commits, commands

```
Lane C: (R) tests + stubs → build → ctest RED (named failures)  → commit "test(s-rta-0925 rclick): RED ..."
        (G) edits C1-C4 → build → ctest GREEN                    → commit "fix(s-rta-0925 rclick): ..."
Lane A: edits A1-A10 → build → ctest unchanged                   → commit "fix(s-rta-0925 twin): ..."
Lane B: (R) tests + stubs → build → ctest RED                    → commit "test(s-rta-0925 link): RED ..."
        (G) edits B1-B7 → build → ctest GREEN                    → commit "fix(s-rta-0925 link): ..."
Then the live gate (§5), then docs (§4), then the handoff.
```
Build: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)`; tests: `cd build && ctest --output-on-failure`
(single target: `ctest -R test_right_click_reset --output-on-failure`). Configure once after editing
`tests/CMakeLists.txt`: `cmake -B build -DCMAKE_BUILD_TYPE=Release`.

RED convention = this repo's own (`tests/test_manual_write.cpp:1-11`, VERIFIED): tests are written against no-op
stubs so ctest reports NAMED failures; the failing-case count is the evidence pasted into the report. The RED
commit adds ONLY the declarations/stubs listed; it does not touch any existing behaviour.

---

## 1. Lane C — right-click reset (`rclick`)

### C-R. RED test target `test_right_click_reset` (new file `tests/test_right_click_reset.cpp`)

Stubs added in the RED commit, inside `class ResettableSlider` (`src/ui/UniversalParamControl.h:23-44`), nothing else
changed (in particular `mouseDown` stays as-is so the pin RC1 still passes):
```cpp
    std::function<void()> onResetToDefault;                                   // stub: never fired
    void resetToDefault() {}                                                  // stub: no-op
    static bool childRightClickResets(const juce::MouseEvent&, const juce::Component&) { return false; }  // stub
```

CMake block, appended to `tests/CMakeLists.txt` after `:1185` (mirror of the `test_composition_tier_oracle` block
`:70-99`, VERIFIED shape):
```cmake
# --- test_right_click_reset (s-rta-0925 rclick: ResettableSlider right-click reset
# on the thumb AND on the IncDec text box; UniversalParamControl arms its inner
# slider by default and a reset touches the bound connection). Headless JUCE
# widgets under ScopedJuceInitialiser_GUI -- no window, no peer; mouse events are
# synthesised and delivered by calling mouseDown() directly.
add_executable(test_right_click_reset
    test_right_click_reset.cpp
    ${SRC_DIR}/ui/UniversalParamControl.cpp
    ${SRC_DIR}/connect/ConnPicker.cpp
    ${SRC_DIR}/signal/SignalRegistry.cpp
)
target_include_directories(test_right_click_reset PRIVATE ${SRC_DIR})
target_link_libraries(test_right_click_reset PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_right_click_reset PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_right_click_reset PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_right_click_reset)
catch_discover_tests(test_right_click_reset)
```
Link closure (VERIFIED by include audit): `UniversalParamControl.cpp` uses `sourceFromPicker` (`:638`,
`connect/ConnPicker.cpp`, self-contained) and `SignalRegistry::getNumSignals/getSignalAt` (`:400-615`,
`signal/SignalRegistry.cpp`; every `signal/*Signal.h` is header-only, `ls src/signal`); it includes
`connect/ManualWrite.h` but calls nothing from it (`:3` only); it uses only `AudioDNALookAndFeel::k*` constexpr
constants (`LookAndFeel.h:10-16`), never the class. Rule if the linker still names a symbol: add the ONE `.cpp`
that defines it; do not add MainComponent.cpp/Renderer.cpp.

Shared harness (top of the test file; the same helper is copied into Lane B's test):
```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/UniversalParamControl.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ConnClock.h"

// A synthesised mouse-down as JUCE's own dispatcher would build it
// (juce_Component.cpp:81-95 HierarchyChecker::eventWithNearestParent): eventComponent
// = originator = the component the click landed on.
static juce::MouseEvent clickOn(juce::Component& target, bool rightButton)
{
    const juce::ModifierKeys mods(rightButton ? juce::ModifierKeys::rightButtonModifier
                                              : juce::ModifierKeys::leftButtonModifier);
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), {2.0f, 2.0f}, mods,
             0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, now, {2.0f, 2.0f}, now, 1, false };
}
template <typename T> static T* firstChildOfType(juce::Component& parent)
{
    for (int i = 0; i < parent.getNumChildComponents(); ++i)
        if (auto* c = dynamic_cast<T*>(parent.getChildComponent(i))) return c;
    return nullptr;
}
```
Every `TEST_CASE` begins with `juce::ScopedJuceInitialiser_GUI gui;` (ref-counted init/shutdown; components are
destroyed before it goes out of scope). `MouseEvent`'s public 15-arg ctor: `juce_MouseEvent.h:77-89` VERIFIED;
`Desktop::getMainMouseSource()`: `juce_Desktop.h:321` VERIFIED.

| id | TEST_CASE (tag `[rclick]`) | Steps | HEAD / RED-stub outcome |
|---|---|---|---|
| RC1 (pin) | "ResettableSlider: right-click on its own area resets to the armed default" | `ResettableSlider s; s.setRange(0,1,0.01); s.setDefaultValue(0.7); s.setValue(0.2, dontSend); s.mouseDown(clickOn(s,true));` → `s.getValue()==Approx(0.7)` | PASSES on HEAD (`UniversalParamControl.h:30-39`) — the one pin, like test_manual_write's case (i) |
| RC2 | "ResettableSlider: onResetToDefault fires once, after onValueChange" | as RC1; `int order=0, seenValue=0, seenReset=0; s.onValueChange=[&]{ seenValue=++order; }; s.onResetToDefault=[&]{ seenReset=++order; }; s.mouseDown(clickOn(s,true));` → `seenValue==1 && seenReset==2`; second right-click (value now == default, so JUCE sends no value notification, `juce_Slider.cpp:218` VERIFIED) → `seenReset` incremented again, `seenValue` unchanged | RED: `seenReset==0` |
| RC3 | "ResettableSlider: a right-click relayed from the IncDec text box resets; from a +/- button or the slider itself it does not" | `s.setSliderStyle(IncDecButtons); s.setTextBoxStyle(TextBoxLeft,false,30,20); s.lookAndFeelChanged(); s.setRange(1,99,1); s.setDefaultValue(1); s.setValue(5,dontSend); auto* box=firstChildOfType<juce::Label>(s); auto* btn=firstChildOfType<juce::Button>(s); REQUIRE(box); REQUIRE(btn);` → `childRightClickResets(clickOn(*box,true), s)==true`; `(clickOn(*btn,true), s)==false`; `(clickOn(s,true), s)==false`; `(clickOn(*box,false), s)==false`; then `s.resetToDefault()` → `s.getValue()==1` | RED: predicate stub returns false; `resetToDefault()` no-op leaves 5 |
| RC4 | "UniversalParamControl: the inner slider is armed without any setDefaultValue call" | `UniversalParamControl c; c.setParamValue(0.2f); auto* inner=firstChildOfType<ResettableSlider>(c); REQUIRE(inner); inner->mouseDown(clickOn(*inner,true));` → `c.getParamValue()==Approx(0.5f)` (class default, `.h:122`) | RED on HEAD: inner slider unarmed (`hasDefault_` false) → stays 0.2 |
| RC5 | "UniversalParamControl: a right-click reset touches a bound connection (Decaying grip)" | `ParamConnection conn; conn.source.kind=ConnSource::Kind::Signal; LiveValue live; UniversalParamControl c; c.bindConnection(&conn,&live); c.setDefaultValue(1.0f); c.setParamValue(0.3f); inner->mouseDown(clickOn(*inner,true));` → `conn.grip.kind==ParamConnection::Grip::Kind::Decaying` and `c.getParamValue()==Approx(1.0f)` | RED on HEAD: grip stays `None` (thumb path never touches `conn_`, diag R3) |

Expected RED tally: 4 failing (RC2-RC5), 1 passing (RC1). `ParamConnection::grip` / `source` are public struct
members (`ParamConnection.h:95-113` VERIFIED); `ConnSource::Kind::Signal` (`:34` VERIFIED).
Builder notes (VERIFIED): `firstChildOfType<ResettableSlider>(c)` is `valueSlider_` -- the first `addAndMakeVisible`
in the UPC ctor (`UniversalParamControl.cpp:28`; z-order = add order; the range sliders are added later).
`s.lookAndFeelChanged()` is public (`juce_Slider.h:1004`) and (re)creates the IncDec text box + buttons
(`juce_Slider.cpp:601,623-624`), so RC3 never depends on when JUCE creates them. Tests deliver ONLY right-clicks
directly to `mouseDown()`: a synthesised LEFT click would run `Slider::mouseDrag` against a zero-size slider.

### C-G. GREEN edits

C1. `src/ui/UniversalParamControl.h:22-44` — replace the whole `ResettableSlider` class (anchor: `// Slider that
resets to default on right-click`). Drop the inherited-ctor line: no site constructs it with arguments (grep
`ResettableSlider>|new ResettableSlider|ResettableSlider(` over `src/`: zero hits, VERIFIED).
```cpp
// Slider that resets to default on right-click -- on its own thumb/track AND on
// the text box of an IncDecButtons/TextBox slider (s-rta-0925 rclick). JUCE
// delivers a child's mouse-down to the child, not to us, so a nested mouse
// listener relays it (juce_Component.cpp:2221 -> MouseListenerList::sendMouseEvent
// walks the parent chain for listeners registered with
// wantsEventsForAllNestedChildComponents=true).
class ResettableSlider : public juce::Slider
{
public:
    ResettableSlider() { addMouseListener(&childRelay_, true); }
    ~ResettableSlider() override { removeMouseListener(&childRelay_); }

    void setDefaultValue(double val) { defaultVal_ = val; hasDefault_ = true; }

    // Fired after every right-click reset (thumb or text-box path), AFTER the
    // value notification. Owners use it to touch the parameter's connection
    // grip so a reset is visible on a signal-driven parameter.
    std::function<void()> onResetToDefault;

    void resetToDefault()
    {
        if (!hasDefault_) return;
        setValue(defaultVal_, juce::sendNotificationSync);   // no notification if already at default (JUCE)
        if (onResetToDefault) onResetToDefault();
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown()) { resetToDefault(); return; }
        juce::Slider::mouseDown(e);
    }

    // Decision for a mouse-down relayed from a NESTED child. Never for a
    // juce::Button child: Button::mouseUp fires its click on any mouse button
    // (juce_Button.cpp:484-502), so a reset there would be followed by a +/- step.
    // Never for the slider itself: that path is mouseDown() above (no double reset).
    static bool childRightClickResets(const juce::MouseEvent& e, const juce::Component& owner)
    {
        return e.eventComponent != &owner
            && e.mods.isRightButtonDown()
            && dynamic_cast<const juce::Button*>(e.eventComponent) == nullptr;
    }

private:
    struct ChildRelay final : public juce::MouseListener
    {
        explicit ChildRelay(ResettableSlider& s) : owner(s) {}
        void mouseDown(const juce::MouseEvent& e) override
        {
            if (childRightClickResets(e, owner)) owner.resetToDefault();
        }
        ResettableSlider& owner;
    };
    ChildRelay childRelay_{*this};
    double defaultVal_ = 0.0;
    bool hasDefault_ = false;
};
```
Precondition check (VERIFIED): `Component::addMouseListener(listener, true)` inserts deep listeners at index 0 and
counts them (`juce_Component.cpp:108-122`); `sendMouseEvent` walks `parentComponent` calling the first
`numDeepMouseListeners` of each ancestor (`:162-165`); the listener receives `eventWithNearestParent()` whose
`eventComponent` is the live child (`:81-95`). The IncDec children are created lazily by `lookAndFeelChanged`
(`juce_Slider.cpp:601,623-624`) — later than our ctor, which is fine because the walk is by ancestry, not by
registration on the child.

C2. `src/ui/UniversalParamControl.cpp:26-27` — after the two grip lambdas (anchor: `valueSlider_.onDragEnd = ...`)
add:
```cpp
    // s-rta-0925 rclick: never a dead right-click -- armed with the class default
    // until the owner calls setDefaultValue() with the real one; and a reset on
    // the thumb path touches the bound connection exactly like +/- do below.
    valueSlider_.setDefaultValue(static_cast<double>(defaultValue_));
    valueSlider_.onResetToDefault = [this] { if (conn_) conn_->gripTouch(connNow()); };
```
(`defaultValue_ = 0.5f` is a default member initialiser, `.h:122`, so it is set before the ctor body runs.)

C3. `src/ui/UniversalParamControl.cpp:293-302` — the name-area right-click branch (anchor: `// Right-click anywhere
→ reset to default value`). Replace its body (lines `:296-300`) with the single call so both paths are one path:
```cpp
    if (event.mods.isRightButtonDown())
    {
        // One path with the thumb right-click: slider value notification ->
        // onValueChanged; then onResetToDefault -> conn_->gripTouch (s-rta-0925).
        valueSlider_.resetToDefault();
        return;
    }
```
Semantics change, deliberate: when the knob already sits exactly at its default, `onValueChanged` is no longer
re-fired (the thumb path never did; owners' handlers are plain idempotent model writes, e.g.
`CompositionInspector.cpp:137-139`). The duplicate `onValueChanged` (`:298`, diag R3 minor) disappears.

C4. `src/ui/CompositionInspector.cpp:408-412` — inside the `bind` lambda of `bindScalarControls()` (anchor:
`auto bind = [this](UniversalParamControl& c, CompScalar s) {`), FIRST statement, before the `if (composition_)`:
```cpp
        c.setDefaultValue(compScalarDefs()[static_cast<size_t>(s)].defaultNorm);   // s-rta-0925: opacity 1.0, speed 0.25 (=1.0x), transforms 0.5
```
`compScalarDefs()` table: `src/connect/ScalarParams.h:107-120` VERIFIED (opacity 1.0f, speed 0.25f, rest 0.5f);
`Speed` maps `val * 4.0f` (`CompositionInspector.cpp:146`) so 0.25 == 1.0x; the transform knobs' visual defaults
are 0.5 (`:171-175`). `ScalarParams.h` is visible via `model/Composition.h:5`. This arms all 7 remaining UPCs
(after Lane A) with their real defaults; C2 is the fallback for every other inspector's unarmed UPC (now 0.5
instead of dead — see RISKS).

No other production edit. The four IncDec sliders (`CompositionInspector.cpp:50-54`, `:91-96`) are already armed and
are `ResettableSlider`s, so C1 alone fixes them.

---

## 2. Lane A — remove the Video "Opacity" twin (`twin`)

All deletions; `src/ui/CompositionInspector.h/.cpp` only (+ one inventory line). Apply bottom-up.

| # | Site (anchor) | Edit |
|---|---|---|
| A1 | `.cpp:545-550` (anchor `// Ruling 11: opacityControl_ is the twin knob`) | delete the 4-line comment and `syncScalar(opacityControl_, ...)` (`:550`); keep `:549` `syncScalar(masterControl_, ...)` |
| A2 | `.cpp:481` (`// Video` in `getPreferredHeight`) | delete the line |
| A3 | `.cpp:414` (`bind(opacityControl_, CompScalar::Opacity);   // ruling 11: one master`) | delete; keep `:413` |
| A4 | `.cpp:359-362` (`// Video section` in `resized`) | delete the 4 lines |
| A5 | `.cpp:257-258` (`paintSectionHeader(..., "Video")` + its `y +=`) | delete both lines |
| A6 | `.cpp:151-162` (`// --- Video Opacity ---` block through `addAndMakeVisible(opacityControl_);`) | delete the block |
| A7 | `.h:127-130` comment on `bindScalarControls` | drop the sentence "Ruling 11: masterControl_ and opacityControl_ share the same CompScalar::Opacity connection (one master)." |
| A8 | `.h:101-102` (`// --- Video ---` + `UniversalParamControl opacityControl_;`) | delete both lines |
| A9 | `.h:19` (`//   [Video]                   ▶ Opacity slider` in the class doc) | delete the line |
| A10 | `.harmony/APP-INVENTORY.md:74` "Composition master/speed; Video opacity; Transform" | drop "Video opacity;" |

Done = build clean; Composition tab shows Composition{Master, Speed} then Transform; `ctest` unchanged (no test
names `opacityControl_` — grep VERIFIED: only LayerInspector and planning docs). CLAUDE.md needs no edit (its
"Video section (Opacity, Width, Height…)" line describes the LAYER inspector).

---

## 3. Lane B — one master: TopBar fader ↔ `CompScalar::Opacity` (`link`)

### B-R. RED test target `test_master_opacity_link` (new file `tests/test_master_opacity_link.cpp`)

Stubs in the RED commit: `src/ui/TopBar.h` public `void syncMasterFromComposition();` and in `TopBar.cpp`
`void TopBar::syncMasterFromComposition() {}`; and the accessor return type change
`ResettableSlider& getMasterLevelSlider() { return masterLevelSlider_; }` (`TopBar.h:39`; the only caller
`MainComponent.cpp:629` compiles unchanged against the narrower type — `ResettableSlider` is-a `juce::Slider`).

CMake block (append after the Lane C block). Closure: `TopBar.cpp` needs `FeatureBus::read()` (`FeatureBus.h:106`
out-of-line, `features/FeatureBus.cpp`) and a bare `Composition` (same three model/connect files the oracle test links,
`tests/CMakeLists.txt:70-77` VERIFIED; `Clip.cpp`/`Layer.cpp` include only `connect/ConnSerialization.h`); everything
else TopBar touches is header-only (`LookAndFeel.h` constexpr colours, `Composition.h` inline, `FeatureSnapshot.h`).
```cmake
# --- test_master_opacity_link (s-rta-0925 link: the TopBar master fader is a
# second widget-grip view of Composition::masterOpacity / CompScalar::Opacity --
# writes the model, grips Held on drag, follows the model / eff() on sync, and
# a right-click reset touches the grip). Headless JUCE widgets, same harness as
# test_right_click_reset; the timer read-back is exercised by calling
# TopBar::syncMasterFromComposition() directly (no dispatch loop is run).
add_executable(test_master_opacity_link
    test_master_opacity_link.cpp
    ${SRC_DIR}/ui/TopBar.cpp
    ${SRC_DIR}/features/FeatureBus.cpp
    ${SRC_DIR}/model/Clip.cpp
    ${SRC_DIR}/model/Layer.cpp
    ${SRC_DIR}/connect/ConnSerialization.cpp
)
target_include_directories(test_master_opacity_link PRIVATE ${SRC_DIR})
target_link_libraries(test_master_opacity_link PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_master_opacity_link PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_master_opacity_link PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_master_opacity_link)
catch_discover_tests(test_master_opacity_link)
```
Fixture per TEST_CASE: `juce::ScopedJuceInitialiser_GUI gui; Composition comp; comp.initDefault(); FeatureBus bus;
TopBar bar(bus, comp); auto& fader = bar.getMasterLevelSlider(); constexpr auto kOp =
static_cast<size_t>(CompScalar::Opacity);` (`TopBar(const FeatureBus&, Composition&)`, `TopBar.h:15` VERIFIED;
`FeatureBus` default ctor `FeatureBus.h:91`, used the same way in `tests/test_feature_bus.cpp:13`). The fader is `setRange(0,1,0.01)`
(`TopBar.cpp:198`) — compare with `Approx(x).margin(0.006)`.

| id | TEST_CASE (tag `[link]`) | Steps | HEAD / RED-stub outcome |
|---|---|---|---|
| ML1 | "fader write lands on Composition::masterOpacity" | `fader.setValue(0.3, sendNotificationSync)` → `comp.masterOpacity == Approx(0.3f).margin(0.006)`; and `comp.scalarConns[kOp].grip.kind == None` (a plain value write is not a grip) | RED on HEAD: TopBar's fader has no `onValueChange` (`TopBar.cpp:197-202`) → stays 1.0 |
| ML2 | "fader drag grips Held on CompScalar::Opacity and releases on drag end" | `REQUIRE(bool(fader.onDragStart)); REQUIRE(bool(fader.onDragEnd)); fader.onDragStart();` → `grip.kind == Held`; `fader.onDragEnd();` → `None` | RED on HEAD: both std::functions empty |
| ML3 | "sync: not connected → fader shows the manual field" | `comp.masterOpacity = 0.8f; bar.syncMasterFromComposition();` → `fader.getValue() == Approx(0.8).margin(0.006)`; and `comp.masterOpacity` still 0.8 (sync never writes back) | RED: stub no-op → fader stays 1.0 |
| ML4 | "sync: connected → fader shows toNorm(eff()), not the manual field" | `comp.scalarConns[kOp].source.kind = ConnSource::Kind::Signal; comp.scalarLive[kOp].v.store(0.25f); comp.masterOpacity = 1.0f; bar.syncMasterFromComposition();` → `fader == Approx(0.25)`; then `comp.scalarConns[kOp].gripHeld(); comp.scalarLive[kOp].v.store(NAN); comp.masterOpacity = 0.6f; sync` → `fader == Approx(0.6)` (gripped = twin NaN = manual, `LiveValue.h:35-39`) | RED: stub |
| ML5 (guard) | "sync: the thumb never moves while the fader is being dragged" | `fader.setValue(0.5, sync); fader.onDragStart(); comp.masterOpacity = 0.1f; bar.syncMasterFromComposition();` → `fader == Approx(0.5)`; `fader.onDragEnd(); sync` → `Approx(0.1)` | passes on the stub (no-op never moves); has teeth only once ML3 is green — say so in the report |
| ML6 | "right-click on the fader resets to 1.0 on the model and touches the grip" | `fader.setValue(0.4, sync); fader.mouseDown(clickOn(fader,true));` → `comp.masterOpacity == Approx(1.0f)` and `grip.kind == Decaying` | RED on HEAD: slider resets (armed, `TopBar.cpp:200`) but the model stays 0.4 and the grip stays None |

Expected RED tally: 5 failing (ML1-ML4, ML6), 1 passing (ML5 guard). `LiveValue::v` is a public atomic
(`LiveValue.h:17-19` VERIFIED).

### B-G. GREEN edits

B1. `src/ui/TopBar.h`
- `:39` accessor: `ResettableSlider& getMasterLevelSlider() { return masterLevelSlider_; }` with the comment
  `// Test seam (tests/test_master_opacity_link.cpp); production wiring lives in TopBar.cpp.`
- public, after `setDspLoad` (`:34`): 
  ```cpp
  // s-rta-0925 link: pull the fader from the model -- manual field when not
  // connected, toNorm(eff()) when a signal drives it (same rule as
  // CompositionInspector::syncFromComposition). Skipped mid-drag. Called from
  // timerCallback (15 Hz) and directly by tests.
  void syncMasterFromComposition();
  ```
- private, after `:95` `ResettableSlider masterLevelSlider_;`: `bool masterDragging_ = false;`

B2. `src/ui/TopBar.cpp`
- `:2` add `#include "connect/ConnClock.h"` (as `LayerStrip.cpp:2` does; `compScalarDefs`/`CompScalar` already visible
  through `model/Composition.h:5`).
- after `:202` (anchor `masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);`):
  ```cpp
    // s-rta-0925 link: this fader is a SHORTCUT to the Composition tab's "Master"
    // knob -- a second widget-grip view of CompScalar::Opacity, the exact
    // LayerStrip::opacitySlider_ pattern (LayerStrip.cpp:414-436): value write
    // on change, Held grip for the duration of the drag (a real release exists),
    // Decaying touch on a right-click reset. The old Renderer::masterLevel_ this
    // fader used to drive (a second, compounding multiply) is gone.
    masterLevelSlider_.onValueChange = [this] {
        composition_.masterOpacity = static_cast<float>(masterLevelSlider_.getValue());
    };
    masterLevelSlider_.onDragStart = [this] {
        masterDragging_ = true;
        composition_.scalarConns[static_cast<size_t>(CompScalar::Opacity)].gripHeld();
    };
    masterLevelSlider_.onDragEnd = [this] {
        masterDragging_ = false;
        composition_.scalarConns[static_cast<size_t>(CompScalar::Opacity)].release(connNow());
    };
    masterLevelSlider_.onResetToDefault = [this] {
        composition_.scalarConns[static_cast<size_t>(CompScalar::Opacity)].gripTouch(connNow());
    };
  ```
- `timerCallback` (`:224-240`): add `syncMasterFromComposition();` as the last statement (after the repaint).
- new definition (place after `timerCallback`):
  ```cpp
  void TopBar::syncMasterFromComposition()
  {
      if (masterDragging_) return;
      const auto s = static_cast<size_t>(CompScalar::Opacity);
      const auto& def = compScalarDefs()[s];
      const bool connected = composition_.scalarConns[s].isConnected();
      const float shown = connected ? def.toNorm(composition_.eff(CompScalar::Opacity))
                                    : composition_.masterOpacity;
      masterLevelSlider_.setValue(static_cast<double>(shown), juce::dontSendNotification);
  }
  ```
  `dontSendNotification` never reaches `onValueChange` (`juce_Slider.cpp:351-353` VERIFIED) — no write-back loop.
  Knob → fader: the knob writes the same field (`CompositionInspector.cpp:137-139`) and the fader polls it.
  Fader → knob: the inspector's existing 10 Hz refresh (`MainComponent.cpp:3415-3417` → `InspectorPanel::refresh`
  `:180-186` → `CompositionInspector::refresh` → `syncFromComposition` `:468`, VERIFIED).

B3. `src/MainComponent.cpp:628-632` — delete the `// Wire TopBar master level` block (it would overwrite TopBar's
lambda: MainComponent assigns after TopBar's ctor ran).

B4. Legacy hidden v1 slider — delete: `src/MainComponent.h:272-273` (`masterLevelLabel_`, `masterLevelSlider_`);
`src/MainComponent.cpp:3702` (`masterLevelSlider_.setValue(deck.masterVideoLevel, ...)`; shorten the `:3699` comment
to `// Restore input gain`), `:3606` (`deck.masterVideoLevel = ...`), `:2427-2428` (two `setVisible(false)`),
`:523-532` (`// Master video level slider` block), `:327` (`setupLabel(masterLevelLabel_, "Video Level")`). Grep after:
`grep -n 'masterLevel' src/MainComponent.*` → zero hits. `PresetManager.h:87` / `.cpp:506,557` untouched.

B5. `src/render/Renderer.cpp:676-706` — delete the `masterLevel_` dim block (anchor `// Apply master level
(dim/blackout) using DST_COLOR blend to multiply` through the closing `}` before `// S167-L4b:`). In the surviving
comment `:711` change "the masterLevel_ block just above" → "the former masterLevel_ block (removed s-rta-0925: it was
a second, compounding multiply)". `src/render/Renderer.h:342-344` (`setMasterLevel/getMasterLevel` + comment) and
`:401` (`std::atomic<float> masterLevel_{1.0f};`) — delete. The `:721-747` block already dims by
`eff(CompScalar::Opacity)` unconditionally; net effect one fewer quad draw.

B6. Readers keep their JSON keys, sourced from the model (`composition_` is a `Composition&` in both servers,
`ApiServer.h:165`, `TestServer.h:123`; `CompScalar` visible via `model/Composition.h`):
- `src/api/ApiServer.cpp:287` → `obj->setProperty("masterLevel", static_cast<double>(composition_.eff(CompScalar::Opacity)));`
- `src/api/ApiServer.cpp:1136` → same for `"master_level"`.
- `src/test/TestServer.cpp:595` → same for `"master_level"`.
- `src/test/TestServer.cpp:657-658` → `// Reset master opacity (s-rta-0925: the one master)` / `composition_.masterOpacity = 1.0f;`
- `src/test/TestServer.cpp:1348-1354` stale comment → replace with: `// masterOpacity/masterSpeed are read on the GL
  thread through eff() (Renderer.cpp, S167-L4b block); this plain-float write matches every other message-thread
  writer of these fields.`
Off-thread read of a float + atomic twin is the same shape as the existing `:314` read of `masterOpacity`.
Verification: `grep -rn 'masterLevel_\|setMasterLevel\|getMasterLevel' src` → zero hits.

B7. Docs — §4.

---

## 4. Docs (after all three lanes are green)

- `.harmony/APP-INVENTORY.md:56` TopBar row: "Master slider" → "Master slider (= Composition master opacity;
  two-way linked with the Composition tab's Master knob, s-rta-0925)". `:144` `/api/status`: "masterLevel" →
  "masterLevel (= composition master opacity eff(), s-rta-0925)". `:164` `/api/state`: same note for `master_level`.
  `:74` already edited in A10.
- `CLAUDE.md` "UI Patterns → ResettableSlider" paragraph: append one sentence — "Right-click also resets from the
  text box of IncDecButtons/TextBox sliders (nested-child relay, s-rta-0925); `UniversalParamControl` arms its inner
  slider with 0.5 by default, so owners that skip `setDefaultValue()` get a 0.5 reset rather than a dead click — still
  call it with the real default." `CLAUDE.md` has no `masterLevel` mention (grep VERIFIED) — nothing else to change.
- Handoff must state: production REST still has NO master writer (diag §1c) — deferred decision.

---

## 5. Gates

Build/test gate (per lane): `cmake --build build --config Release` exit 0; `ctest` — RED tallies exactly as tabled,
then all green; no existing test changes (`tests/test_composition*.cpp`, `tests/test_manual_write.cpp:94-100` untouched).

Live gate (app running; TopBar + Composition tab visible; port 7070; Boris's own list from the ruling):
1. Drag TopBar fader to 0.30 → "Master" knob reads 0.30 within ~100 ms; `curl -s :7070/api/composition | jq .masterOpacity` == 0.30 and `curl -s :7070/api/status | jq .masterLevel` == 0.30.
2. Drag "Master" knob to 0.80 → fader shows 0.80 within ~100 ms; both REST keys 0.80.
3. OSC `/audiodna/master 0.5` (UDP 8000) → both controls 0.5 (Decaying grip, expires after `gripHoldMs` 250 ms).
4. MIDI/keyboard binding "Master Opacity" → both controls follow.
5. Connect Master to a signal (triangle → Audio/RMS): BOTH thumbs follow; dragging either holds it (Held) and glides back on release; the other mirrors throughout.
6. Compounding gone: one control at 0.5 → ~50% brightness (was 25% with fader 0.5 + knob 0.5). Optional PSNR via `/api/render_frame` before/after.
7. Right-click the TopBar fader → 1.0 on both.
8. Right-click reset, Composition tab — every control: Clip Loops text box → 1; the three Per-Type cycle text boxes → their defaults (16/8/4 per `setupCycleSlider` calls); Master → 1.0; Speed → 1.0x; Position X/Y, Scale, Rotation, Anchor → centre (0.5); the 8 Dashboard knobs → 0.5 (INFERRED already working, `MacroPanel.cpp:11`). Right-click on a "+"/"-" button still steps by one (pre-existing JUCE behaviour, accepted — the reset target is the number box).
9. Right-click on a CONNECTED knob (Master driven by RMS) → the thumb visibly jumps to 1.0 and holds ~250 ms before the signal resumes (R3 fixed).
10. Composition tab layout: Composition{Master, Speed} directly followed by Transform; no "Video" header.
Screenshots for 8-10 into `.harmony/.reports/s-rta-0925/` (Builder names them in the report).

---

## SCOPE (surgical fence)

IN: the files/lines above only. OUT (do not touch; name in the handoff): production-REST master writer
(`ApiServer.h:82` sibling, diag §3 step 5 — decision for Harmony/Boris); LayerInspector's own master/opacity twin
(`LayerInspector.cpp:145-152/:182-185`, diag RISKS — ask Boris); output-window dimming (`OutputWindow.cpp:67-172`
applies neither master today, unchanged); Master Signal slider (`diag-mastersignal.md`); real defaults for other
inspectors' unarmed UPCs (they now get the 0.5 fallback from C2 — a follow-up applies `layerScalarDefs()` /
`clipScalarDefs()` the way C4 does); `Knob`/`MacroPanel`; `Composition.h` (no new model field); no new thread, mutex,
or atomic; no `Renderer` change beyond the deletion in B5; no `PresetManager` format change.

---

## RISKS

- Headless widget construction at test runtime is INFERRED, not executed here (no prior GUI ctest in this repo).
  Fallback, capped at 30 minutes: if `ScopedJuceInitialiser_GUI` + `TopBar`/`UniversalParamControl` construction
  asserts or crashes under ctest, keep the test files, mark both targets `if(AUDIODNA_GUI_TESTS)`-gated OFF, report
  the exact failure, and let the live gate (§5) be the oracle for this session. Do not spend the session on the
  harness.
- The relay's JUCE routing (child mouse-down reaching a listener registered on the slider) is VERIFIED by reading
  `juce_Component.cpp:108-122,158-165,2221` but not executed; RC3 tests the decision, not the routing. Gate item 8
  (Clip Loops number box) is the executed proof. If the number box does not reset live, the fallback is to register
  the relay on the `Label` child directly after `lookAndFeelChanged()` (children are recreated there,
  `juce_Slider.cpp:601`).
- `/api/status.masterLevel` and `/api/state.master_level` change meaning for any external consumer (now the one
  master, eff()). Both default to 1.0 and the fader was `masterLevel_`'s only live writer — a consumer that watched
  the fader keeps seeing the fader. Keys kept.
- C3 unifies the two right-click paths: a knob already exactly at its default no longer re-fires `onValueChanged`.
  Only a connected knob whose live signal value coincides with the default could notice — measure-zero.
- C2's 0.5 fallback turns every previously DEAD unarmed UPC (ClipInspector 12/8, LayerInspector 24/7,
  SignalInspector, MappingEditor — diag coverage audit, INFERRED counts) into a "resets to 0.5" control. Correct for
  transforms/bidirectional params, possibly wrong for a few (e.g. an unarmed opacity-like knob). Strictly better
  than dead; the follow-up in SCOPE fixes defaults per inspector.
- Legacy v1 deck presets no longer carry the master level in either direction (B4 choice). The Save/Load buttons'
  reachability in the v2 layout is ASSUMED unknown (diag). Reversible: apply `deck.masterVideoLevel` on load as the
  diag proposed if Boris wants it.
- Right-click on the IncDec "+"/"-" still steps (JUCE fires the click on any button). Accepted and documented; a
  full fix needs a custom `LookAndFeel::createSliderButton` — out of scope.
- Strongest counterargument to this plan: "two new GUI test targets for a UI-wiring fix is over-engineering; the live
  gate is the real oracle." Why it loses: the contract being restored regressed silently once (render-dead master),
  the closures are five model/UI files with no GL, the RED tallies are cheap named evidence, and the fallback above
  bounds the cost.

STATUS: COMPLETE — plan is Builder-executable with zero open questions inside its scope; the REST master writer and the LayerInspector twin remain decisions for Harmony/Boris (named, not blocking).
