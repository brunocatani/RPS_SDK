# Consumer validation and cleanup

Validate a consumer against the family contracts it actually uses.

1. Negotiate the exact major, minimum minor and table bytes; check descriptor identity and Core dependency before casting. Bind every required local permission and fail closed on any missing family.
2. Register one stable owner name. Keep callback code/userdata alive until successful owner-thread removal. Do not call ROCK from synchronous Grab listeners or Configuration visitors.
3. Initialize sized records and arrays. Check `Status`, then validity, counts, frame/generations and feature identity. `RequestQueued` is admission only.
4. Use Grab for occupancy and Hands for physical firing roles. Use `Presented` for final same-frame poses. Keep early control writes out of observation-only phases.
5. Bound event processing and handle stream loss. Keep one cursor per owner/family and reset on owner replacement. Do not mistake other scopes' sequence gaps for lost contacts.
6. Renew only the authority your feature currently owns. Clear drives, visuals, suppression, reservations, scopes and diagnostics on normal shutdown and lifecycle loss. Clear registered external bodies before destroying them.
7. Remove callbacks and unregister the owner on the permitted thread; retain state if teardown returns an error. Do not unload a live callback DLL.
8. Compile examples against current headers and test missing provider, permission failure, generation reset, duplicate ownership, lease expiry and command rejection. A successful build is not an in-game test.

Your plugin also owns F4SEVR bootstrap validation. Check the executable module version, not the loader's flat-compatibility query value, and validate live bytes before any plugin-owned binary patch. None of the ROCK family grants authorizes an unrelated patch or native pointer lifetime.
