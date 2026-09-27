# Quartz window-list sampler: every 1 s, count Audio-DNA windows named like Output + on-screen layer-0 windows.
import sys, time, Quartz
out = open(sys.argv[1], "a"); n = 0; outnamed = set(); maxl0 = 0
while True:
    wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
    a = [w for w in wl if str(w.get('kCGWindowOwnerName', '')) == 'Audio-DNA']
    o = [str(w.get('kCGWindowName', '')) for w in a if 'Output' in str(w.get('kCGWindowName', ''))]
    l0 = len([w for w in a if w.get('kCGWindowIsOnscreen') and w.get('kCGWindowLayer') == 0])
    n += 1; outnamed.update(o); maxl0 = max(maxl0, l0)
    out.seek(0); out.truncate(); out.write(f"{time.strftime('%T')} samples {n} Output-named Audio-DNA windows {sorted(outnamed)} max on-screen layer-0 Audio-DNA windows {maxl0}\n"); out.flush()
    time.sleep(1)
