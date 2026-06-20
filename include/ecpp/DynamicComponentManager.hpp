#pragma once

#include "Coordinator.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <cstring>

namespace ecpp {

// Manages runtime-defined components for scripting languages (Lua, Python)
// Stores data externally to keep the core C++ Archetype system hyper-optimized.
class DynamicComponentManager {
public:
    DynamicComponentManager(Coordinator& coordinator) : mCoordinator(coordinator) {}

    void RegisterDynamicComponent(const std::string& name, size_t sizeInBytes) {
        mComponentSizes[name] = sizeInBytes;
    }

    void AddDynamicComponent(Entity entity, const std::string& name, void* data) {
        size_t size = mComponentSizes[name];
        auto& storage = mComponentData[name];
        
        if (storage.find(entity) == storage.end()) {
            storage[entity] = std::vector<uint8_t>(size);
        }
        std::memcpy(storage[entity].data(), data, size);
    }

    void* GetDynamicComponent(Entity entity, const std::string& name) {
        return mComponentData[name][entity].data();
    }

    bool HasDynamicComponent(Entity entity, const std::string& name) {
        auto& storage = mComponentData[name];
        return storage.find(entity) != storage.end();
    }

private:
    Coordinator& mCoordinator;
    std::unordered_map<std::string, size_t> mComponentSizes;
    std::unordered_map<std::string, std::unordered_map<Entity, std::vector<uint8_t>>> mComponentData;
};

} // namespace ecpp
