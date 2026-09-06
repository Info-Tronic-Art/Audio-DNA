#include "BPMTracker.h"
#include <aubio/aubio.h>
#include <cstring>
#include <cmath>
#include <algorithm>

BPMTracker::BPMTracker(int hopSize, int bufSize, int sampleRate)
    : hopSize_(hopSize), sampleRate_(sampleRate)
{
    tempo_ = new_aubio_tempo(
        "default",                              // method (only "default" supported)
        static_cast<uint_t>(bufSize),
        static_cast<uint_t>(hopSize),
        static_cast<uint_t>(sampleRate)
    );

    input_  = new_fvec(static_cast<uint_t>(hopSize));
    output_ = new_fvec(1);

    // Configure for VJ / music-reactive use
    aubio_tempo_set_silence(tempo_, -70.0f);     // silence gate (dB)
    aubio_tempo_set_threshold(tempo_, 0.3f);     // peak-picking threshold

    // Pre-zero all pipeline buffers (already zero-initialized, but be explicit)
    std::fill(std::begin(medianBuffer_), std::end(medianBuffer_), 0.0f);
    std::fill(std::begin(sortBuffer_), std::end(sortBuffer_), 0.0f);

    // Pre-zero downbeat detection buffers
    std::fill(std::begin(beatScores_), std::end(beatScores_), 0.0f);
    std::fill(std::begin(positionScoreSums_), std::end(positionScoreSums_), 0.0f);
    std::fill(std::begin(positionScoreCounts_), std::end(positionScoreCounts_), 0);

    // P23: Smart BPM recovery
    hopsPerSecBpm_ = static_cast<float>(sampleRate) / static_cast<float>(hopSize);
    silenceEntryHops_ = static_cast<int>(0.3f * hopsPerSecBpm_);  // ~300ms to declare silence
    silenceExitHops_ = static_cast<int>(0.1f * hopsPerSecBpm_);   // ~100ms to resume
}

BPMTracker::~BPMTracker()
{
    if (output_) del_fvec(output_);
    if (input_)  del_fvec(input_);
    if (tempo_)  del_aubio_tempo(tempo_);
}

void BPMTracker::process(const float* samples)
{
    // Copy samples into Aubio's fvec_t
    std::memcpy(input_->data, samples, static_cast<size_t>(hopSize_) * sizeof(float));

    aubio_tempo_do(tempo_, input_, output_);

    // Check if a beat was detected this hop
    bool beat = (fvec_get_sample(output_, 0) != 0.0f);

    // Read raw BPM and confidence from Aubio
    float rawBpm = aubio_tempo_get_bpm(tempo_);
    float conf   = aubio_tempo_get_confidence(tempo_);

    runPipeline(rawBpm, conf, beat);
}

void BPMTracker::processRawBPM(float rawBpm, float conf, bool beat)
{
    runPipeline(rawBpm, conf, beat);
}

