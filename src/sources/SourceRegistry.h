#pragma once
#include "sources/ProceduralSource.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

// SourceRegistry: manages all available procedural source types and active instances.
//
// Factory registration: each source type registers a creation function.
// Active instances: sources that are currently in use by clips.
class SourceRegistry
{
public:
    SourceRegistry();

    using SourceFactory = std::function<std::unique_ptr<ProceduralSource>()>;

    // Register a source factory by ID
    void registerSource(const std::string& id, SourceFactory factory);

    // Register all built-in sources
    void registerDefaults();

    // Create a new source instance by ID
    std::unique_ptr<ProceduralSource> createSource(const std::string& id) const;

    // Get list of all registered source IDs
    std::vector<std::string> getRegisteredIds() const;

    // Get display name for a registered source
    std::string getDisplayName(const std::string& id) const;

    // Get category for a registered source
    std::string getCategory(const std::string& id) const;

    // Check if a source ID is registered
    bool isRegistered(const std::string& id) const;

private:
    struct SourceInfo
    {
        SourceFactory factory;
        std::string displayName;
        std::string category;
    };

    std::unordered_map<std::string, SourceInfo> registry_;
};
