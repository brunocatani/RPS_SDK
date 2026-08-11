# AGENTS.md — RPS_SDK

These instructions apply to the standalone SDK repository in addition to the
workspace root contract.

## Ownership

- `SDK/<PROVIDER>/` owns public headers, documentation, examples, and SDK-only
  contract tests for that provider.
- Runtime implementation remains owned by its runtime project. Public ABI
  headers must be synchronized and verified before either side is committed.
- Public examples must use generic names and behavior. Never name the internal
  consumer project that supplied a design pattern.
- Authority-changing examples must ship inert or narrowly gesture-gated, show
  their matching clear/release path, and fail closed on lifecycle loss.
- Do not deploy example DLLs. They are build verification and teaching assets.

## Build

```powershell
cmake --preset custom-fast
cmake --build --preset custom-fast --config Release --target ROCKSDKExamplePlugins -- /m:1 /p:CL_MPCount=2
ctest --preset custom-fast -j 4
```

Run `custom-tests` as an independent second tree when validating repository
bootstrap or build-system changes. Release packaging remains user-gated.
