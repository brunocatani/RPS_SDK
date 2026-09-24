# Discovery and interface permissions

The current API negotiates 13 families independently. Include `ROCK/Discovery.h`, `ROCK/Client.h` and only the feature headers your mod consumes. See the [interface/version map](ApiIndex.md).

## Discovery

Resolve `ROCKAPI_QueryInterfaceV1` from the already-loaded `ROCK.dll`, with the function-pointer type in `ROCK/Discovery.h`:

```cpp
Status ROCK_CALL query(
    InterfaceId interfaceId,
    std::uint32_t exactMajor,
    std::uint32_t minimumMinor,
    std::uint32_t minimumTableBytes,
    const InterfaceDescriptorV1** outDescriptor) noexcept;
```

The success result is `Status::Ok`. Check it before reading the descriptor. Check descriptor size, identity, exact major, compatible minor, table byte size, Core dependency and table pointer before casting. The SDK client performs these checks. Tables and descriptors are immutable for the lifetime of the loaded DLL. Discovery requires no world, skeleton, owner registration or physics instance; it does not establish gameplay readiness.

Unknown interface, unsupported major, insufficient minor and insufficient table extent have distinct status values. All unsuccessful queries with a valid output address store a null descriptor. Major zero and table extent zero are invalid requests. A V1 request is never satisfied with V2. The negotiation implementation supports multiple descriptors for different majors of the same family; production currently exposes only the implemented V1s.

The old `ROCKAPI_GetApi`, `ROCKAPI_GetProviderApi`, `ROCKAPI_GetDescriptorV1` and `GetROCKConfigurationApi` exports have been removed from the current ROCK runtime. There is no forwarding table or fallback export that disguises the new layout as the old one.

## Registration and permissions

Register a nonempty, null-terminated mod name of at most 63 bytes through Core. Names identify registrations and duplicate active names return `OwnerConflict`. An owner token is issued by ROCK; never invent, serialize, reuse after unregister, or share it with another mod. Registration provides Core Read permission only.

Bind each required `(owner, interface, exact major)` through `Core.bindInterface`. Permissions are local to that interface major: Read is bit 0 and Write is bit 1. Core has Read and Callbacks (bit 2), but no Write permission. Hands and References are Read-only. Unsupported bits and an empty permission request are rejected. Binding is idempotent; adding permissions preserves resources and event cursors. Dropping permissions or changing an already-bound major requires closing the owner and creating a new registration. A rejected binding leaves the previous binding intact.

Write does not imply Read. Discovery grants no permission. Acquiring Collision Write cannot authorize Grab commands, Input suppression, or Animation publications. Cached readbacks, events and caller-visible state remain readable after an owner fault, while new writes, callbacks and bindings are refused. The consumer should stop its active behavior, unregister and reinitialize deliberately.

One unregister operation retires every resource belonging to the owner: frame and phase callbacks, synchronous grab listeners, queued commands, pending reservations, offhand reservation, external body scopes, touch targets, native-animation publications, hand visuals, weapon targets/drives, handling authority and diagnostics. Completed provider-owned PA attachments are also released. A manually attached peer hand is preserved. An ordinary successful loose-object grab may have become player-owned; unregister does not confiscate it.


## SDK helper

`Client::connect(query, name)` negotiates Core and registers the owner. `client.acquire(permissionMask, table)` negotiates the typed table at its declared major, minimum minor and full `sizeof(Table)`, then binds the family. Check every result. Request Read and Write together when a feature both publishes and observes; Write alone does not grant readback.

An acquisition failure leaves the output pointer null. Stop partially initialized behavior and close the owner on its allowed thread. `Client::close()` is explicit, preserves the owner on a failed teardown, and treats an already-removed owner as closed. Do not destroy callback userdata or unload code while registration remains live.

## Version domains

All current interfaces are major 1. Hands 1.1 adds `getRoles`; WeaponParts 1.1 adds `querySourcePath`. Their full current table sizes are checked by the client. FRIK API 2.3, the ROCK DLL version, and SDK package version are independent numbers. Core `Presented` and PAPER `PresentationComplete` are current event/phase contracts; use current matching headers and runtimes rather than inferring them from an unrelated version string.

## Compatible Weapon 1.1 discovery

Include `ROCK/WeaponV1_1.h` and acquire `weapon::v1_1::Api` with Read + Write to negotiate major 1, minimum minor 1, and the extended table size. The original `weapon::ApiV1` remains a 1.0 request with its unchanged size and behavior; both consumer types can coexist without rebuilding old mods. The extension still requires Core 1.0. An older provider returns `UnsupportedMinor`; do not access the new slots.

A committed [inventory switch](modular/WeaponV1_1.md) is ROCK-owned completion work, so owner unregister cannot roll it back or confiscate a retained player-held weapon. Uncommitted requests are cancelled; no consumer callback is kept for committed completion.
