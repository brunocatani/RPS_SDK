# Consumer recipes

## Transition-driven runtime monitor

Request `RuntimeState` and `FrameCallbacks`. In FrameComplete, compare config
revision, provider generation, weapon generation, reload sequences and status
flags with your retained value. Log or update UI only when something changes.

## PAPER-only weapon classifier

Request `ReloadObservations`, wait for a nonzero catalog sequence, then read
`PaperReloadCatalogStateV1::classification`. Use `primaryFamily` for one
best-effort label, `familyFlags` for multi-label behavior, keyword flags as raw
evidence, and family-evidence flags to explain provenance. Invalidate on
`weaponGenerationKey`, not just form ID.

## Reload part inspector

Copy `PaperReloadEvidenceV1` pages for the current catalog. Each record carries
part/reload/support/socket/action roles, source/interaction node IDs, bounds,
OMOD identity, attach-point identity and point counts. Request
`ReloadEvidenceGeometry` only when you truly need the point cloud.

## Passive animation viewer

Request `ReloadAnimationTelemetry`, never `ReloadAnimationEvidence`. Query the
catalog and live state, then copy clips and only the selected clip's tracks,
markers, triggers or samples. Label every track by acquisition mode and track
space. This recipe does not start exact preharvest.

## Reload-stage HUD

Request `ReloadStageIdentification`. React when `stageFlags` changes and display
independent magazine, slide, bolt, rest and fire flags. These flags may coexist.
Use `copyReloadStagePartsV1` with the same snapshot sequence when explaining
which transforms caused an aggregate state.

## Motion-path consumer

Request `WeaponMotionCatalog`. On `WeaponMotionCatalogChanged`, query the new
catalog, copy parts and stages, then copy exactly the published key count for
each stage. Copy follower descriptors and paths separately. Cache by catalog
sequence and weapon generation; discard the whole aggregate on `StaleSnapshot`.

## Manipulation progress

Request `WeaponManipulationTelemetry`. Use the frame state to detect a new event
sequence and query right/left hand state. `normalizedProgress` is the projection
onto PAPER's selected path; check flags and residual before treating it as a
high-confidence gameplay signal.

## Development authoring session

Request `DevelopmentCaptureControl` from an explicitly enabled tool. Publish a
short lease for exact animation harvest, motion compilation and the desired
cache access. Read state periodically, surface denied bits to the developer,
and clear immediately when the session closes. The user's INI remains the
authority.

## Final native-pose observations

Request `NativePosePipeline` and `FrameCallbacks`; handle `PaperEventKindV1::PresentationComplete` before querying final native-pose frame and hand solutions. `FrameComplete` is the earlier control/lease boundary. The presentation event does not advance leases. Inspect validity, frame and generation identity before using residuals or finger transforms.
