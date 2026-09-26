# plan-rclick — Right-click reset fix (Composition tab: every slider), minimal + buildable

Session: s-rta-0925 · Author: Architect (Fable) · Date: 2026-09-25
Input: the right-click diagnosis is §4 of `.harmony/.reports/s-rta-0925/diag-opacity.md:211-267` (R1/R2/R3);
`diag-rclick.md` was never written as a separate file. Ruling: `.harmony/binding-decisions.md:530,535` (Boris:
"right-click on a slider does not reset to default. Fix or plan to fix"; "fails on ALL sliders in the Composition tab").
Labels: VERIFIED = read at the cited line or executed this session; INFERRED = follows from cited code, not executed;
ASSUMED = named as such.

---

QUESTION: Smallest buildable change so that a right-click on ANY Composition-tab slider resets it to its real default,
including connected (signal-driven) knobs, with a RED-first test and a surgical scope.

APPROACH (answer first):
Three defects, three edits, one class at the centre. (R1) `UniversalParamControl` never arms its inner slider and
CompositionInspector never calls `setDefaultValue` on its 8 knobs, so `ResettableSlider::mouseDown` swallows the
click (`src/ui/UniversalParamControl.h:30-39`, VERIFIED) — fix: arm the inner slider at construction and, in
`CompositionInspector::bindScalarControls`, feed each knob its descriptor default from `compScalarDefs()[s].defaultNorm`
(`src/connect/ScalarParams.h:23,107-120`: opacity 1.0, speed 0.25 = 1.0x, transforms 0.5). (R2) The four IncDec
sliders ARE armed but their surface is a child `Label` + two child `Button`s, and JUCE only forwards the text box's
clicks to the slider for `LinearBar` styles (`juce_Slider.cpp:601-636`, VERIFIED) — fix: `ResettableSlider` registers
ONE deep mouse listener on itself (`addMouseListener(&childListener_, true)`) and treats non-Button child right-clicks
as a reset. (R3) The thumb-path reset never touches the connection, so a connected knob visibly does nothing — fix:
`ResettableSlider::onResetToDefault` hook; `UniversalParamControl` installs the same three-step sequence its
name-area reset already does (`UniversalParamControl.cpp:296-300`) and the name-area path calls the same routine.
Net: ~40 product lines in 3 files, 1 new ctest target (spike-proven to run headless on this Mac), 2 doc paragraphs.

---

## 1. Evidence gathered this session (beyond diag §4)

E1 VERIFIED (run): a plain console process on this Mac can `juce::ScopedJuceInitialiser_GUI` + construct a
`juce::Slider` (IncDecButtons/TextBoxLeft → 3 children: 1 `Label`, 2 `Button`s) + build a `juce::MouseEvent` from
`Desktop::getInstance().getMainMouseSource()` (returns by VALUE) and deliver it via the public `Component::mouseDown`,
with no window, no display context, exit 0. Rig: scratchpad `spike/` (CMake `add_subdirectory` of
`build/_deps/juce-src`, link `juce::juce_gui_basics` only). Output:
```
gui init ok / children=3 label=1 button=1
direct right: value=16 resets=1 notifies=1
label right: value=16 ... / button right: value=8 (no reset) / label left: baseDowns=0
stock right-click on linear slider: value=0 (0.3 = untouched)   <- stock juce::Slider treats right-click as a DRAG START
no-default right: value=0.3 resets=0
```
So `tests/CMakeLists.txt:1138-1142`'s "NEVER juce_gui_basics (no display context under ctest)" is a caution, not a
fact — this plan's new target is the documented exception (its CMake comment says why).

E2 VERIFIED (run, against HEAD's REAL class via `#include "ui/UniversalParamControl.h"`, `-I src`, no `src/*.cpp`
linked, project test defines): the header compiles standalone in a gui_basics-only TU, and of the five planned
behavioural cases, 1/2/3 PASS on HEAD (pins) and 4/5 FAIL on HEAD (true RED) — see §4.

