#include "ui/RecordPanel.h"

RecordPanel::RecordPanel()
{
    // Buttons. Texts double as accessibility titles: "Record Take"/"Play Take"
    // are deliberately NOT "Record"/"Play", so the Browser's "Record" tab button
    // keeps a unique title.
    addAndMakeVisible(recordBtn_);
    recordBtn_.setComponentID("recordTake");
    recordBtn_.onClick = [this] {
        if (lastStatus_.recording)
        {
            runAction([this] { return onStop ? onStop() : std::string(); });
            return;
        }
        RecordRequest request;
        request.name = juce::File::createLegalFileName(nameEditor_.getText().trim());
        request.audio = recordAudioToggle_.getToggleState();
        request.overdub = lastView_.recordSendsOverdub;
        runAction([this, request] {
            const auto result = onRecord ? onRecord(request) : std::string();
            // A take started: clear the name so the next take does not reuse
            // (and overwrite) the same folder -- blank means date and time.
            if (result.empty())
                nameEditor_.clear();
            return result;
        });
    };

    addAndMakeVisible(playBtn_);
    playBtn_.setComponentID("playTake");
    playBtn_.onClick = [this] {
        if (lastStatus_.playing)
        {
            runAction([this] { return onStopPlay ? onStopPlay() : std::string(); });
            return;
        }
        const bool withAudio = playWithAudioToggle_.getToggleState();
        runAction([this, withAudio] { return onPlay ? onPlay(withAudio) : std::string(); });
    };

    addAndMakeVisible(loadBtn_);
    loadBtn_.setComponentID("loadTake");
    loadBtn_.onClick = [this] {
        const auto start = takesRoot_.isDirectory()
            ? takesRoot_
            : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        chooser_ = std::make_shared<juce::FileChooser>("Choose a take folder", start);
        const auto flags = juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectDirectories;
        juce::Component::SafePointer<RecordPanel> safe(this);
        chooser_->launchAsync(flags, [safe](const juce::FileChooser& fc) {
            if (safe == nullptr)
                return;
            const auto folder = fc.getResult();
            if (folder.isDirectory())
                safe->runAction([p = safe.getComponent(), folder] {
                    return p->onLoad ? p->onLoad(folder) : std::string();
                });
        });
    };

    addAndMakeVisible(revealBtn_);
    revealBtn_.setComponentID("revealTake");
    revealBtn_.onClick = [this] {
        if (lastView_.revealFolder.isNotEmpty())
            juce::File(lastView_.revealFolder).revealToUser();
    };

    addAndMakeVisible(repairBtn_);
    repairBtn_.setComponentID("repairAudio");
    repairBtn_.onClick = [this] {
        runAction([this] { return onRepair ? onRepair() : std::string(); });
    };

    // Ruling 19: a real, visible "record audio" switch -- on at every launch,
    // not remembered between launches.
    addAndMakeVisible(recordAudioToggle_);
    recordAudioToggle_.setComponentID("recordAudio");
    recordAudioToggle_.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(playWithAudioToggle_);
    playWithAudioToggle_.setComponentID("playWithAudio");
    playWithAudioToggle_.onClick = [this] {
        // Only clickable while the model enables it, i.e. while the shown
        // value IS the user's own choice.
        playWithAudioPref_ = playWithAudioToggle_.getToggleState();
    };

    addAndMakeVisible(nameEditor_);
    nameEditor_.setComponentID("takeName");
    nameEditor_.setTextToShowWhenEmpty("Name (blank = date and time)",
                                       juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    // Format: D14 -- the future render item is shown greyed with a tooltip, not hidden.
    addAndMakeVisible(formatSelector_);
    formatSelector_.addItem("Take (timelines and audio)", 1);
    formatSelector_.addItem("Render... (coming)", 2);
    formatSelector_.setItemEnabled(2, false);
    formatSelector_.setSelectedId(1, juce::dontSendNotification);
    formatSelector_.setTooltip("Rendering a take to video is coming in a later build.");

    // Status lines -- BELOW the button rows, so a hover tooltip over a button
    // never covers them.
    addAndMakeVisible(statusLabel_);
    statusLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));

    addAndMakeVisible(warningLabel_);
    warningLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    warningLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kMeterYellow));

    addAndMakeVisible(noticeLabel_);
    noticeLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    noticeLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    addAndMakeVisible(takesRootLabel_);
    takesRootLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    takesRootLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    // ---- Routines strip (s-rta-0926 lane 3): a pad row + Save Routine ----
    addAndMakeVisible(routinesLabel_);
    routinesLabel_.setText("Routines", juce::dontSendNotification);
    routinesLabel_.setFont(juce::Font(juce::FontOptions(11.0f)).boldened());
    routinesLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));

    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
    {
        auto& pad = routinePads_[static_cast<size_t>(i)];
        addAndMakeVisible(pad);
        pad.setComponentID("routinePad" + juce::String(i));
        pad.onClick = [this, i] {
            runRoutineAction([this, i] {
                if (lastRoutineView_.pads[i].firing)
                    return onFireRoutine ? onFireRoutine(i) : std::string();
                return onStopRoutine ? onStopRoutine(i) : std::string();
            });
        };
    }

    addAndMakeVisible(saveRoutineLabel_);
    saveRoutineLabel_.setText("Save Routine", juce::dontSendNotification);
    saveRoutineLabel_.setFont(juce::Font(juce::FontOptions(11.0f)).boldened());
    saveRoutineLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));

    addAndMakeVisible(fromBarLabel_);
    fromBarLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    fromBarLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    addAndMakeVisible(fromBarEditor_);
    fromBarEditor_.setComponentID("routineFromBar");
    fromBarEditor_.setInputRestrictions(5, "0123456789");
    fromBarEditor_.setJustification(juce::Justification::centred);
    fromBarEditor_.setText("1", juce::dontSendNotification);
    fromBarEditor_.setTooltip("The first bar of the take to save, counted from 1.");

    addAndMakeVisible(toBarLabel_);
    toBarLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    toBarLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    addAndMakeVisible(toBarEditor_);
    toBarEditor_.setComponentID("routineToBar");
    toBarEditor_.setInputRestrictions(5, "0123456789");
    toBarEditor_.setJustification(juce::Justification::centred);
    toBarEditor_.setText("4", juce::dontSendNotification);
    toBarEditor_.setTooltip("The last bar of the take to save, counted from 1 (inclusive).");

    addAndMakeVisible(routineNameEditor_);
    routineNameEditor_.setComponentID("routineName");
    routineNameEditor_.setTextToShowWhenEmpty("Name (blank = Routine N)",
                                              juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    addAndMakeVisible(saveRoutineBtn_);
    saveRoutineBtn_.setComponentID("saveRoutine");
    saveRoutineBtn_.setTooltip("Saves the chosen bars of the loaded take onto the first empty pad.");
    saveRoutineBtn_.onClick = [this] {
        // review-routine-strip-r1.md: the raw editors accept blank/0 (and
        // toBar < fromBar); clamp before onSaveRoutine ever sees them.
        const auto range = clampRoutineBarRange(fromBarEditor_.getText().getIntValue(),
                                                 toBarEditor_.getText().getIntValue());
        const auto name = routineNameEditor_.getText().trim();
        runRoutineAction([this, name, range] {
            const auto result = onSaveRoutine ? onSaveRoutine(name, range.fromBar, range.toBar) : std::string();
            if (result.empty())
                routineNameEditor_.clear();   // saved: clear the name like the take name field does
            return result;
        });
    };

    addAndMakeVisible(routineNoticeLabel_);
    routineNoticeLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    routineNoticeLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    applyView(juce::Time::getMillisecondCounterHiRes() / 1000.0);   // row 1 until the first refresh
    applyRoutines(juce::Time::getMillisecondCounterHiRes() / 1000.0);
}

