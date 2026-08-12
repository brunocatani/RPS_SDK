# Version and table-extent matrix

PAPER extends API V1 additively. Consumers negotiate the last family they need;
they do not require the current full table merely to use the base contract.

| Family | Last slot | Table bytes | Feature/capability check |
|---|---:|---:|---|
| Base runtime, native capture, authority, callbacks | 14 | 120 | Requested base capability |
| Reload observations and geometry | 21 | 176 | Limits feature bits plus grant |
| Raw animation telemetry/evidence | 30 | 248 | Named support helper plus grant |
| Reload-stage identification | 32 | 264 | `supportsReloadStageIdentificationV1` |
| Native-pose pipeline | 34 | 280 | `supportsNativePosePipelineV1` |
| Weapon-motion catalog | 41 | 336 | `supportsWeaponMotionCatalogV1` |
| Weapon-manipulation telemetry | 45 | 368 | `supportsWeaponManipulationTelemetryV1` |
| Weapon-motion diagnostics/store | 46 | 376 | `supportsWeaponMotionDiagnosticsV1` |
| Development-capture control | 49 | 400 | `supportsDevelopmentCaptureControlV1` |

The preferred descriptor export is `PAPERAPI_GetProviderDescriptorV1`. The
legacy `PAPERAPI_GetProviderApi` fallback is accepted only for base-table
requests because it cannot prove an appended extent.

`PAPER_API_VERSION` is the ABI version. `PAPER_MOD_VERSION` is a diagnostic
provider build number and must not be used to infer table layout.
