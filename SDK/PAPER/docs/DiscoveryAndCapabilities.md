# Discovery and capabilities

Discovery and authorization are separate checks. Table extent proves a slot is
present. Provider feature bits advertise an appended family. Registration
grants the owner permission to call that family. Production consumers check all
three when an optional family is used.

## Consumer capabilities

| Capability | Grants |
|---|---|
| `RuntimeState` | Runtime and configuration snapshots. |
| `CapturedTransforms` | Native animation transform capture. |
| `NativeHandPose` | Hand-in-weapon and 15 finger-local transforms. |
| `AnimationAuthority` | Leased or explicit animation-ownership arbitration. |
| `FrameCallbacks` | PAPER event callback registration. |
| `ReloadObservations` | Reload catalog, evidence records, and frame observations. |
| `ReloadEvidenceGeometry` | Bounded evidence point copying. |
| `ReloadAnimationEvidence` | Passive telemetry plus exact off-screen clip evidence demand. |
| `ReloadAnimationTelemetry` | Passive graph, live clip, track, marker, and trigger telemetry only. |
| `ReloadStageIdentification` | Aggregate and per-part reload-stage state. |
| `NativePosePipeline` | Read-only resolved pose-pipeline snapshots. |
| `WeaponMotionCatalog` | Parts, stages, 24-key paths, followers, and follower paths. |
| `WeaponMotionDiagnostics` | Learning recorders and compiled-store diagnostics. |
| `WeaponManipulationTelemetry` | Per-frame and per-hand motion-path projection. |
| `DevelopmentCaptureControl` | Lease-bound development capture requests and status. |

Never request `All` as a shortcut. It can create demand for work your mod does
not consume. Request the smallest stable set and treat a partial grant as a
failed integration unless your mod has an explicit reduced mode.

## Feature and extent checks

The current descriptor advertises feature bits for reload observations,
geometry, animation evidence, passive telemetry, stage identification, native
pose, motion catalog, motion diagnostics, manipulation telemetry, and
development capture.

Named helpers cover the appended families:

- `supportsReloadAnimationEvidenceV1`
- `supportsReloadAnimationTelemetryV1`
- `supportsReloadStageIdentificationV1`
- `supportsNativePosePipelineV1`
- `supportsWeaponMotionCatalogV1`
- `supportsWeaponMotionDiagnosticsV1`
- `supportsWeaponManipulationTelemetryV1`
- `supportsDevelopmentCaptureControlV1`

Reload observation and geometry consumers should initialize with
`PAPER_PROVIDER_API_V1_RELOAD_OBSERVATION_TABLE_BYTES`, inspect
`PaperReloadObservationLimitsV1::featureBits`, and then check their granted
capability. Base-table discovery alone does not prove appended slots exist.

## Owner lifetime

An owner token belongs to its registration and provider generation. Explicit unregistration and full provider shutdown/reinitialization retire it. Normal `RuntimeReset` preserves registrations/callbacks while clearing authority, capture leases and observation state. Discard old generation-dependent data and rearm deliberately; do not create duplicate registrations on ordinary resets.

`PresentationComplete` observation requires `FrameCallbacks` plus `NativePosePipeline`. It follows final native-pose publication and does not advance the control lease clock.
