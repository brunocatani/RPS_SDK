# RPS SDK

RPS SDK is the standalone public SDK and example repository for the local VR
mod stack. Each provider owns one directory under `SDK/`, allowing independent
headers, documentation, buildable consumers, and contract tests to coexist
without coupling the SDK release history to a runtime plugin repository.

The initial module is `SDK/ROCK`. It contains:

- the complete V1 public headers;
- the 90-slot API reference and lifecycle documentation;
- focused integration recipes;
- a shared FO4VR/F4SE example runtime;
- 17 buildable generic example plugins;
- a configure-time FO4VR loader compatibility gate.

Runtime project names used to derive behavior are intentionally absent from the
public examples. Each example describes a generic capability that another mod
can adopt.

## Layout

```text
RPS_SDK/
├── SDK/
│   └── ROCK/
│       ├── include/
│       ├── docs/
│       └── examples/
├── tests/
├── CMakeLists.txt
└── CMakePresets.json
```

Future public SDK modules should use the same `SDK/<PROVIDER>/` boundary.

## Validate

Set `VCPKG_ROOT` when vcpkg is not otherwise configured. The local Windows
fallback is `C:/vcpkg` when that installation exists. In the monorepo the build
discovers the sibling CommonLibF4VR checkout automatically; elsewhere, pass
`-DCOMMON_LIB_F4VR_PATH=<path>` during configuration.

```powershell
cmake --preset custom-fast
cmake --build --preset custom-fast --target ROCKSDKExamplePlugins -- /m:1 /p:CL_MPCount=2
ctest --preset custom-fast -j 4
```

The example build never deploys DLLs. Copy and rename only the example target
you are adapting.
