# API version matrix

This snapshot describes the current source ABI, checked 2026-09-11.
A provider mod-version string alone does not establish support for newer V1 calls.

| API | Status | Binary contract | Current surface |
| --- | --- | --- | --- |
| V1 | Current | Append-only, 99 x64 function pointers / 792 bytes | Complete public hand, weapon, contact, input, animation, authority, command, scoped publication, touch-grab, raycast, guarded player-controller, reference details, Power Armor classification/point grabs, and diagnostic surface. |

## Compatibility rules

`ROCK_PROVIDER_API_VERSION` remains `1` while new functions are appended and structures are prefix-extended. A V1 version match alone does not prove that a newer slot exists.

Consumers requiring a newer family should initialize with its named `ROCK_PROVIDER_API_V1_*_TABLE_BYTES` constant. Consumers supporting multiple provider revisions can initialize V1 without a minimum extent and then use `getProviderLimitsV1`/`getProviderLimitsExtV1` plus the inline `supports...V1` helpers.

The safe descriptor is the only supported way to require a nonzero minimum table extent. Legacy provider-table discovery remains available for consumers that use only the original V1 prefix.

Public structures carry `size`/`version` where extensibility requires it. Initialize structures with their default constructor, preserve zeroed reserved fields, and use `getPublicStructureSizeV1` when adapting to another header revision.

## Provider and game versions

The PA additions occupy slots 95–98 after the original 760-byte input-context
boundary. Require `ROCK_PROVIDER_API_V1_POWER_ARMOR_TABLE_BYTES` and the relevant
capability grants; an older V1 provider can load successfully while lacking this
family. PA request/output structures currently require exact size/version;
`getPublicStructureSizeV1` exposes their sizes for compatibility checks.

The SDK provider mod version and the API version are separate values. The example F4SE plugins additionally gate FO4VR identity and `Fallout4VR.exe` file version `1.2.72.0`; they do not confuse that executable version with F4SEVR's loader compatibility runtime.
