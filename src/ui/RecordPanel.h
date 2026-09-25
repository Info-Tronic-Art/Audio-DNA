#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "ui/RecordPanelModel.h"
#include <functional>
#include <string>

// RecordPanel -- the Browser's "Record" tab (s-rta-0924b step 4, Lane S4-C).
// A THIN VIEW: every button's text/enabled/tooltip/tone and every status line
// comes from deriveRecordPanelView() (ui/RecordPanelModel.h, ctest-pinned)
// applied to RecorderHost::Status, which the app hands to refresh() at
// ~4 Hz. The panel never talks to the recorder itself: each action is a
// std::function the app wires to its perf* funnel (the SAME functions
// the /api/perf/* REST endpoints call) and returns "" on success or the
// refusal text, which the panel shows as a notice for kNoticeSeconds or
// until the recorder's situation changes. No modal dialog anywhere
// (spec R10): Load uses FileChooser::launchAsync, which does not block the
// message loop.
class RecordPanel : public juce::Component
{
public:
    RecordPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

    // What pressing "Record Take" / "Record Over" asks for.
    struct RecordRequest { juce::String name; bool audio = true; bool overdub = false; };

    // Wired by the app to its perf* funnel. Each returns "" on success,
    // else the refusal/failure text. Called on the message thread (button clicks).
    std::function<std::string(const RecordRequest&)> onRecord;
    std::function<std::string()> onStop;
    std::function<std::string(bool withAudio)> onPlay;
    std::function<std::string()> onStopPlay;
    std::function<std::string(const juce::File& takeFolder)> onLoad;
    std::function<std::string()> onRepair;

    // The recorder's current published status (RecorderHost::status()). Read right after an action returns, so
    // a pressed button shows its new label at once instead of at the next 4 Hz refresh (fix plan D5), and on a
    // tab switch. Message thread; every host transition publishes synchronously, so the read is post-action.
    std::function<RecorderHost::Status()> onStatus;

    // The fixed takes folder: the Load dialog's starting folder and the caption.
    void setTakesRoot(const juce::File& root);

    // Stores a one-line notice (refusal or recorder notify) with the current
    // time and the recorder situation it was raised in -- `raisedIn` is
    // status() read AFTER the event that produced the text (the funnel notifies after the host call returns;
    // the host publishes synchronously at every transition). Shown until it expires or that situation changes;
    // applied at the next refresh(), so a burst of notifications never repaints faster than the 4 Hz refresh.
    void setNotice(const std::string& text, const RecorderHost::Status& raisedIn);

    // Model -> widgets. Called at ~4 Hz by the app.
    void refresh(const RecorderHost::Status& status, double nowSeconds);

private:
    void applyView(double nowSeconds);
    void runAction(const std::function<std::string()>& action);
    void forgetNotice();
    static void applyButton(juce::TextButton& button, const RecordPanelView::Button& spec);

    juce::TextButton recordBtn_{"Record Take"};
    juce::TextButton playBtn_{"Play Take"};
    juce::TextButton loadBtn_{"Load Take..."};
    juce::TextButton revealBtn_{"Show in Finder"};
    juce::TextButton repairBtn_{"Repair Audio"};

    juce::ToggleButton recordAudioToggle_{"Record audio"};
    juce::ToggleButton playWithAudioToggle_{"Play with audio"};
    juce::TextEditor nameEditor_;

    juce::ComboBox formatSelector_;

    juce::Label statusLabel_;
    juce::Label warningLabel_;
    juce::Label noticeLabel_;
    juce::Label takesRootLabel_;

    juce::File takesRoot_;
    std::shared_ptr<juce::FileChooser> chooser_;

    RecorderHost::Status lastStatus_;
    RecordPanelView lastView_;
    bool playWithAudioPref_ = true;   // the user's own choice; the model may force the shown value off
    juce::String notice_;
    double noticeAt_ = -1.0;
    RecordPanelNoticeKey noticeKey_;

    static constexpr int kLabelHeight = 14;
    static constexpr int kControlHeight = 28;
    static constexpr int kRowSpacing = 6;

    // Disabled controls in this panel render at this opacity so they don't
    // read as active controls (the app LookAndFeel doesn't dim disabled
    // components -- see drawButtonBackground/drawButtonText in
    // src/ui/LookAndFeel.cpp, which draw purely from buttonColourId/
    // textColourOffId with no isEnabled() check). Component::setAlpha()
    // is a Component-level compositing property, so this stays scoped to
    // RecordPanel without touching the shared LookAndFeel.
    static constexpr float kDisabledAlpha = 0.4f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordPanel)
};
