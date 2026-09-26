#pragma once
#include "routing/Route.h"
#include "signal/SignalRegistry.h"
#include "connect/ParamConnection.h"
#include "features/SignalDepth.h"
#include <array>
#include <vector>
#include <string>
#include <cstdint>

// MacroBank: 8 dashboard link knobs per scope (clip, layer, global).
// Each link can be driven by a Signal or set manually.
// Each link distributes its value to linked parameters with per-link range/invert.
// Resolume calls these "Dashboard Links" — 8 per scope (composition, layer, clip).
class MacroBank
{
public:
    static constexpr int kNumMacros = 8;

    enum class Scope : uint8_t { Clip, Layer, Global };

    struct MacroLink
    {
        RouteTarget target;   // Which parameter this macro drives
        float outputMin = 0.0f;
        float outputMax = 1.0f;
        bool inverted = false;
    };

    struct Macro
    {
        std::string name;           // User-editable name (e.g., "Intensity")
        float manualValue = 0.5f;   // Value when source is Manual
        float currentValue = 0.5f;  // Computed value (after source + manual)

        // Source: 0 = Manual, >0 = Signal ID
        uint32_t sourceSignalId = 0;

        // Links to parameters
        std::vector<MacroLink> links;

        // Unique ID for use as a route source
        uint32_t id = 0;

        bool isManual() const { return sourceSignalId == 0; }

        // s167-l2: the universal connection. Replaces sourceSignalId as the
        // sanctioned way to drive a macro (s166 spec section 2.2), but
        // sourceSignalId/links are NOT removed here -- MacroPanel.cpp (src/
        // ui/*, outside this lane's fence) still reads/writes them, and
        // ConnectionEngine::tick (src/connect/ConnectionEngine.cpp) ticks
        // `conn` independently, additively, ahead of Lane 3's repoint of the
        // UI onto it.
        ParamConnection conn;
    };

    MacroBank() = default;
    explicit MacroBank(Scope scope) : scope_(scope)
    {
        for (int i = 0; i < kNumMacros; ++i)
        {
            macros_[static_cast<size_t>(i)].name = "Link " + std::to_string(i + 1);
        }
    }

    Scope getScope() const { return scope_; }

    // Access macros
    Macro& getMacro(int index) { return macros_[static_cast<size_t>(index)]; }
    const Macro& getMacro(int index) const { return macros_[static_cast<size_t>(index)]; }

    // Update all macro values from signal registry.
    // For manual macros, currentValue = manualValue.
    // For signal-driven macros, currentValue = applyDepth(manualValue,
    // signal value, signalDepth) -- Master Signal (s-rta-0925 mastersignal
    // Step 1): at signalDepth < 1 a signal-driven macro sits partway toward
    // its manual value, at 0 it sits exactly there (D3: the ONE store this
    // macro's currentValue gets each tick -- see the sole caller,
    // MainComponent::tickFeaturePipeline, and the critic-plan-mastersignal.
    // md finding F1 note in MacroPanel::refresh(), which used to be a
    // SECOND, un-hoisted writer of this same field).
    void updateValues(const SignalRegistry& signals, float signalDepth = 1.0f)
    {
        for (auto& macro : macros_)
        {
            if (macro.isManual())
            {
                macro.currentValue = macro.manualValue;
            }
            else
            {
                macro.currentValue = applyDepth(macro.manualValue,
                                                signals.getCachedValue(macro.sourceSignalId),
                                                signalDepth);
            }
        }
    }

    // Get macro value for use as a route source.
    float getMacroValue(int index) const
    {
        if (index >= 0 && index < kNumMacros)
            return macros_[static_cast<size_t>(index)].currentValue;
        return 0.0f;
    }

    // Set unique IDs for macros (called during initialization)
    void assignIds(uint32_t startId)
    {
        for (int i = 0; i < kNumMacros; ++i)
            macros_[static_cast<size_t>(i)].id = startId + static_cast<uint32_t>(i);
    }

private:
    Scope scope_ = Scope::Global;
    std::array<Macro, kNumMacros> macros_;
};
