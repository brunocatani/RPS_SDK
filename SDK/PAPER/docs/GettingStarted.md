# Getting started

PAPER is discovered at runtime. Include `PAPERApi.h`, negotiate the smallest
table extent your feature needs, register a named consumer, verify the granted
capability mask, and unregister every owner and callback on the PAPER game
thread before your plugin stops using the provider.

## Add the SDK

With the repository in your source tree:

```cmake
add_subdirectory(path/to/RPS_SDK EXCLUDE_FROM_ALL)
target_link_libraries(MyPlugin PRIVATE RPS::PAPER)
```

With an installed package:

```cmake
find_package(RPS_SDK CONFIG REQUIRED)
target_link_libraries(MyPlugin PRIVATE RPS::PAPER)
```

There is no PAPER import library. `RPS::PAPER` is an interface target that
publishes the header include path.

## Discover and register

```cpp
#include "PAPERApi.h"

using namespace paper::api;

const auto init = PaperApi::initialize(
    PAPER_API_VERSION,
    PAPER_PROVIDER_API_V1_RELOAD_STAGE_TABLE_BYTES);
if (init != 0 || !PaperApi::inst || !PaperApi::inst->isReady()) {
    return false;
}

PaperConsumerRegistrationV1 registration{};
std::snprintf(registration.modName, sizeof(registration.modName), "MyPlugin");
registration.requestedCapabilities =
    static_cast<std::uint32_t>(
        PaperConsumerCapabilityV1::ReloadStageIdentification) |
    static_cast<std::uint32_t>(PaperConsumerCapabilityV1::FrameCallbacks);

PaperConsumerHandleV1 handle{};
const auto result =
    PaperApi::inst->registerConsumerV1(&registration, &handle);
if (result != PaperResultV1::Ok || handle.ownerToken == 0 ||
    (handle.grantedCapabilities & registration.requestedCapabilities) !=
        registration.requestedCapabilities) {
    return false;
}
```

Do this after game data is available. Provider calls other than `getVersion`,
`getModVersion`, and `isReady` are game-thread-only.

## Initialization result codes

`PaperApi::initialize` currently returns an integer for source compatibility.

| Code | Meaning |
|---:|---|
| 0 | Descriptor or base legacy discovery succeeded. |
| 1 | `PAPER.dll` is not loaded, or the platform is not Windows. |
| 2 | The legacy provider export is absent. |
| 3 | The legacy export returned a null table. |
| 4 | The provider API version is older than requested. |
| 5 | The descriptor is invalid, its table is too short, or an appended extent was requested from a legacy-only provider. |

## Shutdown order

On the PAPER game thread:

1. Clear any development-capture request or animation authority you own.
2. Unregister event callbacks.
3. Unregister the consumer token.
4. Stop retaining sequences and provider-derived cached state.

`unregisterConsumerV1` is the final owner cleanup boundary, but explicit clear
calls make intent and runtime diagnostics unambiguous.
