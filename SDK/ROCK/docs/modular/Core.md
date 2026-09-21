# Core API

Interface ID **1**, version **1.0**. Include `ROCK/Core.h`; link the header-only `RPS::ROCKCore` target. Namespace: `rock::api::core`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

Core owns registration, bindings, build identity, lifecycle and callback scheduling. Registration provides Read; add Callbacks with permission mask 5 when scheduling work. There is no Core Write permission. Never use Core as a replacement all-features snapshot. `SnapshotV1.stateSequence` identifies changes while `frameIndex` identifies the shared frame; fields from other providers must be joined with their sample/generation guards.

Frame/phase userdata is borrowed only during the invocation. Remove callbacks before releasing userdata. Fault cleanup is owner-wide. `NativeGraphOutput` observes the existing native authored-pose capture boundary; it is not a new thread or an independently scheduled animation pass.

## Animation phases

| Phase | Use |
| --- | --- |
| `BeforeRock` | Early provider control boundary; obey the supplied gates. |
| `AfterRock` | Observe/control at the post-ROCK boundary. |
| `Complete` | Control completion; final rendered hand presentation may still be pending. |
| `NativeGraphOutput` | Native animation graph observation; does not advance the provider frame clock. |
| `Presented` | Final hand readback after FRIK world final, once per skeleton frame. Observation only; submit control/animation writes in earlier phases. |

