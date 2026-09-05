#include <catch2/catch_test_macros.hpp>
#include "recording/SessionRecorder.h"

TEST_CASE("SessionRecorder fresh instance", "[recording]")
{
    SessionRecorder recorder;

    REQUIRE(recorder.getNumEvents() == 0);
    REQUIRE(recorder.getDuration() == 0.0);
    REQUIRE_FALSE(recorder.isRecording());
    REQUIRE_FALSE(recorder.isPlaying());
}
