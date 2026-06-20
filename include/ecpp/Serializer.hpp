#pragma once

#include "Coordinator.hpp"
#include <fstream>
#include <unordered_map>
#include <functional>
#include <iostream>

namespace ecpp {

// A lightweight binary serializer for ECS State.
class Serializer {
public:
    Serializer(Coordinator& coordinator) : mCoordinator(coordinator) {}

    // Register serialization lambdas for a component type.
    // saveFunc: writes component data to the ostream.
    // loadFunc: reads component data from the istream.
    template <typename T>
    void RegisterComponent(std::function<void(const T&, std::ostream&)> saveFunc,
                           std::function<void(T&, std::istream&)> loadFunc) {
        ComponentType type = mCoordinator.GetComponentType<T>();
        
        mSaveFuncs[type] = [this, saveFunc](Entity e, std::ostream& os) {
            auto& comp = mCoordinator.GetComponent<T>(e);
            saveFunc(comp, os);
        };
        
        mLoadFuncs[type] = [this, loadFunc](Entity e, std::istream& is) {
            T comp;
            loadFunc(comp, is);
            mCoordinator.AddComponent(e, comp);
        };
    }

    // Saves all active entities and their registered components to a binary file.
    void SaveToFile(const std::string& filepath) {
        std::ofstream os(filepath, std::ios::binary);
        if (!os.is_open()) {
            std::cerr << "Failed to open file for saving: " << filepath << "\n";
            return;
        }

        auto activeEntities = mCoordinator.GetActiveEntities();
        
        // Write total number of entities
        uint32_t entityCount = activeEntities.size();
        os.write(reinterpret_cast<const char*>(&entityCount), sizeof(uint32_t));

        for (Entity entity : activeEntities) {
            os.write(reinterpret_cast<const char*>(&entity), sizeof(Entity));
            
            Signature sig = mCoordinator.GetEntitySignature(entity);
            
            for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
                if (sig.test(i) && mSaveFuncs.find(i) != mSaveFuncs.end()) {
                    ComponentType typeId = static_cast<ComponentType>(i);
                    os.write(reinterpret_cast<const char*>(&typeId), sizeof(ComponentType));
                    mSaveFuncs[typeId](entity, os);
                }
            }
            
            // Marker for end of components for this entity
            ComponentType endMarker = MAX_COMPONENTS;
            os.write(reinterpret_cast<const char*>(&endMarker), sizeof(ComponentType));
        }
    }

    // Loads entities and components from file.
    // Returns a map from the old Entity ID (in the file) to the newly created Entity ID.
    // Use this map to fix any components that store Entity IDs (like HierarchyComponent).
    std::unordered_map<Entity, Entity> LoadFromFile(const std::string& filepath) {
        std::unordered_map<Entity, Entity> idMap;
        
        std::ifstream is(filepath, std::ios::binary);
        if (!is.is_open()) {
            std::cerr << "Failed to open file for loading: " << filepath << "\n";
            return idMap;
        }

        uint32_t entityCount = 0;
        if (!is.read(reinterpret_cast<char*>(&entityCount), sizeof(uint32_t))) return idMap;

        for (uint32_t i = 0; i < entityCount; ++i) {
            Entity oldEntity;
            is.read(reinterpret_cast<char*>(&oldEntity), sizeof(Entity));
            
            Entity newEntity = mCoordinator.CreateEntity();
            idMap[oldEntity] = newEntity;

            while (true) {
                ComponentType typeId;
                is.read(reinterpret_cast<char*>(&typeId), sizeof(ComponentType));
                
                if (typeId == MAX_COMPONENTS) {
                    break; // End of components for this entity
                }
                
                if (mLoadFuncs.find(typeId) != mLoadFuncs.end()) {
                    mLoadFuncs[typeId](newEntity, is);
                }
            }
        }
        
        return idMap;
    }

private:
    Coordinator& mCoordinator;
    std::unordered_map<ComponentType, std::function<void(Entity, std::ostream&)>> mSaveFuncs;
    std::unordered_map<ComponentType, std::function<void(Entity, std::istream&)>> mLoadFuncs;
};

} // namespace ecpp
