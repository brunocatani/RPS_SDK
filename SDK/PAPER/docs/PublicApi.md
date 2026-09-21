# Public API families and types

The V1 table is grouped by append-only family. [ApiIndex.md](ApiIndex.md) lists
all slots in ABI order.

## Core

`PaperResultV1`, consumer registration/handle, runtime/config state, captured
transforms, native hand pose, animation authority, event registration and
`PaperEventV1`.

## Reload observations

Limits, catalog state, node hierarchy, evidence records/points, weapon/form
identity and classification, frame state and two-phase node observations.

## Animation evidence

Limits, catalog/live states, clips, tracks, Qs samples, annotations, triggers
and skeleton records. Acquisition and track-space enums preserve provenance.

## Reload stages and native pose

Aggregate stage state, per-part evidence, fire correlation, pose frame state,
per-hand resolved solution, target/application/compatibility enums and residual
metrics.

## Weapon motion

Limits, catalog, parts, stages, 24-key paths, followers, follower paths,
learning state, recorder diagnostics, manipulation state and compiled-store
state.

## Development capture

Mode/cache/scope/config/state enums, lease request and complete inspectable
policy/progress state. This is the only public activation control for the
development subsystems.

All records are standard-layout, trivially copyable and guarded by compile-time
size assertions in the public header. Reserved fields must remain zero in
consumer-created inputs.

## Final native-pose observations

Request `NativePosePipeline` and `FrameCallbacks`; handle `PaperEventKindV1::PresentationComplete` before querying final native-pose frame and hand solutions. `FrameComplete` is the earlier control/lease boundary. The presentation event does not advance leases. Inspect validity, frame and generation identity before using residuals or finger transforms.
