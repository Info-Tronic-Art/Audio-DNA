#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "core/EffectScope.h"
#include "effects/EffectLibrary.h"
#include "ui/UniversalParamControl.h"
#include "routing/MacroBank.h"
#include "ui/LookAndFeel.h"
#include <vector>
#include <memory>
#include <functional>
#include <optional>

// EffectStackView: vertical list of effects, each collapsible.
//
// Collapsed (single line):
//   [B] [icon] Ripple                  0.72
//
// Expanded (click to expand):
//   [B] [icon] Ripple
//     Frequency  0.50  [-][+] [═══╪══════]
//       ← Volume ▓▓▓▓▓░░░░░░ (source viz)
//     Amplitude  0.72  [-][+] [══════╪═══]
//       ← Beat Position ▓▓▓▓▓▓▓░░░ (viz)
//     Speed      0.30  [-][+] [══╪═══════]
//       ← Manual
class EffectStackView : public juce::Component,
                        public juce::DragAndDropTarget
{
public:
    EffectStackView();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the effect list to display (clip / layer / global effects). The scope
    // is captured alongside the vector so the three structural edits below can be
    // wrapped as undo commands that re-resolve the chain by coordinate (never via
    // the stored effects_ pointer). Hosts pass their scope here; a None scope
    // (default) leaves edits un-undoable but never mis-targets another chain.
    void setEffects(std::vector<Clip::EffectSlot>* effects,
                    EffectScope scope = EffectScope::none());

    // Set the effect library for parameter name lookup
    void setEffectLibrary(EffectLibrary* lib) { effectLibrary_ = lib; }

    // Set signal registry for source pickers
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }

    // Set macro bank for macro-driven parameters
    void setMacroBank(MacroBank* bank) { macroBank_ = bank; }

    // Refresh display from current effect data
    void refresh();

    // L9 (modulation-freeze fix, 2026-09-05): compute+apply signal/macro-
    // driven param values and write them into *effects_ — the part of the
    // old refresh() that the render path actually depends on every GL frame.
    // Split out so it can be driven unconditionally from
    // MainComponent::tickFeaturePipeline()'s 120Hz message-thread timer
    // instead of only running while this view's owning Inspector tab is
    // active. refresh() still calls this first, then does display-sync/paint
    // only — on-tab visible behavior is unchanged.
    void tickModulation();

    // Get preferred height for layout
    int getPreferredHeight() const;

    // DragAndDropTarget (for FX drops from browser)
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;

    // Callbacks
    std::function<void(int effectIndex, int paramIndex, float value)> onParamChanged;
    std::function<void(int effectIndex, bool bypassed)> onBypassChanged;
    std::function<void(int effectIndex, float dryWet)> onDryWetChanged;
    std::function<void(const juce::String& effectName)> onEffectAdded;
    std::function<void(int effectIndex)> onEffectRemoved;

    // Undo v1 step 7: route the three structural edits (add / remove / bypass
    // toggle) through the undo host so each becomes one command. Fired AFTER the
    // view has performed the live mutation and rebuilt itself (mutate-then-push).
    // Carries the scope captured AT THE GESTURE (the view's current scope_) plus a
    // whole-vector before/after snapshot and a human-readable description. Null in
    // headless / when no host is wired — the view then behaves exactly as before.
    using PerformEditFn =
        std::function<void(const EffectScope& scope,
                           std::vector<Clip::EffectSlot> before,
                           std::vector<Clip::EffectSlot> after,
                           const juce::String& description)>;
    PerformEditFn onPerformEdit;

    // GL fence for structural effect-list edits (family-fence fix round 2,
    // 2026-07-28): itemDropped's push_back and the delete button's erase both
    // reallocate *effects_, which the GL thread reads directly (clip.effects)
    // or via a copy (layer.layerEffects) — same crash-class exposure as the
    // structural deck/clip commands. Same shape as core/DeckCommands.h's
    // DeckFenceHook but declared locally: this UI-layer header must not
    // depend on core/. The host wires it to UndoService::withDeckDetached; a
    // null hook (default) runs the mutation directly — the headless case (no
    // UI test harness links this component today).
    using EffectFenceHook = std::function<void(const std::function<void()>&)>;
    void setFenceHook(EffectFenceHook hook) { fenceHook_ = std::move(hook); }

private:
    void runFenced(const std::function<void()>& m) { if (fenceHook_) fenceHook_(m); else if (m) m(); }
    EffectFenceHook fenceHook_;

    // One row per effect in the stack
    struct EffectRow
    {
        int effectIndex = 0;
        bool expanded = false;

        juce::TextButton bypassBtn{"B"};
        juce::TextButton deleteBtn{"X"};
        juce::Rectangle<int> headerBounds;

        // Dry/wet control (always first when expanded)
        std::unique_ptr<UniversalParamControl> dryWetControl;

        // Effect-specific parameter controls
        std::vector<std::unique_ptr<UniversalParamControl>> paramControls;

        // L9 fix-round (blocking review finding): the last value actually
        // PUSHED to the display for each paramControls[p] — parallel array,
        // same size, resized alongside paramControls in rebuildRows().
        // nullopt means "never pushed yet" so the first tick after a rebuild
        // always pushes regardless of what value it computes. Deliberately
        // NOT fx.paramValues[p] — that field is overwritten unconditionally
        // every tick by tickModulation() itself, so diffing against it
        // measures per-tick delta, not delta-since-last-displayed (see
        // tickModulation()).
        std::vector<std::optional<float>> lastPushedValue;
    };

    std::vector<std::unique_ptr<EffectRow>> rows_;
    std::vector<Clip::EffectSlot>* effects_ = nullptr;
    EffectScope scope_;   // which chain effects_ points at (for undo commands)
    EffectLibrary* effectLibrary_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;
    MacroBank* macroBank_ = nullptr;

    static constexpr int kHeaderHeight = 26;
    static constexpr int kParamIndent = 12;

    // L9 cost trap: tickModulation() now runs unconditionally at 120Hz
    // (previously ~10Hz and only while this tab was active). setSourceValue()
    // triggers an unconditional repaint(), so skip it when the modulated
    // value hasn't moved by more than this SINCE THE LAST VALUE ACTUALLY
    // PUSHED to the display (EffectRow::lastPushedValue) — not since the
    // last tick. A tick-to-tick epsilon check would suppress a slow
    // modulator's display update forever (fix-round finding); this one only
    // suppresses a truly-static repeat (e.g. a Manual/flatlined source),
    // since any genuinely-moving connected signal crosses this small a
    // threshold within a handful of 120Hz ticks regardless of how slowly it
    // moves.
    static constexpr float kModulationChangeEpsilon = 0.0005f;

    bool fxDropHighlight_ = false;

    void rebuildRows();
    void toggleExpand(int rowIndex);

    void mouseDown(const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectStackView)
};
