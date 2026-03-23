#include "SignalRegistry.h"
#include "signal/ChainedSignal.h"

void SignalRegistry::initDefaults()
{
    signals_.clear();
    cachedValues_.clear();

    // Audio signals — matching ARCHITECTURE_V2.md Section 9 labels
    auto addAudio = [&](const std::string& name, MappingSource src, Signal::Category cat) {
        auto sig = std::make_unique<AudioSignal>(name, src, cat);
        sig->setId(nextId_++);
        signals_.push_back(std::move(sig));
        cachedValues_.push_back(0.0f);
    };

    // Default visible signals (12 from architecture):
    // Waveform (special — not a Signal), Volume, Sub, Bass, Mid, Air,
    // Tempo, Beat Position, Beat In Bar, Bar Position, Hit, Energy State
    addAudio("Volume",        MappingSource::RMS,              Signal::Category::Amplitude);
    addAudio("Sub Bass",      MappingSource::BandSub,          Signal::Category::Bands);
    addAudio("Bass",          MappingSource::BandBass,         Signal::Category::Bands);
    addAudio("Mid",           MappingSource::BandMid,          Signal::Category::Bands);
    addAudio("Air",           MappingSource::BandBrilliance,   Signal::Category::Bands);
    addAudio("Tempo",         MappingSource::BPM,              Signal::Category::Rhythm);
    addAudio("Beat Position", MappingSource::BeatPhase,        Signal::Category::Rhythm);
    addAudio("Hit",           MappingSource::OnsetStrength,    Signal::Category::Rhythm);

    // Additional amplitude signals (hidden by default)
    auto addHiddenAudio = [&](const std::string& name, MappingSource src, Signal::Category cat) {
        auto sig = std::make_unique<AudioSignal>(name, src, cat);
        sig->setId(nextId_++);
        sig->setVisible(false);
        signals_.push_back(std::move(sig));
        cachedValues_.push_back(0.0f);
    };

    addHiddenAudio("Peak",           MappingSource::Peak,             Signal::Category::Amplitude);
    addHiddenAudio("Punch",          MappingSource::DynamicRange,     Signal::Category::Amplitude);
    addHiddenAudio("Hits Per Second", MappingSource::TransientDensity, Signal::Category::Amplitude);
    addHiddenAudio("Low Mid",        MappingSource::BandLowMid,       Signal::Category::Bands);
    addHiddenAudio("High Mid",       MappingSource::BandHighMid,      Signal::Category::Bands);
    addHiddenAudio("Presence",       MappingSource::BandPresence,     Signal::Category::Bands);
    addHiddenAudio("Hit Strength",   MappingSource::OnsetStrength,    Signal::Category::Rhythm);
    addHiddenAudio("Bar Position",   MappingSource::BarPhase,         Signal::Category::Rhythm);
    addHiddenAudio("Phrase Position", MappingSource::PhrasePhase,     Signal::Category::Rhythm);
    addHiddenAudio("Bar Count",      MappingSource::BarCount,         Signal::Category::Rhythm);
    addHiddenAudio("Brightness",     MappingSource::SpectralCentroid, Signal::Category::Amplitude);
    addHiddenAudio("Change",         MappingSource::SpectralFlux,     Signal::Category::Amplitude);
    addHiddenAudio("Noisiness",      MappingSource::SpectralFlatness, Signal::Category::Amplitude);
    addHiddenAudio("Note",           MappingSource::DominantPitch,    Signal::Category::Pitch);
    addHiddenAudio("Note Confidence", MappingSource::PitchConfidence, Signal::Category::Pitch);
    addHiddenAudio("Chord Change",   MappingSource::HarmonicChange,   Signal::Category::Pitch);

    // Default modulation signals (2 visible)
    {
        auto mod1 = std::make_unique<OscillatorSignal>("Mod 1", OscillatorSignal::WaveShape::Sine, 1.0f);
        mod1->setId(nextId_++);
        signals_.push_back(std::move(mod1));
        cachedValues_.push_back(0.0f);
    }
    {
        auto mod2 = std::make_unique<EnvelopeSignal>("Mod 2", 4.0f);
        mod2->setId(nextId_++);
        signals_.push_back(std::move(mod2));
        cachedValues_.push_back(0.0f);
    }

    // P24: Clip Position signal (hidden by default)
    {
        auto clipPos = std::make_unique<ClipPositionSignal>("Clip Position");
        clipPos->setId(nextId_++);
        clipPos->setVisible(false);
        signals_.push_back(std::move(clipPos));
        cachedValues_.push_back(0.0f);
    }
}

void SignalRegistry::addSignal(std::unique_ptr<Signal> signal)
{
    signal->setId(nextId_++);
    // P24: Wire ChainedSignal to this registry for cached value lookups
    if (auto* chained = dynamic_cast<ChainedSignal*>(signal.get()))
        chained->setRegistry(this);
    signals_.push_back(std::move(signal));
    cachedValues_.push_back(0.0f);
}

bool SignalRegistry::removeSignal(uint32_t id)
{
    for (size_t i = 0; i < signals_.size(); ++i)
    {
        if (signals_[i]->getId() == id)
        {
            signals_.erase(signals_.begin() + static_cast<ptrdiff_t>(i));
            cachedValues_.erase(cachedValues_.begin() + static_cast<ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

Signal* SignalRegistry::getSignal(uint32_t id)
{
    for (auto& sig : signals_)
        if (sig->getId() == id) return sig.get();
    return nullptr;
}

const Signal* SignalRegistry::getSignal(uint32_t id) const
{
    for (const auto& sig : signals_)
        if (sig->getId() == id) return sig.get();
    return nullptr;
}

Signal* SignalRegistry::getSignalByName(const std::string& name)
{
    for (auto& sig : signals_)
        if (sig->getName() == name) return sig.get();
    return nullptr;
}

Signal* SignalRegistry::getSignalAt(int index)
{
    if (index >= 0 && index < static_cast<int>(signals_.size()))
        return signals_[static_cast<size_t>(index)].get();
    return nullptr;
}

const Signal* SignalRegistry::getSignalAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(signals_.size()))
        return signals_[static_cast<size_t>(index)].get();
    return nullptr;
}

std::vector<Signal*> SignalRegistry::getSignalsByCategory(Signal::Category category)
{
    std::vector<Signal*> result;
    for (auto& sig : signals_)
        if (sig->getCategory() == category)
            result.push_back(sig.get());
    return result;
}

void SignalRegistry::evaluateAll(const FeatureSnapshot& snapshot)
{
    for (size_t i = 0; i < signals_.size(); ++i)
    {
        cachedValues_[i] = signals_[i]->getValue(snapshot);
    }
}

float SignalRegistry::getCachedValue(uint32_t signalId) const
{
    for (size_t i = 0; i < signals_.size(); ++i)
    {
        if (signals_[i]->getId() == signalId)
            return cachedValues_[i];
    }
    return 0.0f;
}

ClipPositionSignal* SignalRegistry::getClipPositionSignal()
{
    for (auto& sig : signals_)
    {
        if (auto* clipPos = dynamic_cast<ClipPositionSignal*>(sig.get()))
            return clipPos;
    }
    return nullptr;
}
