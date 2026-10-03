import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
u = [w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]
print('UserNotificationCenter windows %d %s' % (len(u), [(w.get('kCGWindowNumber'), w.get('kCGWindowIsOnscreen'), w.get('kCGWindowLayer')) for w in u][:6]))
