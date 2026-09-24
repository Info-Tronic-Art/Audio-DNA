#pragma once
#include "recording/PerfState.h"
#include <string>

struct Composition;

// PerfStateCapture -- s-rta-0923 step 3 plan section 3.2: "capturing FROM a
// live Composition is step 3's job" (PerfState.h's own header comment).
// Separate translation unit from PerfState.{h,cpp} on purpose (T17): the
// model-free test targets that link PerfState.cpp (test_audio_store,
// test_audio_tap_sync) do NOT link model/Clip.cpp or model/Layer.cpp, so a
// capture function needing Composition/Deck/Layer/Clip cannot live inside
// PerfState.cpp without dragging the whole model into those targets.
//
// `bpm`/`audioAction` are passed in rather than read off some ambient
// singleton -- RecorderHost's Dispatch::capturePerfState lambda (Lane S3-B)
// is expected to call this as
// `capturePerfState(composition_, snap.bpm, lastAudioAction_)`.
PerfState capturePerfState(const Composition& comp, float bpm, const std::string& audioAction);