void BPMTracker::runPipeline(float rawBpm, float conf, bool beat)
{
    rawBPM_       = rawBpm;
    confidence_   = conf;
    beatDetected_ = beat;

    // Manual mode: skip stabilization pipeline, just run phase from locked BPM.
    // No real onset can be trusted while the operator has overridden the
    // tracker, so the predicted phase wrap drives beatInBar_/barCount_ (P24).
    if (manualMode_.load(std::memory_order_relaxed))
    {
        predictedBeatRegime_ = true;
        updatePhase(beat, conf);
        return;
    }

    // === Stage 1: BPM Range Gate ===
    // Reject zero/invalid, fold into [60, 200] range
    if (rawBpm <= 0.0f)
    {
        // No valid BPM from aubio this hop — just update phase. If we're
        // already locked, a real onset isn't arriving this hop either, so
        // the predicted wrap takes over (P24); if never locked, updatePhase()
        // is a no-op anyway (lockedBPM_ <= 0).
        predictedBeatRegime_ = (lockedBPM_ > 0.0f);
        updatePhase(beat, conf);
        return;
    }

    float gatedBPM = foldBPMToRange(rawBpm);

    // === P23: Smart BPM Recovery ===
    // During silence, hold the last good BPM and keep phase running.
    // P24: the predicted phase wrap also drives beatInBar_/barCount_ forward
    // here, since no real onset can arrive during held silence.
    if (inSilence_ && lockedBPM_ > 0.0f)
    {
        predictedBeatRegime_ = true;
        updatePhase(false, 0.0f); // No beats during silence, phase free-runs
        return;
    }

    // === Stage 2: Confidence Gate ===
    // Only accept estimates with sufficient confidence
    if (conf < kConfidenceThreshold)
    {
        // Low confidence — hold last good value, don't feed median. A real
        // onset still arrived (rawBpm > 0), so this is NOT a predicted-beat
        // regime; scoreBeat() remains the sole incrementer here (unchanged).
        predictedBeatRegime_ = false;
        updatePhase(beat, conf);
        return;
    }
    lastConfidentBPM_ = gatedBPM;

    // === Stage 3: Octave Error Correction ===
    // If we have a locked BPM, correct octave jumps relative to it
    float correctedBPM = (lockedBPM_ > 0.0f) ? correctOctaveError(gatedBPM) : gatedBPM;

    // === Stage 4: Median Filter ===
    float medianBPM = pushAndMedian(correctedBPM);

    // === Stage 5: Hysteresis Lock ===
    if (lockedBPM_ <= 0.0f)
    {
        // First valid BPM — lock immediately after we have enough median samples
        if (medianCount_ >= kMedianWindowSize / 2)
        {
            lockedBPM_ = medianBPM;
            trackerState_ = STATE_LOCKED;
        }
        else
        {
            candidateBPM_ = medianBPM;
            trackerState_ = STATE_LOCKING;
        }
    }
    else
    {
        // We have a locked BPM — check if it needs to change
        float diff = std::fabs(medianBPM - lockedBPM_);

        if (diff > kBPMChangeThreshold)
        {
            // Median is drifting away from locked BPM
            if (std::fabs(medianBPM - candidateBPM_) < kBPMChangeThreshold)
            {
                // Candidate is consistent — increment counter
                ++consistencyCounter_;
                trackerState_ = STATE_LOCKING;

                if (consistencyCounter_ >= kHysteresisHops)
                {
                    // Candidate has been stable long enough — accept new BPM
                    lockedBPM_ = medianBPM;
                    candidateBPM_ = 0.0f;
                    consistencyCounter_ = 0;
                    trackerState_ = STATE_LOCKED;
                }
            }
            else
            {
                // New candidate — reset counter
                candidateBPM_ = medianBPM;
                consistencyCounter_ = 1;
                trackerState_ = STATE_LOCKING;
            }
        }
        else
        {
            // Median is close to locked BPM — reset any pending change
            candidateBPM_ = 0.0f;
            consistencyCounter_ = 0;
            trackerState_ = STATE_LOCKED;
        }
    }

    // A real onset arrived and was accepted this hop — scoreBeat() is the
    // sole incrementer (unchanged pre-P24 behavior).
    predictedBeatRegime_ = false;
    updatePhase(beat, conf);
}

void BPMTracker::updatePhase(bool beat, float conf)
{
    if (lockedBPM_ <= 0.0f)
    {
        phase_ = 0.0f;
        return;
    }

    // Free-running phase driven by locked BPM
    float lockedPeriodSamples = (static_cast<float>(sampleRate_) * 60.0f) / lockedBPM_;
    phase_ += static_cast<float>(hopSize_) / lockedPeriodSamples;

    // Wrap at 1.0
    bool wrapped = (phase_ >= 1.0f);
    if (wrapped)
        phase_ -= std::floor(phase_);

    // Hard reset on high-confidence beat detection from aubio
    if (beat && conf >= kBeatResetConfidence)
    {
        phase_ = 0.0f;
    }

    // P24: while a real onset cannot arrive this hop (predictedBeatRegime_,
    // set by runPipeline), the predicted phase wrap is what drives
    // beatInBar_/barCount_ forward instead of scoreBeat(). scoreBeat() is
    // itself gated on !predictedBeatRegime_ (see feedDownbeatFeatures), so
    // the two paths are mutually exclusive by construction -- never both.
    if (wrapped && predictedBeatRegime_)
    {
        advancePredictedBeat();
    }
}

