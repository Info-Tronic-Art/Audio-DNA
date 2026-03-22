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


@pytest.fixture(autouse=True)
def reset_between_tests(app):
    """Reset app state before each test for isolation."""
    app.reset()
    yield
