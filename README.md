# RPS SDK

RPS SDK is the standalone public SDK and example repository for the local VR
mod stack. Each provider owns one directory under `SDK/`, allowing independent
headers, documentation, buildable consumers, and contract tests to coexist
without coupling the SDK release history to a runtime plugin repository.

Available modules are `SDK/ROCK`, `SDK/PAPER`, and `SDK/UI`. Together they contain:

- the current independently versioned ROCK feature headers plus PAPER and UI contracts;
- complete callable, type, permission/capability and lifecycle documentation;
- focused integration recipes;
- provider-local FO4VR/F4SE example runtimes;
- 29 buildable generic example plugins and an inert compiled UI integration fragment;
- a configure-time FO4VR loader compatibility gate.

Runtime project names used to derive behavior are intentionally absent from the
public examples. Each example describes a generic capability that another mod
can adopt.

## Layout

```text
RPS_SDK/
├── SDK/
│   ├── ROCK/
│   │   ├── include/
│   │   ├── docs/
│   │   └── examples/
│   ├── PAPER/
│       ├── include/
│       ├── docs/
│       └── examples/
│   └── UI/
│       ├── include/
│       ├── docs/
│       └── examples/
├── tests/
├── CMakeLists.txt
└── CMakePresets.json
```

Consumers may add this repository with `add_subdirectory()` and link the
explicit header-only `RPS::ROCK<Family>` targets, `RPS::PAPER`, or `RPS::UI`. The aggregate `RPS::SDK` remains available for include paths; use explicit ROCK family targets for new consumers.
Installed packages expose the same targets through
`find_package(RPS_SDK CONFIG REQUIRED)`.

## Validate

Set `VCPKG_ROOT` when vcpkg is not otherwise configured. The local Windows
fallback is `C:/vcpkg` when that installation exists. In the monorepo the build
discovers the sibling CommonLibF4VR checkout automatically; elsewhere, pass
`-DCOMMON_LIB_F4VR_PATH=<path>` during configuration.

```powershell
cmake --preset custom-fast
cmake --build --preset custom-fast --config Release --target RPSSDKExamplePlugins -- /m:1 /p:CL_MPCount=2
ctest --preset custom-fast -j 4
```

The example build never deploys DLLs. Copy and rename only the example target
you are adapting.

## Runtime boundaries

ROCK exposes 13 families and 121 table members through `ROCKAPI_QueryInterfaceV1`. Hands and WeaponParts are 1.1; the remaining families are 1.0. Each family has its own permissions. PAPER retains 50 V1 calls with separate capabilities and a final-pose `PresentationComplete` event. RPS UI has three independently discovered tables: eight panel calls, six input calls and four cooperation/retirement calls.

See [ROCK](SDK/ROCK/README.md), [PAPER](SDK/PAPER/README.md), and [UI](SDK/UI/README.md). The monolithic ROCK headers/target remain legacy support only and cannot negotiate the current DLL. FRIK API 2.3 and the static RPS Framework package version are independent version domains.

PALM owns a separate native section API in its own SDK. SCISSORS exports a limited development table whose actor-grab admission is disabled; there is no SDK/SCISSORS module. Neither is another ROCK family.
