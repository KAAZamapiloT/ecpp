#pragma once

#include <mutex>
#include <atomic>
#include <memory>

namespace ecpp {

// Wrapper for std::mutex that is safely copyable (using shared_ptr internally).
// This is required because ECS components are frequently copied/moved in memory arrays.
class EMutex {
public:
    EMutex() : mMutex(std::make_shared<std::mutex>()) {}
    
    void lock() { mMutex->lock(); }
    void unlock() { mMutex->unlock(); }
    bool try_lock() { return mMutex->try_lock(); }

private:
    std::shared_ptr<std::mutex> mMutex;
};

// Wrapper for std::lock_guard. Use this to acquire an EMutex.
using ELock = std::lock_guard<EMutex>;

// Wrapper for std::atomic that is copyable.
template <typename T>
class EAtomic {
public:
    EAtomic(T val = T()) : mValue(val) {}
    EAtomic(const EAtomic& other) : mValue(other.mValue.load()) {}
    EAtomic& operator=(const EAtomic& other) { 
        mValue.store(other.mValue.load()); 
        return *this; 
    }
    
    T load() const { return mValue.load(); }
    void store(T val) { mValue.store(val); }
    T operator=(T val) { mValue = val; return val; }
    operator T() const { return load(); }

private:
    std::atomic<T> mValue;
};

} // namespace ecpp
