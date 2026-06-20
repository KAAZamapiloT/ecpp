#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>

namespace ecpp {

class JobSystem {
public:
    JobSystem() = default;

    void Init(uint32_t numThreads = 0) {
        if (numThreads == 0) {
            numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) numThreads = 4;
        }
        
        for (uint32_t i = 0; i < numThreads; ++i) {
            mThreads.emplace_back([this]() {
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lock(mMutex);
                        mCondition.wait(lock, [this]() { return mStop || !mJobs.empty(); });
                        if (mStop && mJobs.empty()) return;
                        
                        job = std::move(mJobs.front());
                        mJobs.pop();
                    }
                    job();
                }
            });
        }
    }

    ~JobSystem() {
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mStop = true;
        }
        mCondition.notify_all();
        for (std::thread& thread : mThreads) {
            if (thread.joinable()) thread.join();
        }
    }

    // Dispatches a batched task across available threads.
    // The task is given a start and end index to process.
    void Dispatch(uint32_t elementCount, std::function<void(uint32_t start, uint32_t end)> task) {
        if (elementCount == 0) return;
        
        uint32_t numThreads = std::max(1u, (uint32_t)mThreads.size());
        uint32_t groupSize = (elementCount + numThreads - 1) / numThreads; 
        
        std::atomic<uint32_t> completedGroups{0};
        uint32_t numGroups = (elementCount + groupSize - 1) / groupSize;

        std::mutex waitMutex;
        std::condition_variable waitCondition;

        for (uint32_t i = 0; i < numGroups; ++i) {
            uint32_t start = i * groupSize;
            uint32_t end = std::min(start + groupSize, elementCount);
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mJobs.push([start, end, task, &completedGroups, &waitCondition, numGroups]() {
                    task(start, end);
                    if (++completedGroups == numGroups) {
                        waitCondition.notify_one();
                    }
                });
            }
        }
        mCondition.notify_all();

        // Block main thread until all groups are completed
        std::unique_lock<std::mutex> waitLock(waitMutex);
        waitCondition.wait(waitLock, [&]() { return completedGroups.load() == numGroups; });
    }

private:
    std::vector<std::thread> mThreads;
    std::queue<std::function<void()>> mJobs;
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mStop = false;
};

} // namespace ecpp
