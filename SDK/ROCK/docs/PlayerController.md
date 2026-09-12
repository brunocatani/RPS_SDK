# Logical input and player-controller access

These V1 additions are guarded by table extent and per-owner capability. They
expose copied values and a narrow native jump request; they do not grant generic
controller movement ownership.

| Slot | Function | Minimum bytes | Capability |
| --- | --- | --- | --- |
| 90 | `getLogicalInputActionStateV1(owner, action, outState)` | 728 | `InputObservability` |
| 91 | `getPlayerControllerStateV1(owner, queryFlags, outState)` | 736 | `PlayerController` |
| 92 | `requestPlayerControllerJumpV1(owner, request)` | 744 | `PlayerController` |

Use the named `ROCK_PROVIDER_API_V1_LOGICAL_INPUT_ACTION_STATE_TABLE_BYTES`,
`ROCK_PROVIDER_API_V1_PLAYER_CONTROLLER_STATE_TABLE_BYTES`, and
`ROCK_PROVIDER_API_V1_PLAYER_CONTROLLER_JUMP_TABLE_BYTES` constants and matching
`supportsLogicalInputActionStateV1`, `supportsPlayerControllerStateV1`, and
`supportsPlayerControllerJumpV1` helpers.

## Logical Jump observation

`RockProviderLogicalInputActionV1::Jump` is the currently defined action. Its
state reports availability/reason, held level, sample and press sequences,
sample age, frame and world/skeleton/provider generations. It observes the
configured logical action, so a consumer need not guess a controller button ID.
Reset edge state on unavailability or lifecycle change; never reconsume a press
just because another render or physics callback occurred.

## Controller snapshot

`RockProviderPlayerControllerStateV1` reports implementation, support state,
position, velocity, support normal, radius/height in game units, flags and
generation identity. Query flags choose the optional bounded penetration check.
The returned flags indicate what was resolved; zero/default fields alone do not
establish a usable controller, valid normal or a clear penetration query.
There is no public controller pointer to retain.

## Guarded jump

`RockProviderPlayerControllerJumpRequestV1` contains requested height in game
units and nonzero world/skeleton/provider generation guards. Height must be
finite, greater than zero and at most 256 game units. Submit from the owning provider callback while
the current lifecycle allows the action. ROCK validates owner/capability,
request shape, height, lifecycle and controller/penetration conditions before
entering its native jump path. Inspect the result and subsequent controller
state; this call is not an interaction-command queue and returns no command ID.
It is not a teleport, velocity setter, arbitrary movement mode or authorization
to write controller internals.
