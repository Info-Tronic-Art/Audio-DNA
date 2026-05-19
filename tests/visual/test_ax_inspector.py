"""
Tests for the accessibility tree inspector.

Unit tests mock the AX API so they run without a real application.
Integration test connects to a running Audio-DNA instance (skipped if not available).
"""

import json
import sys
from unittest.mock import MagicMock, patch

import pytest

# The module under test imports macOS-only frameworks. Mock them before import
# so tests can run even on platforms without pyobjc (CI Linux, etc.).
_ax_mocks_installed = False


def _ensure_ax_mocks():
    """Install mock modules for ApplicationServices and Cocoa if needed."""
    global _ax_mocks_installed
    if _ax_mocks_installed:
        return

    if "ApplicationServices" not in sys.modules:
        mock_as = MagicMock()
        mock_as.kAXErrorSuccess = 0
        sys.modules["ApplicationServices"] = mock_as

    if "Cocoa" not in sys.modules:
        sys.modules["Cocoa"] = MagicMock()

    _ax_mocks_installed = True


_ensure_ax_mocks()

import ax_inspector  # noqa: E402 — must come after mock setup


# ---------------------------------------------------------------------------
# Override conftest fixtures — ax_inspector tests don't need the running app
# ---------------------------------------------------------------------------

@pytest.fixture(scope="session")
def app():
    """No-op override: ax_inspector tests don't need the Audio-DNA app."""
    return None


@pytest.fixture(autouse=True)
def reset_between_tests(app):
    """No-op override: ax_inspector tests manage their own state."""
    yield


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _make_mock_element(
    role: str = "AXWindow",
    title: str = None,
    value: str = None,
    description: str = None,
    position: tuple = None,
    size: tuple = None,
    children: list = None,
):
    """Build a mock AXUIElement with attribute lookup support.

    Args:
        role: AXRole string.
        title: AXTitle string.
        value: AXValue string.
        description: AXDescription string.
        position: (x, y) tuple or None.
        size: (width, height) tuple or None.
        children: List of child mock elements.

    Returns:
        A mock object that responds to AXUIElementCopyAttributeValue.
    """
    attrs = {
        "AXRole": role,
        "AXTitle": title,
        "AXValue": value,
        "AXDescription": description,
        "AXChildren": children or [],
    }

    if position is not None:
        pos_obj = MagicMock()
        pos_obj.x = position[0]
        pos_obj.y = position[1]
        attrs["AXPosition"] = pos_obj

    if size is not None:
        size_obj = MagicMock()
        size_obj.width = size[0]
        size_obj.height = size[1]
        attrs["AXSize"] = size_obj

    element = MagicMock()
    element._attrs = attrs
    return element


def _mock_copy_attribute(element, attribute, _):
    """Mock for AXUIElementCopyAttributeValue that reads from element._attrs."""
    attrs = getattr(element, "_attrs", {})
    if attribute in attrs and attrs[attribute] is not None:
        return (0, attrs[attribute])  # 0 = kAXErrorSuccess
    return (-25201, None)  # kAXErrorAttributeUnsupported


# ---------------------------------------------------------------------------
# Unit tests: walk_element
# ---------------------------------------------------------------------------

class TestWalkElement:
    """Tests for the recursive tree walker."""

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_single_element(self, _mock_copy):
        """Walk a leaf element with no children."""
        elem = _make_mock_element(
            role="AXButton",
            title="Play",
            value=None,
            description="Play button",
            position=(100, 200),
            size=(80, 30),
        )

        result = ax_inspector.walk_element(elem)

        assert result["role"] == "AXButton"
        assert result["title"] == "Play"
        assert result["value"] is None
        assert result["description"] == "Play button"
        assert result["position"] == {"x": 100.0, "y": 200.0}
        assert result["size"] == {"width": 80.0, "height": 30.0}
        assert result["children_count"] == 0
        assert result["children"] == []

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_nested_children(self, _mock_copy):
        """Walk a tree with nested children."""
        child_a = _make_mock_element(role="AXButton", title="OK")
        child_b = _make_mock_element(role="AXButton", title="Cancel")
        parent = _make_mock_element(
            role="AXGroup",
            title="Toolbar",
            children=[child_a, child_b],
        )

        result = ax_inspector.walk_element(parent)

        assert result["role"] == "AXGroup"
        assert result["children_count"] == 2
        assert len(result["children"]) == 2
        assert result["children"][0]["title"] == "OK"
        assert result["children"][1]["title"] == "Cancel"

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_empty_tree(self, _mock_copy):
        """Walk an element with no attributes set."""
        elem = _make_mock_element(role=None, title=None, value=None, description=None)
        # Override role to None for the attribute lookup
        elem._attrs["AXRole"] = None

        result = ax_inspector.walk_element(elem)

        assert result["role"] is None
        assert result["title"] is None
        assert result["value"] is None
        assert result["description"] is None
        assert result["position"] is None
        assert result["size"] is None
        assert result["children_count"] == 0


