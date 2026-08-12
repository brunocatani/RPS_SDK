# PAPER SDK examples

These 12 DLLs are focused teaching consumers. They are not a mod collection and
the build does not deploy them. Copy one pattern into your own plugin, give it a
unique name/version, request only the capabilities you use, and retain the
shutdown ordering.

| Example | Demonstrates |
|---|---|
| `PAPERSDKRuntimeMonitor` | Discovery, registration, callbacks, runtime/config transitions. |
| `PAPERSDKNativeHandPoseReader` | Hand-in-weapon and 15 finger-local transforms. |
| `PAPERSDKReloadCatalogDumper` | Reload catalog, evidence and PAPER-owned weapon classification. |
| `PAPERSDKReloadFrameObserver` | Coherent two-phase node observations. |
| `PAPERSDKAnimationTelemetryObserver` | Passive animation catalog/live telemetry without exact harvesting. |
| `PAPERSDKReloadStageObserver` | Magazine, slide, bolt, rest and fire stage transitions. |
| `PAPERSDKNativePoseInspector` | Read-only resolved pose and compatibility diagnostics. |
| `PAPERSDKWeaponMotionCatalogReader` | Parts, stages, 24-key paths and followers. |
| `PAPERSDKWeaponMotionDiagnostics` | Learning and compiled-store state. |
| `PAPERSDKManipulationProgressObserver` | Per-hand path projection and normalized progress. |
| `PAPERSDKDevelopmentCaptureController` | Explicit, bounded, policy-gated development capture. |
| `PAPERSDKAnimationAuthorityLease` | Temporary animation ownership coordination. |

The two mutating examples ship with their demo switches set to `false`. They
retain bounded leases and explicit clear calls even after a developer opts in.

Build the complete repository with the required local preset:

```powershell
cmake --preset custom-fast
cmake --build --preset custom-fast --target PAPERSDKExamplePlugins -- /m:1 /p:CL_MPCount=2
ctest --preset custom-fast -j 4
```

Every example uses the same FO4VR-native Query/Load bootstrap. CMake rejects a
source regression that compares the F4SE loader compatibility version with the
VR executable constant or removes the exact executable identity/version gate.
