# Data ownership and coherent observations

ROCK has one runtime and one owner registry. Its public data is divided by responsibility; there is no combined all-features frame.

| Question | Owner and readback |
| --- | --- |
| Is the world/skeleton/provider usable? | Core `SnapshotV1`, lifecycle flags and generations. |
| Where are the physical hands and HMD? | Hands frames, head pose and final presented hand/finger poses. |
| Which physical hand is firing/support? | Hands `RolesV1`; assignments can change during a handoff. |
| Is a hand occupied or available? | Grab `HandInteractionStateV1`; includes native carry, loose objects and attachments. |
| What is equipped? | Weapon identity, terminal transition history, grip/handling state and composition. |
| Which exact weapon node is observed? | WeaponParts source key + weapon-generation key, source pose/path. |
| What can collide or report contact? | Collision environment, descriptors, contacts and scoped external bodies. |
| What does a touched world reference mean? | Grab target identity joined to References native/Power Armor queries. |
| Who controls a mechanism or visual pose? | Touch state or Animation authority, each with its own permissions and leases. |

## Joining readbacks

Use `SampleV1`, the record's own frame/generation fields, and feature-specific identities. Never combine values merely because two queries succeeded. World, skeleton, provider and collision generations invalidate different resources; weapon-generation keys additionally fence weapon catalogs. `publicationSequence`, `frameIndex`, event sequence and source key are different identities.

Presented hand records retain their capture frame. In early phases they can be one presentation behind the current control frame. Subscribe to Core `Presented` when same-frame final hand observation is required. Firing roles describe physical Left/Right assignment, not a swap of controller identities or the player's preferred handedness.

## Values and borrowed memory

Native nodes, references, worlds and Havok pointers do not cross these feature contracts. Source keys and body IDs must never be cast to pointers. Caller arrays remain caller-owned. Configuration strings and callback records are borrowed only during invocation. Copy the needed values into bounded storage and release all borrowed references before returning.

Consult [shared declarations](modular/Abi.md) for exact primitive types and each [family reference](ApiIndex.md) for records, flags, bounds and function signatures.