class TestDepthLimiting:
    """Tests for the --depth recursion limiter."""

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_depth_zero_skips_children(self, _mock_copy):
        """Depth 0 returns the root element without walking children."""
        child = _make_mock_element(role="AXButton", title="Nested")
        parent = _make_mock_element(role="AXWindow", title="Main", children=[child])

        result = ax_inspector.walk_element(parent, max_depth=0)

        assert result["children_count"] == 1
        assert result["children"] == []

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_depth_one_walks_immediate_children(self, _mock_copy):
        """Depth 1 walks immediate children but not grandchildren."""
        grandchild = _make_mock_element(role="AXStaticText", title="Label")
        child = _make_mock_element(
            role="AXGroup", title="Panel", children=[grandchild]
        )
        root = _make_mock_element(role="AXWindow", title="Main", children=[child])

        result = ax_inspector.walk_element(root, max_depth=1)

        assert len(result["children"]) == 1
        assert result["children"][0]["title"] == "Panel"
        assert result["children"][0]["children_count"] == 1
        assert result["children"][0]["children"] == []

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_unlimited_depth(self, _mock_copy):
        """Depth -1 walks the full tree."""
        grandchild = _make_mock_element(role="AXStaticText", title="Deep")
        child = _make_mock_element(
            role="AXGroup", title="Middle", children=[grandchild]
        )
        root = _make_mock_element(role="AXWindow", title="Root", children=[child])

        result = ax_inspector.walk_element(root, max_depth=-1)

        assert len(result["children"]) == 1
        assert len(result["children"][0]["children"]) == 1
        assert result["children"][0]["children"][0]["title"] == "Deep"


class TestJsonSerialization:
    """Tests for JSON output correctness."""

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_output_is_valid_json(self, _mock_copy):
        """The walk result serializes to valid JSON."""
        elem = _make_mock_element(
            role="AXApplication",
            title="Audio-DNA",
            position=(0, 0),
            size=(1920, 1080),
        )

        result = ax_inspector.walk_element(elem)
        output = json.dumps(result, indent=2)

        parsed = json.loads(output)
        assert parsed["role"] == "AXApplication"
        assert parsed["position"]["x"] == 0.0

    @patch("ax_inspector.AXUIElementCopyAttributeValue", side_effect=_mock_copy_attribute)
    def test_nested_tree_serializes(self, _mock_copy):
        """A nested tree round-trips through JSON correctly."""
        child = _make_mock_element(role="AXButton", title="Click Me", value="0")
        root = _make_mock_element(
            role="AXWindow", title="Main Window", children=[child]
        )

        result = ax_inspector.walk_element(root)
        output = json.dumps(result)
        parsed = json.loads(output)

        assert parsed["children_count"] == 1
        assert parsed["children"][0]["role"] == "AXButton"
        assert parsed["children"][0]["value"] == "0"


class TestAppLookup:
    """Tests for application discovery and error handling."""

    @patch("ax_inspector.check_accessibility_permissions", return_value=False)
    def test_permissions_denied(self, _mock_perms):
        """Raises AccessibilityPermissionError when AX permissions are denied."""
        with pytest.raises(ax_inspector.AccessibilityPermissionError):
            ax_inspector.inspect_app(app_name="Audio-DNA")

    @patch("ax_inspector.check_accessibility_permissions", return_value=True)
    @patch("ax_inspector.find_pid_by_name", return_value=None)
    def test_app_not_found(self, _mock_find, _mock_perms):
        """Raises AppNotFoundError when the app is not running."""
        with pytest.raises(ax_inspector.AppNotFoundError):
            ax_inspector.inspect_app(app_name="NonExistentApp")

    @patch("ax_inspector.walk_element")
    @patch("ax_inspector.AXUIElementCreateApplication")
    @patch("ax_inspector.check_accessibility_permissions", return_value=True)
    def test_inspect_by_pid(self, _mock_perms, _mock_create, _mock_walk):
        """inspect_app with explicit PID skips name lookup."""
        _mock_walk.return_value = {"role": "AXApplication", "children": []}

        result = ax_inspector.inspect_app(pid=12345)

        _mock_create.assert_called_once_with(12345)
        assert result["pid"] == 12345
        assert result["tree"]["role"] == "AXApplication"

    @patch("ax_inspector.walk_element")
    @patch("ax_inspector.AXUIElementCreateApplication")
    @patch("ax_inspector.find_pid_by_name", return_value=99999)
    @patch("ax_inspector.check_accessibility_permissions", return_value=True)
    def test_inspect_by_name(self, _mock_perms, _mock_find, _mock_create, _mock_walk):
        """inspect_app finds PID by app name when pid is not provided."""
        _mock_walk.return_value = {"role": "AXApplication", "children": []}

        result = ax_inspector.inspect_app(app_name="Audio-DNA")

        _mock_find.assert_called_once_with("Audio-DNA")
        assert result["pid"] == 99999


