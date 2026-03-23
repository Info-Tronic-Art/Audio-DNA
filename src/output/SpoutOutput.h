#pragma once

#include <string>
#include <atomic>

// SpoutOutput: Inter-app GPU texture sharing on Windows.
//
// Currently a stub — Windows implementation requires the Spout2 SDK
// (https://github.com/leadedge/Spout2) which uses DirectX shared textures.
//
// On macOS, this class is a no-op.
class SpoutOutput
{
public:
    SpoutOutput() = default;
    ~SpoutOutput() = default;

    void init() {}
    void publishTexture(unsigned int texId, int width, int height)
    {
        (void)texId; (void)width; (void)height;
    }
    void setServerName(const std::string& name) { name_ = name; }
    void shutdown() {}

    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    bool isInitialized() const { return false; }

private:
    std::string name_ = "Audio-DNA";
    bool enabled_ = false;
};
