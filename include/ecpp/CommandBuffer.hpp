#pragma once

#include "Coordinator.hpp"
#include <mutex>
#include <vector>
#include <functional>

namespace ecpp {

// A thread-safe command queue for deferred ECS modifications.
// Used when multithreading Archetype iteration (JobSystem) where structural changes
// would otherwise cause race conditions.
class CommandBuffer {
public:
    void AddCommand(std::function<void(Coordinator&)> cmd) {
        std::lock_guard<std::mutex> lock(mMutex);
        mCommands.push_back(cmd);
    }

    // Sugar for adding components
    template<typename T>
    void QueueAddComponent(Entity entity, T component) {
        AddCommand([entity, comp = std::move(component)](Coordinator& coord) mutable {
            coord.AddComponent(entity, std::move(comp));
        });
    }

    // Sugar for removing components
    template<typename T>
    void QueueRemoveComponent(Entity entity) {
        AddCommand([entity](Coordinator& coord) {
            coord.RemoveComponent<T>(entity);
        });
    }

    // Sugar for destroying entities
    void QueueDestroyEntity(Entity entity) {
        AddCommand([entity](Coordinator& coord) {
            coord.DestroyEntity(entity);
        });
    }

    // Sugar for creating entities
    void QueueCreateEntity(std::function<void(Entity, Coordinator&)> initializer = nullptr) {
        AddCommand([initializer](Coordinator& coord) {
            Entity e = coord.CreateEntity();
            if (initializer) initializer(e, coord);
        });
    }

    // Plays back all recorded commands sequentially on the main thread
    void Execute(Coordinator& coordinator) {
        std::lock_guard<std::mutex> lock(mMutex);
        for (auto& cmd : mCommands) {
            cmd(coordinator);
        }
        mCommands.clear();
    }

private:
    std::mutex mMutex;
    std::vector<std::function<void(Coordinator&)>> mCommands;
};

} // namespace ecpp
