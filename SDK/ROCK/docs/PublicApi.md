# ROCK public API

The current runtime exports 13 independently negotiated interfaces. Start with the [interface and callable index](ApiIndex.md), [installation](Install.md), [first consumer](GettingStarted.md), and [runtime contract](RuntimeContract.md).

`ROCKAPI_QueryInterfaceV1` is the only discovery export. Register one Core owner, then bind each family's local permissions. There is no public all-features frame, global capability mask, or independent configuration export. All current majors are 1; Hands and WeaponParts are minor 1, the others minor 0.

The complete [modular overview](modular/Overview.md) explains ownership and composition. [Migration](modular/Migration.md) maps the retired combined API to current families. The old monolithic headers remain available for explicitly older consumers; they do not discover the current DLL.

Each family reference contains all methods, threading/permission information, and the exact public declarations. Source equality and callable coverage are checked by the SDK documentation test. API availability and compiler checks do not certify an in-game interaction.
