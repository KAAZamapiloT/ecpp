#pragma once

#include "Types.hpp"
#include "System.hpp"
#include "JobSystem.hpp"
#include <cassert>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <algorithm>

namespace ecpp {

class SystemManager {
public:
    template <typename T>
    std::shared_ptr<T> RegisterSystem() {
        std::type_index typeName = std::type_index(typeid(T));
        assert(mSystems.find(typeName) == mSystems.end() && "System already registered.");
        
        auto system = std::make_shared<T>();
        mSystems[typeName] = system;
        mSystemExecutionOrder.push_back(system.get());
        return system;
    }

    template <typename T>
    std::shared_ptr<T> GetSystem() {
        std::type_index typeName = std::type_index(typeid(T));
        return std::static_pointer_cast<T>(mSystems[typeName]);
    }

    template <typename T>
    void SetSignature(Signature signature) {
        std::type_index typeName = std::type_index(typeid(T));
        mSignatures[typeName] = signature;
    }

    // Graph dependency: SystemA must run before SystemB
    template <typename SystemA, typename SystemB>
    void AddDependency() {
        std::type_index typeA = std::type_index(typeid(SystemA));
        std::type_index typeB = std::type_index(typeid(SystemB));
        mDependencies[typeA].push_back(typeB);
        SortExecutionOrder();
    }

    // Called by Coordinator whenever a totally new Archetype combination is discovered
    void ArchetypeCreated(Archetype* archetype) {
        for (auto const& pair : mSystems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& sysSig = mSignatures[type];

            // If the new archetype has all components required by this system
            if ((archetype->signature & sysSig) == sysSig) {
                system->mArchetypes.push_back(archetype);
            }
        }
    }

    // Executes all systems automatically in topological order
    void UpdateAll(Coordinator& coordinator, float dt) {
        for (System* system : mSystemExecutionOrder) {
            system->Update(coordinator, dt);
        }
    }

private:
    std::unordered_map<std::type_index, Signature> mSignatures;
    std::unordered_map<std::type_index, std::shared_ptr<System>> mSystems;
    
    // Dependency Graph
    std::unordered_map<std::type_index, std::vector<std::type_index>> mDependencies;
    std::vector<System*> mSystemExecutionOrder;

    void SortExecutionOrder() {
        std::vector<System*> sorted;
        std::unordered_map<std::type_index, int> inDegree;
        
        for (auto const& pair : mSystems) inDegree[pair.first] = 0;
        
        for (auto const& pair : mDependencies) {
            for (auto const& dep : pair.second) {
                inDegree[dep]++;
            }
        }

        std::vector<std::type_index> zeroInDegree;
        for (auto const& pair : inDegree) {
            if (pair.second == 0) zeroInDegree.push_back(pair.first);
        }

        while (!zeroInDegree.empty()) {
            auto type = zeroInDegree.back();
            zeroInDegree.pop_back();
            sorted.push_back(mSystems[type].get());

            for (auto const& dep : mDependencies[type]) {
                inDegree[dep]--;
                if (inDegree[dep] == 0) zeroInDegree.push_back(dep);
            }
        }

        mSystemExecutionOrder = sorted;
    }
};

} // namespace ecpp
