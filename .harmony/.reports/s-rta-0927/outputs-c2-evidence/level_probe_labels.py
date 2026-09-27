"""RED/GREEN for the level-probe constants update, WITHOUT opening a window and without any input:
the level probe's own discovery functions are IMPORTED (never run -- every function that could launch the app,
drive AppleScript or open the window is replaced by a raise first) and fed the Output menu titles of the NEW code.
  argv[1] = a file with the titles, one per line (from tool_uitoggle_snapshot's real-display block or from the
            live app's /api/state.outputs.displays[].label + "All Outputs Off")
  OLD = main f4507e8's tests/visual/test_output_window_level.py (scratch export), NEW = the lane's."""
import importlib.util, os, sys
sys.dont_write_bytecode = True
SP = "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/outputs2"
WT = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5"
items = [l.strip() for l in open(sys.argv[1]) if l.strip()]
print(f"Output menu titles fed to the level probe ({sys.argv[2] if len(sys.argv) > 2 else sys.argv[1]}): {items}")

def load(tag, path):
    sys.path.insert(0, os.path.dirname(path))   # ax_inspector sits next to it
    spec = importlib.util.spec_from_file_location("level_" + tag, path)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    sys.path.pop(0)
    def never(*_a, **_k):
        raise RuntimeError("never run: this check only calls the pure item picker")
    for fn in ("main", "_spawn_app", "_click_output_item", "_open_output_window", "_osascript", "_press_escape",
               "_measure", "_close_output_window", "_teardown", "_terminate_app"):
        setattr(m, fn, never)
    return m

for tag, path in (("OLD main f4507e8", SP + "/basetree/tests/visual/test_output_window_level.py"),
                  ("NEW lane", WT + "/tests/visual/test_output_window_level.py")):
    m = load(tag.split()[0].lower(), path)
    print(f"== {tag}: DISABLED_ITEM={m.DISABLED_ITEM!r} FULLSCREEN_PREFIX={m.FULLSCREEN_PREFIX!r} MAIN_SUFFIX={m.MAIN_SUFFIX!r}")
    try:
        print(f"   open item  -> PICKS {m._pick_main_fullscreen_item(items)!r}")
    except m.ProbeBlocked as e:
        print(f"   open item  -> REFUSES (ProbeBlocked -> the probe exits BLOCKED before opening anything): {e}")
    print(f"   close item -> {'PRESENT' if m.DISABLED_ITEM in items else 'ABSENT (teardown rung 1 would fail)'}: {m.DISABLED_ITEM!r}")