FRIK API 2.3, a ROCK interface version, and a ROCK mod version are separate version domains.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `getModVersion` | Read | Any; no borrowed-callback reentry | Return diagnostic ROCK mod version string. |
| `isProviderReady` | Read | Any; no borrowed-callback reentry | Report live provider initialization. |
| `getFrameSnapshot` | Read | Any; no borrowed-callback reentry | Copy Core lifecycle/readiness state and its frame/generation stamps. |
| `registerConsumerV1` | Owner lifetime | Any; no borrowed-callback reentry | Create a named owner with Core Read; bind feature permissions separately. |
| `unregisterConsumerV1` | Owner lifetime | Game owner | Revoke an owner and all owner-bound resources. |
| `registerAnimationPhaseCallbackV1` | Callbacks | Game owner | Register one owner animation-phase callback. |
| `unregisterAnimationPhaseCallbackV1` | Callbacks | Game owner | Remove an owner animation-phase callback. |
| `registerFrameCallbackForOwnerV1` | Callbacks | Game owner | Register a frame callback bound to a consumer. |
| `unregisterFrameCallbackForOwnerV1` | Callbacks | Game owner | Remove one owner callback. |
| `getSample` | Read | Any; no borrowed-callback reentry | Read the publication identity used to join family observations. |
| `bindInterface` | Owner lifetime | Any; no borrowed-callback reentry | Bind an exact interface major and its local permissions to an owner. |
| `copyEvents` | Read | Any; no borrowed-callback reentry | Read this owner/family event stream with retention and loss information. |

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/Core.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::core {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::Core;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    inline constexpr std::uint32_t kMaxOwners = 64;
    inline constexpr std::uint32_t kMaxFrameCallbacks = 16;
    inline constexpr std::uint32_t kMaxPhaseCallbacks = 16;
    inline constexpr std::uint32_t kEventCapacityPerOwner = 256;



    enum class LifecycleFlag : std::uint32_t
    {
        None = 0,
        WorldAvailable = 1u << 0,
        SkeletonReady = 1u << 1,
        ProviderReady = 1u << 2,
        MenuBlocking = 1u << 3,
        ConfigBlocking = 1u << 4,
        LoadingOrWorldTransition = 1u << 5,
        GeneratedBodiesValid = 1u << 6,
        PhysicsWriteAllowed = 1u << 7,
        VisualWriteAllowed = 1u << 8,
    };

    enum class LifecycleReason : std::uint32_t
    {
        None = 0,
        GameLoaded = 1,
        SkeletonReady = 2,
        SkeletonDestroying = 3,
        PowerArmorChanged = 4,
        WorldAvailable = 5,
        WorldChanged = 6,
        WorldUnavailable = 7,
        ProviderReady = 8,
        ProviderLost = 9,
        MenuBlocked = 10,
        ConfigBlocked = 11,
        GeneratedBodiesRebuilt = 12,
        GeneratedBodiesInvalidated = 13,
        TransitionSettled = 14,
        Shutdown = 15,
    };

    enum class AnimationPhaseV1 : std::uint32_t
    {
        BeforeRock = 1,
        AfterRock = 2,
        Complete = 3,
        NativeGraphOutput = 4,
        // Final readback after FRIK's world final, once per skeleton frame.
        // Observation only: publish control/animation writes in the earlier
        // phases. Complete retains its existing control-completion meaning.
        Presented = 5,
    };

    enum class AnimationPhaseContextFlagV1 : std::uint32_t
    {
        None = 0,
        RockEnabled = 1u << 0,
        ProviderReady = 1u << 1,
        SkeletonReady = 1u << 2,
        MenuBlocking = 1u << 3,
        ConfigBlocking = 1u << 4,
        VisualWritesAllowed = 1u << 5,
    };



    struct RegistrationV1
    {
        std::uint32_t size{ sizeof(RegistrationV1) };
        std::uint32_t version{ 1 };
        char modName[64]{};
        std::uint32_t reserved[7]{};
    };

    struct OwnerV1
    {
        std::uint32_t size{ sizeof(OwnerV1) };
        std::uint32_t version{ 1 };
        std::uint64_t ownerToken{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[6]{};
    };

    struct SnapshotV1
    {
        std::uint32_t size{ sizeof(SnapshotV1) };
        std::uint32_t version{ 1 };
        std::uint64_t frameIndex{ 0 };
        std::uint32_t frikSkeletonReady{ 0 };
        std::uint32_t menuBlocking{ 0 };
        std::uint32_t configBlocking{ 0 };
        std::uint32_t providerReady{ 0 };
        std::uint32_t lifecycleFlags{ 0 };
        LifecycleReason lastLifecycleReason{ LifecycleReason::None };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t stableFrameCount{ 0 };
        float deltaSeconds{ 0.0f };
        std::uint64_t stateSequence{ 0 };
    };

    struct AnimationPhaseContextV1
    {
        std::uint32_t size{ sizeof(AnimationPhaseContextV1) };
        std::uint32_t version{ 1 };
        AnimationPhaseV1 phase{ AnimationPhaseV1::BeforeRock };
        std::uint32_t flags{ 0 };
        std::uint64_t frameIndex{ 0 };
        float deltaSeconds{ 0.0f };
        std::uint32_t worldGeneration{ 0 };
        std::uint32_t skeletonGeneration{ 0 };
        std::uint32_t providerGeneration{ 0 };
        std::uint32_t reserved[7]{};
    };

    using FrameCallbackV1 = void(ROCK_CALL*)(const SnapshotV1*, void*);
    using PhaseCallbackV1 = void(ROCK_CALL*)(const AnimationPhaseContextV1*, void*);

    // Permission bits are local to this interface major. Discovery grants none.
    enum class PermissionV1 : std::uint32_t { Read=1, Callbacks=4 };
    inline constexpr std::uint32_t kSupportedPermissions = 5;

    struct EventV1 {
        std::uint32_t size{sizeof(EventV1)};
        std::uint32_t kind{};
        std::uint64_t sequence{};
        std::uint64_t frameIndex{};
        std::uint64_t ownerToken{};
        std::uint32_t worldGeneration{};
        std::uint32_t skeletonGeneration{};
        std::uint32_t providerGeneration{};
        std::uint32_t lifecycleFlags{}; std::uint32_t reason{}; std::uint32_t providerReady{};
    };
    enum class EventKindV1 : std::uint32_t { Lifecycle=1, OwnerRevoked=2, PhysicsInitialized=3, PhysicsShutdown=4 };

    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        Status(ROCK_CALL* getModVersion)(OwnerToken ownerToken, const char** outValue) noexcept;
        Status(ROCK_CALL* isProviderReady)(OwnerToken ownerToken, std::uint32_t* outValue) noexcept;
        Status(ROCK_CALL* getFrameSnapshot)(OwnerToken ownerToken, SnapshotV1* outSnapshot) noexcept;
        Status(ROCK_CALL* registerConsumerV1)(const RegistrationV1* registration, OwnerV1* outHandle) noexcept;
        Status(ROCK_CALL* unregisterConsumerV1)(std::uint64_t ownerToken) noexcept;
        Status(ROCK_CALL* registerAnimationPhaseCallbackV1)(std::uint64_t ownerToken, PhaseCallbackV1 callback, void* userData, std::uint64_t* outCallbackToken) noexcept;
        Status(ROCK_CALL* unregisterAnimationPhaseCallbackV1)(std::uint64_t ownerToken, std::uint64_t callbackToken) noexcept;
        Status(ROCK_CALL* registerFrameCallbackForOwnerV1)(std::uint64_t ownerToken, FrameCallbackV1 callback, void* userData, std::uint64_t* outCallbackToken) noexcept;
        Status(ROCK_CALL* unregisterFrameCallbackForOwnerV1)(std::uint64_t ownerToken, std::uint64_t callbackToken) noexcept;
        // Family-local publication identity; compare before combining readbacks.
        Status(ROCK_CALL* getSample)(OwnerToken owner, SampleV1* outSample) noexcept;
        Status(ROCK_CALL* bindInterface)(OwnerToken, InterfaceId, std::uint32_t exactMajor, std::uint32_t permissions) noexcept;
        Status(ROCK_CALL* copyEvents)(OwnerToken, std::uint64_t afterSequence, EventV1*, std::uint32_t capacity, StreamV1*) noexcept;
    };
}

