#pragma once

#include "Types.hpp"
#include <array>
#include <cassert>
#include <queue>

namespace ecpp {

class EntityManager {
public:
    EntityManager() {
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
            mAvailableEntities.push(entity);
        }
        mAliveEntities.reset();
    }

    Entity CreateEntity() {
        assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

        Entity id = mAvailableEntities.front();
        mAvailableEntities.pop();
        ++mLivingEntityCount;
        mAliveEntities.set(id, true);

        return id;
    }

    void DestroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Invalidate the destroyed entity's signature
        mSignatures[entity].reset();

        // Put the destroyed ID at the back of the queue
        mAvailableEntities.push(entity);
        --mLivingEntityCount;
        mAliveEntities.set(entity, false);
    }

    void SetSignature(Entity entity, Signature signature) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        mSignatures[entity] = signature;
    }

    Signature GetSignature(Entity entity) {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        return mSignatures[entity];
    }

    bool IsAlive(Entity entity) const {
        return mAliveEntities.test(entity);
    }

    uint32_t GetLivingEntityCount() const {
        return mLivingEntityCount;
    }

private:
    std::queue<Entity> mAvailableEntities{};
    std::array<Signature, MAX_ENTITIES> mSignatures{};
    std::bitset<MAX_ENTITIES> mAliveEntities{};
    uint32_t mLivingEntityCount{};
};

} // namespace ecpp
