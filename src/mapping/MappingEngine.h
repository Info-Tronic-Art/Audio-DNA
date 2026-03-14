#pragma once
#include "mapping/MappingTypes.h"
#include "analysis/FeatureSnapshot.h"
#include "effects/EffectChain.h"
#include "features/Smoother.h"
#include <vector>

// MappingEngine: processes a list of Mappings each frame.
//
// Pipeline per mapping:
//   1. Extract source value from FeatureSnapshot
//   2. Normalize to [0, 1] via (raw - inputMin) / (inputMax - inputMin)
//   3. Apply curve transform
//   4. Scale to [outputMin, outputMax]
//   5. Smooth via per-mapping EMA
//   6. Write to target effect parameter
//
// Multiple mappings targeting the same effect parameter are summed
// and clamped to [0, 1].
//
// Called on the render thread each frame. No allocation in processFrame().
class MappingEngine
{
public:
    MappingEngine() = default;

    // Add a mapping. Returns its index. Pre-allocates smoother state.
    int addMapping(const Mapping& mapping);

    // Remove a mapping by index. Returns false if index out of range.
    bool removeMapping(int index);

    // Get/set mapping by index.
    Mapping* getMapping(int index);
    const Mapping* getMapping(int index) const;
    int getNumMappings() const { return static_cast<int>(mappings_.size()); }

    // Process all mappings for one frame.
    // Reads from snapshot, writes to effect parameters in chain.
    void processFrame(const FeatureSnapshot& snapshot, EffectChain& chain);

    // Clear all mappings.
    void clearAll();

    // Extract a raw source value from a snapshot.
    static float extractSource(MappingSource source, const FeatureSnapshot& snap);

    // Apply a curve transform to a [0,1] value.
    // steppedN: number of steps for Stepped curve (default 4).
    static float applyCurve(MappingCurve curve, float x, int steppedN = 4);

private:
    std::vector<Mapping>  mappings_;
    std::vector<Smoother> smoothers_;
};
