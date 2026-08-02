#pragma once

#include <string>
#include <atomic>

#if __APPLE__

// SyphonOutput: Publishes the rendered composition as a Syphon server.
//
// Syphon enables zero-copy GPU texture sharing between applications on macOS.
// Other apps (MadMapper, VDMX, OBS) can receive the texture and display it.
//
// Usage:
//   1. Call init() with the NSOpenGLContext from JUCE's OpenGL context.
//   2. Call publishTexture() at the end of each renderOpenGL() with the
//      final composited texture ID.
//   3. Call shutdown() on context close.
//
// Thread safety: init/shutdown/publishTexture must be called from the GL
// thread — impl_ itself is only ever touched there, exactly as documented
// below. setEnabled()/isEnabled() and isInitialized() are backed by atomics
// so they may be read from any thread (message-thread menu toggle, HTTP
// thread REST status/toggle endpoints) without racing the GL thread.
class SyphonOutput
{
public:
    SyphonOutput();
    ~SyphonOutput();

    // Initialize with the GL context. Must be called from GL thread.
    // nsOpenGLContext should be the NSOpenGLContext* from JUCE's OpenGL context.
    void init(void* nsOpenGLContext);

    // Publish a GL texture to connected Syphon clients.
    // Must be called from the GL thread.
    void publishTexture(unsigned int texId, int width, int height);

    // Set the Syphon server name (visible to clients).
    void setServerName(const std::string& name);

    // Clean up. Must be called from GL thread before context is destroyed.
    void shutdown();

    // Enable/disable publishing without destroying the server. Safe to call
    // from any thread (message thread menu toggle, HTTP thread REST endpoint).
    void setEnabled(bool enabled) { enabled_.store(enabled, std::memory_order_relaxed); }
    bool isEnabled() const { return enabled_.load(std::memory_order_relaxed); }

    // Safe to call from any thread — see class comment. Mirrors
    // impl_ != nullptr; kept as a separate atomic rather than making impl_
    // itself atomic so impl_ stays exactly what it always was: a raw pointer
    // touched only on the GL thread.
    bool isInitialized() const { return initialized_.load(std::memory_order_relaxed); }

    SyphonOutput(const SyphonOutput&) = delete;
    SyphonOutput& operator=(const SyphonOutput&) = delete;

private:
    void* impl_ = nullptr;  // Opaque pointer to Obj-C implementation. GL-thread-only.
    std::atomic<bool> initialized_{false};  // Cross-thread-readable mirror of (impl_ != nullptr).
    std::atomic<bool> enabled_{false};
    std::string serverName_ = "Audio-DNA";
};

#else

// Stub for non-macOS platforms
class SyphonOutput
{
public:
    void init(void*) {}
    void publishTexture(unsigned int, int, int) {}
    void setServerName(const std::string&) {}
    void shutdown() {}
    void setEnabled(bool) {}
    bool isEnabled() const { return false; }
    bool isInitialized() const { return false; }
};

#endif // __APPLE__
