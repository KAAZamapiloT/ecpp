#pragma once

#include "EntityManager.hpp"
#include "SystemManager.hpp"
#include "HierarchyComponent.hpp"
#include "EventManager.hpp"
#include "Archetype.hpp"
#include <memory>
#include <vector>
#include <any>
#include <typeindex>
#include <unordered_map>
#include <functional>

namespace ecpp {

template <typename T> struct ComponentAddedEvent { Entity entity; T& component; };
template <typename T> struct ComponentRemovedEvent { Entity entity; T& component; };

class Coordinator {
public:
    struct EntityRecord {
        Archetype* archetype = nullptr;
        size_t row = 0;
    };

    void Init() {
        mEntityManager = std::make_unique<EntityManager>();
        mSystemManager = std::make_unique<SystemManager>();
        mEventManager = std::make_unique<EventManager>();

        Signature emptySig;
        mArchetypes[emptySig] = std::make_unique<Archetype>(emptySig, mComponentInfos);
        
        RegisterComponent<HierarchyComponent>();
    }

    // --- Entities ---
    Entity CreateEntity() {
        Entity entity = mEntityManager->CreateEntity();
        
        Signature emptySig;
        Archetype* emptyArch = mArchetypes[emptySig].get();
        size_t row = emptyArch->AddEntity(entity);
        mEntityRecords[entity] = {emptyArch, row};
        
        return entity;
    }

    void DestroyEntity(Entity entity) {
        if (!mEntityManager->IsAlive(entity)) return;

        if (HasComponent<HierarchyComponent>(entity)) {
            // Copy hierarchy component by value because tree modifications move memory!
            HierarchyComponent hierarchy = GetComponent<HierarchyComponent>(entity);
            
            RemoveChild(hierarchy.parent, entity);

            Entity currentChild = hierarchy.firstChild;
            while (currentChild != INVALID_ENTITY) {
                if (HasComponent<HierarchyComponent>(currentChild)) {
                    HierarchyComponent childHierarchy = GetComponent<HierarchyComponent>(currentChild);
                    Entity nextChild = childHierarchy.nextSibling;
                    
                    if (childHierarchy.skipOnParentDestroy) {
                        auto& realChild = GetComponent<HierarchyComponent>(currentChild);
                        realChild.parent = INVALID_ENTITY;
                        realChild.prevSibling = INVALID_ENTITY;
                        realChild.nextSibling = INVALID_ENTITY;
                    } else {
                        DestroyEntity(currentChild);
                    }
                    currentChild = nextChild;
                } else {
                    currentChild = INVALID_ENTITY;
                }
            }
        }

        EntityRecord record = mEntityRecords[entity];
        Entity swappedEntity = record.archetype->RemoveEntity(record.row);
        if (swappedEntity != INVALID_ENTITY) {
            mEntityRecords[swappedEntity].row = record.row;
        }
        
        mEntityRecords.erase(entity);
        mEntityManager->DestroyEntity(entity);
    }

    // --- Components ---
    template <typename T>
    void RegisterComponent() {
        ComponentType type = mNextComponentType++;
        mComponentTypes[typeid(T)] = type;
        
        ComponentTypeInfo info;
        info.size = sizeof(T);
        info.moveConstruct = [](void* dst, void* src) {
            new (dst) T(std::move(*static_cast<T*>(src)));
        };
        info.destroy = [](void* ptr) {
            static_cast<T*>(ptr)->~T();
        };
        mComponentInfos[type] = info;
    }

    template <typename T>
    void AddComponent(Entity entity, T component) {
        EntityRecord record = mEntityRecords[entity];
        Archetype* oldArch = record.archetype;
        Signature newSig = oldArch->signature;
        ComponentType newType = GetComponentType<T>();
        
        newSig.set(newType, true);
        Archetype* newArch = GetOrCreateArchetype(newSig);
        size_t newRow = newArch->AddEntity(entity);
        
        for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
            if (oldArch->signature.test(i)) {
                void* src = oldArch->GetComponentPtr(i, record.row);
                void* dst = newArch->GetComponentPtr(i, newRow);
                mComponentInfos[i].moveConstruct(dst, src);
            }
        }
        
        void* dst = newArch->GetComponentPtr(newType, newRow);
        new (dst) T(std::move(component));
        
