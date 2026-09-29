#pragma once
// s-rta-0929 vupload P2 (plan-vupload.md R-13 + adoption VU11): JUCE runs every OpenGLContext of the process on ONE render
// thread (juce_OpenGLContext.cpp: a plain std::thread, QoS DEFAULT: 21 in 218 / 218 diag-vfps launches). Raising it to
// USER_INTERACTIVE halves the lost display-link ticks and the wake-up latency. Called on the render thread from EVERY
// context's newOpenGLContextCreated (the preview's Renderer and each Output window's presenter): idempotent and cheap,
// and JUCE may re-create its thread with a context. Returns the thread's QoS class after the call (-1 off macOS).
#if defined(__APPLE__)
 #include <pthread.h>
 #include <sys/qos.h>
#endif

inline int raiseRenderThreadQos()
{
#if defined(__APPLE__)
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    return static_cast<int>(qos_class_self());
#else
    return -1;
#endif
}
