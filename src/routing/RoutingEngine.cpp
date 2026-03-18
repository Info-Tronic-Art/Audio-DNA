#include "RoutingEngine.h"
#include <algorithm>
#include <cmath>

uint32_t RoutingEngine::addRoute(const Route& route)
{
    Route r = route;
    r.id = nextRouteId_++;
    routes_.push_back(r);
    smoothers_.emplace_back(0.15f);
    lastValues_.push_back(0.0f);
    return r.id;
}

bool RoutingEngine::removeRoute(uint32_t routeId)
{
    for (size_t i = 0; i < routes_.size(); ++i)
    {
        if (routes_[i].id == routeId)
        {
            routes_.erase(routes_.begin() + static_cast<ptrdiff_t>(i));
            smoothers_.erase(smoothers_.begin() + static_cast<ptrdiff_t>(i));
            lastValues_.erase(lastValues_.begin() + static_cast<ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

Route* RoutingEngine::getRoute(uint32_t routeId)
{
    for (auto& r : routes_)
        if (r.id == routeId) return &r;
    return nullptr;
}

const Route* RoutingEngine::getRoute(uint32_t routeId) const
{
    for (const auto& r : routes_)
        if (r.id == routeId) return &r;
    return nullptr;
}

Route* RoutingEngine::getRouteAt(int index)
{
    if (index >= 0 && index < static_cast<int>(routes_.size()))
        return &routes_[static_cast<size_t>(index)];
    return nullptr;
}

std::vector<Route*> RoutingEngine::getRoutesForTarget(const RouteTarget& target)
{
    std::vector<Route*> result;
    for (auto& r : routes_)
    {
        if (r.targetScope == target.scope
            && r.targetLayerId == target.layerId
            && r.targetClipId == target.clipId
            && r.targetEffectIndex == target.effectIndex
            && r.targetParamIndex == target.paramIndex)
        {
            result.push_back(&r);
        }
    }
    return result;
}

std::vector<Route*> RoutingEngine::getRoutesForSource(uint32_t signalId)
{
    std::vector<Route*> result;
    for (auto& r : routes_)
        if (r.sourceType == Route::SourceType::Signal && r.sourceId == signalId)
            result.push_back(&r);
    return result;
}

void RoutingEngine::processFrame(const SignalRegistry& signals, const ParamWriter& writer)
{
    for (size_t i = 0; i < routes_.size(); ++i)
    {
        const auto& route = routes_[i];
        if (!route.enabled)
            continue;

        // Get source value
        float raw = 0.0f;
        if (route.sourceType == Route::SourceType::Signal)
        {
            raw = signals.getCachedValue(route.sourceId);
        }
        // Macro sources are handled by the MacroBank before routing

        // Apply dial range (input sensitivity)
        float dialRange = route.dialRangeMax - route.dialRangeMin;
        float normalized = (dialRange > 1e-8f)
            ? std::clamp((raw - route.dialRangeMin) / dialRange, 0.0f, 1.0f)
            : 0.0f;

        // Apply threshold
        if (normalized < route.threshold)
        {
            // Below threshold: apply falloff toward 0
            lastValues_[i] += route.falloff * (0.0f - lastValues_[i]);
            normalized = lastValues_[i];
        }
        else
        {
            // Above threshold: apply gain
            normalized = std::clamp((normalized - route.threshold) * route.gain, 0.0f, 1.0f);
            lastValues_[i] = normalized;
        }

        // Invert
        if (route.inverted)
            normalized = 1.0f - normalized;

        // Scale to output range
        float value = route.outputMin + normalized * (route.outputMax - route.outputMin);

        // Smooth
        float smoothed = smoothers_[i].process(value);

        // Write to target
        writer(route, smoothed);
    }
}

void RoutingEngine::clearAll()
{
    routes_.clear();
    smoothers_.clear();
    lastValues_.clear();
}
