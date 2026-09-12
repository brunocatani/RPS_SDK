# RPS UI V1 API reference

Namespace: `rpsui::sdk`. Version: `RPSUI_API_VERSION == 1`. Flavor:
`RPSUI_API_FLAVOR == 0x31535052`. Export: `RPSUI_RequestApi(uint32_t)`.
The header is normative for signatures, sized fields, enum values, and defaults.
This table has metadata before its function pointers; its offsets are unrelated
to ROCK's slot table.

| Function | Contract |
| --- | --- |
| `isFrameworkReady` | Renderer ready and a ROCK provider callback registered. Does not certify depth, projection, active gameplay or delivery of the next render callback. |
| `registerConsumer` | Copy identity strings; grant requested supported bits; return an owner token. Duplicate global consumer ID is invalid. |
| `unregisterConsumer` | Remove this owner and all its panels; clear shared pointer state and update depth demand. No callback-drain guarantee. |
| `registerPanel` | Require a registered owner, unique global panel ID, dimensions/widths/flags and callback; return a closed panel handle. |
| `unregisterPanel` | Remove one matching owner/panel registration and its active interaction. |
| `submitPanelPresentation` | Open, replace pose or close; validate explicit sequence and open pose; update state sequence. |
| `resetPanelSize` | Restore default physical width and pixel-derived height; update state sequence. |
| `getPanelState` | Copy the host's resolved pose, state sequence, open state, pointer hand, validity, primary level and resize handles. |

## Capacity and validation

The host has 16 consumer registrations and 8 panels total, shared across all
mods. Panel pixel width is 640–4096; pixel height is 480–4096. Registration
requires finite positive minimum width, maximum at least minimum, and default
within that interval. String buffers are 64 bytes for IDs and 96 for names,
including the terminator. Panel IDs are global, not owner-local.

Use default-initialized structures so `structSize`/`apiVersion` and reserved
fields are correct. Open poses require finite components, an approximately
orthonormal right-handed basis, width within registered limits, positive height,
and matching physical/pixel aspect ratio. Closing does not require a valid pose.
Do not infer acceptance of malformed inputs from reserved ABI space.

`PanelPresentationV1::sequence` is a submission-order guard. The sequence in
`PanelStateV1` also changes for host state updates. `PanelRenderFrameV1` has
separate render `frameSequence` and `deviceGeneration` identities. They are not
ROCK frame indices or interchangeable clocks.

## Features and flags

| Feature | Scope |
| --- | --- |
| `MultipleWorldPanels` | Shared bounded panel registry. |
| `SceneDepthOcclusion` | Compose against validated captured scene depth. |
| `CentralPointerRouting` | Host selects a panel and physical pointer hand. |
| `ContextualTwoAxisScroll` | Routed two-axis stick input. |
| `PhysicalPanelResize` | Host-owned edge/corner dragging. |
| `ConsumerRenderCallbacks` | Consumer draws its panel texture. |
| `SharedStereoComposition` | Host composites panels into stereo output. |
| `ShapedPanels` | Transparent, fixed and circular-input panel flags. |
| `ConfigNavigation` | Opt-in pointing-hand raw stick and grip-back routing. |

Current source has a discovery inconsistency: `ConfigNavigation` is implemented
and included in consumer feature grants, but omitted from `ApiV1::featureBits`.
Request it and check `ConsumerHandleV1::grantedFeatures`; do not infer it solely
from the global bitset. This documentation records that limitation without
changing the runtime. Unsupported required grants should disable the feature.

| `PanelFlagV1` | Effect |
| --- | --- |
| `Transparent` | Clear to transparent and alpha-composite consumer drawing. |
| `FixedSize` | Disable physical resize affordances. |
| `CircularInput` | Accept hits inside the normalized inscribed circle. Does not draw a circular UI for the consumer. |
| `ConfigNavigation` | Opt in to pointing-hand raw stick and gated physical grip-back. Hold-based wheels leave it unset. |

Zero flags retain opaque rectangular resizable panels. Unknown flag bits are
rejected. `panelContains` is the shared bounds helper for rectangular/circular
input. `PhysicalHandV1` is `None`, `Left`, or `Right`; `ResizeHandleV1` names
the four edges and four corners plus `None`.

## Render frame and results

`PanelRenderFrameV1` supplies borrowed `d3dDevice`, `d3dContext`,
`renderTargetView`, pixel and physical dimensions, delta time, pointer pixel
coordinates, stick axes, pointer hand, primary/back levels and resize state.
Use pointer coordinates and clicks only when `pointerValid` is set. Derive
edges locally; `backDown` is a level, not a queued event.

`ResultV1` declares `Ok`, `InvalidArgument`, `VersionMismatch`,
`FrameworkNotReady`, `CapacityReached`, `OwnerNotRegistered`,
`PanelNotRegistered`, `OwnershipMismatch`, `InvalidPose`, `CallbackFailed`,
and `InternalError`. Declaration is not proof every result is emitted:
current registration/presentation size/version failures return
`InvalidArgument`; requesting an unsupported API version returns a null table.
Asynchronous render failure is not returned by a prior presentation call.
