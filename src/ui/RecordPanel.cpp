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
    nameEditor_.setTooltip("The name of the next take. Leave it blank to name it by date and time.");

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

    applyView(juce::Time::getMillisecondCounterHiRes() / 1000.0);   // row 1 until the first refresh
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

void RecordPanel::setNotice(const std::string& text)
{
    notice_ = juce::String(text);
    noticeAt_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void RecordPanel::refresh(const RecorderHost::Status& status, double nowSeconds)
{
    lastStatus_ = status;
    applyView(nowSeconds);
}

void RecordPanel::visibilityChanged()
{
    // A tab switch shows the current state at once, not up to 250 ms later.
    if (isVisible())
        applyView(juce::Time::getMillisecondCounterHiRes() / 1000.0);
}

void RecordPanel::runAction(const std::function<std::string()>& action)
{
    // Clear the old notice first: anything the funnel notifies DURING the
    // action (via the app's notify fan-out) survives; a returned
    // refusal replaces it.
    notice_.clear();
    noticeAt_ = -1.0;
    const auto result = action();
    if (!result.empty())
        setNotice(result);
    applyView(juce::Time::getMillisecondCounterHiRes() / 1000.0);
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
            button.setColour(juce::TextButton::textColourOffId, juce::Colour(AudioDNALookAndFeel::kTextPrimary));
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
    lastView_ = deriveRecordPanelView(lastStatus_, in);
    const auto& v = lastView_;

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

    statusLabel_.setText(v.statusText, juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(v.statusTone == RecordPanelView::Tone::Recording ? AudioDNALookAndFeel::kMeterRed
                                      : v.statusTone == RecordPanelView::Tone::Playing   ? AudioDNALookAndFeel::kMeterGreen
                                                                                         : AudioDNALookAndFeel::kTextPrimary));
    warningLabel_.setText(v.warningText, juce::dontSendNotification);
    noticeLabel_.setText(v.noticeText, juce::dontSendNotification);
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

    // Row C: name + the two switches (switches keep their width; the name stretches).
    auto rowC = area.removeFromTop(kControlHeight);
    playWithAudioToggle_.setBounds(rowC.removeFromRight(130));
    recordAudioToggle_.setBounds(rowC.removeFromRight(110));
    rowC.removeFromRight(4);
    nameEditor_.setBounds(rowC.reduced(1, 2));

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
}
