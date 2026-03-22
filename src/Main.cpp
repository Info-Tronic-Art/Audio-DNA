#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"

class AudioDNAApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "Audio-DNA"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override          { return false; }

    void initialise(const juce::String& commandLine) override
    {
        // Parse command-line arguments
        bool testMode = false;
        int testPort = 8080;

        auto args = juce::StringArray::fromTokens(commandLine, " ", "\"");
        for (const auto& arg : args)
        {
            if (arg == "--test-mode")
                testMode = true;
            else if (arg.startsWith("--test-port="))
                testPort = arg.fromFirstOccurrenceOf("=", false, false).getIntValue();
        }

        mainWindow_ = std::make_unique<MainWindow>(getApplicationName(), testMode, testPort);
    }

    void shutdown() override
    {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name, bool testMode = false, int testPort = 8080)
            : DocumentWindow(name,
                             juce::Colour(0xff1a1a2e),
                             DocumentWindow::allButtons)
        {
            auto* mainComp = new MainComponent(testMode, testPort);

            setUsingNativeTitleBar(true);
            setContentOwned(mainComp, true);
            setResizable(true, true);
            setResizeLimits(1280, 720, 3840, 2160);

            // Set the menu bar from MainComponent's model
            #if JUCE_MAC
                // On macOS, use the native menu bar at the top of the screen
                juce::MenuBarModel::setMacMainMenu(mainComp->getMenuBarModel());
            #else
                // On Windows/Linux, set the menu bar on the window
                setMenuBar(mainComp->getMenuBarModel());
            #endif

            // Open maximized to fill the screen
            auto display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
            if (display != nullptr)
            {
                auto area = display->userArea;
                setBounds(area);
            }
            else
            {
                centreWithSize(getWidth(), getHeight());
            }
            setVisible(true);
        }

        ~MainWindow() override
        {
            #if JUCE_MAC
                juce::MenuBarModel::setMacMainMenu(nullptr);
            #else
                setMenuBar(nullptr);
            #endif
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow_;
};

START_JUCE_APPLICATION(AudioDNAApplication)