#pragma pack(pop)
```

</details>

## Bootstrap and consumer helper

`Discovery.h` defines the sole discovery export. `Client.h` is an optional consumer-side helper with explicit `close()`; it does not automatically unregister on destruction. Keep its owner and callback context alive until successful teardown.

<details>
<summary>Complete public declarations: ROCK/Discovery.h</summary>

```cpp
#pragma once
#include "Abi.h"
#pragma pack(push, 8)
namespace rock::api {
    struct InterfaceDescriptorV1 {
        std::uint32_t size{sizeof(InterfaceDescriptorV1)};
        InterfaceId interfaceId{};
        std::uint32_t major{};
        std::uint32_t minor{};
        std::uint32_t tableByteSize{};
        std::uint32_t requiredCoreMajor{1};
        std::uint32_t requiredCoreMinor{};
        std::uint32_t reserved{};
        const void* table{};
    };
    using QueryInterfaceV1 = Status(ROCK_CALL*)(InterfaceId, std::uint32_t exactMajor,
        std::uint32_t minimumMinor, std::uint32_t minimumTableBytes,
        const InterfaceDescriptorV1** outDescriptor) noexcept;
    inline constexpr char kQueryExportName[] = "ROCKAPI_QueryInterfaceV1";
    static_assert(sizeof(InterfaceDescriptorV1)==40);
}

#pragma pack(pop)
```

</details>

<details>
<summary>Complete public declarations: ROCK/Client.h</summary>

```cpp
#pragma once
#include "Discovery.h"
#include "Core.h"
#include <cstring>

namespace rock::api {
    // Consumer-side helper only. It is not part of the DLL ABI and imports no
    // feature tables beyond Core. Pass the query export resolved from ROCK.dll.
    class Client {
        QueryInterfaceV1 _query{};
        const core::ApiV1* _core{};
        OwnerToken _owner{};
    public:
        Client()=default;
        Client(const Client&)=delete;
        Client& operator=(const Client&)=delete;
        // Close on the game thread before destroying callback userdata. Plugin
        // DLL unload with live callbacks is outside this process-lifetime API.
        Status connect(QueryInterfaceV1 query,const char* name) noexcept {
            if (_owner) return Status::Busy;
            if (!query || !name) return Status::InvalidArgument;
            core::RegistrationV1 registration{};
            std::size_t length=0;
            while (length<sizeof(registration.modName) && name[length]) ++length;
            if (!length || length==sizeof(registration.modName)) return Status::InvalidArgument;
            std::memcpy(registration.modName,name,length);
            const InterfaceDescriptorV1* descriptor{};
            auto status=query(core::kInterfaceId,1,0,sizeof(core::ApiV1),&descriptor);
            if (status!=Status::Ok) return status;
            if (!descriptor || descriptor->size!=sizeof(InterfaceDescriptorV1) ||
                descriptor->interfaceId!=core::kInterfaceId || descriptor->major!=1 ||
                descriptor->tableByteSize<sizeof(core::ApiV1) || !descriptor->table) return Status::InvalidSize;
            const auto* coreApi=static_cast<const core::ApiV1*>(descriptor->table);
            core::OwnerV1 owner{};
            status=coreApi->registerConsumerV1(&registration,&owner);
            if (status!=Status::Ok) return status;
            _query=query; _core=coreApi; _owner=owner.ownerToken;
            return Status::Ok;
        }
        template<class Table> Status acquire(std::uint32_t permissions,const Table*& output) const noexcept {
            output=nullptr;
            if (!_owner || !_query) return Status::OwnerNotRegistered;
            const InterfaceDescriptorV1* descriptor{};
            auto status=_query(Table::interfaceId,Table::majorVersion,Table::minorVersion,sizeof(Table),&descriptor);
            if (status!=Status::Ok) return status;
            if (!descriptor || descriptor->size!=sizeof(InterfaceDescriptorV1) ||
                descriptor->interfaceId!=Table::interfaceId || descriptor->major!=Table::majorVersion ||
                descriptor->minor<Table::minorVersion || descriptor->tableByteSize<sizeof(Table) ||
                descriptor->requiredCoreMajor!=core::kMajor || descriptor->requiredCoreMinor>core::kMinor || !descriptor->table) return Status::InvalidSize;
            status=_core->bindInterface(_owner,Table::interfaceId,Table::majorVersion,permissions);
            if (status==Status::Ok) output=static_cast<const Table*>(descriptor->table);
            return status;
        }
        Status close() noexcept {
            if (!_owner) return Status::Ok;
            const auto status=_core->unregisterConsumerV1(_owner);
            if (status==Status::Ok || status==Status::OwnerNotRegistered) {
                _owner=0; _core=nullptr; _query=nullptr;
            }
            return status;
        }
        [[nodiscard]] OwnerToken owner() const noexcept { return _owner; }
        [[nodiscard]] const core::ApiV1* core() const noexcept { return _core; }
    };
}
```

</details>
