#pragma once

#include <string>
#include <cstdint>
#include <atomic>

// NdiOutput: Network video output via NDI protocol.
//
// NDI (Network Device Interface) enables sending video over the local network
// to any NDI-compatible receiver. Unlike Syphon, NDI requires CPU-side pixel
// data (no zero-copy GPU sharing).
//
// The NDI SDK must be installed separately (https://ndi.video/download-ndi-sdk/).
// This class compiles as a stub when the SDK is not available.
//
// When AUDIODNA_HAS_NDI is defined, the implementation uses:
//   - NDIlib_send_create() to create a sender
//   - NDIlib_send_send_video_v2() to send RGBA frames
//
// Threading: sendFrame() is called from a dedicated thread, not the GL thread.
// The GL thread does glReadPixels into a shared buffer, and the NDI thread
// picks it up (same triple-buffer pattern as VideoRecorder).
class NdiOutput
{
public:
    NdiOutput() = default;
    ~NdiOutput() { shutdown(); }

    // Initialize the NDI sender with a source name.
    bool init(const std::string& sourceName)
    {
        sourceName_ = sourceName;
#if AUDIODNA_HAS_NDI
        // TODO: Initialize NDI SDK when available
        return false;
#else
        return false;
#endif
    }

    // Send an RGBA frame. Called from the encoder/NDI thread, not the GL thread.
    void sendFrame(const uint8_t* rgba, int width, int height)
    {
        (void)rgba; (void)width; (void)height;
#if AUDIODNA_HAS_NDI
        // TODO: NDIlib_send_send_video_v2()
#endif
    }

    // Clean up.
    void shutdown()
    {
#if AUDIODNA_HAS_NDI
        // TODO: NDIlib_send_destroy()
#endif
    }

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    bool isInitialized() const { return false; }

private:
    std::string sourceName_ = "Audio-DNA";
    bool enabled_ = false;
};