        Entity swappedEntity = oldArch->RemoveEntity(record.row);
        if (swappedEntity != INVALID_ENTITY) {
            mEntityRecords[swappedEntity].row = record.row;
        }
        
        mEntityRecords[entity] = {newArch, newRow};
        mEntityManager->SetSignature(entity, newSig);

        T& compRef = *static_cast<T*>(dst);
        EmitEvent(ComponentAddedEvent<T>{entity, compRef});
    }

    template <typename T>
    void RemoveComponent(Entity entity) {
        EntityRecord record = mEntityRecords[entity];
        Archetype* oldArch = record.archetype;
        Signature newSig = oldArch->signature;
        ComponentType typeToRemove = GetComponentType<T>();
        
        T& compRef = *static_cast<T*>(oldArch->GetComponentPtr(typeToRemove, record.row));
        EmitEvent(ComponentRemovedEvent<T>{entity, compRef});

        newSig.set(typeToRemove, false);
        Archetype* newArch = GetOrCreateArchetype(newSig);
        size_t newRow = newArch->AddEntity(entity);
        
        for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
            if (newSig.test(i)) {
                void* src = oldArch->GetComponentPtr(i, record.row);
                void* dst = newArch->GetComponentPtr(i, newRow);
                mComponentInfos[i].moveConstruct(dst, src);
            }
        }
        
        Entity swappedEntity = oldArch->RemoveEntity(record.row);
        if (swappedEntity != INVALID_ENTITY) {
            mEntityRecords[swappedEntity].row = record.row;
        }
        
