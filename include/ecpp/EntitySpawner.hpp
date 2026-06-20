#pragma once

#include "Coordinator.hpp"
#include <functional>
#include <unordered_map>
#include <string>
#include <cassert>

namespace ecpp {

// A utility class to create entities based on predefined templates (Prefabs)
class EntitySpawner {
public:
    using PrefabBuilder = std::function<void(Entity, Coordinator&)>;

    EntitySpawner(Coordinator& coordinator) : mCoordinator(coordinator) {}

    // Register a new prefab by name and its construction logic
    void RegisterPrefab(const std::string& name, PrefabBuilder builder) {
        assert(mPrefabs.find(name) == mPrefabs.end() && "Prefab name already registered.");
        mPrefabs[name] = builder;
    }

    // Spawns a new entity using the registered prefab builder
    Entity Spawn(const std::string& name) {
        assert(mPrefabs.find(name) != mPrefabs.end() && "Prefab name not registered.");
        
        Entity entity = mCoordinator.CreateEntity();
        mPrefabs[name](entity, mCoordinator);
        return entity;
    }

private:
    Coordinator& mCoordinator;
    std::unordered_map<std::string, PrefabBuilder> mPrefabs;
};

} // namespace ecpp
