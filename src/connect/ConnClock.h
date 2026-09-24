#pragma once
#include <juce_core/juce_core.h>

// The ONE engine clock (s-rta-0923 lane 3, plan section 3.2). Every grip
// timestamp taken outside a test (ConnectionEngine::Context::now in
// MainComponent::tickFeaturePipeline, C3; every manualWrite/manualRelease
// call site, C3/C4) goes through this. Tests pass explicit `now` values, as
// they already do today (test_connection.cpp) -- unchanged.
inline double connNow() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }
