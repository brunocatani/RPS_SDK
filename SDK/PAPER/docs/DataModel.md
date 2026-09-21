# Data model and coherent copying

PAPER publishes value-only snapshots. No public record transfers ownership of
an engine node, animation graph, clip, Havok body, or runtime allocation.

## Identity layers

- `paperProviderGeneration` invalidates owner and callback assumptions after a
  shutdown/reinitialization, not an ordinary RuntimeReset event.
- `worldGeneration`, `skeletonGeneration`, and `rockProviderGeneration`
  describe upstream runtime identity.
- `weaponGenerationKey` invalidates all weapon-specific catalog and motion
  assumptions.
- `catalogSequence` identifies a stable catalog revision used by paginated copy
  calls.
- `snapshotSequence` identifies one coherent frame or diagnostic snapshot.
- `frameIndex` orders completed PAPER frames but is not a replacement for the
  generation keys above.

## Safe copy pattern

1. Query the catalog/frame state.
2. Capture its generation key and sequence.
3. Copy a bounded page with the same sequence.
4. On `StaleSnapshot`, discard the partial aggregate and restart from step 1.
5. Advance `firstNode`, `firstPart`, `firstClip`, or the relevant cursor by the
   returned `outCopied` count.
6. Stop at the count published by the state, or on zero copied rows.

Never mix pages from different sequences. `OutOfRange` means the requested
index/child identifier is not valid for that snapshot, not that a new catalog
should be guessed.

## Transforms and units

`PaperTransformV1` stores a 3x3 rotation, translation in Fallout game units,
and uniform scale. `PaperReloadQsTransformV1` stores translation xyz,
quaternion xyzw, and independent xyz scale.

Reload observations distinguish node-local and weapon-local transforms.
Animation tracks explicitly report `RigBoneLocal` or `WeaponRootLocal` space.
Do not combine transforms whose spaces or generation keys differ.

## Bounded limits

The V1 contract caps consumers/callbacks at 16, captured transforms at 192,
reload nodes at 2048, evidence records at 100, stage keys at 24, followers at
10, and motion recorders at 128. Query the relevant limits structure rather
than allocating from public maxima when a smaller runtime bound is published.
