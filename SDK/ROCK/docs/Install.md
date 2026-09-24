# Install the ROCK SDK

Use the RPS SDK headers from the same contract generation as the ROCK runtime. The modular headers require Windows x64 and C++20; the complete example plugins use C++23 through their F4SE/CommonLib dependencies.

## Installed package

```cmake
find_package(RPS_SDK CONFIG REQUIRED)
target_link_libraries(MyPlugin PRIVATE RPS::ROCKCore RPS::ROCKHands)
```

These are header-only targets. Add the other `RPS::ROCK<Family>` targets your feature actually consumes; see the [complete list](ApiIndex.md). They neither link a ROCK import library nor install the runtime.

## Source checkout

```cmake
set(RPS_SDK_BUILD_ROCK_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RPS_SDK_BUILD_PAPER_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RPS_SDK_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/RPS_SDK)
target_link_libraries(MyPlugin PRIVATE RPS::ROCKCore RPS::ROCKHands)
```

`external/RPS_SDK` is the SDK checkout inside your own project. Include `<ROCK/Client.h>` and `<ROCK/Hands.h>` for the observer below. Feature headers include the common ABI vocabulary, without importing every other feature table.

## Runtime requirements

Your plugin owns its F4SEVR bootstrap and runtime guard. Current ROCK integrates FRIK 0.79 / API 2.3. Resolve the already-loaded `ROCK.dll` at an appropriate plugin/game lifecycle point. Absence or failed negotiation leaves the dependent feature inactive. A table pointer alone is not permission or gameplay readiness.

The SDK still contains legacy headers and `RPS::ROCK`; those do not negotiate this modular contract. `RPS::SDK` aggregates include paths for ROCK, PAPER and UI, but new modular code should name its family targets explicitly for language requirements and dependency clarity.

Next: [first consumer](GettingStarted.md), then [runtime rules](RuntimeContract.md).

## Opt into inventory equip

For explicit right/left inventory equip and retained switching, link `RPS::ROCKWeapon` and include `ROCK/WeaponV1_1.h`. Request `weapon::v1_1::Api` through the SDK client; installing a header does not establish that the loaded DLL provides minor 1. Existing consumers using `ROCK/Weapon.h` remain on the compatible 1.0 prefix. See [the full integration](modular/WeaponV1_1.md).
