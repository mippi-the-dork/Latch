# First Use — Technical

**Latch v2.0.3 · Source-backed reference**

## Where the UI comes from

`FLatchModule` registers the Outliner label column and an observer column through `FSceneOutlinerModule`. `LatchLabelColumn.h` inserts a reserved Keep Visible pin slot into each eligible row; it keeps labels stable when the inactive pin is hidden.

## Input dispatch

`FInputProcessor::HandleMouseButtonDownEvent` and the row/registry handling in `LatchInteraction.cpp` distinguish modifiers, clicked rows and the relevant live Outliner. The registry tests whether a row has live children before latching its expansion.

## Display derivation

The red state comes from `ULatchSubsystem::GetExpansionState`. Yellow is derived by `FRegistry::UpdateOutliner` from `RequiredTargets`, logical expansion and physically exposed paths. The pin is based on `IsExposureLatched`. A source-reviewed UI gesture still needs an in-editor test as recorded in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
