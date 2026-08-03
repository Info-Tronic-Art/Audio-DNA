"""
AX Press — presses a JUCE button by its Accessibility title, so visual-test
UI-state driving (e.g. expanding/collapsing the SignalBar) no longer requires
a human operator.

JUCE 7+ ships AccessibilityHandler, which maps real juce::Button instances to
NSAccessibility AXButton elements (see ax_inspector.py). This module walks
that tree via the Accessibility C API (pyobjc ApplicationServices/Cocoa —
the same dependency set ax_inspector.py already uses) to find a button by its
exact AXTitle and invoke AXPress on it directly. Synthetic coordinate clicks
on the SignalBar's tiny (20x14px) size-control buttons are a documented flake
(.harmony/gotchas.md 2026-07-30 #9); AXPress presses the element itself, so
target size and screen position never matter.

AppleScript/osascript CANNOT be used for this: `entire contents of window 1`
returns only the window's ~245 direct children and does not recurse into
JUCE's nested AXGroups, so `whose title is "..."` fails with -1700 (verified
2026-08-03 via ax_inspector.py --output, which found the buttons only by
walking AXChildren recursively).

Usage:
    # Press a button by exact title (default app: "Audio-DNA")
    python ax_press.py "▼"

    # Target a different app, or wait longer for it to appear
    python ax_press.py "OK" --app SomeOtherApp --timeout 10

As a library:
    from ax_press import press_button, expand_signalbar, collapse_signalbar
    press_button("▼")       # SignalBar: Normal/Minimized -> next larger
    expand_signalbar()          # same button, semantic name
    collapse_signalbar()        # SignalBar: Expanded/Normal -> next smaller
"""

import argparse
import sys
import time
from typing import Any, List, Optional

from ApplicationServices import (
    AXUIElementCreateApplication,
    AXUIElementCopyAttributeValue,
    AXUIElementPerformAction,
    kAXErrorSuccess,
    kAXPressAction,
)

from ax_inspector import check_accessibility_permissions, find_pid_by_name

# SignalBar size-control glyphs (src/ui/SignalBar.cpp:23-31):
#   growButton_.setButtonText("\xe2\x96\xbc")   -> UTF-8 for "▼" (down triangle)
#   growButton_.onClick = [this] { grow(); };   -> Minimized/Normal -> larger
#   shrinkButton_.setButtonText("\xe2\x96\xb2") -> UTF-8 for "▲" (up triangle)
#   shrinkButton_.onClick = [this] { shrink(); }-> Expanded/Normal -> smaller
# grow() cycles Minimized -> Normal -> Expanded (SignalBar.cpp:84-98); the
# probe (test_mapping_tick.py state 3/4) needs Expanded, so "▼" (grow) is
# the button to press to reach it from the default Normal state.
GROW_GLYPH = "▼"    # "▼" BLACK DOWN-POINTING TRIANGLE -> SignalBar::grow()
SHRINK_GLYPH = "▲"  # "▲" BLACK UP-POINTING TRIANGLE  -> SignalBar::shrink()

DEFAULT_APP_NAME = "Audio-DNA"
DEFAULT_TIMEOUT_S = 5.0
POLL_INTERVAL_S = 0.2


class AXPressError(Exception):
    """Base exception for ax_press errors."""


class AppNotFoundError(AXPressError):
    """Raised when the target application cannot be found."""


class AccessibilityPermissionError(AXPressError):
    """Raised when accessibility permissions are not granted."""


class ButtonNotFoundError(AXPressError):
    """Raised when no element with the given title is found."""


class AmbiguousButtonError(AXPressError):
    """Raised when more than one element matches the given title."""


def _get_ax_attribute(element: Any, attribute: str) -> Any:
    """Safely read a single AXUIElement attribute (mirrors ax_inspector.py).

    Args:
        element: The AXUIElement to query.
        attribute: The attribute name (e.g., "AXRole", "AXTitle").

    Returns:
        The attribute value, or None if the attribute is missing or unreadable.
    """
    err, value = AXUIElementCopyAttributeValue(element, attribute, None)
    if err == kAXErrorSuccess:
        return value
    return None


def _find_buttons_by_title(element: Any, title: str, matches: List[Any]) -> None:
    """Recursively walk the AX tree, collecting AXButton elements whose
    AXTitle equals `title` exactly.

    Args:
        element: The AXUIElement subtree root to search.
        title: Exact title to match.
        matches: List to append matching elements to (mutated in place).
    """
    role = _get_ax_attribute(element, "AXRole")
    elem_title = _get_ax_attribute(element, "AXTitle")

    if (
        role is not None
        and str(role) == "AXButton"
        and elem_title is not None
        and str(elem_title) == title
    ):
        matches.append(element)

    children = _get_ax_attribute(element, "AXChildren") or []
    for child in children:
        _find_buttons_by_title(child, title, matches)


