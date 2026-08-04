"""
Output-window LEVEL probe — regression guard for the "black overlay on every
Space" bug (.harmony/black-overlay-rootcause.md, 2026-08-03).

WHAT IT PROVES
    The fullscreen output window on the MAIN display must be created at
    NSNormalWindowLevel. CoreGraphics reports that as kCGWindowLayer == 0.

    Today it is created at NSFloatingWindowLevel (kCGWindowLayer == 3):

        src/ui/OutputWindow.cpp:337   setAlwaysOnTop(true);   <- the defect
            (inside goFullscreenOnDisplay, OutputWindow.cpp:328-342)

    JUCE 8.0.4 maps always-on-top to NSFloatingWindowLevel
    (build/_deps/juce-src/modules/juce_gui_basics/native/
     juce_NSViewComponentPeer_mac.mm:595-601) and never sets an NSWindow
    Spaces-participation bit, so macOS defaults a non-normal-level window to
    Transient = "floats in spaces" -> an opaque black rectangle on every
    desktop Space. Dropping the call yields NSNormalWindowLevel -> Managed ->
    bound to one Space.

    THIS PROBE IS EXPECTED TO FAIL (exit 1) AGAINST TODAY'S CODE. That failure
    is the deliverable. It passes once the main-display path stops asking for
    always-on-top. (Boris's ruling keeps always-on-top for NON-main displays;
    this probe only ever opens and measures the "(main)" display item, so it
    does not contradict that ruling.)

SCREEN SAFETY
    The window under test is a real, opaque, full-display black window on the
    machine running this probe. Everything here is built around never leaving
    one on the screen, and around never CLAIMING the screen is clean without
    having looked:
      * it refuses to open the window at all if it cannot measure it
        (Screen Recording permission pre-check, before launch);
      * teardown runs from a finally block on EVERY exit path, including
        assertion failure, exception, Ctrl-C and the run timeout;
      * teardown is driven by observed reality, never by what the run
        believed: it rescans the process table and the window server, and
        cleans up anything this run may have launched even when it never
        managed to confirm the launch (2026-08-03: the first version timed out
        waiting for NSWorkspace to notice an app that was in fact alive and
        healthy, declared "nothing was ever opened", skipped teardown and left
        an orphan running);
      * teardown escalates: Output menu "Disabled" -> Escape key -> SIGTERM
        -> SIGKILL. Killing the process provably removes the window (the app
        creates no OS-level artifact that can outlive it — see
        "NO OS-LEVEL ARTIFACT SURFACE" in .harmony/black-overlay-rootcause.md);
      * a watchdog thread hard-kills whatever this run launched, and exits the
        process, if teardown itself wedges;
      * the exit code goes RED if the screen could not be verified clean or an
        orphan process is left behind, even when the level assertion passed.

    Attach mode (AUDIODNA_NO_SPAWN=1) never quits the app it attached to —
    with one exception: if the output window survives both UI close routes,
    terminating the app is the last rung of the ladder, because a stranded
    fullscreen black window is worse than an unexpected app exit.

USAGE
    Run it STANDALONE from the repo root (it is not a pytest test — see NOTE):

        .venv/bin/python tests/visual/test_output_window_level.py

    Against an app that is already running (does not launch, does not quit it;
    still closes the output window it opened):

        AUDIODNA_NO_SPAWN=1 .venv/bin/python tests/visual/test_output_window_level.py

    Environment:
        AUDIODNA_APP                 override the .app bundle path
        AUDIODNA_NO_SPAWN=1          attach to a running app instead of launching
        OW_LEVEL_SCREENSHOT=0        skip the teardown screenshot
        OW_LEVEL_IGNORE_SCREEN_PERM=1
                                     proceed past the Screen Recording
                                     pre-check (the window-name match still
                                     has to succeed — the assertion is never
                                     weakened, only the pre-check is skipped)

    Exit codes:
        0  PASS      — window found at layer 0, screen verified clean
        1  FAIL      — window found at the wrong layer (expected today: 3)
        2  BLOCKED   — could not run the measurement, or teardown could not
                       verify the screen is clean, or an orphan process was
                       left behind (see the printed banner)
        3  WATCHDOG  — the run wedged; whatever it launched was force-killed

    Timing: the app gets LAUNCH_READY_TIMEOUT_S (60s) to start and answer
    /api/health — nothing is on screen during that wait. The 60s SIGALRM hard
    cap is armed only once the measurement begins, i.e. it bounds exactly the
    phase in which a black window can exist.

NOTE — why this is not a pytest test
    tests/visual/conftest.py has an autouse fixture that pulls in the session
    `app` fixture, which spawns the binary DIRECTLY (vj_controller.py:39
    subprocess.Popen). Direct binary exec is forbidden for behavioral runs in
    this repo (.harmony/gotchas.md 2026-07-17: always launch via `open`), and
    that fixture also owns the app's lifecycle, which would fight this probe's
    teardown ladder. So this module deliberately defines no `test_*` function:
    `pytest tests/visual/` imports it and collects nothing from it.
"""

import os
import signal
import subprocess
import sys
import threading
import time
from typing import List, Optional, Tuple

try:
    import Quartz
except ImportError as exc:  # pragma: no cover - environment guard
    print(
        "BLOCKED: pyobjc Quartz is not importable — the window layer cannot be\n"
        "         read without it, and this probe will NOT substitute a weaker\n"
        "         check. Install it into the repo venv:\n"
        "             .venv/bin/pip install pyobjc-framework-Quartz\n"
        f"         (import error: {exc})",
        file=sys.stderr,
    )
    sys.exit(2)

import requests

from ax_inspector import check_accessibility_permissions, find_pid_by_name

# --- what we are measuring -------------------------------------------------

# DocumentWindow title, src/ui/OutputWindow.cpp:295 (verified 2026-08-03).
OUTPUT_WINDOW_NAME = "Audio-DNA Output"

# CFBundleName of build/AudioDNA_artefacts/Release/Audio-DNA.app (verified).
APP_NAME = "Audio-DNA"

EXPECTED_LAYER = 0        # NSNormalWindowLevel   -> Managed -> one Space
FLOATING_LAYER = 3        # NSFloatingWindowLevel -> Transient -> all Spaces

