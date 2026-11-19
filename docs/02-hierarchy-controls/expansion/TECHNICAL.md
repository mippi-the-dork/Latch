# Expansion Latches — Technical

**Latch v2.0.3 · Source-backed reference**

## Stored state

<!-- Media target: IMAGE | expansion-state-model | docs/assets/media/expansion-state-model.png -->
<figure class="doc-media" data-kind="image" data-media-id="expansion-state-model">
  <img src="../../assets/media-placeholder.png" alt="Diagram of Unlatched, LatchedOpen and LatchedClosed subsystem values" loading="lazy">
  <figcaption>Latch records expansion intent as one of three stored states rather than saving physical tree row visibility.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw the ELatchExpansionState enum as Unlatched, LatchedOpen and LatchedClosed, linked to a saved item key in ULatchSubsystem. Separate the saved state from current STreeView physical expansion.</span></figcaption>
</figure>


`ULatchSubsystem` exposes `ELatchExpansionState::{Unlatched,LatchedOpen,LatchedClosed}`; `ToggleCurrentExpansionLatch` records the row's current **logical** expansion. `SetExpansionState` writes the keyed state and triggers editor-configuration persistence when enabled.

## Eligibility

`FRegistry::ToggleLatch` and `ToggleLatchForInteraction` require `IsSupportedItem` and `HasLiveChildren`. No red latch is authored for a leaf. Context menu SetLatch operations also reject non-parent rows.

## Reconciliation

`FRegistry::UpdateOutliner` reconciles stored latch intent with Scene Outliner tree expansion while respecting transient search and suspended modes. Physical expansion can temporarily differ from logical state when exposing a target path; that difference does not overwrite the stored expansion latch.

## Lifecycle

`ULatchSubsystem` stores actor/folder-specific keys, but parenthood can change as children move or are removed. Review the [Identity page](../../04-settings-and-persistence/identity/DESIGN.md) for what happens when a parent loses its last child.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