void BPMTracker::advancePredictedBeat()
{
    // Mirrors the beatInBar_/downbeatDetected_ bookkeeping scoreBeat()'s
    // locked branch does on a real onset, but driven by the predicted phase
    // wrap instead. Deliberately does NOT touch totalBeatsScored_/
    // beatScores_/analyzeDownbeatPosition() -- there is no real spectral
    // score to push during silence/manual mode, so the downbeat-position
    // statistics are left untouched rather than polluted with fake data.
    //
    // barCount_ itself is NOT incremented here: updatePhrase()'s existing
    // downbeatDetected_ rising-edge detection (run every hop from
    // feedDownbeatFeatures, unconditionally, even during silence/manual
    // mode) remains the sole place barCount_ advances, for either a real or
    // a predicted beat -- one incrementer, not two racing.
    beatCounter_ = (beatCounter_ + 1) % kBeatsPerBar;
    beatInBar_ = static_cast<uint8_t>(beatCounter_);
    downbeatDetected_ = (beatCounter_ == 0);
}

// --- Static helpers ---

float BPMTracker::foldBPMToRange(float bpm)
{
    // Fold into [kMinBPM, kMaxBPM] by halving or doubling
    while (bpm > kMaxBPM && bpm > 0.0f)
        bpm *= 0.5f;
    while (bpm < kMinBPM && bpm > 0.0f)
        bpm *= 2.0f;

    // Final clamp (safety)
    return std::clamp(bpm, kMinBPM, kMaxBPM);
}

float BPMTracker::correctOctaveError(float bpm) const
{
    if (lockedBPM_ <= 0.0f)
        return bpm;

    float ratio = bpm / lockedBPM_;

    // Check for double (1.8 < ratio < 2.2)
    if (ratio > 1.8f && ratio < 2.2f)
        return bpm * 0.5f;

    // Check for half (0.45 < ratio < 0.55)
    if (ratio > 0.45f && ratio < 0.55f)
        return bpm * 2.0f;

    return bpm;
}

float BPMTracker::pushAndMedian(float bpm)
{
    // Push into circular buffer
    medianBuffer_[medianPos_] = bpm;
    medianPos_ = (medianPos_ + 1) % kMedianWindowSize;
    if (medianCount_ < kMedianWindowSize)
        ++medianCount_;

    // Copy valid entries to sort buffer and find median via nth_element
    std::copy(medianBuffer_, medianBuffer_ + medianCount_, sortBuffer_);

    int mid = medianCount_ / 2;
    std::nth_element(sortBuffer_, sortBuffer_ + mid, sortBuffer_ + medianCount_);

    return sortBuffer_[mid];
}

// === Downbeat Detection ===

void BPMTracker::feedDownbeatFeatures(float bassEnergy, float spectralFlux, float harmonicChange,
                                      uint8_t structuralState)
{
    cachedBassEnergy_ = bassEnergy;
    cachedSpectralFlux_ = spectralFlux;
    cachedHarmonicChange_ = harmonicChange;

    // Score the beat if one was detected this hop. Gated on
    // !predictedBeatRegime_ too (P24): aubio's own -70dB silence gate and our
    // RMS-based inSilence_ hysteresis use different thresholds, so a stray
    // beatDetected_ flag can in principle land on a hop where the predicted
    // wrap already advanced beatInBar_/beatCounter_ in updatePhase() --
    // skipping scoreBeat() here keeps the two paths mutually exclusive.
    if (beatDetected_ && lockedBPM_ > 0.0f && !predictedBeatRegime_)
    {
        scoreBeat();
    }

    // Always update bar phase (even between beats, for smooth sawtooth)
    updateBarPhase();

    // Update phrase tracking
    updatePhrase(structuralState);
}

void BPMTracker::scoreBeat()
{
    // Compute downbeat score from cached spectral features
    float score = kDownbeatWeightBass * cachedBassEnergy_
                + kDownbeatWeightFlux * cachedSpectralFlux_
                + kDownbeatWeightHCDF * cachedHarmonicChange_;

    // Store in circular buffer
    beatScores_[beatScorePos_] = score;
    beatScorePos_ = (beatScorePos_ + 1) % kBeatScoreBufferSize;
    ++totalBeatsScored_;

    if (downbeatLocked_)
    {
        // Already locked — advance beat counter
        beatCounter_ = (beatCounter_ + 1) % kBeatsPerBar;
        beatInBar_ = static_cast<uint8_t>(beatCounter_);
        downbeatDetected_ = (beatCounter_ == 0);

        // Periodically re-check if locked position is still correct (every 16 beats)
        if (totalBeatsScored_ % 16 == 0)
        {
            analyzeDownbeatPosition();
        }
    }
    else
    {
        // Not locked yet — analyze every 4 beats once we have enough data
        if (totalBeatsScored_ >= kBeatsPerBar && totalBeatsScored_ % kBeatsPerBar == 0)
        {
            analyzeDownbeatPosition();
        }

        // Even before locking, assign a position based on beat count mod 4
        beatInBar_ = static_cast<uint8_t>(totalBeatsScored_ % kBeatsPerBar);
        downbeatDetected_ = false;
    }
}

