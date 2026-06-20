#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <any>

namespace ecpp {

class EventManager {
public:
    template <typename T>
    void AddListener(std::function<void(const T&)> listener) {
        std::type_index type = std::type_index(typeid(T));
        
        if (mListeners.find(type) == mListeners.end()) {
            mListeners[type] = std::vector<std::function<void(const T&)>>();
        }
        
        std::any_cast<std::vector<std::function<void(const T&)>>&>(mListeners[type]).push_back(listener);
    }

    template <typename T>
    void EmitEvent(const T& event) {
        std::type_index type = std::type_index(typeid(T));
        
        if (mListeners.find(type) != mListeners.end()) {
            auto& listeners = std::any_cast<std::vector<std::function<void(const T&)>>&>(mListeners[type]);
            for (auto& listener : listeners) {
                listener(event);
            }
        }
    }

private:
    std::unordered_map<std::type_index, std::any> mListeners;
};

} // namespace ecpp
