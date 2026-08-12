# Development capture and storage

PAPER's normal user mode is deliberately dormant: it plays its normal runtime
animation behavior without exact harvesting, motion compilation, persistent
cache reads/writes, live learning, or configuration watching unless the user
has enabled those policies.

## Two-layer permission model

The user's INI is the policy ceiling. A consumer request is demand beneath that
ceiling; it cannot elevate it.

1. `PaperDevelopmentCaptureModeV1` selects User, Observe, Harvest, or Capture.
2. `PaperWeaponMotionCacheAccessV1` selects Off, ReadOnly, or ReadWrite.
3. The INI-derived allowed scope mask determines which work is permissible.
4. `AllowApiActivation` determines whether consumer capabilities and explicit
   leases may activate dormant work.
5. A consumer requests a bounded subset for 1–1200 frames and renews it while
   needed.

If API activation is disabled, even legacy high-cost capability demand remains
dormant. INI auto-start is independent and may activate allowed scopes without
a consumer lease.

## Scopes

| Scope | Meaning |
|---|---|
| `PassiveObservation` | Maintain passive reload/animation observations. |
| `ExactAnimationHarvest` | Load and sample exact off-screen weapon animation evidence. |
| `WeaponMotionCompilation` | Compile authored/learned observations into value-only paths. |
| `CompiledCacheRead` | Look up validated compiled motion records. |
| `CompiledCacheWrite` | Persist compiled value-only records. |
| `LiveMotionLearning` | Learn candidate motion strokes from live native observations. |

Dependencies are expanded by PAPER. Request the desired outcome, inspect
`allowedScopes`, `activeScopes`, and `deniedScopes`, and do not assume every bit
became active.

## Storage behavior

The 128 MiB animation sample budget is process memory for bounded raw evidence,
not a promise of persistent disk storage. The persistent weapon-motion cache
contains compiled, value-only paths and validation metadata. It does not store
native graph objects, engine pointers, or the raw animation sample catalog.

ReadOnly cache access performs no directory creation, file creation, timestamp
touching, pruning, or writes. ReadWrite uses bounded file, entry, session, and
disk budgets configured by the user, with schema/environment invalidation and
atomic replacement.

## Consumer pattern

- Ship capture control disabled in example and production defaults.
- Request `DevelopmentCaptureControl` only in an explicit development tool.
- Use a short lease and renew before half its lifetime expires.
- Show mode, cache access, allowed/active/denied scopes, lease remaining,
  preharvest state, sample bytes, clip counts, motion counts and pending writes.
- Clear the lease on stop, world transition, owner reset and user cancellation.
- Never treat `PermissionDenied` as an error in the user's installation; it is
  an intentional policy decision.
