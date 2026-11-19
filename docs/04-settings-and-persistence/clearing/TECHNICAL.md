# Clearing Latch State — Technical

**Latch v2.0.3 · Source-backed reference**

## State management API

`ULatchSubsystem` exposes `ClearExpansionState`, `ClearItem`, `ClearActorState`, `ClearAllExpansionStates`, `ClearAllExposureStates`, `ClearAll` and `ClearSavedStateFromDisk`.

## Branch cleanup

`FRegistry::ClearBranch` and `ClearBranches` traverse live Outliner hierarchy items, filter supported tree entries and remove their matching keyed states. This is an imperative editor operation over existing tree relationships, not a change to Actor attachments.

## Persistent config cleanup

`ClearSavedStateFromDisk` removes `Latch.State.V3` from `GEditorPerProjectIni`. With persistence enabled, a later state change can save remaining in-memory values again; clearing saved records and clearing current live state should be distinguished.

## UI sources

`FRegistry::PopulateContextMenu` supplies contextual clear commands. `ULatchSettings::ClearSavedLatchState` exposes the Project Settings button.

## No content damage

These functions do not delete Actor/Folder objects. Native Unreal Delete and Latch Clear are independent operations; confirm which submenu you are executing.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
