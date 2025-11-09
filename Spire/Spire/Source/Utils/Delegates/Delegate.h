#pragma once
#include "Utils/MacroDisableCopy.h"
#include "pch.h"

namespace SpireVoxel {
    template<typename... Args>
    class Delegate;

    template<typename... Args>
    class DelegateSubscriber;

    // Collection of subscribers to a delegate
    template<typename... Args>
    class DelegateSubscribers {
        using FuncType = std::function<void(Args...)>;
        friend class Delegate<Args...>;
        friend class DelegateSubscriber<Args...>;

    private:
        DelegateSubscribers() = default;

        DISABLE_COPY(DelegateSubscribers);

    public:
        int AddCallback(const FuncType &func) {
            int id = mNextCallbackId++;
            mCallbacks[id] = func;
            return id;
        }

        void RemoveCallback(int id) {
            mCallbacks.erase(id);
        }

    private:
        std::unordered_map<int, FuncType> mCallbacks;
        int mNextCallbackId = 0;
        std::shared_ptr<bool> m_isValid = std::make_shared<bool>(true);
    };

    // Similar to unreal delegates, lambdas can subscribe and then all the subscribers can be called at once (fancy std::vector<std::function>)
    template<typename... Args>
    class Delegate {
        using FuncType = std::function<void(Args...)>;
        friend DelegateSubscribers<Args...>;

    public:
        Delegate() = default;

        DISABLE_COPY(Delegate);

    public:
        void Broadcast(Args... args) {
            for (auto &[id, func] : mSubscribers.mCallbacks)
                func(args...);
        }

        DelegateSubscribers<Args...> &GetSubscribers() { return mSubscribers; }

        // ReSharper disable once CppNonExplicitConversionOperator
        operator DelegateSubscribers<Args...> &() { return GetSubscribers(); }

    private:
        DelegateSubscribers<Args...> mSubscribers;
    };
} // SpireVoxel