void RecordPanel::setTakesRoot(const juce::File& root)
{
    takesRoot_ = root;
    auto shown = root.getFullPathName();
    const auto home = juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName();
    if (home.isNotEmpty() && shown.startsWith(home))
        shown = "~" + shown.substring(home.length());
    takesRootLabel_.setText("Takes: " + shown, juce::dontSendNotification);
    takesRootLabel_.setTooltip(root.getFullPathName());
}

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

void RecordPanel::refresh(const RecorderHost::Status& status, double nowSeconds)
{
    lastStatus_ = status;
    applyView(nowSeconds);
    applyRoutines(nowSeconds);
}

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

void RecordPanel::applyButton(juce::TextButton& button, const RecordPanelView::Button& spec)
{
    button.setButtonText(spec.text);
    button.setEnabled(spec.enabled);
    button.setTooltip(spec.tooltip);
    button.setAlpha(spec.enabled ? 1.0f : kDisabledAlpha);
    switch (spec.tone)
    {
        case RecordPanelView::Tone::Recording:
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(AudioDNALookAndFeel::kMeterRed));
            // Fix plan F8: black on kMeterRed is 5.5:1 (WCAG AA); kTextPrimary on it was 2.9:1.
            button.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            break;
        case RecordPanelView::Tone::Playing:
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(AudioDNALookAndFeel::kMeterGreen));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(AudioDNALookAndFeel::kBackground));
            break;
        case RecordPanelView::Tone::Neutral:
        case RecordPanelView::Tone::Warning:
            // The LookAndFeel's own button colours -- same as every other button in the app.
            button.removeColour(juce::TextButton::buttonColourId);
            button.removeColour(juce::TextButton::textColourOffId);
            break;
    }
}

