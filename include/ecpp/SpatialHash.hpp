#pragma once

#include "Types.hpp"
#include <unordered_map>
#include <vector>
#include <cmath>

namespace ecpp {

// A high-performance 2D grid spatial partitioning utility.
// Standalone architecture: The engine manually updates the grid, keeping the ECS pure.
// Perfect for rendering culling and proximity queries (e.g., getting only entities on screen).
class SpatialHash {
public:
    // cellSize determines how large each grid square is in world units.
    SpatialHash(float cellSize) : mCellSize(cellSize) {}

    // Inserts or updates an entity's position in the spatial grid.
    void UpdateEntity(Entity entity, float x, float y) {
        uint64_t newCell = Hash(x, y);

        auto it = mEntityCells.find(entity);
        if (it != mEntityCells.end()) {
            if (it->second == newCell) {
                return; // Hasn't crossed cell boundaries, do nothing
            }
            RemoveEntityFromCell(entity, it->second);
        }

        mCells[newCell].push_back(entity);
        mEntityCells[entity] = newCell;
    }

    // Removes an entity entirely from the grid.
    void RemoveEntity(Entity entity) {
        auto it = mEntityCells.find(entity);
        if (it != mEntityCells.end()) {
            RemoveEntityFromCell(entity, it->second);
            mEntityCells.erase(it);
        }
    }

    // Returns all entities within the AABB (Bounding Box) defined by min/max coordinates.
    // Extremely fast way to query "entities currently on screen".
    std::vector<Entity> QueryAABB(float minX, float minY, float maxX, float maxY) const {
        std::vector<Entity> result;

        int startCellX = static_cast<int>(std::floor(minX / mCellSize));
        int startCellY = static_cast<int>(std::floor(minY / mCellSize));
        int endCellX = static_cast<int>(std::floor(maxX / mCellSize));
        int endCellY = static_cast<int>(std::floor(maxY / mCellSize));

        for (int x = startCellX; x <= endCellX; ++x) {
            for (int y = startCellY; y <= endCellY; ++y) {
                uint64_t cellHash = HashInt(x, y);
                auto it = mCells.find(cellHash);
                if (it != mCells.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return result;
    }

    void Clear() {
        mCells.clear();
        mEntityCells.clear();
    }

private:
    float mCellSize;

    // Maps a cell hash to a list of entities currently in that cell
    std::unordered_map<uint64_t, std::vector<Entity>> mCells;
    
    // Maps an entity to its current cell hash
    std::unordered_map<Entity, uint64_t> mEntityCells;

    uint64_t Hash(float x, float y) const {
        int cellX = static_cast<int>(std::floor(x / mCellSize));
        int cellY = static_cast<int>(std::floor(y / mCellSize));
        return HashInt(cellX, cellY);
    }

    uint64_t HashInt(int x, int y) const {
        // Pack two 32-bit signed integers into a 64-bit unsigned hash
        uint32_t ux = *reinterpret_cast<const uint32_t*>(&x);
        uint32_t uy = *reinterpret_cast<const uint32_t*>(&y);
        return (static_cast<uint64_t>(ux) << 32) | static_cast<uint64_t>(uy);
    }

    void RemoveEntityFromCell(Entity entity, uint64_t cellHash) {
        auto& vec = mCells[cellHash];
        for (size_t i = 0; i < vec.size(); ++i) {
            if (vec[i] == entity) {
                vec[i] = vec.back(); // Swap with back
                vec.pop_back();      // Remove back
                break;
            }
        }
    }
};

} // namespace ecpp