# Output menu, src/ui/MenuBarModel.cpp:125-143 (menu name "Output",
# getMenuBarNames() line 10). The fullscreen item label is built at runtime
# from the display's totalArea — "Fullscreen: <W>x<H> (main)" for the main
# display (MenuBarModel.cpp:135-142) — so it is DISCOVERED, never hardcoded.
OUTPUT_MENU = "Output"
DISABLED_ITEM = "Disabled"          # MenuBarModel.cpp:127 -> closeOutput()
FULLSCREEN_PREFIX = "Fullscreen: "
MAIN_SUFFIX = "(main)"

# --- budgets ---------------------------------------------------------------

# The 60s hard cap covers the MEASUREMENT phase — the only phase in which a
# black window can be on the screen. The alarm is armed immediately before the
# window is opened, not at launch: time spent waiting for a cold app to become
# ready is time with nothing on screen, and folding it into the same budget is
# what made the first version give up on a launch that had actually succeeded.
RUN_TIMEOUT_S = 60.0          # hard cap on the measurement phase (SIGALRM)
TEARDOWN_GRACE_S = 20.0       # extra time the watchdog allows for teardown
LAUNCH_READY_TIMEOUT_S = 60.0  # `open` -> pid found AND /api/health answering
WINDOW_APPEAR_TIMEOUT_S = 6.0
WINDOW_GONE_TIMEOUT_S = 5.0
OSASCRIPT_TIMEOUT_S = 15.0
POLL_INTERVAL_S = 0.25
OPEN_ATTEMPTS = 3             # menu click is a documented ~15% drop
CLOSE_ATTEMPTS = 3

# The flag this probe passes to `open`. Also used to attribute a process to
# this run when the launch was never observed (see _is_ours).
TEST_MODE_FLAG = "--test-mode"

# TestServer binds ::1 ONLY and needs --test-mode; 127.0.0.1 returns empty and
# is indistinguishable from a dead server, and 7070 binds WITHOUT --test-mode
# so checking it is a false green (.harmony/gotchas.md 2026-08-03, 2026-08-02).
HEALTH_URL = "http://[::1]:8080/api/health"

EXIT_PASS, EXIT_FAIL, EXIT_BLOCKED, EXIT_WATCHDOG = 0, 1, 2, 3


class ProbeBlocked(Exception):
    """Raised when the measurement cannot be performed (exit 2)."""


class LevelMismatch(Exception):
    """Raised when the window exists but is at the wrong layer (exit 1)."""


class RunTimeout(Exception):
    """Raised by SIGALRM when the measurement phase exceeds RUN_TIMEOUT_S."""


# ===========================================================================
# small helpers
# ===========================================================================


def _say(msg: str) -> None:
    """Print a progress line to stderr, unbuffered."""
    print(msg, file=sys.stderr, flush=True)


def _repo_root() -> str:
    """Absolute path of the repo root (this file lives in tests/visual/)."""
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def _default_app_bundle() -> str:
    """Default .app bundle path (the Release artefact conftest.py also uses)."""
    return os.path.join(
        _repo_root(), "build", "AudioDNA_artefacts", "Release", "Audio-DNA.app"
    )


