# Configuration API

Interface ID **13**, version **1.0**. Include `ROCK/Configuration.h`; link the header-only `RPS::ROCKConfiguration` target. Namespace: `rock::api::configuration`. All table calls return `rock::api::Status`. Follow the [shared runtime contract](../RuntimeContract.md) and [discovery rules](../DiscoveryAndCapabilities.md).

Configuration owns ROCK's compiled catalog and configuration store. Consumers enumerate and write through this interface; they do not maintain a second ROCK writer. Consumer/developer ownership, default behavior, revision semantics, file-watch reload and override persistence stay in the existing store.

`revision` is an any-thread, no-I/O read. `visit` and `setValue` use the serialized configuration store and are not restricted to the FRIK animation owner thread. Schedule them on a configuration/game task outside latency-sensitive render, physics, input and animation callbacks; writes perform file I/O. Visitor records and strings are borrowed for the current invocation only; copy them before returning and do not call ROCK from the visitor. Invalid visitor behavior revokes the owning consumer.

`setValue` selects an option by its case-insensitive key and owning Group; section is a presentation label, not a second identity. A successful write is applied through the next existing configuration reload, and revision advances after application. Use the caller-provided error buffer for failure detail. Examples and compiled defaults remain references/authority according to the existing configuration contract; no production INIs ship with the SDK.

## Entry points

The order below is local to this interface. Permissions never transfer between families. Exact argument types, enums, record defaults and bounds appear in the complete declarations below.

| Function | Permission | Thread | Purpose |
| --- | --- | --- | --- |
| `revision` | Read | Any; no borrowed-callback reentry | Read the applied configuration revision without file I/O. |
| `visit` | Read | Serialized configuration task | Visit the compiled settings catalog for one owning group. |
| `setValue` | Write | Serialized configuration task | Persist one supported setting through ROCK and report validation errors. |

## Persistence and ownership

The Documents known folder owns `My Games/Fallout4VR/Mods_Config/ROCK/ROCK.ini` and optional `ROCK_Developer.ini`. `Group::Consumer` and `Group::Developer` select the owning catalog. Existing consumer files load unchanged; omitted keys use compiled defaults. A missing consumer file is generated from compiled defaults. Developer changes persist overrides only: restoring a default removes that override, and an empty developer file is removed. Loading an existing developer file preserves its supplied entries. Both files support hot reload, including developer-file creation/removal.

Reference example INIs never load at runtime, ship with the plugin, or create production files. A configuration host displays catalog defaults and descriptions; it must not copy example INIs or duplicate the catalog. Mandatory physics, collision and core input behavior is not an optional setting. `Status::Ok` from `setValue` means persistence succeeded; observe a later revision for application.

## Types, flags, bounds, and signatures

These declarations are taken from the current public header. Initialize sized records and every sized array element with `{}`; check status, validity flags and copied counts before reading results.

<details>
<summary>Complete public declarations: ROCK/Configuration.h</summary>

```cpp
#pragma once
#include "Abi.h"

#pragma pack(push, 8)
namespace rock::api::configuration {
    inline constexpr InterfaceId kInterfaceId = InterfaceId::Configuration;
    inline constexpr std::uint32_t kMajor = 1;
    inline constexpr std::uint32_t kMinor = 0;
    enum class PermissionV1 : std::uint32_t { Read=1, Write=2 };
    inline constexpr std::uint32_t kSupportedPermissions = 3;
    enum class Group : std::uint32_t { Consumer, Developer };
    enum class ValueType : std::uint32_t { Boolean, Integer, Float, String };
    struct SettingV1 {
        const char* section;
        const char* key;
        const char* value;
        const char* defaultValue;
        const char* category;
        const char* description;
        ValueType type;
        std::uint32_t overridden;
    };
    // Strings and the setting record are borrowed for this synchronous callback only.
    using VisitorV1 = void(ROCK_CALL*)(const SettingV1*, void*);
    struct ApiV1 {
        static constexpr InterfaceId interfaceId=kInterfaceId;
        static constexpr std::uint32_t majorVersion=kMajor;
        static constexpr std::uint32_t minorVersion=kMinor;
        Status(ROCK_CALL* revision)(OwnerToken, std::uint64_t* outRevision) noexcept;
        Status(ROCK_CALL* visit)(OwnerToken, Group, VisitorV1, void*) noexcept;
        Status(ROCK_CALL* setValue)(OwnerToken, Group, const char* section, const char* key,
            const char* value, char* error, std::uint32_t errorCapacity) noexcept;
    };
    static_assert(sizeof(SettingV1)==56);
}

#pragma pack(pop)
```

</details>
