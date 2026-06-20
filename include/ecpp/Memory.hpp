#pragma once

#include <cstdint>
#include <cassert>
#include <memory>

namespace ecpp {

// A highly efficient bump-pointer allocator for temporary per-frame data.
// It pre-allocates a chunk of memory and rapidly hands out pointers.
class LinearAllocator {
public:
    LinearAllocator(size_t size) : mSize(size), mOffset(0) {
        mStart = new uint8_t[mSize];
    }
    ~LinearAllocator() {
        delete[] (uint8_t*)mStart;
    }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        size_t padding = 0;
        size_t currentAddress = (size_t)mStart + mOffset;

        if (alignment != 0 && mOffset % alignment != 0) {
            padding = alignment - (currentAddress % alignment);
        }

        if (mOffset + padding + size > mSize) {
            assert(false && "LinearAllocator is out of memory");
            return nullptr;
        }

        mOffset += padding;
        void* nextAddress = (void*)((uint8_t*)mStart + mOffset);
        mOffset += size;

        return nextAddress;
    }

    // Clears all allocations instantly (O(1))
    void Reset() {
        mOffset = 0;
    }

private:
    void* mStart;
    size_t mSize;
    size_t mOffset;
};

// Extremely fast allocator for millions of fixed-size blocks.
// Prevents heap fragmentation for frequently spawned/destroyed objects (like bullets).
class PoolAllocator {
public:
    PoolAllocator(size_t blockSize, size_t numBlocks) 
        : mBlockSize(blockSize), mNumBlocks(numBlocks) {
        assert(blockSize >= sizeof(void*));
        
        size_t totalSize = blockSize * numBlocks;
        mStart = new uint8_t[totalSize];

        mFreeList = mStart;
        
        // Link all blocks in the free list
        void* currentBlock = mStart;
        for (size_t i = 0; i < numBlocks - 1; ++i) {
            void* nextBlock = (uint8_t*)currentBlock + blockSize;
            *(void**)currentBlock = nextBlock;
            currentBlock = nextBlock;
        }
        *(void**)currentBlock = nullptr;
    }

    ~PoolAllocator() {
        delete[] (uint8_t*)mStart;
    }

    void* Allocate() {
        if (mFreeList == nullptr) {
            assert(false && "PoolAllocator is out of memory");
            return nullptr;
        }

        void* freeBlock = mFreeList;
        mFreeList = *(void**)mFreeList;
        return freeBlock;
    }

    void Free(void* block) {
        if (block == nullptr) return;
        *(void**)block = mFreeList;
        mFreeList = block;
    }

private:
    void* mStart;
    size_t mBlockSize;
    size_t mNumBlocks;
    void* mFreeList;
};

} // namespace ecpp
