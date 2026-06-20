#pragma once

#include "Types.hpp"
#include <vector>
#include <unordered_map>
#include <cstring>
#include <cassert>

namespace ecpp {

// Defines how to handle generic memory for a component during archetype moves.
struct ComponentTypeInfo {
    size_t size;
    void (*moveConstruct)(void* dst, void* src);
    void (*destroy)(void* ptr);
};

// Represents a specific combination of components and holds the actual memory chunks.
class Archetype {
public:
    Signature signature;
    std::vector<Entity> entities;
    
    // Map of ComponentType to raw byte arrays containing the contiguous data
    std::unordered_map<ComponentType, std::vector<uint8_t>> componentData;

    Archetype(Signature sig, const std::unordered_map<ComponentType, ComponentTypeInfo>& infos) 
        : signature(sig), mInfos(infos) {
        // Initialize an empty byte array for every component type in this archetype's signature
        for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
            if (signature.test(i)) {
                componentData[i] = std::vector<uint8_t>();
            }
        }
    }

    size_t GetEntityCount() const { 
        return entities.size(); 
    }

    // Allocates space for a new entity. Returns the row index.
    size_t AddEntity(Entity entity) {
        entities.push_back(entity);
        size_t rowIndex = entities.size() - 1;
        
        for (auto& pair : componentData) {
            ComponentType type = pair.first;
            size_t size = mInfos.at(type).size;
            pair.second.resize(pair.second.size() + size);
        }
        return rowIndex;
    }

    // Removes an entity by swapping it with the last entity to maintain density.
    // Returns the Entity ID of the swapped entity so the Coordinator can update its row index.
    Entity RemoveEntity(size_t rowIndex) {
        assert(rowIndex < entities.size() && "Archetype row out of bounds");
        
        size_t lastIndex = entities.size() - 1;
        Entity swappedEntity = INVALID_ENTITY;

        if (rowIndex != lastIndex) {
            swappedEntity = entities[lastIndex];
            entities[rowIndex] = swappedEntity;
            
            // Swap components
            for (auto& pair : componentData) {
                ComponentType type = pair.first;
                const ComponentTypeInfo& info = mInfos.at(type);
                
                uint8_t* arrayPtr = pair.second.data();
                void* dst = arrayPtr + (rowIndex * info.size);
                void* src = arrayPtr + (lastIndex * info.size);
                
                info.destroy(dst);
                info.moveConstruct(dst, src);
            }
        } else {
            // Just destroy the last element
            for (auto& pair : componentData) {
                ComponentType type = pair.first;
                const ComponentTypeInfo& info = mInfos.at(type);
                uint8_t* arrayPtr = pair.second.data();
                void* dst = arrayPtr + (rowIndex * info.size);
                info.destroy(dst);
            }
        }
        
        entities.pop_back();
        for (auto& pair : componentData) {
            ComponentType type = pair.first;
            size_t size = mInfos.at(type).size;
            pair.second.resize(pair.second.size() - size);
        }
        
        return swappedEntity;
    }

    void* GetComponentPtr(ComponentType type, size_t rowIndex) {
        assert(signature.test(type) && "Archetype does not have this component");
        return componentData[type].data() + (rowIndex * mInfos.at(type).size);
    }

private:
    const std::unordered_map<ComponentType, ComponentTypeInfo>& mInfos;
};

} // namespace ecpp
