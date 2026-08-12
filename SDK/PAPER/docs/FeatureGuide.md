# Feature guide

Choose a developer goal first, then negotiate only the table extent and
capabilities required for it.

| Goal | Minimum extent | Capability | Runtime cost |
|---|---|---|---|
| Runtime/config monitor | Base | `RuntimeState`, `FrameCallbacks` | Cheap state copies |
| Read native hand/finger capture | Base | `NativeHandPose`, `FrameCallbacks` | Existing capture only |
| Inspect captured animation nodes | Base | `CapturedTransforms`, `FrameCallbacks` | Existing capture only |
| Coordinate an external animation owner | Base | `AnimationAuthority`, `FrameCallbacks` | Mutating, leased |
| Discover reload nodes/parts/classification | Reload observation | `ReloadObservations`, `FrameCallbacks` | Passive catalog/frames |
| Copy evidence point geometry | Reload observation | `ReloadObservations`, `ReloadEvidenceGeometry` | Larger bounded copies |
| Inspect active graphs and clips | Reload animation | `ReloadAnimationTelemetry`, `FrameCallbacks` | Passive/live only |
| Acquire exact off-screen clip samples | Reload animation | `ReloadAnimationEvidence`, `FrameCallbacks` | Expensive and policy-gated |
| Observe magazine/slide/bolt/fire stages | Reload stage | `ReloadStageIdentification`, `FrameCallbacks` | Derived observation |
| Diagnose resolved PAPER hand targets | Native pose | `NativePosePipeline`, `FrameCallbacks` | Read-only diagnostics |
| Consume part motion paths | Motion catalog | `WeaponMotionCatalog`, `FrameCallbacks` | Cache/compile dependent |
| Inspect learning and cache state | Motion diagnostics | `WeaponMotionDiagnostics`, `FrameCallbacks` | May expose dormant state only |
| Track hand manipulation progress | Manipulation | `WeaponManipulationTelemetry`, `FrameCallbacks` | Read-only projection |
| Explicitly activate development work | Development capture | `DevelopmentCaptureControl`, `FrameCallbacks` | Lease and INI gated |

## Reload observations

The catalog contains the complete node hierarchy, baseline local and
weapon-local transforms, form identity, evidence roles, bounds, weapon
classification, keyword vocabulary and family evidence. PAPER owns this
taxonomy, so a PAPER-only consumer does not need another SDK header to
understand it.

Per-frame observations publish the same target in two phases:
`NativeGraphOutput` and `PostRock`. These are observations, not inferred stage
labels. Use the reload-stage family when you need aggregate semantic states.

## Animation telemetry and evidence

`ReloadAnimationTelemetry` reads loaded graph bindings, live activity, clips,
tracks, samples, annotations, triggers and skeleton records. Negotiating this
capability must not start exact clip preharvest.

`ReloadAnimationEvidence` includes the same read surface and expresses demand
for exact off-screen authored evidence. The actual work remains capped by the
user's development policy. Exact samples are weapon-root-local; passive live
samples are rig-bone-local.

## Native pose

The native hand pose family returns the captured hand-in-weapon transform and
15 finger-local transforms. The later native-pose pipeline explains PAPER's
resolved target, final presented pose, compatibility decision, application
result, motion qualification and residual error for both hands.

Both surfaces are read-only. They support diagnostics, compatibility decisions,
visualization and authoring analysis; they are not generic IK control.

## Weapon motion

The catalog publishes parts, primary/return stages, exactly 24 normalized path
keys per complete stage, and up to 10 follower paths. Records identify whether
the path came from exact authored evidence, live learning or a hydrated
compiled cache.

Manipulation telemetry projects current observed part motion onto these paths
and reports normalized progress per hand. It does not drive the part.