def find_button(
    title: str,
    app_name: str = DEFAULT_APP_NAME,
    pid: Optional[int] = None,
) -> Any:
    """Find the unique AXButton with the given exact title in the named app.

    Args:
        title: Exact AXTitle to match (e.g. "▼").
        app_name: The application name to find (ignored if pid is provided).
        pid: Explicit PID to connect to (overrides app_name lookup).

    Returns:
        The matching AXUIElement.

    Raises:
        AccessibilityPermissionError: If accessibility permissions are denied.
        AppNotFoundError: If the target app is not running.
        ButtonNotFoundError: If no AXButton with that title exists.
        AmbiguousButtonError: If more than one AXButton has that title.
    """
    if not check_accessibility_permissions():
        raise AccessibilityPermissionError(
            "Accessibility permissions not granted to this process (usually "
            "your terminal app or its python interpreter). Grant it in "
            "System Settings > Privacy & Security > Accessibility, then "
            "restart the terminal and retry."
        )

    if pid is None:
        pid = find_pid_by_name(app_name)
        if pid is None:
            raise AppNotFoundError(
                f"Application '{app_name}' not found. Is it running?"
            )

    app_ref = AXUIElementCreateApplication(pid)

    matches: List[Any] = []
    _find_buttons_by_title(app_ref, title, matches)

    if len(matches) == 0:
        raise ButtonNotFoundError(
            f"No AXButton with title {title!r} found in '{app_name}' "
            f"(pid {pid})."
        )
    if len(matches) > 1:
        raise AmbiguousButtonError(
            f"{len(matches)} AXButton elements with title {title!r} found "
            f"in '{app_name}' (pid {pid}) — refusing to guess which one to "
            f"press."
        )

    return matches[0]


def press_button(
    title: str,
    app_name: str = DEFAULT_APP_NAME,
    timeout: float = DEFAULT_TIMEOUT_S,
    pid: Optional[int] = None,
) -> bool:
    """Find a JUCE button by its exact accessibility title and press it.

    Retries the (app lookup + button lookup) pair until `timeout` elapses,
    since the AX tree can be transiently incomplete right after launch or a
    layout change. Permission errors and ambiguous-title errors are NOT
    transient and are raised immediately without retrying.

    Args:
        title: Exact AXTitle of the button to press (e.g. "▼").
        app_name: The application name to find (ignored if pid is provided).
        timeout: Seconds to keep retrying app/button lookup before giving up.
        pid: Explicit PID to connect to (overrides app_name lookup).

    Returns:
        True on success (AXPress delivered without an AX error).

    Raises:
        AccessibilityPermissionError: If accessibility permissions are denied.
        AppNotFoundError: If the app never appears within `timeout`.
        ButtonNotFoundError: If no matching AXButton appears within `timeout`.
        AmbiguousButtonError: If more than one AXButton has that title.
        AXPressError: If AXUIElementPerformAction itself fails.
    """
    deadline = time.time() + timeout
    last_error: Optional[Exception] = None

    while True:
        try:
            button = find_button(title, app_name=app_name, pid=pid)
            break
        except (AppNotFoundError, ButtonNotFoundError) as e:
            last_error = e
            if time.time() >= deadline:
                raise
            time.sleep(POLL_INTERVAL_S)

    err = AXUIElementPerformAction(button, kAXPressAction)
    if err != kAXErrorSuccess:
        raise AXPressError(
            f"AXUIElementPerformAction(AXPress) on {title!r} failed with "
            f"AXError {err}."
        )
    return True


def expand_signalbar(app_name: str = DEFAULT_APP_NAME, timeout: float = DEFAULT_TIMEOUT_S) -> bool:
    """Press the SignalBar's grow control (▼) to move it one step larger
    (Minimized -> Normal -> Expanded; see GROW_GLYPH comment for evidence).
    """
    return press_button(GROW_GLYPH, app_name=app_name, timeout=timeout)


def collapse_signalbar(app_name: str = DEFAULT_APP_NAME, timeout: float = DEFAULT_TIMEOUT_S) -> bool:
    """Press the SignalBar's shrink control (▲) to move it one step
    smaller (Expanded -> Normal -> Minimized; see SHRINK_GLYPH comment for
    evidence).
    """
    return press_button(SHRINK_GLYPH, app_name=app_name, timeout=timeout)


def main():
    """CLI entry point: press a button by title in a running app."""
    parser = argparse.ArgumentParser(
        description="Press a macOS Accessibility button by its exact AXTitle.",
    )
    parser.add_argument(
        "title",
        help='Exact AXTitle of the button to press (e.g. "▼").',
    )
    parser.add_argument(
        "--app",
        dest="app_name",
        default=DEFAULT_APP_NAME,
        help=f"Application name to find (default: {DEFAULT_APP_NAME}).",
    )
    parser.add_argument(
        "--pid",
        type=int,
        default=None,
        help="PID of the target application (overrides --app).",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT_S,
        help=f"Seconds to retry app/button lookup (default: {DEFAULT_TIMEOUT_S}).",
    )
    args = parser.parse_args()

    try:
        press_button(
            args.title,
            app_name=args.app_name,
            timeout=args.timeout,
            pid=args.pid,
        )
    except AccessibilityPermissionError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except AppNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(2)
    except ButtonNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(3)
    except AmbiguousButtonError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(4)
    except AXPressError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(5)

    print(f"Pressed {args.title!r} in {args.app_name!r}.", file=sys.stderr)


if __name__ == "__main__":
    main()
