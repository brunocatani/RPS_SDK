# ROCK configuration integration

Configuration is interface 13 of the modular API. Include `ROCK/Configuration.h`, link `RPS::ROCKConfiguration`, and acquire Read for catalog/revision observation or Read + Write for settings changes.

The [complete Configuration contract](modular/Configuration.md) owns signatures, defaults/persistence rules and visitor lifetime. `GetROCKConfigurationApi` is removed from the current runtime. A configuration host uses the compiled catalog and the owning Consumer/Developer group; it must not copy example INIs or implement another ROCK writer.

`revision` has no file I/O. `visit` and `setValue` use the serialized store and can run on a configuration task rather than FRIK's animation thread. Keep them out of latency-sensitive callbacks; never reenter ROCK from a borrowed visitor. Successful persistence is applied through the normal reload, after which revision advances.
