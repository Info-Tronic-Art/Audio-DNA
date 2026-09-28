"""
Pytest configuration for Eyes visual tests.

Provides fixtures that spawn the Audio-DNA app in test mode and manage
its lifecycle across the test session.
"""

import os
import pytest
from vj_controller import VJAppController


def _default_executable():
    """Find the built executable."""
    # macOS app bundle path
    mac_path = os.path.join(
        os.path.dirname(__file__),
        "..",
        "..",
        "build",
        "AudioDNA_artefacts",
        "Release",
        "Audio-DNA.app",
        "Contents",
        "MacOS",
        "Audio-DNA",
    )
    if os.path.exists(mac_path):
        return os.path.abspath(mac_path)

    # Linux/Windows path
    linux_path = os.path.join(
        os.path.dirname(__file__),
        "..",
        "..",
        "build",
        "AudioDNA_artefacts",
        "Release",
        "Audio-DNA",
    )
    if os.path.exists(linux_path):
        return os.path.abspath(linux_path)

    return None


@pytest.fixture(scope="session")
def app():
    """Spawn Audio-DNA in test mode for the entire test session.

    The app starts once, all tests share the same instance.
    Use the reset fixture (below) to clean state between tests.

    Environment variables:
        AUDIODNA_EXE: Override the path to the executable.
        AUDIODNA_TEST_PORT: Override the HTTP port (default: 8080).
        AUDIODNA_NO_SPAWN: Set to "1" to skip spawning (use already-running app).
    """
    exe = os.environ.get("AUDIODNA_EXE", _default_executable())
    port = int(os.environ.get("AUDIODNA_TEST_PORT", "8080"))
    no_spawn = os.environ.get("AUDIODNA_NO_SPAWN", "0") == "1"

    controller = VJAppController(port=port, executable=None if no_spawn else exe)

    if not no_spawn and exe is None:
        pytest.skip(
            "Audio-DNA executable not found. Build with -DAUDIODNA_BUILD_TEST_SERVER=ON"
        )

    controller.start(timeout=20)
    yield controller

    if not no_spawn:
        controller.stop()


# Tier-1 modules that capture at 256x256 (s-rta-0927 follow-ups F1, tier1-diag H2). test_performance captures at
# 512 on the default canvas and is deliberately not in the set.
TIER1_CANVAS_256 = {"test_sources", "test_effects", "test_audio_reactivity", "test_time_sweep"}


@pytest.fixture(scope="module", autouse=True)
def tier1_canvas_256(request, tmp_path_factory):
    """H2: with the composition AND the capture lock both 256x256 the canvas never resizes between captures, so
    stateful sources keep their state (ProceduralSource::resize re-creates the ping-pong FBOs; resolveCanvas).
    Requests `app` only for the four modules: other files in tests/visual override `app` (gotchas 2026-05-18)."""
    name = request.module.__name__.rsplit(".", 1)[-1]
    if name not in TIER1_CANVAS_256:
        yield
        return
    app = request.getfixturevalue("app")
    before = app.get_composition_params()
    app.set_composition_params(outputWidth=256, outputHeight=256)
    # The GL thread applies a size pair a frame after it first sees it; a capture returns only once the canvas is at
    # the lock size, so this render is the "size applied" barrier.
    app.render_frame(str(tmp_path_factory.mktemp("canvas") / "warmup.png"), time_val=0.0, width=256, height=256)
    yield
    app.set_composition_params(outputWidth=before["outputWidth"], outputHeight=before["outputHeight"])


@pytest.fixture(autouse=True)
def reset_between_tests(app):
    """Reset app state before each test for isolation."""
    app.reset()
    yield
