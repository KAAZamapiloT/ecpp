#pragma once

#include "Coordinator.hpp"
#include "NetworkComponent.hpp"
#include <unordered_map>
#include <vector>

namespace ecpp {

// A utility class demonstrating how to use Reactive Hooks for Delta Compression.
// Tracks which components were added/modified during a frame so you only send 
// the changed data across the network.
class NetworkState {
public:
    NetworkState(Coordinator& coordinator) : mCoordinator(coordinator) {}

    template <typename T>
    void TrackComponentChanges() {
        ComponentType type = mCoordinator.GetComponentType<T>();
        
        mCoordinator.OnComponentAdded<T>([this, type](Entity e, T& comp) {
            mDirtyEntities[e].push_back(type);
        });

        mCoordinator.OnComponentRemoved<T>([this, type](Entity e, T& comp) {
            mDirtyEntities[e].push_back(type); // Need to sync removal
        });
    }

    // Call this manually when you modify a component if you want delta sync
    template <typename T>
    void MarkDirty(Entity entity) {
        mDirtyEntities[entity].push_back(mCoordinator.GetComponentType<T>());
    }

    std::vector<ComponentType> GetAndClearDirtyComponents(Entity entity) {
        auto it = mDirtyEntities.find(entity);
        if (it != mDirtyEntities.end()) {
            auto deltas = it->second;
            mDirtyEntities.erase(it);
            return deltas;
        }
        return {};
    }

    void ClearAllDeltas() {
        mDirtyEntities.clear();
    }

private:
    Coordinator& mCoordinator;
    std::unordered_map<Entity, std::vector<ComponentType>> mDirtyEntities;
};

} // namespace ecpp
