# First Use — Technical

**Latch v2.0.3 · Source-backed reference**

## Where the UI comes from

`FLatchModule` registers the Outliner label column and an observer column through `FSceneOutlinerModule`. `LatchLabelColumn.h` inserts a reserved Keep Visible pin slot into each eligible row; it keeps labels stable when the inactive pin is hidden.

## Input dispatch

<!-- Media target: IMAGE | first-use-gesture-routing | docs/assets/media/first-use-gesture-routing.png -->
<figure class="doc-media" data-kind="image" data-media-id="first-use-gesture-routing">
  <img src="../../assets/media-placeholder.png" alt="Diagram of mouse gestures routed to Latch's input processor and registry" loading="lazy">
  <figcaption>Normal, Alt and Shift expansion interactions take separate paths through Latch and the native Outliner.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Illustrate a flow from native Outliner expander hit testing through FInputProcessor to normal click, Alt-click ToggleLatch and Shift-click recursive expansion. Show pin clicks separately through SKeepVisiblePin and FRegistry.</span></figcaption>
</figure>


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