void BPMTracker::analyzeDownbeatPosition()
{
    // Accumulate scores by position (mod 4) from the circular buffer
    float sums[kBeatsPerBar] = {};
    int counts[kBeatsPerBar] = {};

    int numBeats = std::min(totalBeatsScored_, kBeatScoreBufferSize);

    for (int i = 0; i < numBeats; ++i)
    {
        // The most recent beat is at (beatScorePos_ - 1), going backward
        int idx = (beatScorePos_ - 1 - i + kBeatScoreBufferSize) % kBeatScoreBufferSize;
        // Position in bar: the most recent beat is at position (totalBeatsScored_ - 1 - i) mod 4
        int pos = static_cast<int>((totalBeatsScored_ - 1 - i) % kBeatsPerBar);
        sums[pos] += beatScores_[idx];
        counts[pos]++;
    }

    // Find position with highest average score — that's the downbeat candidate
    int bestPos = 0;
    float bestAvg = -1.0f;
    for (int p = 0; p < kBeatsPerBar; ++p)
    {
        if (counts[p] > 0)
        {
            float avg = sums[p] / static_cast<float>(counts[p]);
            if (avg > bestAvg)
            {
                bestAvg = avg;
                bestPos = p;
            }
        }
    }

    if (downbeatLocked_)
    {
        // Re-validation: check if the locked position is still the strongest
        if (bestPos != lockedDownbeatPos_)
        {
            ++downbeatConsistency_;
            if (downbeatConsistency_ >= kDownbeatLockThreshold)
            {
                // Position has shifted — relock
                lockedDownbeatPos_ = bestPos;
                // Recalculate beatCounter_ to align with new downbeat
                int currentBeatGlobal = static_cast<int>((totalBeatsScored_ - 1) % kBeatsPerBar);
                beatCounter_ = (currentBeatGlobal - lockedDownbeatPos_ + kBeatsPerBar) % kBeatsPerBar;
                downbeatConsistency_ = 0;
            }
        }
        else
        {
            downbeatConsistency_ = 0;  // Still consistent
        }
    }
    else
    {
        // Not locked — check consistency for initial lock
        if (bestPos == lockedDownbeatPos_ || totalBeatsScored_ <= kBeatsPerBar)
        {
            if (totalBeatsScored_ <= kBeatsPerBar)
                lockedDownbeatPos_ = bestPos;
            ++downbeatConsistency_;
        }
        else
        {
            lockedDownbeatPos_ = bestPos;
            downbeatConsistency_ = 1;
        }

        if (downbeatConsistency_ >= 2 && totalBeatsScored_ >= kDownbeatLockThreshold)
        {
            downbeatLocked_ = true;
            // Set beatCounter_ relative to locked downbeat position
            int currentBeatGlobal = static_cast<int>((totalBeatsScored_ - 1) % kBeatsPerBar);
            beatCounter_ = (currentBeatGlobal - lockedDownbeatPos_ + kBeatsPerBar) % kBeatsPerBar;
            beatInBar_ = static_cast<uint8_t>(beatCounter_);
            downbeatDetected_ = (beatCounter_ == 0);
        }
    }
}

void BPMTracker::updateBarPhase()
{
    if (lockedBPM_ <= 0.0f)
    {
        barPhase_ = 0.0f;
        return;
    }

    // barPhase = (beatInBar + beatPhase) / beatsPerBar
    barPhase_ = (static_cast<float>(beatInBar_) + phase_) / static_cast<float>(kBeatsPerBar);

    // Clamp to [0, 1) for safety
    if (barPhase_ >= 1.0f)
        barPhase_ -= std::floor(barPhase_);
    if (barPhase_ < 0.0f)
        barPhase_ = 0.0f;
}