def _as_applescript_string(value: str) -> str:
    """Quote a Python string for embedding in AppleScript source."""
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def _osascript(script: str, timeout: float = OSASCRIPT_TIMEOUT_S) -> str:
    """Run an AppleScript snippet and return its stdout.

    Args:
        script: AppleScript source.
        timeout: Hard timeout in seconds — osascript can block on a modal
            dialog or a wedged app, and this probe must never block forever.

    Returns:
        stdout, stripped.

    Raises:
        ProbeBlocked: If osascript fails or times out. The message includes
            stderr, which is where the Accessibility-permission error lands.
    """
    try:
        proc = subprocess.run(
            ["osascript", "-e", script],
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        raise ProbeBlocked(f"osascript timed out after {timeout}s")

    if proc.returncode != 0:
        raise ProbeBlocked(
            f"osascript failed (rc={proc.returncode}): {proc.stderr.strip()}\n"
            "If this says 'not allowed assistive access', grant Accessibility "
            "to the terminal running this probe in System Settings > Privacy "
            "& Security > Accessibility."
        )
    return proc.stdout.strip()


def _process_alive(pid: int) -> bool:
    """Whether a PID is still running (EPERM counts as alive)."""
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    return True


def _pgrep(args: List[str]) -> List[int]:
    """Run pgrep and return the matching PIDs (empty list on no match).

    pgrep reads the live process table on every call, which is why it is the
    PRIMARY discovery mechanism here: it cannot go stale.
    """
    try:
        proc = subprocess.run(
            ["pgrep"] + args, capture_output=True, text=True, timeout=10
        )
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return []
    if proc.returncode not in (0, 1):  # 1 == no match, which is not an error
        return []
    pids = []
    for line in proc.stdout.split():
        try:
            pid = int(line)
        except ValueError:
            continue
        if pid != os.getpid():
            pids.append(pid)
    return pids


def _find_app_pids(app_bundle: Optional[str] = None) -> List[int]:
    """Every live Audio-DNA process, by three independent mechanisms.

    NSWorkspace alone is NOT trustworthy here. `runningApplications()` is kept
    current by notifications delivered on a run loop, and this script never
    runs one, so a snapshot taken before launch can stay stale for the whole
    process lifetime — the probe's first version polled it for 20s, never saw
    the app it had just launched, concluded nothing had been opened and left a
    live orphan behind. pgrep reads the process table directly and is the
    primary mechanism; NSWorkspace is kept as a third opinion and pumped
    first so it has a chance to catch up.

    Args:
        app_bundle: If given, also match the bundle's executable path.

    Returns:
        Sorted list of distinct PIDs (never includes this process).
    """
    pids = set(_pgrep(["-x", APP_NAME]))

    if app_bundle:
        exe = os.path.join(app_bundle, "Contents", "MacOS", APP_NAME)
        pids.update(_pgrep(["-f", exe]))

    # Give NSWorkspace's notification-driven cache a chance to update before
    # asking it (no-op if there is nothing queued).
    try:
        Quartz.CFRunLoopRunInMode(Quartz.kCFRunLoopDefaultMode, 0.15, False)
    except Exception:  # pragma: no cover - defensive, never fatal
        pass
    ws_pid = find_pid_by_name(APP_NAME)
    if ws_pid is not None and ws_pid != os.getpid():
        pids.add(int(ws_pid))

    return sorted(p for p in pids if _process_alive(p))


def _process_info(pid: int) -> Tuple[Optional[float], str]:
    """Start time (epoch seconds) and full argv of a PID.

    Returns:
        (start_time or None, command line or ""). Both are best-effort: a
        process can exit between the scan and this call.
    """
    try:
        proc = subprocess.run(
            ["ps", "-p", str(pid), "-o", "lstart=,command="],
            capture_output=True,
            text=True,
            timeout=10,
        )
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return None, ""
    line = proc.stdout.strip()
    if proc.returncode != 0 or not line:
        return None, ""

    # lstart is 5 fields: "Sun Aug  3 13:38:12 2026", then the command.
    parts = line.split()
    if len(parts) < 6:
        return None, line
    stamp = " ".join(parts[:5])
    command = " ".join(parts[5:])
    try:
        start = time.mktime(time.strptime(stamp, "%a %b %d %H:%M:%S %Y"))
    except ValueError:
        start = None
    return start, command


def _is_ours(pid: int, launch_time: Optional[float], known_pid: Optional[int]) -> bool:
    """Whether this run is the one that launched `pid`.

    Attribution matters because the alternative — "kill every Audio-DNA in the
    process table" — can kill an instance somebody else is deliberately using.
    A process is attributed to this run when:
      * it is the PID this run actually observed become ready, or
      * its argv carries the --test-mode flag this run passes to `open`, AND
        it started at/after this run's `open` call.
    If the start time cannot be read but the flag matches, it still counts:
    preflight refuses to spawn while any Audio-DNA is running, so in spawn
    mode a --test-mode instance is ours by elimination.

    A process that fails this test is never force-quit. It is still asked to
    close an on-screen output window (see _teardown), because a stranded black
    window outranks politeness.
    """
    if known_pid is not None and pid == known_pid:
        return True
    if launch_time is None:
        return False
    start, command = _process_info(pid)
    if TEST_MODE_FLAG not in command:
        return False
    return start is None or start >= launch_time - 5.0


def _health_ok() -> Tuple[bool, str]:
    """One health probe against [::1]:8080.

    Returns:
        (ok, detail) — detail is the body or the error, for diagnostics.
    """
    try:
        r = requests.get(HEALTH_URL, timeout=2)
    except Exception as exc:
        return False, repr(exc)
    if r.status_code == 200:
        return True, r.text.strip()[:120]
    return False, f"HTTP {r.status_code}"


# ===========================================================================
# CoreGraphics window inspection — the actual measurement
# ===========================================================================


def _onscreen_windows() -> List[dict]:
    """Snapshot every on-screen window, excluding desktop elements.

    Returns:
        List of dicts with keys: number, name (str or None), layer (int),
        pid (int), owner (str or None), bounds (dict or None).
    """
    info = Quartz.CGWindowListCopyWindowInfo(
        Quartz.kCGWindowListOptionOnScreenOnly
        | Quartz.kCGWindowListExcludeDesktopElements,
        Quartz.kCGNullWindowID,
    )
    windows = []
    for w in info or []:
        name = w.get(Quartz.kCGWindowName)
        owner = w.get(Quartz.kCGWindowOwnerName)
        bounds = w.get(Quartz.kCGWindowBounds)
        windows.append(
            {
                "number": int(w.get(Quartz.kCGWindowNumber, -1)),
                "name": str(name) if name is not None else None,
                "layer": int(w.get(Quartz.kCGWindowLayer, -999)),
                "pid": int(w.get(Quartz.kCGWindowOwnerPID, -1)),
                "owner": str(owner) if owner is not None else None,
                "bounds": dict(bounds) if bounds is not None else None,
            }
        )
    return windows


def _find_output_windows(pid: int) -> List[dict]:
    """Find the output window(s): exact title match AND owned by `pid`.

    Matching on the owning process as well as the name means a stale window
    from another app (or a second Audio-DNA instance) cannot satisfy the
    probe.
    """
    return [
        w
        for w in _onscreen_windows()
        if w["pid"] == pid and w["name"] == OUTPUT_WINDOW_NAME
    ]


def _find_output_windows_any() -> List[dict]:
    """Every on-screen window titled "Audio-DNA Output", whatever owns it.

    The teardown verdict uses this rather than the per-PID lookup: the screen
    does not care which process a black window belongs to, and at teardown
    time the probe may not have a trustworthy PID at all.
    """
    return [w for w in _onscreen_windows() if w["name"] == OUTPUT_WINDOW_NAME]


def _window_names_readable() -> bool:
    """Whether window titles are visible to this process at all.

    macOS redacts kCGWindowName without Screen Recording permission. If NO
    on-screen window has a name, a title-based "nothing found" result proves
    nothing, so the teardown verdict must not be trusted.
    """
    return any(w["name"] for w in _onscreen_windows())


def _wait_for_output_window(pid: int, timeout: float) -> Optional[dict]:
    """Poll until the output window appears, or `timeout` elapses."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        matches = _find_output_windows(pid)
        if matches:
            return matches[0]
        time.sleep(POLL_INTERVAL_S)
    return None


def _wait_for_output_window_gone(pid: int, timeout: float) -> bool:
    """Poll until no output window owned by `pid` is on screen."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        if not _find_output_windows(pid):
            return True
        time.sleep(POLL_INTERVAL_S)
    return not _find_output_windows(pid)


def _describe_pid_windows(pid: int) -> str:
    """Human-readable dump of every on-screen window owned by `pid`.

    Used for diagnostics when the name match fails — it distinguishes "the
    window never opened" from "macOS redacted the window name".
    """
    rows = [w for w in _onscreen_windows() if w["pid"] == pid]
    if not rows:
        return f"    (no on-screen windows owned by pid {pid})"
    out = []
    for w in rows:
        b = w["bounds"] or {}
        out.append(
            f"    #{w['number']} layer={w['layer']} name={w['name']!r} "
            f"bounds={b.get('X')},{b.get('Y')} "
            f"{b.get('Width')}x{b.get('Height')}"
        )
    return "\n".join(out)


def _main_display_bounds() -> Tuple[float, float]:
    """Width/height of the main display, for diagnostic comparison only."""
    rect = Quartz.CGDisplayBounds(Quartz.CGMainDisplayID())
    return float(rect.size.width), float(rect.size.height)


# ===========================================================================
# app lifecycle
# ===========================================================================


def _spawn_app(app_bundle: str) -> str:
    """Run `open` on the bundle. Returns the stderr log path.

    Uses `open --stdout/--stderr ... --args --test-mode`: never direct binary
    exec (.harmony/gotchas.md 2026-07-17), and without --test-mode port 8080
    never binds while ApiServer still binds 7070, so a 7070 check would be a
    false green (.harmony/gotchas.md 2026-08-03).

    The CALLER must record "a launch was attempted" BEFORE calling this, and
    must keep that flag set no matter how this function ends: `open` returns
    as soon as LaunchServices accepts the request, so the app can be alive
    even when everything after this point fails.

    Raises:
        ProbeBlocked: If `open` itself fails.
    """
    stamp = int(time.time())
    stdout_log = f"/tmp/ow-level-{stamp}.out.log"
    stderr_log = f"/tmp/ow-level-{stamp}.err.log"
    _say(f"[launch] open {app_bundle} --args --test-mode")
    _say(f"[launch] stdout -> {stdout_log}   stderr -> {stderr_log}")

    proc = subprocess.run(
        [
            "open",
            "--stdout",
            stdout_log,
            "--stderr",
            stderr_log,
            app_bundle,
            "--args",
            "--test-mode",
        ],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if proc.returncode != 0:
        raise ProbeBlocked(f"`open` failed: {proc.stderr.strip()}")
    return stderr_log


def _wait_for_ready(app_bundle: str, stderr_log: str) -> int:
    """Wait for BOTH readiness signals and return the app's PID.

    Two signals, polled together against one generous deadline:
      * a live PID (pgrep-first — see _find_app_pids), needed to target the
        app by unix id and to match window ownership;
      * /api/health on [::1]:8080, the authoritative readiness signal, which
        also proves --test-mode took effect.

    Both are required. There is deliberately no weaker fallback: a half-started
    app makes the menu drive flaky, and a 7070 check would be a false green.
    Release binds ~12s cold and can take considerably longer, so the deadline
    is LAUNCH_READY_TIMEOUT_S — nothing is on screen during this wait.

    Raises:
        ProbeBlocked: With BOTH signals' last observed state, so the failure
            says which half was missing.
    """
    deadline = time.time() + LAUNCH_READY_TIMEOUT_S
    pid = None
    healthy = False
    detail = "not probed"

    while time.time() < deadline:
        if pid is None:
            pids = _find_app_pids(app_bundle)
            if pids:
                pid = pids[0]
                _say(f"[launch] pid {pid}"
                     + (f" (also found {pids[1:]})" if len(pids) > 1 else ""))
        if not healthy:
            healthy, detail = _health_ok()
            if healthy:
                _say(f"[launch] health OK: {detail}")
        if pid is not None and healthy:
            return pid
        time.sleep(0.5)

    raise ProbeBlocked(
        f"App not ready within {LAUNCH_READY_TIMEOUT_S:.0f}s: "
        f"pid={'found ' + str(pid) if pid else 'NOT FOUND'}, "
        f"health={'OK' if healthy else 'no answer (' + detail + ')'}.\n"
        f"  If the pid was found but health never answered, the app is up but "
        f"TestServer never bound: either the build lacks "
        f"-DAUDIODNA_BUILD_TEST_SERVER=ON, or startup stalled — check "
        f"{stderr_log} and look for a TCC microphone prompt on screen "
        f"(.harmony/gotchas.md 2026-07-25).\n"
        f"  Teardown still runs: a launched app is cleaned up whether or not "
        f"this probe managed to observe it starting."
    )


def _terminate_app(pid: int) -> str:
    """SIGTERM then SIGKILL the app, bounded. Returns a state description.

    Used both as the last rung of the close ladder and as the spawned-app
    quit step. Killing the process is what provably removes the window if the
    UI route failed: the app creates no OS-level artifact that can outlive it
    (.harmony/black-overlay-rootcause.md, "NO OS-LEVEL ARTIFACT SURFACE").
    """
    if not _process_alive(pid):
        return "already gone"

    try:
        os.kill(pid, signal.SIGTERM)
    except ProcessLookupError:
        return "already gone"
    deadline = time.time() + 6.0
    while time.time() < deadline:
        if not _process_alive(pid):
            return "exited on SIGTERM"
        time.sleep(POLL_INTERVAL_S)

    _say("[teardown] SIGTERM did not take — escalating to SIGKILL")
    try:
        os.kill(pid, signal.SIGKILL)
    except ProcessLookupError:
        return "already gone"
    deadline = time.time() + 4.0
    while time.time() < deadline:
        if not _process_alive(pid):
            return "killed with SIGKILL"
        time.sleep(POLL_INTERVAL_S)
    return "STILL ALIVE after SIGKILL"


# ===========================================================================
# menu driving
# ===========================================================================


def _output_menu_items(pid: int, force_open: bool = False) -> List[str]:
    """Read the live item names of the Output menu.

    The fullscreen item's label is built at runtime from the display size, so
    it must be discovered rather than hardcoded (MenuBarModel.cpp:135-142).
    Separators come back as `missing value` and are skipped.

    Args:
        pid: The app's PID (targeted by unix id, so a same-named process
            cannot be driven by accident).
        force_open: Physically open the menu before reading it, then dismiss
            it with Escape. JUCE rebuilds its mac menus from
            getMenuForIndex() on `menuNeedsUpdate:`, and a plain AX read is
            not guaranteed to trigger that, so this is the fallback when a
            plain read comes back without the fullscreen item.
    """
    open_menu = (
        f"        click menu bar item {_as_applescript_string(OUTPUT_MENU)} of menu bar 1\n"
        "        delay 0.4\n"
        if force_open
        else ""
    )
    dismiss_menu = "    delay 0.2\n    key code 53\n" if force_open else ""

    script = f"""
tell application "System Events"
    set targetProc to first process whose unix id is {pid}
    set frontmost of targetProc to true
    delay 0.3
    set out to ""
    tell targetProc
{open_menu}        repeat with mi in (every menu item of menu 1 of menu bar item {_as_applescript_string(OUTPUT_MENU)} of menu bar 1)
            try
                set n to name of mi
                if n is not missing value then set out to out & n & linefeed
            end try
        end repeat
    end tell
{dismiss_menu}    return out
end tell
"""
    raw = _osascript(script)
    return [line.strip() for line in raw.splitlines() if line.strip()]


def _discover_main_fullscreen_item(pid: int) -> str:
    """Discover the 'Fullscreen: <W>x<H> (main)' item name from the live menu.

    Tries a plain AX read first, then a read with the menu physically opened
    (see `force_open`). Never hardcodes a resolution.

    Raises:
        ProbeBlocked: If neither read surfaces a main-display fullscreen item.
    """
    items = _output_menu_items(pid)
    _say(f"[open] Output menu items: {items}")
    try:
        return _pick_main_fullscreen_item(items)
    except ProbeBlocked as first_failure:
        _say(f"[open] {first_failure} — retrying with the menu opened")

    items = _output_menu_items(pid, force_open=True)
    _say(f"[open] Output menu items (menu opened): {items}")
    return _pick_main_fullscreen_item(items)


def _pick_main_fullscreen_item(items: List[str]) -> str:
    """Pick the 'Fullscreen: <W>x<H> (main)' item out of the live menu.

    Raises:
        ProbeBlocked: If no main-display fullscreen item is present.
    """
    for name in items:
        if name.startswith(FULLSCREEN_PREFIX) and name.endswith(MAIN_SUFFIX):
            return name
    raise ProbeBlocked(
        "No 'Fullscreen: <WxH> (main)' item in the Output menu. Items seen: "
        f"{items!r}"
    )


def _click_output_item(pid: int, item: str) -> None:
    """Click one item of the Output menu via System Events."""
    script = f"""
tell application "System Events"
    set targetProc to first process whose unix id is {pid}
    set frontmost of targetProc to true
    delay 0.3
    tell targetProc
        click menu item {_as_applescript_string(item)} of menu 1 of menu bar item {_as_applescript_string(OUTPUT_MENU)} of menu bar 1
    end tell
end tell
"""
    _osascript(script)


def _press_escape(pid: int) -> None:
    """Send Escape to the app (OutputWindow::keyPressed, OutputWindow.cpp:349-358
    hides the window — a weaker close than the menu, used only as a fallback)."""
    script = f"""
tell application "System Events"
    set targetProc to first process whose unix id is {pid}
    set frontmost of targetProc to true
    delay 0.3
    key code 53
end tell
"""
    _osascript(script)


def _open_output_window(pid: int) -> dict:
    """Open the fullscreen output window on the MAIN display and return it.

    Chose the Output MENU over Cmd+F on purpose (both work —
    MainComponent.cpp:2302-2315 handles Cmd+F): Cmd+F is swallowed when a text
    field holds focus (.harmony/gotchas.md 2026-08-02) and it is routed by
    MainComponent's key handler, so it depends on which window has keyboard
    focus. A menu click is independent of focus, and it also lets us target
    the MAIN display explicitly — which is precisely the case under test.

    The click has a documented ~15% drop rate, so it is retried, and success
    is confirmed by the window actually appearing rather than by osascript
    returning cleanly.

    Raises:
        ProbeBlocked: If the window never appears.
    """
    item = _discover_main_fullscreen_item(pid)
    _say(f"[open] using menu item {item!r}")

    last_click_error = None
    for attempt in range(1, OPEN_ATTEMPTS + 1):
        try:
            _click_output_item(pid, item)
        except ProbeBlocked as exc:
            # A dropped click can also leave the menu open, which the next
            # attempt's click dismisses — so retry rather than bailing out.
            last_click_error = exc
            _say(f"[open] click attempt {attempt} errored: {exc}")
        else:
            window = _wait_for_output_window(pid, WINDOW_APPEAR_TIMEOUT_S)
            if window is not None:
                _say(f"[open] output window on screen (attempt {attempt})")
                return window
            _say(f"[open] no output window after attempt {attempt} — retrying")

    raise ProbeBlocked(
        f"Output window never appeared after {OPEN_ATTEMPTS} menu clicks of "
        f"{item!r}"
        + (f" (last click error: {last_click_error})" if last_click_error else "")
        + f". On-screen windows owned by pid {pid}:\n"
        + _describe_pid_windows(pid)
    )


# ===========================================================================
# teardown
# ===========================================================================


def _close_output_window(pid: int) -> str:
    """Close the output window, escalating until the screen is clean.

    Ladder (each rung verified against CGWindowList, never assumed):
        1. Output menu > "Disabled"  (MenuBarModel.cpp:127 -> closeOutput()
           -> MainComponent.cpp:2523-2530, destroys the window)
        2. Escape key                (OutputWindow.cpp:349-358, hides it)
        3. SIGTERM / SIGKILL the app (process death removes the window)

    Returns:
        A description of how it ended, for the final state report.
    """
    if not _process_alive(pid):
        return "app already gone — no window can remain"
    if not _find_output_windows(pid):
        return "no output window was open"

    for attempt in range(1, CLOSE_ATTEMPTS + 1):
        try:
            _click_output_item(pid, DISABLED_ITEM)
        except ProbeBlocked as exc:
            _say(f"[teardown] menu close attempt {attempt} failed: {exc}")
        else:
            if _wait_for_output_window_gone(pid, WINDOW_GONE_TIMEOUT_S):
                return f'closed via Output > "{DISABLED_ITEM}" (attempt {attempt})'
            _say(f"[teardown] window still up after close attempt {attempt}")

    _say("[teardown] menu close failed — trying Escape")
    try:
        _press_escape(pid)
    except ProbeBlocked as exc:
        _say(f"[teardown] Escape failed: {exc}")
    else:
        if _wait_for_output_window_gone(pid, WINDOW_GONE_TIMEOUT_S):
            return "closed via Escape (window hidden, app state may be desynced)"

    _say("[teardown] UI close failed — terminating the app to clear the screen")
    result = _terminate_app(pid)
    if _wait_for_output_window_gone(pid, WINDOW_GONE_TIMEOUT_S):
        return f"closed by terminating the app ({result})"
    return f"WINDOW STILL ON SCREEN after terminating the app ({result})"


def _screenshot() -> Optional[str]:
    """Best-effort screenshot for a human/agent to confirm the screen is clean.

    `pgrep` returning empty does NOT mean the screen is clean
    (.harmony/gotchas.md 2026-08-03) — only looking at it does.
    """
    if os.environ.get("OW_LEVEL_SCREENSHOT", "1") != "1":
        return None
    path = f"/tmp/ow-level-teardown-{int(time.time())}.png"
    try:
        proc = subprocess.run(
            ["screencapture", "-x", path], capture_output=True, timeout=15
        )
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return None
    if proc.returncode != 0 or not os.path.exists(path):
        return None
    return path


def _teardown(
    known_pid: Optional[int],
    launch_attempted: bool,
    attached_pid: Optional[int],
    app_bundle: str,
    launch_time: Optional[float] = None,
) -> bool:
    """Clean up and report, driven by OBSERVED REALITY rather than belief.

    The first version of this probe asked "did I successfully observe a
    launch?" and, when the answer was no, printed "SCREEN STATE: clean (no
    window was ever opened)" and skipped cleanup — while the app it had just
    launched was alive and healthy. "I did not observe it launch" is not "it
    did not launch". So this function ignores what the run believed and
    re-derives the state from the process table and the window server:

      1. Rescan for live Audio-DNA processes (pgrep-first, see
         _find_app_pids). `known_pid` is only a hint, and is folded in.
      2. Quit every process ATTRIBUTABLE to this run (_is_ours: the observed
         pid, or a --test-mode instance started after this run's `open`).
         `launch_attempted` is set BEFORE `open` runs, so a launch whose
         readiness check failed — or that raised anywhere afterwards — still
         gets cleaned up.
      3. Close the output window of anything else only if it actually has one
         on screen, and never force-quit it for merely existing.
      4. Verify the screen POSITIVELY: no on-screen window titled
         "Audio-DNA Output", from ANY owner, plus a check that window titles
         are readable at all (an all-redacted snapshot cannot prove absence).

    Args:
        known_pid: The PID the run thinks it was driving, if any.
        launch_attempted: Whether `open` was invoked at any point.
        attached_pid: In attach mode, the pre-existing app to leave running.
        app_bundle: Bundle path, used to widen the process search.
        launch_time: Epoch seconds recorded immediately before `open`, used
            to attribute processes to this run.

    Returns:
        True only if the screen is POSITIVELY verified clean.
    """
    _say("")
    _say("=== TEARDOWN ===")

    live = set(_find_app_pids(app_bundle))
    if known_pid is not None and _process_alive(known_pid):
        live.add(known_pid)
    _say(f"[teardown] live {APP_NAME} processes: {sorted(live) or 'none'}")
    if launch_attempted and not live:
        _say("[teardown] a launch was attempted but no process is alive "
             "(it exited on its own, or never started)")

    # Only processes attributable to this run get quit (see _is_ours); an
    # attached app never does. An app this run never drove at all (e.g.
    # preflight refused to spawn because one was already running) is not
    # touched — not even to close its window.
    drove_something = launch_attempted or attached_pid is not None
    ours = {
        p
        for p in live
        if launch_attempted
        and p != attached_pid
        and _is_ours(p, launch_time, known_pid)
    }
    unattributed = live - ours - ({attached_pid} if attached_pid is not None else set())

    # Anything with a black window on screen still gets asked to close it,
    # attributed or not — but the close ladder only escalates to killing a
    # process when the UI routes fail, so an unrelated app is not force-quit
    # merely for existing.
    with_windows = {w["pid"] for w in _find_output_windows_any()} if drove_something else set()

    to_close = sorted(
        ours
        | ({attached_pid} if attached_pid is not None else set())
        | (unattributed & with_windows)
    )
    if attached_pid is not None:
        _say(f"[teardown] attached to pid {attached_pid} — leaving it running")
    if unattributed:
        _say(f"[teardown] not attributable to this run, will NOT be quit: "
             f"{sorted(unattributed)}"
             + (f" (asking {sorted(unattributed & with_windows)} to close an "
                f"on-screen output window)" if unattributed & with_windows else ""))
    if not drove_something:
        _say("[teardown] this run drove nothing — leaving every process and "
             "window untouched")

    for pid in to_close:
        close_state = _close_output_window(pid)
        _say(f"[teardown] pid {pid}: {close_state}")

    for pid in sorted(ours):
        _say(f"[teardown] pid {pid}: quitting (this run launched it) -> "
             f"{_terminate_app(pid)}")

    # --- positive verification, after everything above -------------------
    still_live = _find_app_pids(app_bundle)
    stray_windows = _find_output_windows_any()
    names_readable = _window_names_readable()
    # Any output window still on screen counts against us if this run drove
    # anything — including one owned by an unexpected PID, since a window we
    # opened could outlive the pid we knew about. If the run drove nothing,
    # a pre-existing window is reported but is not this run's verdict to make.
    window_clear = (not stray_windows and names_readable) or not drove_something
    shot = _screenshot()

    _say("")
    _say("=== FINAL STATE ===")
    _say(f"  output windows on screen : {len(stray_windows)}"
         f"{' (titles UNREADABLE — cannot prove absence)' if not names_readable else ''}"
         f"{' — pre-existing, not opened by this run' if stray_windows and not drove_something else ''}")
    _say(f"  live {APP_NAME} processes  : {still_live or 'none'}")
    if attached_pid is not None and attached_pid in still_live:
        _say(f"  attached app             : left running as intended "
             f"(pid {attached_pid})")
    for w in stray_windows:
        b = w["bounds"] or {}
        _say(f"    stray: #{w['number']} pid={w['pid']} layer={w['layer']} "
             f"{b.get('Width')}x{b.get('Height')}")
    orphans = [p for p in still_live if p in ours]
    for pid in orphans:
        _say(f"  !!! pid {pid} was launched by this run and is STILL ALIVE")
    if shot:
        _say(f"  screenshot               : {shot}  <- READ IT to confirm")
    else:
        _say("  screenshot               : not taken")

    orphan = bool(orphans)
    if not window_clear or orphan:
        _say("")
        if not window_clear:
            _say("!!! SCREEN NOT VERIFIED CLEAN — an output window may still be up.")
        if orphan:
            _say("!!! ORPHAN PROCESS — this run launched an app it could not quit.")
        _say("!!! Manual remedy: pkill -9 -f Audio-DNA")
        return False

    if not drove_something:
        _say("  SCREEN STATE: untouched by this run (nothing was launched or "
             "driven)")
    else:
        _say("  SCREEN STATE: clean (verified: no 'Audio-DNA Output' window on "
             "screen, titles readable)")
    return True


def _arm_watchdog(state: dict, deadline_s: float) -> threading.Event:
    """Arm a daemon thread that force-kills the app if the run wedges.

    This is the backstop for teardown itself hanging (a blocked osascript, a
    frozen app). A wedged run that strands a fullscreen black window is worse
    than any test outcome, so this rung does not try to be graceful.

    Like teardown, it rediscovers the app from the process table rather than
    trusting the pid the run believes it has — the pid is exactly the thing
    that can be missing when a run goes wrong. It kills only what this run
    launched: an attached app is never force-quit by the watchdog.

    Args:
        state: Live dict owned by main(), read at fire time. Keys: "pid",
            "launch_attempted", "attached_pid".
        deadline_s: Seconds before firing.

    Returns:
        An Event to set when the run finishes normally.
    """
    done = threading.Event()

    def _watch():
        if done.wait(deadline_s):
            return

        attached = state.get("attached_pid")
        known = state.get("pid")
        launch_time = state.get("launch_time")
        targets = set()
        if state.get("launch_attempted"):
            # pgrep only: no ObjC/run-loop calls from this thread. Same
            # attribution rule as teardown — never kill an instance this run
            # did not launch.
            for pid in set(_pgrep(["-x", APP_NAME])) | (
                {known} if known is not None else set()
            ):
                if pid != attached and _is_ours(pid, launch_time, known):
                    targets.add(pid)

        print(
            f"\n!!! WATCHDOG: run exceeded {deadline_s:.0f}s — force-killing "
            f"what this run launched to clear the screen: "
            f"{sorted(targets) or 'nothing attributable to this run'}.",
            file=sys.stderr,
            flush=True,
        )
        for pid in sorted(targets):
            for sig in (signal.SIGTERM, signal.SIGKILL):
                try:
                    os.kill(pid, sig)
                except OSError:
                    break
                time.sleep(2.0)
                if not _process_alive(pid):
                    break
        print(
            "!!! WATCHDOG: done. Verify the screen visually.",
            file=sys.stderr,
            flush=True,
        )
        os._exit(EXIT_WATCHDOG)

    threading.Thread(target=_watch, daemon=True).start()
    return done


# ===========================================================================
# pre-flight checks — run BEFORE anything is put on the screen
# ===========================================================================


def _preflight(app_bundle: str, no_spawn: bool) -> Optional[int]:
    """Verify the probe can actually measure before it opens a black window.

    Returns:
        The PID of an already-running app in attach mode, else None.

    Raises:
        ProbeBlocked: If any precondition fails.
    """
    # Screen Recording governs kCGWindowName for other processes' windows;
    # without it the name comes back empty and the window cannot be
    # identified. Checking FIRST means a machine that cannot measure never
    # gets a black window put on it. Preflight does not prompt.
    if not Quartz.CGPreflightScreenCaptureAccess():
        if os.environ.get("OW_LEVEL_IGNORE_SCREEN_PERM") != "1":
            raise ProbeBlocked(
                "Screen Recording permission is NOT granted to this process.\n"
                "  macOS redacts kCGWindowName for other apps' windows without "
                "it, so the output window could not be identified by title — "
                "and this probe will not fall back to guessing which window it "
                "is.\n"
                "  Grant it to the terminal running this probe in System "
                "Settings > Privacy & Security > Screen Recording, restart the "
                "terminal, and re-run.\n"
                "  Refusing to launch the app: opening an unmeasurable "
                "fullscreen black window would be pure downside.\n"
                "  (Override the pre-check only with "
                "OW_LEVEL_IGNORE_SCREEN_PERM=1 — the title match still has to "
                "succeed.)"
            )
        _say("[preflight] WARNING: Screen Recording preflight says NO but "
             "OW_LEVEL_IGNORE_SCREEN_PERM=1 — continuing.")
    else:
        _say("[preflight] Screen Recording: granted")

    if not check_accessibility_permissions():
        # A warning, not a blocker: TCC attributes osascript/System Events to
        # the responsible parent process, so this python-level check can read
        # False while the menu drive still works. A real failure surfaces as a
        # clear osascript error.
        _say("[preflight] WARNING: this process is not AX-trusted; the menu "
             "drive may fail with 'not allowed assistive access'.")
    else:
        _say("[preflight] Accessibility: granted")

    # pgrep-first (see _find_app_pids), not NSWorkspace: this check is what
    # licenses teardown to treat every live instance as ours in spawn mode, so
    # it has to read the real process table.
    running = _find_app_pids(app_bundle)

    if no_spawn:
        if not running:
            raise ProbeBlocked(
                f"AUDIODNA_NO_SPAWN=1 but no running {APP_NAME} was found."
            )
        _say(f"[preflight] attaching to running {APP_NAME} (pid {running[0]})"
             + (f", ignoring {running[1:]}" if len(running) > 1 else ""))
        return running[0]

    if running:
        raise ProbeBlocked(
            f"{APP_NAME} is already running (pids {running}). `open` would "
            f"just activate it, so --test-mode would not take and the probe "
            f"would measure an unknown instance. Quit it first, or re-run "
            f"with AUDIODNA_NO_SPAWN=1 to attach to it."
        )

    if not os.path.isdir(app_bundle):
        raise ProbeBlocked(
            f"App bundle not found: {app_bundle}\n"
            "  Build it, or point AUDIODNA_APP at the bundle."
        )
    _say(f"[preflight] app bundle: {app_bundle}")
    return None


# ===========================================================================
# the measurement
# ===========================================================================


def _assert_normal_level(window: dict) -> None:
    """Assert the output window sits at NSNormalWindowLevel.

    Raises:
        LevelMismatch: With the observed value spelled out.
    """
    layer = window["layer"]
    b = window["bounds"] or {}
    dw, dh = _main_display_bounds()

    _say("")
    _say("=== MEASUREMENT ===")
    _say(f"  window        : {window['name']!r} (#{window['number']})")
    _say(f"  owner         : {window['owner']!r} pid {window['pid']}")
    _say(
        f"  bounds        : x={b.get('X')} y={b.get('Y')} "
        f"{b.get('Width')}x{b.get('Height')}  (main display {dw:.0f}x{dh:.0f})"
    )
    _say(f"  kCGWindowLayer: {layer}")

    if layer != EXPECTED_LAYER:
        hint = ""
        if layer == FLOATING_LAYER:
            hint = (
                " — this is NSFloatingWindowLevel, i.e. src/ui/OutputWindow.cpp"
                ":337 setAlwaysOnTop(true) is still in goFullscreenOnDisplay. "
                "A non-normal level with no Spaces-participation bit makes "
                "macOS treat the window as Transient, so it floats onto EVERY "
                "desktop Space as an opaque black rectangle."
            )
        raise LevelMismatch(
            f"expected layer {EXPECTED_LAYER} (NSNormalWindowLevel), got "
            f"{layer}{hint}"
        )


def _measure(pid: int) -> None:
    """Open the output window on the main display and assert its level."""
    window = _open_output_window(pid)

    # Re-read rather than trusting the snapshot taken while it was appearing:
    # the level is set in goFullscreenOnDisplay right after setVisible(true),
    # so a snapshot caught mid-open could read a stale value.
    time.sleep(0.5)
    matches = _find_output_windows(pid)
    if not matches:
        raise ProbeBlocked(
            "The output window vanished between opening and measuring. "
            f"On-screen windows owned by pid {pid}:\n{_describe_pid_windows(pid)}"
        )
    if len(matches) > 1:
        _say(
            f"[measure] WARNING: {len(matches)} windows titled "
            f"{OUTPUT_WINDOW_NAME!r} owned by pid {pid} — measuring the first."
        )
    _assert_normal_level(matches[0])


# ===========================================================================
# entry point
# ===========================================================================


def main() -> int:
    """Run the probe. See the module docstring for exit codes."""
    app_bundle = os.environ.get("AUDIODNA_APP", _default_app_bundle())
    no_spawn = os.environ.get("AUDIODNA_NO_SPAWN", "0") == "1"

    outcome = EXIT_BLOCKED
    verdict = "BLOCKED"
    detail = ""

    # Single source of truth for the lifecycle, shared with the watchdog.
    # `launch_attempted` is the load-bearing field: it is set BEFORE `open`
    # runs and is never cleared, so every path after that point — including
    # one where the app is never successfully observed — still cleans up.
    state = {
        "pid": None,            # best known pid, a hint only
        "launch_attempted": False,
        "attached_pid": None,   # attach mode: never quit this one
        "launch_time": None,    # epoch seconds just before `open`
    }

    done = _arm_watchdog(
        state, LAUNCH_READY_TIMEOUT_S + RUN_TIMEOUT_S + TEARDOWN_GRACE_S
    )

    def _on_alarm(_signum, _frame):
        raise RunTimeout(f"measurement exceeded {RUN_TIMEOUT_S:.0f}s")

    try:
        try:
            state["attached_pid"] = _preflight(app_bundle, no_spawn)
            state["pid"] = state["attached_pid"]

            if state["pid"] is None:
                # Set the flag FIRST: `open` returns as soon as LaunchServices
                # accepts the request, so from here on an app may exist no
                # matter what happens next.
                state["launch_attempted"] = True
                state["launch_time"] = time.time()
                stderr_log = _spawn_app(app_bundle)
                state["pid"] = _wait_for_ready(app_bundle, stderr_log)

            # Arm the hard cap only now: everything above happens with nothing
            # on the screen, and the 60s cap is meant to bound the phase in
            # which a black window can be up.
            signal.signal(signal.SIGALRM, _on_alarm)
            signal.setitimer(signal.ITIMER_REAL, RUN_TIMEOUT_S)

            _measure(state["pid"])
            outcome, verdict = EXIT_PASS, "PASS"
            detail = f"output window is at layer {EXPECTED_LAYER} (NSNormalWindowLevel)"

        except LevelMismatch as exc:
            outcome, verdict, detail = EXIT_FAIL, "FAIL", str(exc)
        except (ProbeBlocked, RunTimeout) as exc:
            outcome, verdict, detail = EXIT_BLOCKED, "BLOCKED", str(exc)
        except KeyboardInterrupt:
            outcome, verdict, detail = EXIT_BLOCKED, "BLOCKED", "interrupted (Ctrl-C)"
        except Exception as exc:  # never skip teardown on an unexpected error
            outcome = EXIT_BLOCKED
            verdict = "BLOCKED"
            detail = f"unexpected {type(exc).__name__}: {exc}"
    finally:
        # Stop the measurement alarm so it cannot fire during teardown; the
        # watchdog thread covers teardown from here.
        signal.setitimer(signal.ITIMER_REAL, 0)
        screen_clean = _teardown(
            state["pid"],
            state["launch_attempted"],
            state["attached_pid"],
            app_bundle,
            state["launch_time"],
        )
        done.set()

    _say("")
    _say("=== RESULT ===")
    _say(f"  {verdict}: {detail}")

    if not screen_clean and outcome != EXIT_BLOCKED:
        _say(
            "  (exit forced to BLOCKED: the screen could not be verified "
            f"clean, which outranks the {verdict} verdict)"
        )
        outcome = EXIT_BLOCKED
    return outcome


if __name__ == "__main__":
    sys.exit(main())
