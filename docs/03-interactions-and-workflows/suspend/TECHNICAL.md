# Suspend and Resume — Technical

**Latch v2.0.3 · Source-backed reference**

## Stored flag

<!-- Media target: IMAGE | suspend-state | docs/assets/media/suspend-state.png -->
<figure class="doc-media" data-kind="image" data-media-id="suspend-state">
  <img src="../../assets/media-placeholder.png" alt="Diagram separating the suspended boolean from stored expansion and Keep Visible keys" loading="lazy">
  <figcaption>Suspension changes reconciliation behavior without deleting previously saved item state.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw ULatchSubsystem with independent Expansion, KeepVisible and Suspended fields; show FRegistry choosing either the inactive/native presentation branch or normal latch reconciliation depending on Suspended.</span></figcaption>
</figure>


`ULatchSubsystem::SetSuspended` sets a boolean distinct from `ExpansionStates` and `ExposureKeys`. With persistence enabled, it is stored in `GEditorPerProjectIni` under section `Latch.State.V3` and key `Suspended`.

## Runtime branch

`FRegistry::UpdateOutliner` checks `Subsystem->IsSuspended()` before applying normal latch corrections, restores physical overrides without enforcing latches, clears the search snapshot and presents rows using native Unreal behavior.

## Reapply

`FRegistry::ToggleSuspend` changes the subsystem flag and schedules a hierarchy update; subsequent reconciliation reloads saved per-item intent and recalculates Mixed View.

## Important boundary

Suspension doesn't call `ClearAll` or erase Actor/Folder keys. It is deliberately separate from persistent state clearing. The temporary visible tree state is not level data.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