class TestCLI:
    """Tests for the argparse-based CLI entry point."""

    @patch("ax_inspector.inspect_app")
    def test_help_flag(self, _mock_inspect, capsys):
        """--help prints usage and exits without calling inspect_app."""
        with pytest.raises(SystemExit) as exc_info:
            ax_inspector.main.__wrapped__ if hasattr(ax_inspector.main, "__wrapped__") else None
            # Simulate --help by patching sys.argv
            with patch("sys.argv", ["ax_inspector.py", "--help"]):
                ax_inspector.main()
        assert exc_info.value.code == 0
        _mock_inspect.assert_not_called()

    @patch("ax_inspector.inspect_app")
    def test_app_not_found_exit_code(self, _mock_inspect, capsys):
        """Exit code 2 when app is not found."""
        _mock_inspect.side_effect = ax_inspector.AppNotFoundError("not running")

        with patch("sys.argv", ["ax_inspector.py"]):
            with pytest.raises(SystemExit) as exc_info:
                ax_inspector.main()

        assert exc_info.value.code == 2

    @patch("ax_inspector.inspect_app")
    def test_permission_denied_exit_code(self, _mock_inspect, capsys):
        """Exit code 1 when permissions denied."""
        _mock_inspect.side_effect = ax_inspector.AccessibilityPermissionError("denied")

        with patch("sys.argv", ["ax_inspector.py"]):
            with pytest.raises(SystemExit) as exc_info:
                ax_inspector.main()

        assert exc_info.value.code == 1

    @patch("ax_inspector.inspect_app")
    def test_json_output_to_stdout(self, _mock_inspect, capsys):
        """Default output goes to stdout as valid JSON."""
        _mock_inspect.return_value = {
            "app_name": "Audio-DNA",
            "pid": 123,
            "tree": {"role": "AXApplication", "children": []},
        }

        with patch("sys.argv", ["ax_inspector.py"]):
            ax_inspector.main()

        captured = capsys.readouterr()
        parsed = json.loads(captured.out)
        assert parsed["app_name"] == "Audio-DNA"

    @patch("ax_inspector.inspect_app")
    def test_output_to_file(self, _mock_inspect, tmp_path):
        """--output writes JSON to a file."""
        _mock_inspect.return_value = {
            "app_name": "Audio-DNA",
            "pid": 456,
            "tree": {"role": "AXApplication", "children": []},
        }
        out_file = str(tmp_path / "tree.json")

        with patch("sys.argv", ["ax_inspector.py", "--output", out_file]):
            ax_inspector.main()

        with open(out_file) as f:
            parsed = json.loads(f.read())
        assert parsed["pid"] == 456

    @patch("ax_inspector.inspect_app")
    def test_depth_flag_passed(self, _mock_inspect):
        """--depth flag is forwarded to inspect_app."""
        _mock_inspect.return_value = {
            "app_name": "Audio-DNA",
            "pid": 1,
            "tree": {},
        }

        with patch("sys.argv", ["ax_inspector.py", "--depth", "3", "--pid", "1"]):
            ax_inspector.main()

        _mock_inspect.assert_called_once_with(
            app_name="Audio-DNA",
            pid=1,
            max_depth=3,
        )

    @patch("ax_inspector.inspect_app")
    def test_pid_flag_passed(self, _mock_inspect):
        """--pid flag is forwarded to inspect_app."""
        _mock_inspect.return_value = {
            "app_name": "Audio-DNA",
            "pid": 7777,
            "tree": {},
        }

        with patch("sys.argv", ["ax_inspector.py", "--pid", "7777"]):
            ax_inspector.main()

        _mock_inspect.assert_called_once_with(
            app_name="Audio-DNA",
            pid=7777,
            max_depth=-1,
        )


# ---------------------------------------------------------------------------
# Integration test — requires running Audio-DNA
# ---------------------------------------------------------------------------

def _audio_dna_running() -> bool:
    """Check if Audio-DNA is running (for integration test gating)."""
    try:
        from Cocoa import NSWorkspace as _NSWorkspace

        workspace = _NSWorkspace.sharedWorkspace()
        for app in workspace.runningApplications():
            if app.localizedName() == "Audio-DNA":
                return True
    except Exception:
        pass
    return False


@pytest.mark.skipif(
    not _audio_dna_running(),
    reason="Audio-DNA is not running — skipping integration test",
)
class TestIntegration:
    """Integration tests that connect to a live Audio-DNA instance."""

    def test_live_tree_structure(self):
        """Read the real accessibility tree and verify basic structure."""
        result = ax_inspector.inspect_app(app_name="Audio-DNA")

        assert result["pid"] > 0
        assert result["tree"]["role"] is not None
        # JUCE app root should be AXApplication
        assert result["tree"]["role"] == "AXApplication"

    def test_live_tree_has_children(self):
        """A running JUCE app should have at least one window child."""
        result = ax_inspector.inspect_app(app_name="Audio-DNA", max_depth=1)

        assert result["tree"]["children_count"] > 0
        assert len(result["tree"]["children"]) > 0

    def test_live_tree_json_roundtrip(self):
        """Live tree serializes to JSON and parses back correctly."""
        result = ax_inspector.inspect_app(app_name="Audio-DNA", max_depth=2)

        output = json.dumps(result, indent=2)
        parsed = json.loads(output)

        assert parsed["app_name"] == "Audio-DNA"
        assert isinstance(parsed["tree"]["children"], list)
