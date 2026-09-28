"""Window-only capture of the Audio-DNA main window by Quartz window id (never the full screen). argv: out.png"""
import sys, Quartz
from Quartz import CGWindowListCopyWindowInfo, kCGWindowListOptionAll, kCGNullWindowID, CGWindowListCreateImage, CGRectNull
from Quartz import kCGWindowListOptionIncludingWindow, kCGWindowImageBoundsIgnoreFraming
import CoreFoundation
out = sys.argv[1]
wins = [w for w in CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID)
        if w.get("kCGWindowOwnerName") == "Audio-DNA" and w.get("kCGWindowLayer") == 0]
for w in wins:
    b = w["kCGWindowBounds"]; print("window", w["kCGWindowNumber"], repr(w.get("kCGWindowName")), int(b["Width"]), "x", int(b["Height"]), "onscreen", w.get("kCGWindowIsOnscreen"))
if not wins: print("NO WINDOW"); sys.exit(2)
w = max(wins, key=lambda w: w["kCGWindowBounds"]["Width"] * w["kCGWindowBounds"]["Height"])
img = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, w["kCGWindowNumber"], kCGWindowImageBoundsIgnoreFraming)
if img is None: print("capture failed"); sys.exit(3)
url = CoreFoundation.CFURLCreateWithFileSystemPath(None, out, CoreFoundation.kCFURLPOSIXPathStyle, False)
dest = Quartz.CGImageDestinationCreateWithURL(url, "public.png", 1, None)
Quartz.CGImageDestinationAddImage(dest, img, None); Quartz.CGImageDestinationFinalize(dest)
print("captured window", w["kCGWindowNumber"], Quartz.CGImageGetWidth(img), "x", Quartz.CGImageGetHeight(img), "->", out)
