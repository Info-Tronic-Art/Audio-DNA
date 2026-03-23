#include "signal/ChainedSignal.h"
#include "signal/SignalRegistry.h"
#include <algorithm>

float ChainedSignal::getValue(const FeatureSnapshot& /*snapshot*/) const
{
    if (!registry_) return 0.0f;

    float carrier = registry_->getCachedValue(carrierSignalId_);
    float modulator = registry_->getCachedValue(modulatorSignalId_);

    switch (chainMode_)
    {
        case ChainMode::Multiply:
            return carrier * modulator * gain_;

        case ChainMode::Add:
            return std::clamp(carrier + modulator * modulationDepth_, 0.0f, 1.0f) * gain_;

        case ChainMode::Gate:
            return (modulator > gateThreshold_) ? carrier * gain_ : 0.0f;

        case ChainMode::ScaleRange:
        {
            float lo = modulationDepth_ * (1.0f - modulator);
            float hi = lo + modulator;
            return (lo + carrier * (hi - lo)) * gain_;
        }
    }
    return carrier * modulator * gain_;
}
