# Current PAPER behavior and compatibility

Checked against PAPER source `05a952f` on 2026-09-11. PAPER's public V1 table
remains 50 function pointers / 400 x64 bytes. Recent reload fixes change runtime
coordination, not the public table version. The supported consumer header is
`PAPERApi.h`, the runtime is `PAPER.dll`, and the normal configuration is
`Documents/My Games/Fallout4VR/PAPER_Config/PAPER.ini`.

## Reload and presented-hand baselines

Reload hand animation is anchored to a generation-matched ROCK firing-grip
baseline. When the complete two-hand grip query lacks a baseline, PAPER can use
the canonical firing seat; absent an explicit grip target it reads the presented
hand instead of treating the physics frame as the rendered hand. Failed reads
do not supply a valid baseline. This covers one-hand and visual-only pistol
support without reading a deferred visual claim as if it were already presented.

The vanilla SMG partial-reload support trajectory receives the primary baseline
translation into its corrected model frame. This adjustment preserves authored
rotation, scale and motion trajectory, is weapon-specific, and requires the
matching primary baseline. It is not a generic translation to apply to every
weapon or a transform consumers should apply a second time.

PAPER suppresses its animation authority while fixed-surface climbing is active
or the required hand state cannot safely establish compatibility. Observation
and authority are separate: an animation lease does not override ROCK lifecycle
and climbing safety gates.

## Manual reload and classification

`[Reload] bManualReloadOnly` is part of current runtime configuration. Native
validation failure leaves vanilla automatic reload active and logs that outcome.
Do not promise manual-reload enforcement solely from the configured value.

Weapon classification mirrors authored source evidence; removed synthetic
fallbacks must not be reconstructed from weight or assumed type. Consumers
should honor classification source/validity flags and handle unknown evidence.

## Attach-only manipulation

PAPER maps ROCK's per-hand part report into
`PaperWeaponManipulationHandFlagV1::AttachOnly`. It reports attachment to a part,
not a carry anchor, firing ownership, a generic IK writer, or permission to move
the weapon. Keep observation sequence and weapon generation matched while
copying manipulation progress. Input toggle policy belongs to ROCK: attach-only
parts release with the physical grab button.

## Development work stays opt-in

The existing development-capture ceiling, API-activation gate, renewable lease,
cache access policy and budgets still apply. Passive telemetry must not enable
harvesting, motion-cache writes or background acquisition. The live configuration
watcher is opt-in; ordinary edits apply at the documented lifecycle/restart path
when it is disabled. See [DevelopmentCaptureAndStorage.md](DevelopmentCaptureAndStorage.md).

These are source-verified contracts. The SDK build does not certify in-game
alignment, capture completeness or reload quality for a particular weapon.