void RecordPanel::applyView(double nowSeconds)
{
    RecordPanelInputs in;
    in.nowSeconds = nowSeconds;
    in.recordAudio = recordAudioToggle_.getToggleState();
    in.playWithAudio = playWithAudioPref_;
    in.notice = notice_;
    in.noticeAtSeconds = noticeAt_;
    in.noticeKey = noticeKey_;
    lastView_ = deriveRecordPanelView(lastStatus_, in);
    const auto& v = lastView_;

    // The model stopped showing the stored notice (expired, or its situation is over): forget it, so a return to
    // the same situation within kNoticeSeconds cannot bring it back.
    if (notice_.isNotEmpty() && !v.noticeLive)
        forgetNotice();

    applyButton(recordBtn_, v.record);
    applyButton(playBtn_, v.play);
    applyButton(loadBtn_, v.load);
    applyButton(revealBtn_, v.reveal);
    applyButton(repairBtn_, v.repair);

    recordAudioToggle_.setEnabled(v.recordAudioEnabled);
    recordAudioToggle_.setAlpha(v.recordAudioEnabled ? 1.0f : kDisabledAlpha);
    recordAudioToggle_.setTooltip(v.recordAudioTooltip);

    playWithAudioToggle_.setEnabled(v.playWithAudioEnabled);
    playWithAudioToggle_.setAlpha(v.playWithAudioEnabled ? 1.0f : kDisabledAlpha);
    playWithAudioToggle_.setToggleState(v.playWithAudioValue, juce::dontSendNotification);
    playWithAudioToggle_.setTooltip(v.playWithAudioTooltip);

    nameEditor_.setEnabled(v.nameEnabled);
    nameEditor_.setAlpha(v.nameEnabled ? 1.0f : kDisabledAlpha);
    nameEditor_.setTooltip(v.nameTooltip);

    statusLabel_.setText(v.statusText, juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(v.statusTone == RecordPanelView::Tone::Recording ? AudioDNALookAndFeel::kMeterRed
                                      : v.statusTone == RecordPanelView::Tone::Playing   ? AudioDNALookAndFeel::kMeterGreen
                                                                                         : AudioDNALookAndFeel::kTextPrimary));
    warningLabel_.setText(v.warningText, juce::dontSendNotification);
    noticeLabel_.setText(v.noticeText, juce::dontSendNotification);
}

void RecordPanel::applyPad(juce::TextButton& button, const RoutineBankView::Pad& spec)
{
    button.setButtonText(spec.text);
    button.setEnabled(spec.enabled);
    button.setTooltip(spec.tooltip);
    button.setAlpha(spec.enabled ? 1.0f : kDisabledAlpha);
    switch (spec.tone)
    {
        case RoutineBankView::Tone::Playing:
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(AudioDNALookAndFeel::kMeterGreen));
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(AudioDNALookAndFeel::kBackground));
            break;
        case RoutineBankView::Tone::Warning:
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(AudioDNALookAndFeel::kMeterYellow));
            button.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            break;
        case RoutineBankView::Tone::Neutral:
            button.removeColour(juce::TextButton::buttonColourId);
            button.removeColour(juce::TextButton::textColourOffId);
            break;
    }
}

void RecordPanel::applyRoutines(double nowSeconds)
{
    const auto status = onRoutineStatus ? onRoutineStatus() : RoutineEngine::Status{};
    lastRoutineView_ = deriveRoutineBankView(status);
    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
        applyPad(routinePads_[static_cast<size_t>(i)], lastRoutineView_.pads[i]);

    const bool live = routineNotice_.isNotEmpty() && routineNoticeAt_ >= 0.0
                    && nowSeconds - routineNoticeAt_ <= kNoticeSeconds;
    routineNoticeLabel_.setText(live ? routineNotice_ : juce::String(), juce::dontSendNotification);
    if (!live)
    {
        routineNotice_.clear();
        routineNoticeAt_ = -1.0;
    }
}