E3 VERIFIED (source): `Component::addMouseListener(l, true)` delivers every nested child's events to `l`
(`juce_Component.cpp:138-166` second loop over `numDeepMouseListeners`), with `eventComponent` = the CHILD
(`HierarchyChecker` ctor `:58-66` puts the target first; `eventWithNearestParent()` `:81-91`). Registering the
component ITSELF as its own listener double-delivers its direct events (`:2065-2067`, JUCE's own comment) — so the
listener must be a separate object, and direct events must be filtered out of it. `internalMouseDown` is private
(`juce_Component.h:2697`): no headless seam drives the listener plumbing; it is source-verified + live-gated.

E4 VERIFIED (source): `Button::mouseDown`/`mouseUp` fire the click for ANY mouse button (`juce_Button.cpp:470-502`),
and the auto-repeat timer re-derives "down" from the physical button (`:686-704`), so a +/- right-click cannot be
cleanly converted into a reset without a LookAndFeel-level button subclass — out of scope; +/- keep stock behaviour.

E5 VERIFIED (source): `setSliderStyle()`/`setTextBoxStyle()` recreate the children (`juce_Slider.cpp:496-503` →
`lookAndFeelChanged`), so per-child listener attachment would have to be redone after every style call; a deep
listener registered once on the slider survives recreation. `Label::mouseUp` never opens the editor on a popup-menu
click (`juce_Label.cpp:350-358`), so a right-click on the text box has no competing behaviour.

E6 VERIFIED (grep): no non-default `ResettableSlider` constructor is used anywhere (`ResettableSlider(`/`{` → 0
hits outside the class), so `using juce::Slider::Slider;` can be dropped in favour of an explicit default ctor. Nothing
upstream eats right-clicks: `setInterceptsMouseClicks(true,false)` exists only on the bind/MIDI-learn overlays
(`BindingOverlay.cpp:6`, `MidiLearnOverlay.cpp:6`), which would also kill left-clicks if active — Boris reports
right-click only. Alternative explanation rejected.

E7 VERIFIED: Composition-tab inventory. IncDec (armed): `apClipLoopsSlider_` default 1 (`CompositionInspector.cpp:50-55`),
`opaqueCycleSlider_` 16 / `transparentCycleSlider_` 8 / `effectCycleSlider_` 4 (`:91-116`). UPCs (NOT armed):
`masterControl_`, `speedControl_`, `opacityControl_` (twin, being removed by the sibling lane), `posX/posY/scale/
rotation/anchorControl_` (`CompositionInspector.h:98-109`; setups `:135-176` — `setupTransformParam` `:165-170` lacks
the `pc.setDefaultValue(defVal)` that `LayerInspector.cpp:389` and `ClipInspector.cpp:394` have). Macro knobs: armed 0.5
(`MacroPanel.cpp:11`), rotary/no text box, slider fills the knob body above the two labels (`Knob.cpp:75-85`) → the
direct path works today (INFERRED; gate). Everything else app-wide that owns a UPC already arms it (`ClipInspector.cpp:
335,394,799`, `LayerInspector.cpp:147,184,389`, `EffectStackView.cpp:368,407`).

---

## 2. TRADEOFFS CONSIDERED

- A (chosen): fix in `ResettableSlider` (deep child listener + reset hook) + arm UPCs (ctor fallback + descriptor
  default in CompositionInspector). One class carries the rule for all ~50 sliders; owners stay one-liners.
- B (rejected): only add the missing `setDefaultValue` calls. Leaves the four IncDec sliders dead (they are already
  armed and still fail — R2), contradicting Boris's "ALL".
- C (rejected): intercept child clicks in `UniversalParamControl` (deep listener on the UPC). Bare ResettableSliders
  (the 4 IncDec + ~40 others) have no UPC parent; the fix belongs in the class every slider uses.
- D (rejected): `addMouseListener(this, true)` as diag §4 step 3 suggested. Double-delivers direct events (E3):
  every left-click would start the base drag twice (two `DragInProgress`, spurious `onDragEnd` → `conn_->release`).
  A separate listener object with an `eventComponent != this` filter is the same size and correct.
- E (rejected): per-child `addMouseListener` in an overridden `lookAndFeelChanged()`. Works, but must re-run after
  every style change (E5) — more code, more ways to forget. Kept only as the live-gate fallback (§6).
- F (rejected): also reset on +/- right-click via `Button::setState(buttonNormal)` after its mouseDown. The repeat
  timer re-derives "down" from the physical button (E4) — leaky. If Boris wants it: `AudioDNALookAndFeel::
  createSliderButton` override returning a Button that ignores `isPopupMenu()` clicks. Separate item.
- Hook semantics: "hook set → slider sets value SILENTLY, hook owns notification" (chosen) vs "notify, then hook"
  (rejected: either double-fires `onValueChanged` or, when the connected knob already shows the default, skips the
  model write and `gripTouch` then blips a STALE manual value for 250 ms).

Strongest counterargument to A: it changes a shared base class used by ~50 sliders for a Composition-tab bug.
Why it loses: the change is additive for the direct path (cases 1-2 pin today's behaviour byte-for-byte), the only
new behaviours are on paths that today do nothing (child clicks) or are unreachable (hook), and every slider in the
app has the same R2 defect on its text box — fixing it once is smaller than fixing it per owner.

---

## 3. DECISION / SPEC — exact edits (Builder executes top to bottom)

### 3.1 `src/ui/UniversalParamControl.h` — replace `class ResettableSlider` (`:22-44`) with:

```cpp
// Slider that resets to default on right-click.
//
// Contract (s-rta-0925 rclick lane, .harmony/.reports/s-rta-0925/plan-rclick.md):
//  * Right-click on the slider body (thumb/track/rotary arc) OR on its text box
//    resets to the armed default. Un-armed (no setDefaultValue call): the click
//    is swallowed on purpose -- stock juce::Slider starts a DRAG on any mouse
//    button and would jump the value to the click position.
//  * The text box and the +/- buttons of an IncDecButtons slider are CHILD
//    components, so their clicks never reach Slider::mouseDown by themselves;
//    childListener_ (one deep mouse listener, registered once -- it survives the
//    child recreation that setSliderStyle()/setTextBoxStyle() trigger) forwards
//    them here. +/- buttons keep JUCE's stock click; only non-Button children reset.
//  * Owners that must do more than move the thumb (UniversalParamControl: model
//    write + connection grip touch) install onResetToDefault; when it is set the
//    slider sets its value SILENTLY and the hook owns all notification.
class ResettableSlider : public juce::Slider
{
public:
    ResettableSlider() { addMouseListener(&childListener_, true); }
    ~ResettableSlider() override { removeMouseListener(&childListener_); }

    void setDefaultValue(double val) { defaultVal_ = val; hasDefault_ = true; }
    bool hasDefaultValue() const { return hasDefault_; }

    // Fired after a reset when set (see contract). Never fired when un-armed.
    std::function<void()> onResetToDefault;

    void resetToDefault()
    {
        if (!hasDefault_) return;
        setValue(defaultVal_, onResetToDefault ? juce::dontSendNotification : juce::sendNotificationSync);
        if (onResetToDefault) onResetToDefault();
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const bool fromChild = e.eventComponent != this;   // forwarded by childListener_
        if (e.mods.isRightButtonDown())
        {
            if (!fromChild || dynamic_cast<juce::Button*>(e.eventComponent) == nullptr)
                resetToDefault();
            return;
        }
        if (!fromChild)
            juce::Slider::mouseDown(e);   // a child's left-click is the child's business, never a drag start here
    }

private:
    struct ChildListener : juce::MouseListener
    {
        explicit ChildListener(ResettableSlider& s) : owner(s) {}
        void mouseDown(const juce::MouseEvent& e) override { if (e.eventComponent != &owner) owner.mouseDown(e); }
        ResettableSlider& owner;
    };
    ChildListener childListener_{*this};
    double defaultVal_ = 0.0;
    bool hasDefault_ = false;
};
```
Notes: `using juce::Slider::Slider;` is intentionally dropped (E6) so a listener-less construction cannot exist.
`<functional>` is already included (`:8`). `JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED` inside add/removeMouseListener:
the app builds and tears down UI on the message thread; the test process's main thread is the message thread under
`ScopedJuceInitialiser_GUI` (and ctest builds Release, `build/CMakeCache.txt:255`).

### 3.2 `src/ui/UniversalParamControl.cpp` — four hunks

(a) Ctor, after `valueSlider_.setValue(0.5, juce::dontSendNotification);` (`:12`):
```cpp
    // Armed from birth: right-click resets to 0.5 until the owner calls
    // setDefaultValue() with the real default (s-rta-0925 rclick lane -- an
    // un-armed ResettableSlider swallows the click, which read as "broken").
    valueSlider_.setDefaultValue(static_cast<double>(defaultValue_));
```
(b) Ctor, after `valueSlider_.onDragEnd = ...` (`:27`):
```cpp
    // Thumb-path reset == name-area reset (mouseDown below): the hook owns the
    // model write and the release-less grip touch, so the slider itself sets
    // its value silently (ResettableSlider::resetToDefault contract).
    valueSlider_.onResetToDefault = [this] {
        setParamValue(defaultValue_);
        if (onValueChanged) onValueChanged(defaultValue_);
        if (conn_) conn_->gripTouch(connNow());   // a release-less write (s-rta-0923 lane 3 plan section 4.2)
    };
```
(c) Ctor, after `rangeMaxSlider_.setValue(1.0, juce::dontSendNotification);` (`:88`):
```cpp
    rangeMinSlider_.setDefaultValue(0.0);
    rangeMaxSlider_.setDefaultValue(1.0);
```
(d) `UniversalParamControl::mouseDown` (`:291-302`): replace the body of the `isRightButtonDown()` branch
(`:296-300`, five lines) with the single call
```cpp
        valueSlider_.resetToDefault();   // one reset routine for the name area and the thumb (hook installed in the ctor)
```
keeping the `return;`. Behaviour is identical to today's sequence (`setParamValue` → `onValueChanged` → `gripTouch`)
minus the duplicate `onValueChanged` fire diag §4 R3 flagged (`:296` notify + `:298` explicit).

### 3.3 `src/ui/CompositionInspector.cpp` — `bindScalarControls()` `bind` lambda (`:408-412`), first statement:
```cpp
    auto bind = [this](UniversalParamControl& c, CompScalar s) {
        // Right-click reset lands on the descriptor default (ScalarParams.h:107-120):
        // opacity 1.0, speed 0.25 (= 1.0x), transforms 0.5 -- never the 0.5 fallback.
        c.setDefaultValue(compScalarDefs()[static_cast<size_t>(s)].defaultNorm);
        if (composition_) c.bindConnection(&composition_->scalarConns[static_cast<size_t>(s)],
                                            &composition_->scalarLive[static_cast<size_t>(s)]);
        else c.bindConnection(nullptr, nullptr);
    };
```
`compScalarDefs()` is already visible here (used at `:538`, VERIFIED). Runs for every bound knob, with or without a
composition, on every `setComposition` (`:388-404`) — idempotent. Do NOT add literals to the ctor setups (`:135-176`).

### 3.4 `tests/test_resettable_slider.cpp` (new) — see §4 for the full file. `tests/CMakeLists.txt` — append after the
`test_httplib_bodyless_post` block (`:1185`):
```cmake
# --- test_resettable_slider (s-rta-0925 rclick lane: the right-click reset contract
# of ResettableSlider -- armed reset, text-box (child) clicks reaching the slider,
# +/- buttons excluded, the owner reset hook). Header-only class in
# src/ui/UniversalParamControl.h, so NO src/*.cpp is linked. Links juce_gui_basics:
# a juce::Slider needs the module compiled and a ScopedJuceInitialiser_GUI in-process
# -- proven to run under ctest on macOS with no window and no display context
# (s-rta-0925 spike, plan-rclick.md section 1): synthetic MouseEvents are delivered
# through the PUBLIC Component::mouseDown, never through a peer. This is the one
# deliberate exception to test_record_panel_model's "never juce_gui_basics" note.
add_executable(test_resettable_slider test_resettable_slider.cpp)
target_include_directories(test_resettable_slider PRIVATE ${SRC_DIR})
target_link_libraries(test_resettable_slider PRIVATE
    Catch2::Catch2WithMain
    juce::juce_gui_basics
)
target_compile_definitions(test_resettable_slider PRIVATE
    _USE_MATH_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
if(NOT MSVC)
    target_compile_options(test_resettable_slider PRIVATE
        -Wno-old-style-cast
        -Wno-conversion
        -Wno-sign-conversion
    )
endif()
apply_sanitizers(test_resettable_slider)
catch_discover_tests(test_resettable_slider)
```
(No `JUCE_STANDALONE_APPLICATION` define: E2 compiled and ran without it; the existing juce-linked targets set none.)

### 3.5 Docs (after GREEN)
- `CLAUDE.md:507` (UI Patterns → ResettableSlider): keep the MUST rules; append: "`UniversalParamControl` arms its
  inner slider at 0.5 from construction and owners still call `setDefaultValue()` with the real default
  (CompositionInspector takes it from `compScalarDefs()[s].defaultNorm`). A slider's text box resets on right-click
  too (deep child listener in `ResettableSlider`); `+`/`-` buttons keep the stock click. Owners needing a model write
  or grip touch on reset install `ResettableSlider::onResetToDefault` (the slider then sets its value silently)."
- `CLAUDE.md` Common Pitfalls: add **32** after `:1103`: "Right-click reset needs three things: an ARMED default (an
  un-armed `ResettableSlider` swallows the click on purpose — stock `juce::Slider` treats a right-click as a drag
  start and jumps the value), the click REACHING the slider (IncDecButtons/TextBox sliders put a Label and two
  Buttons on top; `ResettableSlider` forwards non-Button child clicks through one deep mouse listener), and — for a
  connected parameter — a grip touch (`onResetToDefault` hook in `UniversalParamControl`), else the signal keeps
  driving `eff()` and nothing visibly happens. Guarded by `tests/test_resettable_slider.cpp`."
- `.harmony/APP-INVENTORY.md:105` already says "right-click = reset" — no change.

### 3.6 Done criteria
`cmake --build build --config Release` clean; `cd build && ctest` all pass including `test_resettable_slider`
(8 cases); live gate §5 all green with a screenshot per row; docs §3.5 in the same commit.

---

## 4. RED-first test design

Why the unit tests cannot be RED for the R2 plumbing itself: they call the PUBLIC `mouseDown` directly, and HEAD's
`mouseDown` ignores `eventComponent` — so "right-click from the Label resets" passes on HEAD by accident (E2 case 3).
The tests therefore pin the slider-side CONTRACT (what the slider does with a child-sourced event) and the two rules
HEAD gets wrong; the JUCE delivery of child events to a deep listener is E3 (source) + §5 (live). Phasing:

Phase A (compiles on HEAD — only existing API): add the file with cases 1-5 + the CMake block; build + run →
expect **4 and 5 FAIL, 1/2/3 PASS** (E2 proved exactly this against HEAD). That failing run is the RED evidence.
Phase B (new API): add cases 6-8 → target **fails to compile** (`resetToDefault`/`onResetToDefault` undeclared).
Implement §3.1 → all 8 GREEN. Then §3.2/§3.3 (UPC + inspector wiring, live-gated: UPC.cpp's link set —
ConnPicker → ManualWrite → ShaderManager → juce_opengl, `tests/CMakeLists.txt:996-1021` — is out of proportion to
four lines of wiring; §5 rows 3-8 discriminate every one of them).

```cpp
// tests/test_resettable_slider.cpp -- s-rta-0925 rclick lane. See plan-rclick.md section 4.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "ui/UniversalParamControl.h"   // ResettableSlider (header-only)

namespace
{
// One GUI subsystem for the whole run, created on first use (inside main, on the
// message thread) and torn down at exit -- never a namespace-scope static.
juce::ScopedJuceInitialiser_GUI& gui() { static juce::ScopedJuceInitialiser_GUI g; return g; }

juce::MouseEvent clickOn(juce::Component& target, bool rightButton, juce::Point<float> pos = {2.0f, 2.0f})
{
    auto src = juce::Desktop::getInstance().getMainMouseSource();   // by value
    juce::ModifierKeys mods(rightButton ? juce::ModifierKeys::rightButtonModifier
                                        : juce::ModifierKeys::leftButtonModifier);
    const auto t = juce::Time::getCurrentTime();
    return juce::MouseEvent(src, pos, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &target, &target, t, pos, t, 1, false);
}

// IncDecButtons + TextBoxLeft: the shape of the four Composition-tab numeric sliders
// (CompositionInspector.cpp:50-51, 92-93). Children: one Label (text box), two Buttons.
struct IncDec
{
    ResettableSlider s;
    juce::Label* label = nullptr;
    juce::Button* button = nullptr;
    int notifies = 0, dragStarts = 0;
    IncDec()
    {
        gui();
        s.setSliderStyle(juce::Slider::IncDecButtons);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
        s.setRange(1, 64, 1);
        s.setValue(8, juce::dontSendNotification);
        s.setDefaultValue(16);
        s.onValueChange = [this] { ++notifies; };
        s.onDragStart = [this] { ++dragStarts; };
        for (int i = 0; i < s.getNumChildComponents(); ++i)
        {
            auto* c = s.getChildComponent(i);
            if (auto* l = dynamic_cast<juce::Label*>(c)) label = l;
            if (auto* b = dynamic_cast<juce::Button*>(c)) button = b;
        }
        REQUIRE(label != nullptr);
        REQUIRE(button != nullptr);
    }
};
} // namespace

TEST_CASE("ResettableSlider: direct right-click resets an armed slider and notifies once", "[rclick][pin]")
{
    IncDec f;
    f.s.mouseDown(clickOn(f.s, true));
    CHECK(f.s.getValue() == 16.0);
    CHECK(f.notifies == 1);
    CHECK(f.dragStarts == 0);
}

TEST_CASE("ResettableSlider: un-armed right-click is swallowed (no jump, no drag)", "[rclick][pin]")
{
    gui();
    ResettableSlider u;   // stock juce::Slider would start a drag here and jump 0.3 -> 0.0 (spike, section 1)
    u.setSliderStyle(juce::Slider::LinearHorizontal);
    u.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    u.setRange(0.0, 1.0, 0.001);
    u.setValue(0.3, juce::dontSendNotification);
    u.setBounds(0, 0, 200, 20);
    int drags = 0; u.onDragStart = [&] { ++drags; };
    u.mouseDown(clickOn(u, true));
    CHECK(u.getValue() == Catch::Approx(0.3));
    CHECK(drags == 0);
    CHECK_FALSE(u.hasDefaultValue());       // Phase B: compile-RED until 3.1 lands
}

TEST_CASE("ResettableSlider: right-click arriving from the text-box Label resets", "[rclick][pin-of-new-contract]")
{
    IncDec f;
    f.s.mouseDown(clickOn(*f.label, true));   // what childListener_ hands us (eventComponent = the child)
    CHECK(f.s.getValue() == 16.0);
}

TEST_CASE("ResettableSlider: right-click arriving from a +/- Button does not reset", "[rclick][red]")
{
    IncDec f;
    f.s.mouseDown(clickOn(*f.button, true));
    CHECK(f.s.getValue() == 8.0);    // HEAD: 16 (resets) -> RED
    CHECK(f.notifies == 0);
}

TEST_CASE("ResettableSlider: left-click arriving from a child never starts a drag", "[rclick][red]")
{
    IncDec f;
    f.s.mouseDown(clickOn(*f.label, false));
    CHECK(f.dragStarts == 0);        // HEAD: base mouseDown runs -> DragInProgress -> onDragStart -> RED
    CHECK(f.s.getValue() == 8.0);
}

TEST_CASE("ResettableSlider: onResetToDefault hook owns notification", "[rclick][red-compile]")
{
    IncDec f;
    int hooks = 0;
    f.s.onResetToDefault = [&] { ++hooks; };
    f.s.mouseDown(clickOn(f.s, true));
    CHECK(f.s.getValue() == 16.0);   // value moved...
    CHECK(hooks == 1);               // ...the hook fired once...
    CHECK(f.notifies == 0);          // ...and the slider stayed silent (the hook owns the model write)
    f.s.resetToDefault();            // already at default: still touches (a connected knob must re-grip)
    CHECK(hooks == 2);
}

TEST_CASE("ResettableSlider: resetToDefault() on an un-armed slider is a no-op", "[rclick][red-compile]")
{
    gui();
    ResettableSlider u;
    u.setRange(0.0, 1.0, 0.001);
    u.setValue(0.3, juce::dontSendNotification);
    int hooks = 0; u.onResetToDefault = [&] { ++hooks; };
    u.resetToDefault();
    CHECK(u.getValue() == Catch::Approx(0.3));
    CHECK(hooks == 0);
}

TEST_CASE("ResettableSlider: hook-less reset notifies once, and not when already at default", "[rclick][pin]")
{
    IncDec f;
    f.s.resetToDefault();
    CHECK(f.notifies == 1);
    f.s.resetToDefault();            // juce::Slider::setValue skips equal values -> no second notification
    CHECK(f.notifies == 1);
}
```
Expected run after §3.1: 8/8 pass. Build cost: the target compiles juce_gui_basics + graphics/events/core once
(spike: ~47 s wall, -j8, plus link) — comparable to the juce_opengl-linking test targets already in the suite.

---

## 5. Live gate (Boris's scope: every Composition-tab control; app running, Composition tab open)

| # | Control | Right-click where | Expect |
|---|---------|-------------------|--------|
| 1 | Clip Loops (IncDec) | the number box | 1 (set 5 first) |
| 2 | Opaque / Transparent / Effect Beats (IncDec) | the number box | 16 / 8 / 4 |
| 2b | any IncDec `+`/`-` button | the button | stock click (+1 / -1), NOT a reset — by design (§2 F) |
| 3 | Master (UPC) | the slider thumb/track | 1.00; `curl -s :7070/api/composition \| jq .masterOpacity` == 1 (`ApiServer.cpp:314`); the TopBar fader shows 1.0 if the fader-link lane has landed |
| 4 | Speed (UPC) | thumb/track | readout **0.25** (`UniversalParamControl.cpp:185` paints the normalized value) = 1.0x — NOT 0.50 (that would mean the 0.5 fallback won, i.e. §3.3 missing) |
| 5 | Position X/Y, Scale, Rotation, Anchor (UPC) | thumb/track | 0.50 each |
| 6 | Master, connected (triangle → Audio → RMS, with audio playing) | thumb/track | jumps to 1.00, holds ~250 ms (`Composition.h:81` gripHoldMs), glides back to the signal — VISIBLE. Before the fix: nothing visible |
| 7 | any UPC, name/value text area | the label | same as its thumb (one routine) |
| 8 | an expanded UPC's Range min / max sliders | thumb or text box | 0.00 / 1.00 |
| 9 | Dashboard macro knobs 1-8 | the knob body | 0.50 (unchanged behaviour; pin) |
| 10 | Global Effects: add "Ripple", one param + Dry/Wet | thumb | the effect's `EffectLibrary` default / 1.00 (pin) |
| 11 | Left-click/drag on every control above | — | unchanged (drag works, +/- work, text box edits on click) |

Fallback if row 1/2 fails live (text box still dead — the only INFERRED link left): §2 E — override
`lookAndFeelChanged()` in `ResettableSlider` to call the base then `getChildComponent(i)->addMouseListener(&childListener_, false)`
for every child (`juce_Slider.cpp:612` does exactly this for `LinearBar`). Same listener object, same `mouseDown`.

---

## 6. Scope boundaries (surgical)

IN: `src/ui/UniversalParamControl.h` (ResettableSlider only), `src/ui/UniversalParamControl.cpp` (4 hunks in §3.2),
`src/ui/CompositionInspector.cpp` (1 statement in `bind`), `tests/test_resettable_slider.cpp` (new),
`tests/CMakeLists.txt` (1 block), `CLAUDE.md` (2 paragraphs).
OUT (do not touch): `+`/`-` button behaviour (`Slider` children and UPC's own `decrementBtn_/incrementBtn_`);
`Knob`/`MacroPanel` (their name/value Labels are the Knob's children, not the slider's — right-click there stays
inert; follow-up if Boris asks); `AudioDNALookAndFeel`; the twin `opacityControl_` (sibling lane, diag §2);
`LayerInspector`/`ClipInspector`/`SignalInspector`/`MappingEditor`/`TopBar` (all already armed; they inherit the
text-box fix for free); moving `ResettableSlider` to its own header; `tests/CMakeLists.txt:1138-1142`'s "NEVER"
comment (leave; the new block explains the exception); `TestServer`/REST (no mouse seam to add).
MERGE ORDER: land the twin removal (diag §2, deletes `CompositionInspector.cpp:414` and `:150-162`) BEFORE this
lane or in the same worktree — §3.3 edits `:408-412`, two lines above the twin's `bind` line; adjacent hunks
conflict. The fader-link lane (diag §3) touches none of this lane's files (its edit list: TopBar, MainComponent,
Renderer, ApiServer, TestServer — VERIFIED against diag §3).

---

## 7. RISKS

- R-plumbing (INFERRED → live row 1/2): JUCE's deep-listener delivery for the slider's text box is source-verified
  (E3) but not executable headlessly (`internalMouseDown` private). Fallback named in §5. Cheapest refutation: row 1.
- R-buttons: right-click on `+`/`-` still increments (stock, E4). If Boris reads that as "still broken", it is §2 F
  (LookAndFeel button subclass), not this lane.
- R-fallback-0.5: any FUTURE UPC whose owner forgets `setDefaultValue` now resets to 0.5 instead of doing nothing
  (today's name-area path already does that, `UniversalParamControl.cpp:296`; every current owner arms, E7).
- R-hook-contract: `resetToDefault()` is silent when a hook is installed. Only `UniversalParamControl` installs one;
  bare sliders keep `sendNotificationSync` (case 8 pins it). A future owner installing a hook must write its model
  itself — stated in the header comment.
- R-test-infra: first ctest target linking `juce_gui_basics`. Proven on this Mac (E1/E2, Release, no sanitizers —
  `ADNA_SANITIZE` unset in `build/CMakeCache.txt`). Under `-DADNA_SANITIZE=address` JUCE's GUI singletons may report
  leaks at exit — same exposure class as the juce_graphics/juce_opengl targets already in the suite; not default.
  If a CI box without a window server ever runs ctest, `Desktop`/`Displays` init is the first thing to suspect.
- R-edit-mode: right-clicking the text box WHILE its editor is open resets and closes the editor (`Slider::setValue`
  hides it, `juce_Slider.cpp:220-221`) and the TextEditor's own Cut/Copy/Paste popup may still appear (async).
  Cosmetic; not gated.
- R-merge: adjacency with the twin lane (§6).
- Learning for Harmony to log (no Harmony_Main boot here): "JUCE Components ARE unit-testable under ctest on macOS:
  `ScopedJuceInitialiser_GUI` (function-local static) + synthetic `MouseEvent` via the public `mouseDown`, no
  window/peer; `getMainMouseSource()` returns by value. Contradicts tests/CMakeLists.txt:1142's 'never'."

STATUS: COMPLETE — build-ready; two spikes executed (E1/E2) back the test design; one INFERRED link (deep-listener
delivery live) with a named fallback.