void BPMTracker::updatePhrase(uint8_t structuralState)
{
    if (lockedBPM_ <= 0.0f)
    {
        phrasePhase_ = 0.0f;
        barCount_ = 0;
        prevDownbeatDetected_ = false;
        prevStructuralState_ = structuralState;
        return;
    }

    // Detect rising edge of downbeat (new bar)
    bool newBar = downbeatDetected_ && !prevDownbeatDetected_;
    prevDownbeatDetected_ = downbeatDetected_;

    if (newBar)
    {
        ++barCount_;
    }

    // Reset phrase on structural transitions (e.g., drop hits → reset phrase counter)
    // Only reset on transition TO drop (state 2) or FROM breakdown (state 3).
    // Suppressed while predictedBeatRegime_ (silence/manual/locked-with-no-raw-BPM):
    // this method runs every hop unconditionally, including hops where no real
    // onset can arrive, but StructuralDetector keeps classifying off live
    // RMS/flux/onset-rate the whole time -- so a "drop"/"breakdown" transition
    // inferred from room noise during silence is meaningless and must not
    // corrupt the phrase position. Mirrors the !predictedBeatRegime_ guard
    // scoreBeat() already gets in feedDownbeatFeatures(). A real transition
    // during real playback (predictedBeatRegime_ false) still resets, unchanged.
    if (structuralState != prevStructuralState_)
    {
        bool resetTransition = (structuralState == 2)                         // entering drop
                            || (prevStructuralState_ == 3 && structuralState != 3); // leaving breakdown
        if (resetTransition && !predictedBeatRegime_)
        {
            barCount_ = 0;
        }
    }
    prevStructuralState_ = structuralState;

    // phrasePhase = (barCount % phraseBars + barPhase) / phraseBars
    int barInPhrase = static_cast<int>(barCount_) % phraseBars_;
    phrasePhase_ = (static_cast<float>(barInPhrase) + barPhase_) / static_cast<float>(phraseBars_);

    // Clamp to [0, 1)
    if (phrasePhase_ >= 1.0f)
        phrasePhase_ -= std::floor(phrasePhase_);
    if (phrasePhase_ < 0.0f)
        phrasePhase_ = 0.0f;
}

void BPMTracker::resetPhrase()
{
    barCount_ = 0;
    phrasePhase_ = 0.0f;
    prevDownbeatDetected_ = false;
}

void BPMTracker::setManualBPM(float bpm)
{
    if (bpm <= 0.0f) return;
    float folded = foldBPMToRange(bpm);
    lockedBPM_ = folded;
    candidateBPM_ = folded;
    lastConfidentBPM_ = folded;
    trackerState_ = STATE_LOCKED;
    consistencyCounter_ = kHysteresisHops; // Already locked
    phase_ = 0.0f;
}

void BPMTracker::resetBeatPhase()
{
    phase_ = 0.0f;
    beatInBar_ = 0;
    barPhase_ = 0.0f;
    beatCounter_ = 0;
}

void BPMTracker::setManualMode(bool enabled)
{
    manualMode_.store(enabled, std::memory_order_relaxed);
}

// === P23: Smart BPM Recovery ===

void BPMTracker::feedSilenceDetection(float rms)
{
    if (rms < silenceRmsThreshold_)
    {
        if (!inSilence_)
        {
            ++silenceCountdown_;
            if (silenceCountdown_ >= silenceEntryHops_)
            {
                inSilence_ = true;
                silenceHopCount_ = 0;
                silenceCountdown_ = 0;
            }
        }
        else
        {
            ++silenceHopCount_;
            silenceCountdown_ = 0; // reset exit countdown
        }
    }
    else
    {
        if (inSilence_)
        {
            ++silenceCountdown_;
            if (silenceCountdown_ >= silenceExitHops_)
            {
                inSilence_ = false;
                silenceHopCount_ = 0;
                silenceCountdown_ = 0;
            }
        }
        else
        {
            silenceCountdown_ = 0; // reset entry countdown
        }
    }
}

float BPMTracker::silenceDuration() const
{
    if (!inSilence_ || hopsPerSecBpm_ <= 0.0f) return 0.0f;
    return static_cast<float>(silenceHopCount_) / hopsPerSecBpm_;
}
