# First ROCK consumer

This observer registers one owner, acquires Core callbacks and Hands Read, and copies the right hand's final presentation. It performs no gameplay writes. Keep the observer at a stable address and call `stop()` on ROCK's game owner thread before destroying it. Do not unload its callback code while registered.

```cpp
#include <Windows.h>
#include <ROCK/Client.h>
#include <ROCK/Hands.h>

class HandsObserver {
    rock::api::Client client_;
    const rock::api::core::ApiV1* core_{};
    const rock::api::hands::ApiV1* hands_{};
    std::uint64_t callback_{};
    rock::api::hands::PresentedHandPoseV1 lastPose_{};
    bool valid_{};

    static void ROCK_CALL onPhase(
        const rock::api::core::AnimationPhaseContextV1* context,
        void* userData) noexcept
    {
        auto& self = *static_cast<HandsObserver*>(userData);
        if (!context) return;
        const auto ready = static_cast<std::uint32_t>(
            rock::api::core::AnimationPhaseContextFlagV1::ProviderReady);
        if ((context->flags & ready) == 0) {
            self.valid_ = false;
            return;
        }
        if (context->phase != rock::api::core::AnimationPhaseV1::Presented)
            return;
        self.valid_ = false;
        rock::api::hands::PresentedHandPoseV1 pose{};
        if (self.hands_->getPresentedHandPoseV1(
                self.client_.owner(), rock::api::Hand::Right, &pose) !=
            rock::api::Status::Ok) return;
        const auto required = static_cast<std::uint32_t>(
            rock::api::hands::PresentedHandPoseFlagV1::Valid) |
            static_cast<std::uint32_t>(
                rock::api::hands::PresentedHandPoseFlagV1::HandWorldValid);
        if ((pose.flags & required) != required ||
            pose.frameIndex != context->frameIndex ||
            pose.worldGeneration != context->worldGeneration ||
            pose.skeletonGeneration != context->skeletonGeneration ||
            pose.providerGeneration != context->providerGeneration) return;
        self.lastPose_ = pose;
        self.valid_ = true;
    }

public:
    rock::api::Status start() noexcept {
        using rock::api::Status;
        if (client_.owner()) return Status::Busy;
        const auto module = GetModuleHandleW(L"ROCK.dll");
        const auto query = module ? reinterpret_cast<rock::api::QueryInterfaceV1>(
            GetProcAddress(module, rock::api::kQueryExportName)) : nullptr;
        if (!query) return Status::NotReady;
        auto status = client_.connect(query, "Example.HandObserver");
        if (status != Status::Ok) return status;
        status = client_.acquire(5, core_); // Core Read | Callbacks
        if (status == Status::Ok)
            status = client_.acquire(1, hands_); // Hands Read
        if (status == Status::Ok)
            status = core_->registerAnimationPhaseCallbackV1(
                client_.owner(), &onPhase, this, &callback_);
        if (status != Status::Ok) {
            const auto cleanup = stop();
            return cleanup == Status::Ok ? status : cleanup;
        }
        return Status::Ok;
    }

    rock::api::Status stop() noexcept {
        const auto status = client_.close(); // retires every owner callback
        if (status != rock::api::Status::Ok) return status;
        core_ = nullptr;
        hands_ = nullptr;
        callback_ = 0;
        lastPose_ = {};
        valid_ = false;
        return rock::api::Status::Ok;
    }

    // Read on the same game owner thread; this object is not thread-safe.
    bool copyLastPose(rock::api::hands::PresentedHandPoseV1& output) const noexcept {
        if (!valid_) return false;
        output = lastPose_;
        return true;
    }
};
```

Start after plugin discovery at an appropriate game lifecycle point. Once ROCK has established its owner thread, callback registration/removal and owner teardown must run there; a generic F4SE task on another thread may receive `WrongThread`. Preserve the object and retry cleanup on the correct thread if `stop()` fails. `Client` has no automatic teardown destructor.

`copyLastPose` is a stamped observation, not a guarantee that the skeleton still exists. Its consumer must compare the copied generations to current lifecycle state. Do not retain the observer's last pose as authority after a reset or transfer it to a render thread without your own bounded synchronization.

The fragment supplies API integration only. Your DLL still owns F4SE Query/Load, plugin/game lifecycle notifications and teardown. For complete plugin targets, use the [SDK examples](Examples.md).
