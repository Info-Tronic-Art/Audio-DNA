"""
Accessibility Tree Inspector — reads the macOS AXUIElement tree of a running
Audio-DNA (JUCE) application and outputs structured JSON.

JUCE 7+ ships AccessibilityHandler that maps components to NSAccessibility,
so this script can read the full UI hierarchy from outside the process with
zero C++ changes required.

Usage:
    # Find by app name (default: "Audio-DNA")
    python ax_inspector.py

    # Find by PID
    python ax_inspector.py --pid 12345

    # Limit recursion depth
    python ax_inspector.py --depth 3

    # Write to file instead of stdout
    python ax_inspector.py --output tree.json
"""

import argparse
import json
import sys
from typing import Any, Optional

from ApplicationServices import (
    AXIsProcessTrusted,
    AXUIElementCreateApplication,
    AXUIElementCopyAttributeValue,
    AXUIElementCopyAttributeNames,
    kAXErrorSuccess,
)
from Cocoa import NSWorkspace


class AXInspectorError(Exception):
    """Base exception for accessibility inspector errors."""


class AppNotFoundError(AXInspectorError):
    """Raised when the target application cannot be found."""


class AccessibilityPermissionError(AXInspectorError):
    """Raised when accessibility permissions are not granted."""


def find_pid_by_name(app_name: str) -> Optional[int]:
    """Find a running application's PID by its display name.

    Args:
        app_name: The application name to search for (e.g., "Audio-DNA").

    Returns:
        The PID of the first matching application, or None if not found.
    """
    workspace = NSWorkspace.sharedWorkspace()
    for app in workspace.runningApplications():
        if app.localizedName() == app_name:
            return app.processIdentifier()
    return None


def check_accessibility_permissions() -> bool:
    """Check whether this process has accessibility permissions.

    Returns:
        True if accessibility API access is granted.
    """
    return bool(AXIsProcessTrusted())


def _get_ax_attribute(element: Any, attribute: str) -> Any:
    """Safely read a single AXUIElement attribute.

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


def _get_position(element: Any) -> Optional[dict]:
    """Extract the screen position of an AXUIElement.

    Args:
        element: The AXUIElement to query.

    Returns:
        Dict with "x" and "y" keys, or None if position is unavailable.
    """
    value = _get_ax_attribute(element, "AXPosition")
    if value is not None:
        try:
            return {"x": float(value.x), "y": float(value.y)}
        except (AttributeError, TypeError):
            return None
    return None


def _get_size(element: Any) -> Optional[dict]:
    """Extract the size of an AXUIElement.

    Args:
        element: The AXUIElement to query.

    Returns:
        Dict with "width" and "height" keys, or None if size is unavailable.
    """
    value = _get_ax_attribute(element, "AXSize")
    if value is not None:
        try:
            return {"width": float(value.width), "height": float(value.height)}
        except (AttributeError, TypeError):
            return None
    return None


def walk_element(element: Any, max_depth: int = -1, _current_depth: int = 0) -> dict:
    """Recursively walk an AXUIElement and return its tree as a dict.

    Args:
        element: The AXUIElement to inspect.
        max_depth: Maximum recursion depth (-1 for unlimited).
        _current_depth: Internal counter for current depth (do not set manually).

    Returns:
        Dict containing role, title, value, description, position, size,
        children_count, and children (list of child dicts).
    """
    role = _get_ax_attribute(element, "AXRole")
    title = _get_ax_attribute(element, "AXTitle")
    value = _get_ax_attribute(element, "AXValue")
    description = _get_ax_attribute(element, "AXDescription")
    position = _get_position(element)
    size = _get_size(element)

    # Coerce ObjC types to Python primitives for JSON serialization
    if role is not None:
        role = str(role)
    if title is not None:
        title = str(title)
    if value is not None:
        value = str(value)
    if description is not None:
        description = str(description)

    children_elements = _get_ax_attribute(element, "AXChildren") or []
    children_count = len(children_elements)

    children = []
    if max_depth == -1 or _current_depth < max_depth:
        for child in children_elements:
            children.append(
                walk_element(child, max_depth=max_depth, _current_depth=_current_depth + 1)
            )

    return {
        "role": role,
        "title": title,
        "value": value,
        "description": description,
        "position": position,
        "size": size,
        "children_count": children_count,
        "children": children,
    }


def inspect_app(
    app_name: str = "Audio-DNA",
    pid: Optional[int] = None,
    max_depth: int = -1,
) -> dict:
    """Inspect the accessibility tree of a running application.

    Args:
        app_name: The application name to find (ignored if pid is provided).
        pid: Explicit PID to connect to (overrides app_name lookup).
        max_depth: Maximum recursion depth (-1 for unlimited).

    Returns:
        Dict with "app_name", "pid", and "tree" (the root element dict).

    Raises:
        AccessibilityPermissionError: If accessibility permissions are denied.
        AppNotFoundError: If the target app is not running.
    """
    if not check_accessibility_permissions():
        raise AccessibilityPermissionError(
            "Accessibility permissions not granted. "
            "Enable in System Settings > Privacy & Security > Accessibility."
        )

    if pid is None:
        pid = find_pid_by_name(app_name)
        if pid is None:
            raise AppNotFoundError(
                f"Application '{app_name}' not found. Is it running?"
            )

    app_ref = AXUIElementCreateApplication(pid)
    tree = walk_element(app_ref, max_depth=max_depth)

    return {
        "app_name": app_name,
        "pid": pid,
        "tree": tree,
    }


def main():
    """CLI entry point for the accessibility inspector."""
    parser = argparse.ArgumentParser(
        description="Inspect the macOS accessibility tree of a running application.",
    )
    parser.add_argument(
        "--app-name",
        default="Audio-DNA",
        help="Application name to find (default: Audio-DNA).",
    )
    parser.add_argument(
        "--pid",
        type=int,
        default=None,
        help="PID of the target application (overrides --app-name).",
    )
    parser.add_argument(
        "--depth",
        type=int,
        default=-1,
        help="Maximum recursion depth (-1 for unlimited).",
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Write JSON to file instead of stdout.",
    )
    args = parser.parse_args()

    try:
        result = inspect_app(
            app_name=args.app_name,
            pid=args.pid,
            max_depth=args.depth,
        )
    except AccessibilityPermissionError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except AppNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(2)

    output_json = json.dumps(result, indent=2)

    if args.output:
        with open(args.output, "w") as f:
            f.write(output_json)
            f.write("\n")
        print(f"Written to {args.output}", file=sys.stderr)
    else:
        print(output_json)


if __name__ == "__main__":
    main()
