# s-rta-0925 -- Record panel step-4 deferred polish: ONE buildable plan (Architect)

Author: Architect (Fable), 2026-09-25. Inputs: fix-plan section 2 (`.harmony/.reports/s-rta-0924b/step4-visual/fix-plan.md:421-438`),
HANDOFF.md:21-22 and :54, `.harmony/s-rta-0924-work.md:104` (the live observation), the UX critic's D1 section
(`step4-visual/critic-ux.md:53-61`), and the sources read this session: `src/ui/RecordPanel.{h,cpp}`,
`src/ui/RecordPanelModel.h`, `src/ui/BrowserPanel.{h,cpp}`, `src/ui/LookAndFeel.cpp`, `src/MainComponent.cpp` (panel
wiring :1596-1614, notify fan-out :2012-2018, REST wiring :2039-2044, 4 Hz refresh :3384-3408, tick :3355-3360, perf*
funnel :5074-5240), `src/recording/RecorderHost.{h,cpp}`, `src/recording/RecorderClock.h`, `src/api/ApiServer.cpp` (perf
marshalling), `tests/test_record_panel_model.cpp`, `tests/test_recorder_host.cpp`, `tests/CMakeLists.txt`, and JUCE 8.0.4
sources under `build/_deps/juce-src` (TooltipWindow, Component, Button, Font, GlyphArrangement).
Labels: VERIFIED = read on disk at the cited line this session; INFERRED = derived from cited code; ASSUMED = stated so
the builder checks it. Read-only pass: no source edited, no app launched, `ctest -N` run once (445 tests listed).

---

## QUESTION

Plan the three deferred Record-panel polish items -- (1) a stale cyan notice that persists into later recorder states,
(2) D2 the clipped "Comp/Decks" browser tab, (3) D5 a pressed button showing its old label for up to 250 ms -- decide D1
(disabled-reason captions vs a tooltip-based version) on the merits, leave D6 alone, and unit-test what is testable.

## APPROACH (verdict first)

**Tie the notice to the recorder situation it was raised in; size the tabs from their measured labels; give the panel a
`status()` hook and make `arm()` publish synchronously; D1: the tooltip version is right and already exists for the five
buttons -- complete it for the two switches and the name field, model-only.** Four small lanes, all message-thread,
no hot-path code touched, +8 ctest cases (445 -> 453), fail-first on every piece that has logic.

- **Notice (item 1).** Root cause VERIFIED: the notice is the one piece of panel memory not derived from `Status`; it is
  cleared only by the 10 s expiry (`RecordPanelModel.h:38,235`) or by `runAction()` before a PANEL click
  (`RecordPanel.cpp:157-161`). Every REST-driven transition (ApiServer `callAsync` -> perf* funnel, `ApiServer.cpp:1233,
  1250,1278,1298,1315`) bypasses `runAction`, so the refused `perf/play` notice "No take is loaded. Use Load Take...
  first." survives into the `perf/record` states 1 s later (exactly the re-shoot script order,
  `plan-step4-record-panel.md:506-507`; observed live, `s-rta-0924-work.md:104`, HANDOFF.md:54). Fix: a pure
  `RecordPanelNoticeKey` (the situation: recording/playing/overdub/playMode/loadedTakeFolder) captured at `setNotice`
  from the host's published status, compared by the model every frame; the model reports `noticeLive` and the view
  forgets a dead notice so it cannot resurrect.
- **D5.** `runAction` re-applies `lastStatus_` (`RecordPanel.cpp:162-165` -> `:202`). Fix: `RecordPanel::onStatus`
  wired to `recorderHost_.status()`, read after the funnel returns. That only works if every transition publishes
  synchronously -- VERIFIED for disarm/load/play/stopPlay/stopPlayback (`RecorderHost.cpp:364,614,666,680,696`) and
  VERIFIED MISSING for `arm()` (`:143-262`, no `publishStatus()`) and `repairLoadedAudio()` (`:700-716`). Two one-line
  host additions, one RED host check.
- **D2.** `BrowserPanel::resized()` gives each of 6 tabs `width / 6` (`BrowserPanel.cpp:70-76`); the LookAndFeel draws
  button text at 14 px centred with `useEllipsesIfTooBig = false` (`LookAndFeel.cpp:78-86`), so a too-narrow tab clips.
  Fix: pure `tabWidthsFor(labelWidths, padding, totalWidth)` in a new header, labels measured with
  `juce::GlyphArrangement::getStringWidthInt` (JUCE 8: `Font::getStringWidth` is `[[deprecated]]`, `juce_Font.h:466`).
- **D1.** Decision: YES to the tooltip version, and it is already in place for all five buttons -- the model's disabled
  tooltips are reasons (`RecordPanelModel.h:143-165`), `applyButton` sets them regardless of enabled state
  (`RecordPanel.cpp:172`), and JUCE delivers tooltips to disabled components (VERIFIED in source: `TooltipWindow::getTipFor`
  gates only on foreground/no-button-down/modal, `juce_TooltipWindow.cpp:161-172`; `Component::internalMouseEnter` gates
  only on modal, `juce_Component.cpp:2088-2092`; `Component::getComponentAt` hit-tests on visibility only,
  `:1119-1134`; `Button` consults `isEnabled()` for clicks only, `juce_Button.cpp:382,394,643`). What is missing is the
  reason on the two switches and the name field while locked (`RecordPanelModel.h:169-176`, `RecordPanel.cpp:94`).
  Include that: model-only, no layout, tested. The on-screen caption stays deferred (fix-plan D1: clutter, ruling 5).

## TRADEOFFS CONSIDERED

