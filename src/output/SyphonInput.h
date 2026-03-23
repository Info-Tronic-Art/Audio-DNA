#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <utility>

#if __APPLE__

// SyphonInput: Receives textures from a Syphon server.
//
// Enables Audio-DNA to use another app's visual output as a clip source.
// Zero-copy GPU texture sharing on macOS via IOSurface.
//
// Usage:
//   1. Call init() with the NSOpenGLContext.
//   2. Call listServers() to get available Syphon servers.
//   3. Call connectToServer() with the desired app/server name.
//   4. Call getLatestTexture() each frame to get the GL texture ID.
//   5. Call disconnect() when done.
class SyphonInput
{
public:
    SyphonInput();
    ~SyphonInput();

    // Initialize with the GL context.
    void init(void* nsOpenGLContext);

    // Connect to a specific Syphon server.
    bool connectToServer(const std::string& appName, const std::string& serverName);

    // Disconnect from the current server.
    void disconnect();

    // Get the latest texture from the connected server.
    // Returns 0 if no frame available.
    unsigned int getLatestTexture(int& outWidth, int& outHeight);

    // Check if a new frame is available since the last call.
    bool hasNewFrame() const;

    // List available Syphon servers as (appName, serverName) pairs.
    std::vector<std::pair<std::string, std::string>> listServers();

    // Clean up.
    void shutdown();

    bool isConnected() const { return client_ != nullptr; }

    SyphonInput(const SyphonInput&) = delete;
    SyphonInput& operator=(const SyphonInput&) = delete;

private:
    void* context_ = nullptr;  // NSOpenGLContext*
    void* client_ = nullptr;   // SyphonClient* (opaque)
};

#else

// Stub for non-macOS platforms
class SyphonInput
{
public:
    void init(void*) {}
    bool connectToServer(const std::string&, const std::string&) { return false; }
    void disconnect() {}
    unsigned int getLatestTexture(int&, int&) { return 0; }
    bool hasNewFrame() const { return false; }
    std::vector<std::pair<std::string, std::string>> listServers() { return {}; }
    void shutdown() {}
    bool isConnected() const { return false; }
};

#endif // __APPLE__