void RecordPanel::runRoutineAction(const std::function<std::string()>& action)
{
    const auto result = action();
    if (!result.empty())
    {
        routineNotice_ = juce::String(result);
        routineNoticeAt_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    }
    else
    {
        routineNotice_.clear();
        routineNoticeAt_ = -1.0;
    }
    applyRoutines(juce::Time::getMillisecondCounterHiRes() / 1000.0);
}

void RecordPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void RecordPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Row A: the two lifecycle buttons (each flips its own label).
    auto rowA = area.removeFromTop(kControlHeight);
    recordBtn_.setBounds(rowA.removeFromLeft(120).reduced(1, 0));
    rowA.removeFromLeft(2);
    playBtn_.setBounds(rowA.removeFromLeft(120).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Row B: take housekeeping.
    auto rowB = area.removeFromTop(kControlHeight);
    loadBtn_.setBounds(rowB.removeFromLeft(100).reduced(1, 0));
    rowB.removeFromLeft(2);
    revealBtn_.setBounds(rowB.removeFromLeft(110).reduced(1, 0));
    rowB.removeFromLeft(2);
    repairBtn_.setBounds(rowB.removeFromLeft(100).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Row C: the take name -- its placeholder is a whole sentence, so it gets the width (fix plan F4).
    auto rowC = area.removeFromTop(kControlHeight);
    nameEditor_.setBounds(rowC.removeFromLeft(std::min(rowC.getWidth(), 260)).reduced(1, 2));

    area.removeFromTop(kRowSpacing);

    // Row C2: the two switches.
    auto rowC2 = area.removeFromTop(kControlHeight);
    recordAudioToggle_.setBounds(rowC2.removeFromLeft(130));
    rowC2.removeFromLeft(6);
    playWithAudioToggle_.setBounds(rowC2.removeFromLeft(150));

    area.removeFromTop(kRowSpacing);

    // Status, warning and notice lines -- below every button row.
    statusLabel_.setBounds(area.removeFromTop(kLabelHeight + 2));
    warningLabel_.setBounds(area.removeFromTop(kLabelHeight));
    noticeLabel_.setBounds(area.removeFromTop(kLabelHeight));

    area.removeFromTop(kRowSpacing);

    // Row D: format + where takes go.
    auto rowD = area.removeFromTop(kControlHeight);
    formatSelector_.setBounds(rowD.removeFromLeft(200).reduced(1, 0));
    rowD.removeFromLeft(6);
    takesRootLabel_.setBounds(rowD);

    area.removeFromTop(kRowSpacing);

    // ---- Routines strip (s-rta-0926 lane 3): 8 pads, 2 per row, then Save Routine. Two per row
    // (not four) so a running pad's "N: <name> (bar N)" suffix has room at the app's fixed 14pt
    // button font, which clips rather than shrinking (AudioDNALookAndFeel::drawButtonText).
    routinesLabel_.setBounds(area.removeFromTop(kLabelHeight));
    area.removeFromTop(2);

    static constexpr int kPadsPerRow = 2;
    for (int row = 0; row < RoutineEngine::kBankSize / kPadsPerRow; ++row)
    {
        auto padRow = area.removeFromTop(kControlHeight);
        const int padWidth = padRow.getWidth() / kPadsPerRow;
        for (int col = 0; col < kPadsPerRow; ++col)
        {
            const int i = row * kPadsPerRow + col;
            auto cell = padRow.removeFromLeft(padWidth);
            routinePads_[static_cast<size_t>(i)].setBounds(cell.reduced(1, 0));
        }
        area.removeFromTop(2);
    }

    area.removeFromTop(kRowSpacing);

    saveRoutineLabel_.setBounds(area.removeFromTop(kLabelHeight));
    area.removeFromTop(2);

    // Row E: From bar / To bar.
    auto rowE = area.removeFromTop(kControlHeight);
    fromBarLabel_.setBounds(rowE.removeFromLeft(52));
    fromBarEditor_.setBounds(rowE.removeFromLeft(50).reduced(1, 2));
    rowE.removeFromLeft(8);
    toBarLabel_.setBounds(rowE.removeFromLeft(40));
    toBarEditor_.setBounds(rowE.removeFromLeft(50).reduced(1, 2));

    area.removeFromTop(kRowSpacing);

    // Row F: name + the button.
    auto rowF = area.removeFromTop(kControlHeight);
    routineNameEditor_.setBounds(rowF.removeFromLeft(std::max(40, std::min(rowF.getWidth() - 110, 180))).reduced(1, 2));
    rowF.removeFromLeft(4);
    saveRoutineBtn_.setBounds(rowF.reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    routineNoticeLabel_.setBounds(area.removeFromTop(kLabelHeight));
}
