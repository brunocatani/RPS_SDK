# Runtime contract

## Threading

Every table call except `getVersion`, `getModVersion`, and `isReady` is
game-thread-only. A wrong-thread result is `WrongThread` for result-returning
calls; older bool/count calls fail closed. Do not call PAPER from worker jobs,
render callbacks, physics callbacks, destructors on unknown threads, or static
object teardown.

PAPER event callbacks execute on the owner thread. Event and callback pointers
are valid only for the duration of the call. Copy any value needed later.

## Failure behavior

Public records are sized and versioned. Initialize every output with `{}` so its
current `size` and `version` defaults are present. Calls reject undersized or
wrong-version outputs rather than writing a partial incompatible object.

`PaperResultV1` distinguishes invalid arguments, version/size errors, provider
readiness, capacity, ownership, permission, thread, conflict, stale snapshot,
and range failures. Do not collapse all failures into “PAPER absent” in logs.

## Hot-path behavior

FrameComplete is not permission to allocate without bounds, write files, or log
every frame. Cache generation and sequence values, use fixed or reserved
buffers, react only to transitions, and copy data in documented bounded pages.

The examples rate-limit or transition-gate their output. Production consumers
should do the same.

## Authority and cleanup

Animation authority has Arms, Hands, Weapon, and combined ReloadPose flags.
A zero-frame authority request persists until clear/unregister; a nonzero
request expires after that many PAPER frame callbacks. Prefer short renewable
leases when ownership is temporary.

Authority coordinates which system may act. It does not submit an arbitrary
hand pose or IK solution. Clear authority before unregistering and whenever
your owned animation pipeline becomes unavailable.

Development capture is always lease-only: requests must use 1 through 1200
frames. See [DevelopmentCaptureAndStorage.md](DevelopmentCaptureAndStorage.md)
before using it.
