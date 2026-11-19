# Multi-Selection — Technical

**Latch v2.0.3 · Source-backed reference**

## Target resolution

`FRegistry::GetDirectInteractionItems` resolves the clicked row and active Outliner selection. `ToggleLatchForInteraction` uses the clicked row's state/intent to set the batch, rather than independently toggling each parent.

## Child and item checks

`SetLatchForItems` filters supported rows and ignores items without children when authoring a latch. `SetKeepVisibleForItems` limits pins to Actor and Folder rows.

## Pin aggregation

`ToggleKeepVisibleForInteraction` examines the selected eligible pins: if all are enabled, the operation clears them; otherwise it sets them. This prevents each row from oscillating independently from a mixed state.

## Menu operations

The Outliner context-menu integration gathers the selected `FSceneOutlinerTreeItemID` values, resolves live items before executing a command and uses the same registry operations; it should not operate on stale row widgets.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
