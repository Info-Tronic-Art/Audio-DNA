#pragma once
#include "routing/Route.h"
#include "signal/SignalRegistry.h"
#include "features/Smoother.h"
#include <vector>
#include <functional>

// Forward declaration
struct Composition;

// RoutingEngine: processes all routes each render frame.
// Replaces MappingEngine for v2. Called by the render thread.
// No heap allocation in processFrame().
class RoutingEngine
{
public:
    RoutingEngine() = default;

    // Add a route. Returns its ID.
    uint32_t addRoute(const Route& route);

    // Remove a route by ID.
    bool removeRoute(uint32_t routeId);

    // Get route by ID.
    Route* getRoute(uint32_t routeId);
    const Route* getRoute(uint32_t routeId) const;

    // Get all routes
    int getNumRoutes() const { return static_cast<int>(routes_.size()); }
    Route* getRouteAt(int index);

    // Get routes targeting a specific parameter
    std::vector<Route*> getRoutesForTarget(const RouteTarget& target);

    // Get routes sourced from a specific signal
    std::vector<Route*> getRoutesForSource(uint32_t signalId);

    // Process all routes for one frame.
    // Reads signal values from registry, writes to parameter targets.
    // writeParam is called for each route result: (route, computed_value).
    // The caller (render thread) is responsible for writing the value to the
    // actual effect parameter.
    using ParamWriter = std::function<void(const Route& route, float value)>;
    void processFrame(const SignalRegistry& signals, const ParamWriter& writer);

    // Clear all routes.
    void clearAll();

private:
    std::vector<Route> routes_;
    std::vector<Smoother> smoothers_;
    std::vector<float> lastValues_; // For falloff computation
    uint32_t nextRouteId_ = 1;
};
