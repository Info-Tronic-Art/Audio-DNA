#!/usr/bin/env python3
"""patch_juce.py <juce-src> -- DIAG-VFPS TEMPORARY hooks in a PRIVATE JUCE copy (never the build/_deps copy).
Hook: juce::DiagVfpsHook::fn(kind, a, b, c, d), null unless the app installs it. Times are raw mach_absolute_time ticks.
 kind 1 display-link tick: a = now, b = inNow->hostTime, c = outputTime->hostTime, d = display id
 kind 2 swapBuffers: a = t before flushBuffer, b = t after flushBuffer, c = frameTime (JUCE int ms), d = underrun*1e6 + sleepMs*1e3 + minSwapTimeMs
 kind 3 render thread waitForWork: a = t start, b = t end, c = requestRender
 kind 4 display-link lambda for a CachedImage: a = now, b = matched (display == lastDisplay)
"""
import sys
J = sys.argv[1] + "/modules"

def patch(path, old, new):
    p = J + "/" + path
    s = open(p, newline="").read()
    if "\r\n" in s:   # JUCE sources are CRLF: keep them CRLF (a clean diff)
        old, new = old.replace("\n", "\r\n"), new.replace("\n", "\r\n")
    n = s.count(old)
    assert n == 1, (path, n, old[:60])
    open(p, "w", newline="").write(s.replace(old, new))
    print("patched", path)

patch("juce_events/messages/juce_MessageManager.h", "\nclass MessageManagerLock;\n",
      "\nclass MessageManagerLock;\n\n// DIAG-VFPS TEMPORARY (private JUCE copy, lane diag-vfps): timing hook, null unless the app installs it.\n"
      "struct JUCE_API DiagVfpsHook\n{\n    using Fn = void (*) (int kind, double a, double b, double c, double d);\n    static Fn fn;\n};\n")
patch("juce_events/messages/juce_MessageManager.cpp", "MessageManager::MessageManager() noexcept\n",
      "DiagVfpsHook::Fn DiagVfpsHook::fn = nullptr;   // DIAG-VFPS TEMPORARY\n\nMessageManager::MessageManager() noexcept\n")
patch("juce_gui_basics/native/juce_PerScreenDisplayLinks_mac.h",
      "            const auto outputTimeSec = (double) outputTime->videoTime / (double) outputTime->videoTimeScale;\n",
      "            const auto outputTimeSec = (double) outputTime->videoTime / (double) outputTime->videoTimeScale;\n"
      "            if (auto* diagFn = ::juce::DiagVfpsHook::fn)   // DIAG-VFPS TEMPORARY\n"
      "                diagFn (1, (double) mach_absolute_time(), (double) inNow->hostTime, (double) outputTime->hostTime,\n"
      "                        (double) static_cast<const ScopedDisplayLink*> (context)->displayId);\n")
patch("juce_gui_basics/native/juce_PerScreenDisplayLinks_mac.h",
      "        const auto callback = [] (CVDisplayLinkRef,\n                                  const CVTimeStamp*,\n",
      "        const auto callback = [] (CVDisplayLinkRef,\n                                  const CVTimeStamp* inNow,\n")
patch("juce_opengl/opengl/juce_OpenGLContext.cpp",
      "                    if (display == lastDisplay)\n                        triggerRepaint();\n",
      "                    if (auto* diagFn = ::juce::DiagVfpsHook::fn)   // DIAG-VFPS TEMPORARY\n"
      "                        diagFn (4, (double) mach_absolute_time(), display == lastDisplay ? 1.0 : 0.0, 0.0, 0.0);\n"
      "                    if (display == lastDisplay)\n                        triggerRepaint();\n")
patch("juce_opengl/opengl/juce_OpenGLContext.cpp",
      "                std::unique_lock lock { mutex };\n                flags |= (requestRender ? renderRequested : 0);\n                condvar.wait (lock, [this] { return flags > listSafe; });\n",
      "                std::unique_lock lock { mutex };\n                flags |= (requestRender ? renderRequested : 0);\n"
      "               #if JUCE_MAC\n                const double diagT0 = (double) mach_absolute_time();   // DIAG-VFPS TEMPORARY\n               #endif\n"
      "                condvar.wait (lock, [this] { return flags > listSafe; });\n"
      "               #if JUCE_MAC\n                if (auto* diagFn = ::juce::DiagVfpsHook::fn)\n"
      "                    diagFn (3, diagT0, (double) mach_absolute_time(), requestRender ? 1.0 : 0.0, 0.0);\n               #endif\n")
patch("juce_opengl/native/juce_OpenGL_mac.h",
      "    void swapBuffers()\n    {\n        auto now = Time::getMillisecondCounterHiRes();\n        [renderContext flushBuffer];\n",
      "    void swapBuffers()\n    {\n        auto now = Time::getMillisecondCounterHiRes();\n"
      "        const double diagT0 = (double) mach_absolute_time();   // DIAG-VFPS TEMPORARY\n"
      "        [renderContext flushBuffer];\n        const double diagT1 = (double) mach_absolute_time();\n        double diagSleep = 0.0;\n")
patch("juce_opengl/native/juce_OpenGL_mac.h",
      "                    Thread::sleep (2 * (minSwapTime - frameTime));\n",
      "                    Thread::sleep (2 * (minSwapTime - frameTime));\n                    diagSleep = 2.0 * (minSwapTime - frameTime);   // DIAG-VFPS TEMPORARY\n")
patch("juce_opengl/native/juce_OpenGL_mac.h",
      "        lastSwapTime = now;\n    }\n",
      "        if (auto* diagFn = ::juce::DiagVfpsHook::fn)   // DIAG-VFPS TEMPORARY\n"
      "            diagFn (2, diagT0, diagT1, (double) std::min ((uint64_t) std::numeric_limits<int>::max(), (uint64_t) now - (uint64_t) lastSwapTime),\n"
      "                    underrunCounter * 1.0e6 + diagSleep * 1.0e3 + (double) minSwapTimeMs.get());\n"
      "        lastSwapTime = now;\n    }\n")
