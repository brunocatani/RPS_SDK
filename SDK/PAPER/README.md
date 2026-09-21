# PAPER SDK module

`SDK/PAPER` is the standalone C++ consumer contract for `PAPER.dll`. It is a
header-only cross-DLL ABI: consumers link no PAPER library and receive only
bounded, value-only records through a negotiated V1 function table.

The module includes:

- the authoritative `PAPERApi.h` consumer header;
- all 50 V1 table slots and every public capability;
- lifecycle, snapshot, reload, animation, pose, motion, and capture guidance;
- 12 buildable FO4VR/F4SE consumer examples;
- header-sync, API-index, loader, example-safety, and documentation checks.

Start with [GettingStarted.md](docs/GettingStarted.md), then use
[FeatureGuide.md](docs/FeatureGuide.md) to select the smallest capability set
for your mod. [DevelopmentCaptureAndStorage.md](docs/DevelopmentCaptureAndStorage.md)
is required reading before requesting exact animation harvesting, motion
compilation, cache access, or live learning.

Link `RPS::PAPER` when consuming this repository through CMake. The target adds
the correct include directory; it does not add a runtime dependency or install
`PAPER.dll`.

## Current source behavior

See [CurrentBehavior.md](docs/CurrentBehavior.md) for the source-verified interaction and compatibility changes as of 2026-09-11.

## Current observation timing

The public table remains V1 with 50 calls. Final native-pose data is published on `PresentationComplete`, separate from the earlier `FrameComplete` control/lease boundary. See [runtime and registration lifetime](docs/RuntimeContract.md). Current PAPER consumes the modular ROCK API; a PAPER-only consumer still includes only PAPER-owned headers.
