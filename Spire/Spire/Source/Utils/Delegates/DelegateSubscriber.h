#pragma once

#include "Delegate.h"
#include "pch.h"
#include "Utils/MacroDisableCopy.h"

namespace SpireVoxel {
    template<typename... Args>
    class LocalDelegateSubscriber {
    public:
        LocalDelegateSubscriber(Delegate<Args...> &delegate, const std::function<void(Args...)> &func)
            : mSubscribers(delegate.GetSubscribers()),
              m_isValid(mSubscribers.GetIsValidPtr()) {
            mCallbackId = mSubscribers.AddCallback(func);
        }

        LocalDelegateSubscriber(DelegateSubscribers<Args...> &subscribers, const std::function<void(Args...)> &func)
            : mSubscribers(subscribers),
              m_isValid(mSubscribers.GetIsValidPtr()) {
            mCallbackId = mSubscribers.AddCallback(func);
        }

        ~LocalDelegateSubscriber() {
            if (IsValid()) mSubscribers.RemoveCallback(mCallbackId);
        }

        DISABLE_COPY_AND_MOVE(LocalDelegateSubscriber);

        // Check if the delegate has been destroyed
        bool IsValid() const {
            return m_isValid.lock() != nullptr;
        }

    private:
        DelegateSubscribers<Args...> &mSubscribers;
        std::weak_ptr<bool> m_isValid;
        int mCallbackId;
    };
} // SpireVoxel