- **Notice: clear it in the perf* funnel (or the six ApiServer lambdas) like `runAction` does** -- rejected. It clears on
  ACTIONS, not on situation changes; it couples the REST-shared funnel to a panel's memory at 6 more sites; and it is
  the wrong semantics -- a notice is advice about a situation ("No take is loaded", "Saved: x"), so the rule "shown
  while that situation lasts" belongs in the model that already owns what the notice line shows
  (`RecordPanelModel.h:12-13` "every state is derived from Status"). It is also robust to future transitions nobody
  routes through perf* (D6's eventual auto-end of playback would drop playing-era notices with no new clear call).
- **Notice: compare the key in the view only (`refresh()` forgets on mismatch), model untouched** -- rejected as the
  primary rule: the user-visible behaviour would live in 3 untested view lines. Chosen: the model decides (`noticeLive`,
  pinned by ctest) and the view forgets when told (2 lines) -- the forget is what stops a return to the same situation
  within 10 s from resurrecting a dead notice.
- **Notice key fields.** Chosen `{recording, playing, overdub, playMode, loadedTakeFolder}` -- exactly the fields whose
  change is a row change in the 4.3 matrix, and each is written only inside a transition that publishes synchronously
  (VERIFIED write sites: `recording_` :239/:355, `overdub_` :161/:193/:356, `playing_` :662/:676, `playMode_` :645,
  `loadedTakeFolder_` :609; publishes :364,:614,:666,:680,:696 -- and arm's, added here). Excluded on purpose:
  `takeFolder` (a refused arm overwrites `takeFolder_` at :159 BEFORE refusing at :185-210, and the next 120 Hz tick
  publishes it at :502 AFTER the funnel's "Could not start the take" notify -> it would erase that refusal within
  250 ms; it is the "stale Last take after a refused arm" already on record, `review-step4.md:47`); `audioStatus`
  (re-resolved by repair without a transition of its own -- a lingering "Could not repair" expires in 10 s instead);
  `framesWritten` (armed -> recording is the same take); every counter/clock field.
- **D2: proportional fallback vs equal split vs let the last tab overflow when the bar is too narrow** -- proportional
  chosen: the browser column is user-draggable (`MainComponent.cpp:2540-2555`, `vDividerFrac_[2]`), so "too narrow" is
  reachable; scaling every minimum by the same factor clips every label a little rather than one label a lot, keeps
  the longest label the widest, and stays a pure exact-sum function.
- **D2: rename "Comp/Decks" to "Decks" (B7)** -- not taken; fix-plan B7's default is "resize, no rename".
- **D5: return the status from each action lambda / have each app lambda call `refresh()`** -- rejected; one
  `onStatus` hook (same std::function shape as the six actions) also serves `visibilityChanged()`.
- **D1 caption line per button** -- stays deferred (fix-plan D1); the tooltip completion is the cheap version.

---

## DECISION / SPEC

### 0. Facts the plan rests on (all VERIFIED unless marked)

| # | Fact | Where |
|---|---|---|
| F-a | Notice memory: `notice_`/`noticeAt_`; cleared by expiry or `runAction` only | `RecordPanel.h:78-79`, `.cpp:136-140,157-161`; `RecordPanelModel.h:38,235-236` |
| F-b | REST perf calls are marshalled with `MessageManager::callAsync` to the funnel; never through `runAction` | `ApiServer.cpp:1233,1250,1278,1298,1315`; `MainComponent.cpp:2039-2044` |
| F-c | Notify fan-out: stderr + `setNotice(msg)`; contract "every function runs on the message thread" | `MainComponent.cpp:2012-2018`; `RecorderHost.h:48` |
| F-d | The 4 Hz refresh: `timerCallback` (30 Hz) every 8th call -> `refresh(recorderHost_.status(), now)` | `MainComponent.cpp:3384-3408` |
| F-e | `status()` = mutex-guarded copy of `published_`; `publishStatus()` writes it; no thread assert on `status()` | `RecorderHost.cpp:718-825`; `.h:237,367` |
| F-f | `arm()` never publishes; `repairLoadedAudio()` never publishes; every other transition does | `RecorderHost.cpp:143-262, 700-716` vs `:364,614,666,680,696` |
| F-g | Host notifies inside transitions: arm :258 (after `recording_ = true` :239), disarm :314/:336 (before publish :364), tick :393/:430/:484 (before publish :502) | `RecorderHost.cpp` |
| F-h | perfStop folds a finalize error into "Saved: x, but its audio had a problem: ..." after disarm returns | `MainComponent.cpp:5141-5146` |
| F-i | Fresh `RecorderClock` reads t = 0 before its first tick; `liveFramesWritten_ = 0` at arm | `RecorderClock.h:12-13,47`; `RecorderHost.cpp:225,753-754` |
| F-j | Tab bar: `tabWidth = width / 6`, last tab takes the rest; accent line uses button bounds | `BrowserPanel.cpp:64-87, 49-61` |
| F-k | Button text: `Font(FontOptions(14.0f))`, centred, no ellipsis | `LookAndFeel.cpp:78-86` |
| F-l | `GlyphArrangement::getStringWidthInt(const Font&, StringRef)` static; `Font::getStringWidth` deprecated | `juce_GlyphArrangement.h:338`; `juce_Font.h:466-467` |
| F-m | Tooltips reach disabled components (three gates checked, none on `isEnabled`) | `juce_TooltipWindow.cpp:161-172,209-221`; `juce_Component.cpp:1119-1134,2088-2092`; `juce_Button.cpp:382,394,643` |
| F-n | `test_record_panel_model` links juce_core only; 17 TEST_CASEs; JUCE-free target pattern = `test_ring_buffer` | `tests/CMakeLists.txt:1138-1163, 18-24`; `test_record_panel_model.cpp` |
| F-o | ctest lists 445 tests today | `cd build && ctest -N` |
| F-p | TooltipWindow: 600 ms, removable via Preferences "Show Tooltips" | `MainComponent.cpp:566, 2155-2164` |

### 1. Item 1 -- a notice belongs to the situation it was raised in

**Files.** `src/ui/RecordPanelModel.h`, `src/ui/RecordPanel.{h,cpp}`, `src/MainComponent.cpp` (one call site),
`tests/test_record_panel_model.cpp`.

**1.1 Model (`RecordPanelModel.h`).** Add after `kArmedWarnSeconds` (line 39):

```cpp
// ---- the situation a notice belongs to (s-rta-0925 step-4 polish, fix-plan section 2 "stale notice") ----
// A notice is advice about the recorder's situation when it was raised ("No take is loaded...", "Saved: x").
// It is shown while that situation lasts and dropped when it changes -- whichever comes first with the
// kNoticeSeconds expiry. The key holds exactly the Status fields whose change is a row change in the
// click-through matrix (rows 1-2 vs 3-5: loadedTakeFolder; rows 6-12: recording/playing/overdub/playMode), and
// ONLY fields the host publishes synchronously inside the transition that writes them (RecorderHost::disarm,
// load, play, stopPlay, stopPlayback -- and arm, since this lane). INVARIANT for future host edits: a write to
// any of these fields must be followed by publishStatus() in the same function, or a notice raised inside that
// transition is keyed to the OLD situation and silently dropped at the next refresh.
// Deliberately NOT in the key: takeFolder (a refused arm has already overwritten it before refusing, and the
// next tick publishes it AFTER the refusal notice -- it would erase "Could not start the take" within 250 ms),
// audioStatus (repair re-resolves it with no transition of its own), framesWritten (armed -> recording is the
// same take), t / position / every counter (they change every tick).
struct RecordPanelNoticeKey
{
    bool recording = false, playing = false, overdub = false;
    std::string playMode, loadedTakeFolder;
    bool operator==(const RecordPanelNoticeKey&) const = default;
};

inline RecordPanelNoticeKey noticeKeyOf(const RecorderHost::Status& s)
{
    return { s.recording, s.playing, s.overdub, s.playMode, s.loadedTakeFolder };
}
```

`RecordPanelInputs` (line 15-22): add `RecordPanelNoticeKey noticeKey;   // the situation the notice was raised in:
noticeKeyOf(status) read AFTER the event that produced it`. Its default equals `noticeKeyOf(Status{})`, so the existing
expiry test (idle `Status{}`) stays valid unchanged.

`RecordPanelView` (line 24-36): add `bool noticeLive = false;   // the stored notice is shown this frame; false =
expired or its situation is over -- the panel forgets it`.

Notice branch (line 233-238) becomes:

```cpp
    // ---- notice line: a refusal/notify for kNoticeSeconds AND only while the recorder is still in the
    // situation it was raised in (s-rta-0925: "No take is loaded" must not outlive the Load or the Record that
    // answered it; "Saved: x" must not survive into the next take); otherwise, while a take replays with its
    // audio, say what the relabelled Record button will do BEFORE it is pressed (fix plan F5).
    v.noticeLive = in.notice.isNotEmpty() && in.noticeAtSeconds >= 0.0
                && in.nowSeconds - in.noticeAtSeconds <= kNoticeSeconds
                && noticeKeyOf(s) == in.noticeKey;
    if (v.noticeLive)
        v.noticeText = in.notice;
    else if (withAudio && !recording)
        v.noticeText = "Record Over starts a new take on top of this audio; the loaded take is kept.";
```

C++20 notes (INFERRED, standard): a defaulted `operator==` does not stop the struct being an aggregate, so the braced
return in `noticeKeyOf` is fine; `<string>` is already included (line 5).

**1.2 View (`RecordPanel.h/.cpp`).**

Header: change the declaration (line 42-45) to
`void setNotice(const std::string& text, const RecorderHost::Status& raisedIn);` with the comment "Stores a one-line
notice (refusal or recorder notify) with the current time and the recorder situation it was raised in -- `raisedIn` is
`status()` read AFTER the event that produced the text (the funnel notifies after the host call returns; the host
publishes synchronously at every transition). Shown until it expires or that situation changes; applied at the next
refresh(), so a burst of notifications never repaints faster than the 4 Hz refresh." Add private `void forgetNotice();`
and member `RecordPanelNoticeKey noticeKey_;` next to `notice_`/`noticeAt_` (line 78-79). Update the class comment's
last sentence ("returns "" on success or the refusal text, which the panel shows as a notice" -> "... as a notice for
kNoticeSeconds or until the recorder's situation changes").

Source:

```cpp
void RecordPanel::setNotice(const std::string& text, const RecorderHost::Status& raisedIn)
{
    notice_ = juce::String(text);
    noticeAt_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    noticeKey_ = noticeKeyOf(raisedIn);
}

void RecordPanel::forgetNotice()
{
    notice_.clear();
    noticeAt_ = -1.0;
}
```

`runAction` (line 155-166): replace the two clearing lines with `forgetNotice();` (D5 rewrites the rest, section 2).
`applyView` (line 194-230): add `in.noticeKey = noticeKey_;` after `in.noticeAtSeconds = noticeAt_;`, and right after
`const auto& v = lastView_;`:

```cpp
    // The model stopped showing the stored notice (expired, or its situation is over): forget it, so a return to
    // the same situation within kNoticeSeconds cannot bring it back.
    if (notice_.isNotEmpty() && !v.noticeLive)
        forgetNotice();
```

**1.3 App (`MainComponent.cpp:2012-2018`).** `browserPanel_->getRecordPanel().setNotice(msg, recorderHost_.status());`
and extend the comment: "...with the situation it was raised in: status() is read AFTER the event (the host publishes
synchronously at every transition, and the funnel notifies after the host call returns)". Message thread only (G10),
as today; one extra `status()` copy per notify (a struct with ~12 std::strings under a std::mutex -- microseconds;
worst case the 120 Hz "deck unresolved" replay burst, `MainComponent.cpp:4097-4098`).

**1.4 Why every existing notify lands on the right key (INFERRED from F-f/F-g/F-h).**
- Funnel refusals (`:5078,5117,5135,5157,5173,5234`): no transition -> key = current situation; kept until expiry or
  a real change. The refused-arm `takeFolder_` churn is outside the key by design.
- "Saved: x" (`:5141-5146`) and the stop-playback notes (`:5210-5211`): notified after `disarm()`/`stopPlayback()`
  returned and published -> key = idle -> kept while idle, dropped when the next take starts. Expected and wanted.
- Host notifies inside `disarm()` (:314, :336): keyed to the recording situation (pre-publish) -> dropped at the next
  refresh -- harmless, because perfStop re-states the finalize error in the post-publish "Saved: x, but its audio had
  a problem" line (F-h) and the save-failure path returns a refusal the funnel notifies post-publish.
- Host notifies inside `tick()` (:393, :430, :484): key fields do not change inside a tick -> same key -> kept.
- The provisional-save notify inside `arm()` (:258): keyed correctly ONLY once arm publishes before it (section 2.3).

**1.5 Tests (`tests/test_record_panel_model.cpp`) -- RED first.**

(a) Row 9 (`:314-319`): the existing "a fresh notice wins" check must now say where the notice came from -- insert
`in.noticeKey = noticeKeyOf(s);` before `CHECK(deriveRecordPanelView(s, in).noticeText == "x");` and add the mirror
that is RED on the stub:

```cpp
    // s-rta-0925: a notice raised BEFORE Play (idle situation) is over once the take replays -- the hint shows.
    RecordPanelInputs before = inputs();
    before.notice = "x";
    before.noticeAtSeconds = 100.0;
    CHECK(deriveRecordPanelView(s, before).noticeText == "Record Over starts a new take on top of this audio; the loaded take is kept.");
    CHECK_FALSE(deriveRecordPanelView(s, before).noticeLive);
```

(b) New TEST_CASE after the expiry case (`:434`):

```cpp
TEST_CASE("RecordPanelModel notice -- shown only while the recorder is still in the situation it was raised in", "[recordpanel][model][notice]")
{
    auto in = inputs(100.0);
    in.notice = "No take is loaded. Use Load Take... first.";
    in.noticeAtSeconds = 99.0;
    in.noticeKey = noticeKeyOf(RecorderHost::Status{});          // raised idle, nothing loaded (row 1)

    SECTION("still idle: shown and live")
    {
        const auto v = deriveRecordPanelView(RecorderHost::Status{}, in);
        CHECK(v.noticeText == in.notice);
        CHECK(v.noticeLive);
    }
    SECTION("a refused arm changed takeFolder but not the situation: still shown")
    {
        RecorderHost::Status s;
        s.takeFolder = kLastTake;
        CHECK(deriveRecordPanelView(s, in).noticeLive);
    }
    SECTION("recording started (REST, no panel click): gone, and the panel is told to forget it")
    {
        const auto v = deriveRecordPanelView(recordingStatus(true, 0, 0.2), in);
        CHECK(v.noticeText.isEmpty());
        CHECK_FALSE(v.noticeLive);
    }
    SECTION("a take was loaded: the advice is answered, gone")
    {
        CHECK_FALSE(deriveRecordPanelView(loadedStatus("Resolved"), in).noticeLive);
    }
    SECTION("raised while recording: survives every tick (t, frames, counts) but not the stop")
    {
        in.notice = "could not write provisional take.json";
        in.noticeKey = noticeKeyOf(recordingStatus(true, 0, 0.1));   // armed, keyed after arm's publish
        auto later = recordingStatus(true, 48000, 3.0);
        later.points = 9;
        later.gaps = 1;
        CHECK(deriveRecordPanelView(later, in).noticeLive);
        RecorderHost::Status stopped;
        stopped.takeFolder = later.takeFolder;
        CHECK_FALSE(deriveRecordPanelView(stopped, in).noticeLive);
    }
    SECTION("'Saved' raised after the stop stays while idle and goes when the next take starts")
    {
        RecorderHost::Status idle;
        idle.takeFolder = kLastTake;
        in.notice = "Saved: last";
        in.noticeKey = noticeKeyOf(idle);
        CHECK(deriveRecordPanelView(idle, in).noticeLive);
        CHECK_FALSE(deriveRecordPanelView(recordingStatus(true, 0, 0.0), in).noticeLive);
    }
    SECTION("raised while replaying: the position moves on, still shown; Stop Playback ends it")
    {
        auto p = loadedStatus("Resolved");
        playing(p, false);
        in.notice = "handleClipTrigger: deck unresolved";
        in.noticeKey = noticeKeyOf(p);
        p.positionSeconds = 30.0;
        p.unresolved = 5;
        CHECK(deriveRecordPanelView(p, in).noticeLive);
        CHECK_FALSE(deriveRecordPanelView(loadedStatus("Resolved"), in).noticeLive);
    }
    SECTION("expiry still applies in the same situation")
    {
        in.nowSeconds = 99.0 + kNoticeSeconds + 0.5;
        CHECK_FALSE(deriveRecordPanelView(RecorderHost::Status{}, in).noticeLive);
    }
}
```

RED protocol: land a STUB first (repo convention, commit 28477dc "fail-first ... stub + matrix tests"): the struct,
`noticeKeyOf` returning `RecordPanelNoticeKey{}` for every status, `noticeKey`/`noticeLive` fields, `noticeLive` set
from the OLD condition (no key compare). Expected on the stub: sections "recording started", "a take was loaded",
"...but not the stop", "'Saved'...", "Stop Playback ends it" FAIL; the other three pass; row 9's new
`before` checks FAIL. Then implement 1.1 -> all green.

### 2. Item D5 -- a pressed button shows its new label at once

**Files.** `src/ui/RecordPanel.{h,cpp}`, `src/MainComponent.cpp` (one line), `src/recording/RecorderHost.cpp`
(two lines), `tests/test_recorder_host.cpp`.

**2.1 View.** Header, next to the six action functions (`RecordPanel.h:32-37`):

```cpp
    // The recorder's current published status (RecorderHost::status()). Read right after an action returns, so
    // a pressed button shows its new label at once instead of at the next 4 Hz refresh (fix plan D5), and on a
    // tab switch. Message thread; every host transition publishes synchronously, so the read is post-action.
    std::function<RecorderHost::Status()> onStatus;
```

Source:

```cpp
void RecordPanel::visibilityChanged()
{
    // A tab switch shows the current state at once, not up to 250 ms later.
    if (isVisible())
        refresh(onStatus ? onStatus() : lastStatus_, juce::Time::getMillisecondCounterHiRes() / 1000.0);
}

void RecordPanel::runAction(const std::function<std::string()>& action)
{
    // Clear the old notice first: anything the funnel notifies DURING the action (via the app's notify fan-out)
    // survives; a returned refusal replaces it.
    forgetNotice();
    const auto result = action();
    // Fix plan D5: the funnel has returned and the host published the post-action status -- show it now.
    const auto fresh = onStatus ? onStatus() : lastStatus_;
    if (!result.empty())
        setNotice(result, fresh);
    refresh(fresh, juce::Time::getMillisecondCounterHiRes() / 1000.0);
}
```

Order check (INFERRED): forget -> action (a notify inside it calls `setNotice(msg, status())`, post-transition key)
-> `fresh` is the same published copy -> a refusal is re-set with the same key -> `refresh` derives with equal keys ->
`noticeLive` true -> kept. `refresh()` itself is unchanged (`lastStatus_ = status; applyView(now);`).

**2.2 App (`MainComponent.cpp:1596-1614`).** In the S4-B block: `rp.onStatus = [this] { return recorderHost_.status(); };`

**2.3 Host (`RecorderHost.cpp`).** Two additions, both mirrors of `load()`'s line 614:

- `arm()`: insert immediately after `res.takeFolder = takeFolder_;` (currently line ~255, BEFORE the provisional save
  and its notify at :258):

```cpp
    // s-rta-0925 (step-4 polish D5): publish the armed state NOW, not at the next 120 Hz tick. The Record panel
    // reads status() the moment the funnel returns (its button must flip to "Stop Recording" at once), and any
    // notice raised from here on -- the provisional-save failure just below, or anything the funnel says -- is
    // keyed to the RECORDING situation (RecordPanelModel.h RecordPanelNoticeKey), so the next refresh keeps it.
    // Same rule as load()'s publish. clock_ is fresh (t = 0 until the first tick) and liveFramesWritten_ is 0,
    // so the panel reads "Armed, waiting for audio...".
    publishStatus();
```

  Safety (VERIFIED F-i): `publishStatus()` reads `clock_.now()` (fresh clock -> zeros), `recorder_.current()` (valid
  after `recorder_.start` at :238), `liveFramesWritten_` (0 at :225; overdub/no-audio paths publish 0 via
  `tapWasStarted_ == false`, :753-754). The refusal paths (:147-210) return before this line -> unchanged.

- `repairLoadedAudio()`: after `loadedAudio_ = store_.resolve(loadedTake_->audio);` (:714):
  `publishStatus();   // s-rta-0925 D5: the re-resolved audioStatus reaches the panel and REST at once (mirror of load())`.

**2.4 Test (`tests/test_recorder_host.cpp`, TEST_CASE 1 `[host][arm]` at :149) -- RED first.** After
`CHECK(host.isRecording());` (:174):

```cpp
    // s-rta-0925 (D5): arm publishes synchronously -- status() says recording before any tick has run.
    {
        const auto st = host.status();
        CHECK(st.recording);
        CHECK(st.takeFolder == takeFolder.dir.getFullPathName().toStdString());
        CHECK(st.assetId == armRes.assetId);
        CHECK(st.framesWritten == 0);
    }
```

RED today: `published_` is default-constructed until the first tick (`recording == false`). No repair test: the
repository has no `[host][repair]` case and building an Incomplete-asset fixture for a one-line mirror is
disproportionate -- covered by review (same shape as :614) and by the live gate's "Repair Audio" state if Harmony
chooses to exercise it.

### 3. Item D2 -- tabs sized by their labels

**Files.** NEW `src/ui/TabBarLayout.h`, `src/ui/BrowserPanel.{h,cpp}`, NEW `tests/test_tab_bar_layout.cpp`,
`tests/CMakeLists.txt`.

**3.1 Pure function (`src/ui/TabBarLayout.h`, header-only, no JUCE).**

```cpp
#pragma once
#include <algorithm>
#include <vector>

// TabBarLayout -- s-rta-0925 step-4 polish D2. Widths for a row of tab buttons across `totalWidth`: every tab gets
// at least its label width plus `padding` on each side (whole words, never clipped -- CLAUDE.md UI Text Rules),
// and the width left over is shared equally, one extra pixel to the leftmost tabs so the widths sum to
// `totalWidth` exactly (the last tab ends flush with the bar, as before). If even the minimums do not fit, every
// minimum is scaled by the same factor (the longest label keeps the largest share): a narrow bar clips every label
// a little rather than one label a lot. Pure -- the caller measures the labels; pinned by tests/test_tab_bar_layout.cpp.
inline std::vector<int> tabWidthsFor(const std::vector<int>& labelWidths, int padding, int totalWidth)
{
    const int n = static_cast<int>(labelWidths.size());
    std::vector<int> widths(static_cast<size_t>(n), 0);
    if (n == 0 || totalWidth <= 0)
        return widths;

    long long minSum = 0;
    for (int i = 0; i < n; ++i)
    {
        widths[static_cast<size_t>(i)] = std::max(0, labelWidths[static_cast<size_t>(i)]) + 2 * std::max(0, padding);
        minSum += widths[static_cast<size_t>(i)];
    }

    if (minSum <= totalWidth)
    {
        const int extra = totalWidth - static_cast<int>(minSum);
        for (int i = 0; i < n; ++i)
            widths[static_cast<size_t>(i)] += extra / n + (i < extra % n ? 1 : 0);
        return widths;
    }

    // Too narrow: floor(min_i * total / minSum), then the (< n) missing pixels go one each to the leftmost tabs.
    long long sum = 0;
    for (int i = 0; i < n; ++i)
    {
        widths[static_cast<size_t>(i)] = static_cast<int>((static_cast<long long>(widths[static_cast<size_t>(i)]) * totalWidth) / minSum);
        sum += widths[static_cast<size_t>(i)];
    }
    for (int i = 0; sum < totalWidth; ++i, ++sum)
        ++widths[static_cast<size_t>(i % n)];
    return widths;
}
```

**3.2 BrowserPanel.** Header: `static constexpr int kTabTextPadding = 8;   // px each side of a tab label (D2)` next
to `kTabBarHeight` (:62). Source: `#include "ui/TabBarLayout.h"` and replace `resized()`'s tab block (:68-76):

```cpp
    // Tab bar -- D2: each tab is at least its label plus padding, never clipped (TabBarLayout.h); the rest is shared.
    auto tabBar = area.removeFromTop(kTabBarHeight);
    juce::TextButton* tabs[] = { &filesTabBtn_, &fxTabBtn_, &sourcesTabBtn_,
                                 &compDecksTabBtn_, &recordTabBtn_, &milkDropTabBtn_ };
    const juce::Font tabFont(juce::FontOptions(14.0f));   // the font drawButtonText draws with (LookAndFeel.cpp:83)
    std::vector<int> labelWidths;
    for (auto* b : tabs)
        labelWidths.push_back(juce::GlyphArrangement::getStringWidthInt(tabFont, b->getButtonText()));
    const auto widths = tabWidthsFor(labelWidths, kTabTextPadding, tabBar.getWidth());
    for (size_t i = 0; i < std::size(tabs); ++i)
        tabs[i]->setBounds(tabBar.removeFromLeft(widths[i]));
```

`paint()`'s accent line and `updateTabButtonColors()` read button bounds/text -> no change. The measured font is
constructed exactly as `drawButtonText` constructs its own, so both resolve the same default typeface
(`LookAndFeel.cpp:50` `setDefaultSansSerifTypeface`) -- INFERRED identical metrics; the 8 px per side absorbs a
few px either way. Measurement happens only in `resized()` (message thread, in the running app; never headless).

**3.3 Tests (`tests/test_tab_bar_layout.cpp`, Catch2 only, `#include "ui/TabBarLayout.h"`) -- 6 TEST_CASEs, RED first.**

1. `"TabBarLayout -- the Browser's six tabs at the crop width: Comp/Decks gets its whole label"` -- labelWidths
   `{33, 18, 52, 78, 44, 58}` (ASSUMED 14 px approximations for Files/FX/Sources/Comp/Decks/Record/MilkDrop; the
   arithmetic is the pin, not the font), padding 8, total 428 -> `{58, 42, 76, 102, 68, 82}`; sum 428; the fourth
   >= 78 + 16 = 94. Comment the pre-fix rule: 428 / 6 = 71 < 94 (the clip in every step-4 crop).
2. `"exact sum and every minimum whenever the minimums fit"` -- loop over `{ {10,20,30}, {5}, {40,40,40,40} }`
   x totals `{100, 101, 333}` (skip combos that do not fit): sum == total, each width >= label + 16.
3. `"the leftover pixels go to the leftmost tabs, one each"` -- `{10,20,30}`, 8, 200 -> `{57, 67, 76}`.
4. `"equal labels give equal widths"` -- six labels of 20, padding 8: total 216 -> all 36; total 300 -> all 50.
5. `"too narrow: proportional, exact sum, the longest label keeps the largest share"` -- `{10,20,30}`, 8:
   total 54 -> `{13, 18, 23}`; total 55 -> `{14, 18, 23}`; total 1 -> `{1, 0, 0}`.
6. `"edges"` -- empty -> empty; total 0 -> zeros; one tab -> `{total}`; a negative label width counts as 0
   (`{-5}`, 8, 100 -> `{100}`); padding < 0 counts as 0.

RED protocol: stub header returns the OLD rule (`totalWidth / n` for every tab) -> cases 1, 3, 5 FAIL (case 1: 71 < 94
and sum 426 != 428); then implement 3.1 -> all six green.

**3.4 CMake (`tests/CMakeLists.txt`, append after the `test_httplib_bodyless_post` block, same shape as
`test_ring_buffer` :18-24).**

```cmake
# --- test_tab_bar_layout (s-rta-0925 step-4 polish D2: the Browser's tab widths
# follow the measured label widths so "Comp/Decks" is never clipped). Pure
# function in src/ui/TabBarLayout.h -- no JUCE; the caller measures the labels.
add_executable(test_tab_bar_layout test_tab_bar_layout.cpp)
target_include_directories(test_tab_bar_layout PRIVATE ${SRC_DIR})
target_link_libraries(test_tab_bar_layout PRIVATE Catch2::Catch2WithMain)
apply_sanitizers(test_tab_bar_layout)
catch_discover_tests(test_tab_bar_layout)
```

### 4. Item D1 -- the tooltip version, completed (severable: drop this section alone and nothing else changes)

**Decision on the merits.** A caption per dimmed button stays deferred (fix-plan D1). The tooltip-based reason is the
right cheap version because it already exists for the five buttons (F-m, `RecordPanelModel.h:143-165`,
`RecordPanel.cpp:172`) at zero layout cost; what is incomplete is that a LOCKED switch or name field still shows its
enabled-state tooltip: `recordAudioTooltip` while recording (`:169-170`), `playWithAudioTooltip` when nothing is
loaded / while recording / while replaying (`:173-175`), and the name editor's fixed tooltip (`RecordPanel.cpp:94`).
Model-only, ~12 lines, tested. Limits stated plainly: a tooltip needs a 600 ms hover and is off when Preferences
"Show Tooltips" is off (F-p); the status line still names the dominant reason in every idle state.

**4.1 Model (`RecordPanelModel.h:167-176`).**

```cpp
    // ---- toggles and name: a locked control says why in its tooltip (s-rta-0925 D1, the tooltip version --
    // JUCE shows tooltips on disabled components; the on-screen caption stays deferred, fix-plan D1) ----
    v.recordAudioEnabled = !recording && !withAudio;
    v.recordAudioTooltip = recording  ? "Locked while a take is recording."
                         : withAudio  ? "Recording over a take always uses that take's audio."
                                      : "Records the sound the app is listening to, alongside the timelines.";
    v.playWithAudioEnabled = idle && loaded && audioReady;
    v.playWithAudioValue = playing ? withAudio : (v.playWithAudioEnabled && in.playWithAudio);
    if (recording)                       v.playWithAudioTooltip = "Stop the recording first.";
    else if (playing)                    v.playWithAudioTooltip = "Locked while the take replays.";
    else if (loaded && !audioReady)      v.playWithAudioTooltip = "This take's audio is not available, so it replays without audio.";
    else if (!loaded)                    v.playWithAudioTooltip = "Load a take first.";
    else                                 v.playWithAudioTooltip = "Replays the take's own audio instead of the live input.";
    v.nameEnabled = !recording;
    v.nameTooltip = recording ? "Locked while a take is recording. It names the next take."
                              : "The name of the next take. Leave it blank to name it by date and time.";
```

Add `juce::String nameTooltip;` to `RecordPanelView` (next to the two toggle tooltips, :30). Row 4 (idle, loaded,
Incomplete) keeps its text (third branch); row 9's pinned `recordAudioTooltip` (`test:308`) is unchanged.

**4.2 View.** `applyView`: `nameEditor_.setTooltip(v.nameTooltip);` after `nameEditor_.setAlpha(...)` (:221); delete
the constructor's `nameEditor_.setTooltip(...)` (:94) so the model is the single source.

**4.3 Test (`tests/test_record_panel_model.cpp`, new TEST_CASE) -- RED first (today's strings are the generic ones).**

```cpp
TEST_CASE("RecordPanelModel disabled reasons -- a locked switch or name field says why (D1, tooltip version)", "[recordpanel][model]")
{
    const auto rec = deriveRecordPanelView(recordingStatus(true, 4800, 4.2), inputs());       // row 7
    CHECK(rec.recordAudioTooltip == "Locked while a take is recording.");
    CHECK(rec.playWithAudioTooltip == "Stop the recording first.");
    CHECK(rec.nameTooltip == "Locked while a take is recording. It names the next take.");

    const auto idle = deriveRecordPanelView(RecorderHost::Status{}, inputs());                  // row 1
    CHECK(idle.playWithAudioTooltip == "Load a take first.");
    CHECK(idle.recordAudioTooltip == "Records the sound the app is listening to, alongside the timelines.");
    CHECK(idle.nameTooltip == "The name of the next take. Leave it blank to name it by date and time.");

    auto wall = loadedStatus("Missing"); playing(wall, false);                                  // row 8
    CHECK(deriveRecordPanelView(wall, inputs()).playWithAudioTooltip == "Locked while the take replays.");

    CHECK(deriveRecordPanelView(loadedStatus("Incomplete"), inputs()).playWithAudioTooltip     // row 4, unchanged
          == "This take's audio is not available, so it replays without audio.");
    CHECK(deriveRecordPanelView(loadedStatus("Resolved"), inputs()).playWithAudioTooltip       // row 3, unchanged
          == "Replays the take's own audio instead of the live input.");
}
```

### 5. Build, RED protocol, counts

- Scratch build dir (rig rules: never `./build`): from the main checkout,
  `cmake -S . -B build-s4polish -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_JUCE=$PWD/build/_deps/juce-src
  -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$PWD/build/_deps/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$PWD/build/_deps/catch2-src`
  (FetchContent names VERIFIED: `CMakeLists.txt:28` JUCE, `:54` httplib, `tests/CMakeLists.txt:5` Catch2; add
  `-DFETCHCONTENT_SOURCE_DIR_SYPHON=$PWD/build/_deps/syphon-src` only if that directory exists -- ASSUMED, check `ls build/_deps`).
- Step A (RED): stubs (1.5, 3.3) + the three test edits + the CMake block; build only the test targets
  (`cmake --build build-s4polish --target test_record_panel_model test_tab_bar_layout test_recorder_host -j`); run
  `(cd build-s4polish && ctest -R "record_panel_model|tab_bar_layout|recorder_host" --output-on-failure)`; capture the
  named failures (section 1.5 / 2.4 / 3.3 lists) as the fail-first evidence.
- Step B (GREEN): implement 1.1-1.3, 2.1-2.3, 3.1-3.2, 4.1-4.2; rebuild; the same `ctest -R` green; full ctest:
  **445 + 8 = 453** (1 notice case + 1 D1 case + 6 tab-layout cases; host checks live inside case 1). Record the actual.
- Step C: build the app target in the scratch dir; `bash .harmony/probe-step3.sh` from the main checkout must hold
  69/0 (the arm publish only makes `recording: true` visible ~8 ms earlier to the probe's polls).
- Grep gates for the reviewer: `git diff --stat` touches only `src/ui/RecordPanel.{h,cpp}`, `src/ui/RecordPanelModel.h`,
  `src/ui/BrowserPanel.{h,cpp}`, `src/ui/TabBarLayout.h` (new), `src/recording/RecorderHost.cpp` (+2 lines + comments),
  `src/MainComponent.cpp` (2 lines), `tests/test_record_panel_model.cpp`, `tests/test_tab_bar_layout.cpp` (new),
  `tests/test_recorder_host.cpp`, `tests/CMakeLists.txt`; `grep -n "getStringWidth(" src/ui/BrowserPanel.cpp` = 0
  (deprecated API not used); every write to a key field in `RecorderHost.cpp` is followed by `publishStatus()` in the
  same function (`grep -n "recording_ =\|playing_ =\|overdub_ =\|playMode_ =\|loadedTakeFolder_ ="`).
- Commit hygiene: `git add -f` for anything under `.harmony/`; `git show --stat HEAD` after each commit.

### 6. Threading analysis (against CLAUDE.md sacred rules 1-4)

- Every changed line runs on the MESSAGE thread: `BrowserPanel::resized()`; `RecordPanel` button handlers, the 4 Hz
  refresh (`timerCallback`, F-d), `visibilityChanged()`; the notify fan-out (contract F-c; the REST path reaches it
  through `callAsync`, F-b); `RecorderHost::arm/repairLoadedAudio` (`RECORDER_HOST_ASSERT_MESSAGE_THREAD`, :145/:702).
- The only lock involved is `statusMutex_` (`std::mutex`, F-e), already taken today by `status()` at 4 Hz on the message
  thread and by `perfStatusVar()` on the HTTP thread, and by `publishStatus()` at 120 Hz. This plan adds: one
  `publishStatus()` per arm/repair (user-rate), one `status()` copy per notify and per panel action (user-rate; worst
  case the replay's per-event "deck unresolved" notifies at <= 120 Hz). None of it touches the audio callback, the
  analysis thread, the render thread or any GL state; no new mutex; no lock-free channel changed. Rules 1-4 hold.
- `GlyphArrangement::getStringWidthInt` needs a resolvable typeface: called only from `resized()` in the running app,
  never from a ctest target (the pure function is what ctest links).
- No allocation constraints apply (message thread); `std::vector` in `resized()` is fine.

### 7. Gate (Harmony) and what is Boris's to see

Screen-safety law unchanged: `open -g`, window-only `screencapture -l <CGWindowID>`, no GUI input by agents.

1. ctest RED evidence (step A), then 453/453 (step B); probe-step3 69/0 (step C).
2. Re-shoot the notice sequence with the step-4 section 6 script (`plan-step4-record-panel.md:506-509`), window-only:
   - `perf/play` refused -> crop shows the cyan "No take is loaded. Use Load Take... first." (unchanged);
   - `perf/record` 1 s later -> crop "armed-or-recording" shows NO cyan line (pre-fix: still shows it -- the defect);
   - `perf/stop` -> cyan "Saved: s4gate" present (the over-clearing guard: a post-stop notice must survive);
   - `perf/load` -> the "Saved" line is gone (loaded is a new situation; expected), status "Loaded: s4gate ...".
3. Tab bar crop at Boris's browser width: all six labels whole ("Comp/Decks" included), the last tab flush right, the
   cyan accent under the active tab spans exactly that tab. Then drag nothing -- if Harmony can safely narrow the
   window via AX, the proportional fallback is visible; otherwise it is ctest-only.
4. D5 and D1 cannot be verified by an agent without GUI input. For Boris, plain words: "Press Record Take -- the button
   turns red and says Stop Recording the moment you press it, not a blink later." and "Hold the mouse over a greyed-out
   switch or the name box while recording -- it tells you why it is locked." The host test pins D5's mechanism.
5. Critic re-brief scope: RecordPanelModel notice branch + key fields (section 1), the two host publishes (2.3), the
   pure tab function (3.1). Not D6.

---

## RISKS

- **R1 (quiet failure class) The key invariant.** A future host transition that writes a key field without publishing
  synchronously would key its own notifies to the OLD situation and drop them at the next refresh (<= 250 ms) --
  invisible unless someone watches for the message. Mitigations: the invariant is spelled out at the struct; the
  reviewer's grep in section 5; the host test pins arm. The exposure today is exactly the disarm-internal notifies
  (:314, :336), whose text the funnel re-states post-publish (F-h) -- VERIFIED, not assumed.
- **R2 The key is coarser than the matrix on purpose.** armed -> recording (framesWritten) and audio-status changes are
  not situation changes here; a "Could not repair the audio" notice therefore lingers up to 10 s after a later
  successful repair. Accepted (expiry covers it; the status line says "Audio: ready"). Adding `audioStatus` later is
  safe ONLY because repair now publishes -- say so if it is added.
- **R3 "Saved: x" disappears at Load.** By design (a loaded take is a new situation) and the gate expects it; if Boris
  wants "Saved" to outlive a Load, add `loadedTakeFolder` removal to the key -- one field, one test.
- **R4 Font measurement.** If the measured 14 px width differs from the drawn width (different typeface resolution --
  INFERRED equal, not measured), the 8 px per side absorbs small errors; a still-clipped label at Boris's width means the
  bar is genuinely narrower than the six minimums (proportional fallback) -> B7 rename is the remedy, not code.
- **R5 Tooltips on disabled controls are VERIFIED in JUCE source, not yet observed live in this app.** Boris's hover
  (gate 4) is the observation. If it does not show, D1's cheap version is dead for every button too -- then the fix is
  a mouse-transparent overlay or an enabled-but-inert button (not cheap) and D1 goes back to deferred.
- **R6 D5 depends on arm's publish.** Without 2.3, `onStatus()` right after `perfRecord` returns the pre-arm copy and
  the Record button still flips late (the very defect). The host check in 2.4 is RED without it -- do not drop 2.3 to
  "keep the host untouched".
- **R7 Pre-existing wart, out of scope, noted:** when only `take.save` fails at stop, `disarm` returns `ok = false`
  with `error = finalizeError` (`RecorderHost.cpp:340`), so perfStop's line reads "Could not stop the take: " with an
  empty reason (`MainComponent.cpp:5131-5136`). Unchanged by this plan; log it for a later lane.
- **R8 ctest count.** 453 expected; a builder who adds/merges cases records the actual and the reason.

**Strongest counterargument, and why it loses.** "Clear the notice at the top of each perf* function -- six one-liners,
no model change, no key, covers the observed case." It does cover the observed REST case, but: it clears on actions
rather than situations, so any transition not routed through perf* (D6's future auto-end of playback, shutdown, a
host-internal stop) leaves the stale line again with nobody remembering to add a clear; it reaches from the
REST-shared funnel into a panel's memory at six sites; and it puts the rule outside the one place that already owns
the notice line and is ctest-pinned. The key rule is ~20 model lines, one test case, and it is the rule the task
states ("the notice must clear when the recorder state changes") rather than an approximation of it.

---

## SUMMARY (for Harmony)

1. Stale notice: root cause is that the notice is the only panel memory not tied to `Status`; fix = a pure situation
   key (`recording/playing/overdub/playMode/loadedTakeFolder`) captured at `setNotice` from the host's published
   status, compared in the model (`noticeLive`), forgotten by the view. `takeFolder` is deliberately not in the key
   (refused arms churn it). +1 model case, row 9 extended.
2. D5: `RecordPanel::onStatus` -> `recorderHost_.status()` after the funnel returns; requires `arm()` and
   `repairLoadedAudio()` to `publishStatus()` synchronously (both missing today; +4 host checks, RED).
3. D2: pure `tabWidthsFor()` in `src/ui/TabBarLayout.h`, labels measured with `GlyphArrangement::getStringWidthInt`,
   proportional fallback when the bar is too narrow; +6 JUCE-free cases.
4. D1: tooltip version is right and mostly already there; complete it for the two switches and the name field
   (model-only, +1 case; severable).
5. All message-thread; sacred rules untouched. ctest 445 -> 453; probe-step3 69/0 must hold; window-only re-shoot of
   the notice sequence and the tab bar; D5/D1 are Boris's to see (press Record; hover a greyed switch).

REPORT_FILE: .harmony/.reports/s-rta-0925/plan-step4polish.md
STATUS: COMPLETE
