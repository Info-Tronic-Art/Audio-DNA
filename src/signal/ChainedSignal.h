#pragma once
#include "signal/Signal.h"

// Forward declaration to avoid circular include
class SignalRegistry;

// ChainedSignal: one signal modulates another.
// For example, Bass → Envelope → Parameter: Bass shapes the envelope's amplitude.
// The modulator signal's value multiplies the carrier signal's value.
class ChainedSignal : public Signal
{
public:
    // carrierSignalId: the main signal (e.g., envelope or oscillator)
    // modulatorSignalId: the shaping signal (e.g., bass energy)
    ChainedSignal(const std::string& name, uint32_t carrierSignalId, uint32_t modulatorSignalId)
        : Signal(name, Type::Envelope, Category::Modulation)
        , carrierSignalId_(carrierSignalId)
        , modulatorSignalId_(modulatorSignalId) {}

    float getValue(const FeatureSnapshot& snapshot) const override;

    // Chain configuration
    enum class ChainMode : uint8_t { Multiply, Add, Gate, ScaleRange };

    ChainMode getChainMode() const { return chainMode_; }
    void setChainMode(ChainMode mode) { chainMode_ = mode; }

    float getGain() const { return gain_; }
    void setGain(float g) { gain_ = g; }

    float getModulationDepth() const { return modulationDepth_; }
    void setModulationDepth(float d) { modulationDepth_ = d; }

    float getGateThreshold() const { return gateThreshold_; }
    void setGateThreshold(float t) { gateThreshold_ = t; }

    uint32_t getCarrierSignalId() const { return carrierSignalId_; }
    void setCarrierSignalId(uint32_t id) { carrierSignalId_ = id; }

    uint32_t getModulatorSignalId() const { return modulatorSignalId_; }
    void setModulatorSignalId(uint32_t id) { modulatorSignalId_ = id; }

    // Registry pointer for cached value lookups (set by SignalRegistry)
    void setRegistry(const SignalRegistry* reg) { registry_ = reg; }

private:
    uint32_t carrierSignalId_;
    uint32_t modulatorSignalId_;
    ChainMode chainMode_ = ChainMode::Multiply;
    float gain_ = 1.0f;
    float modulationDepth_ = 1.0f;
    float gateThreshold_ = 0.1f;
    const SignalRegistry* registry_ = nullptr;
};
