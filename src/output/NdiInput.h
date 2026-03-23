#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <atomic>

// NdiInput: Receive video from NDI sources on the network.
//
// When AUDIODNA_HAS_NDI is defined, uses the NDI SDK to:
//   - NDIlib_find_create() to discover NDI sources
//   - NDIlib_recv_create() to receive RGBA frames
//
// A background thread receives frames and stores them in a CPU buffer.
// The GL thread uploads via glTexSubImage2D (same pattern as VideoPlayer).
//
// Currently a stub — requires the NDI SDK to be installed.
class NdiInput
{
public:
    NdiInput() = default;
    ~NdiInput() { disconnect(); }

    // Discover NDI sources on the network.
    std::vector<std::string> listSources()
    {
#if AUDIODNA_HAS_NDI
        // TODO: NDIlib_find_get_current_sources()
#endif
        return {};
    }

    // Connect to an NDI source by name.
    bool connectToSource(const std::string& sourceName)
    {
        (void)sourceName;
#if AUDIODNA_HAS_NDI
        // TODO: NDIlib_recv_create()
#endif
        return false;
    }

    // Disconnect from the current source.
    void disconnect()
    {
#if AUDIODNA_HAS_NDI
        // TODO: NDIlib_recv_destroy()
#endif
    }

    // Check if a new frame is available.
    bool hasNewFrame() const { return false; }

    // Get the latest frame data. Returns nullptr if no frame available.
    // Caller must NOT free the returned pointer.
    const uint8_t* getLatestFrame(int& outWidth, int& outHeight) const
    {
        outWidth = 0;
        outHeight = 0;
        return nullptr;
    }

    bool isConnected() const { return false; }
};