        mEntityRecords[entity] = {newArch, newRow};
        mEntityManager->SetSignature(entity, newSig);
    }

    template <typename T>
    T& GetComponent(Entity entity) {
        EntityRecord record = mEntityRecords[entity];
        return *static_cast<T*>(record.archetype->GetComponentPtr(GetComponentType<T>(), record.row));
    }

    template <typename T>
    bool HasComponent(Entity entity) {
        return mEntityManager->GetSignature(entity).test(GetComponentType<T>());
    }

    template <typename T>
    ComponentType GetComponentType() {
        return mComponentTypes.at(typeid(T));
    }

    // --- Systems ---
    template <typename T>
    std::shared_ptr<T> RegisterSystem() {
        return mSystemManager->RegisterSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(Signature signature) {
        mSystemManager->SetSignature<T>(signature);

        // Feed any existing archetypes to the newly registered system
        std::shared_ptr<T> system = mSystemManager->GetSystem<T>();
        for (auto const& pair : mArchetypes) {
            Archetype* arch = pair.second.get();
            if ((arch->signature & signature) == signature) {
                system->mArchetypes.push_back(arch);
            }
        }
    }

    template <typename SystemA, typename SystemB>
    void AddSystemDependency() {
        mSystemManager->AddDependency<SystemA, SystemB>();
    }

    void UpdateSystems(float dt) {
        mSystemManager->UpdateAll(*this, dt);
    }

    // --- Hierarchy ---
    void AddChild(Entity parent, Entity child) {
        if (parent == INVALID_ENTITY || child == INVALID_ENTITY) return;

        if (!HasComponent<HierarchyComponent>(parent)) AddComponent(parent, HierarchyComponent{});
        if (!HasComponent<HierarchyComponent>(child)) AddComponent(child, HierarchyComponent{});

        auto& parentHierarchy = GetComponent<HierarchyComponent>(parent);
        auto& childHierarchy = GetComponent<HierarchyComponent>(child);

        if (childHierarchy.parent != INVALID_ENTITY) RemoveChild(childHierarchy.parent, child);

        childHierarchy.parent = parent;
        childHierarchy.nextSibling = parentHierarchy.firstChild;
        childHierarchy.prevSibling = INVALID_ENTITY;

        if (parentHierarchy.firstChild != INVALID_ENTITY) {
            auto& firstChildHierarchy = GetComponent<HierarchyComponent>(parentHierarchy.firstChild);
            firstChildHierarchy.prevSibling = child;
        }

        parentHierarchy.firstChild = child;
    }

    void RemoveChild(Entity parent, Entity child) {
        if (parent == INVALID_ENTITY || child == INVALID_ENTITY) return;
        if (!HasComponent<HierarchyComponent>(parent) || !HasComponent<HierarchyComponent>(child)) return;

        auto& parentHierarchy = GetComponent<HierarchyComponent>(parent);
        auto& childHierarchy = GetComponent<HierarchyComponent>(child);

        if (childHierarchy.parent != parent) return;

        if (childHierarchy.prevSibling != INVALID_ENTITY) {
            auto& prevSiblingHierarchy = GetComponent<HierarchyComponent>(childHierarchy.prevSibling);
            prevSiblingHierarchy.nextSibling = childHierarchy.nextSibling;
        } else {
            parentHierarchy.firstChild = childHierarchy.nextSibling;
        }

        if (childHierarchy.nextSibling != INVALID_ENTITY) {
            auto& nextSiblingHierarchy = GetComponent<HierarchyComponent>(childHierarchy.nextSibling);
            nextSiblingHierarchy.prevSibling = childHierarchy.prevSibling;
        }

        childHierarchy.parent = INVALID_ENTITY;
        childHierarchy.nextSibling = INVALID_ENTITY;
        childHierarchy.prevSibling = INVALID_ENTITY;
    }

    // --- Singletons ---
    template <typename T>
    void SetSingleton(T data) {
        mSingletons[std::type_index(typeid(T))] = data;
    }

    template <typename T>
    T& GetSingleton() {
        return std::any_cast<T&>(mSingletons[std::type_index(typeid(T))]);
    }

    // --- Events & Hooks ---
    template <typename T>
    void AddEventListener(std::function<void(const T&)> listener) {
        mEventManager->AddListener<T>(listener);
    }

    template <typename T>
    void EmitEvent(const T& event) {
        mEventManager->EmitEvent<T>(event);
    }

    template <typename T>
    void OnComponentAdded(std::function<void(Entity, T&)> callback) {
        AddEventListener<ComponentAddedEvent<T>>([callback](const ComponentAddedEvent<T>& e) {
            callback(e.entity, e.component);
        });
    }

    template <typename T>
    void OnComponentRemoved(std::function<void(Entity, T&)> callback) {
        AddEventListener<ComponentRemovedEvent<T>>([callback](const ComponentRemovedEvent<T>& e) {
            callback(e.entity, e.component);
        });
    }

    // --- Views ---
    template <typename... Components>
    std::vector<Entity> GetEntitiesWith() {
        std::vector<Entity> result;
        Signature querySig;
        (querySig.set(GetComponentType<Components>()), ...);

        for (auto const& pair : mArchetypes) {
            Archetype* arch = pair.second.get();
            if ((arch->signature & querySig) == querySig) {
                result.insert(result.end(), arch->entities.begin(), arch->entities.end());
            }
        }
        return result;
    }

    template <typename... Components>
    std::vector<Archetype*> GetArchetypesWith() {
        std::vector<Archetype*> result;
        Signature querySig;
        (querySig.set(GetComponentType<Components>()), ...);

        for (auto const& pair : mArchetypes) {
            Archetype* arch = pair.second.get();
            if ((arch->signature & querySig) == querySig) {
                result.push_back(arch);
            }
        }
        return result;
    }

    std::vector<Entity> GetActiveEntities() {
        std::vector<Entity> entities;
        entities.reserve(mEntityManager->GetLivingEntityCount());
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
            if (mEntityManager->IsAlive(entity)) {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    Signature GetEntitySignature(Entity entity) {
        return mEntityManager->GetSignature(entity);
    }

private:
    std::unique_ptr<EntityManager> mEntityManager;
    std::unique_ptr<SystemManager> mSystemManager;
    std::unique_ptr<EventManager> mEventManager;
    std::unordered_map<std::type_index, std::any> mSingletons;
    
    std::unordered_map<Entity, EntityRecord> mEntityRecords;
    std::unordered_map<Signature, std::unique_ptr<Archetype>> mArchetypes;
    std::unordered_map<std::type_index, ComponentType> mComponentTypes;
    std::unordered_map<ComponentType, ComponentTypeInfo> mComponentInfos;
    ComponentType mNextComponentType = 0;

    Archetype* GetOrCreateArchetype(Signature sig) {
        if (mArchetypes.find(sig) == mArchetypes.end()) {
            mArchetypes[sig] = std::make_unique<Archetype>(sig, mComponentInfos);
            mSystemManager->ArchetypeCreated(mArchetypes[sig].get());
        }
        return mArchetypes[sig].get();
    }
};

} // namespace ecpp
