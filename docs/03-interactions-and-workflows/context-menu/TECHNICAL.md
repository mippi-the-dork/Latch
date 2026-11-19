# World Outliner Context Menu — Technical

**Latch v2.0.3 · Source-backed reference**

## Menu integration

<!-- Media target: IMAGE | context-menu-pipeline | docs/assets/media/context-menu-pipeline.png -->
<figure class="doc-media" data-kind="image" data-media-id="context-menu-pipeline">
  <img src="../../assets/media-placeholder.png" alt="Diagram of native Scene Outliner menu extension and live item resolution" loading="lazy">
  <figcaption>Latch adds commands to Unreal's existing menu and resolves selected tree items at execution time.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw FLatchModule's native Outliner menu modifier, Latch submenu, selected tree item IDs, FRegistry ResolveLiveItems and PopulateContextMenu. Separate actor and folder eligibility checks.</span></figcaption>
</figure>


`FLatchModule::StartupModule` extends the native Scene Outliner context menu using `OnCreateActorBrowserColumns`, its public menu modifier hook, and a `UToolMenus` dynamic section. It invokes previously bound menu modifiers before adding the Latch submenu.

## Live item resolution

The menu context yields selected row IDs. Before activation, `FRegistry::ResolveLiveItems` resolves those IDs against the current Outliner to avoid operating on stale row widgets. `FRegistry::PopulateContextMenu` tests `IsSupportedItem`, child eligibility and pin scope.

## Action ownership

`SetLatchForItems`, `LatchCurrentSelection`, `ClearBranch`, `ClearBranches`, `ClearAllExpansion`, `ClearAllKeepVisible`, `ClearAll` and `ToggleSuspend` live in `LatchInteraction.cpp`. `ULatchSubsystem` owns the persistent keyed state, so menu actions and direct mini-controls share the same state rather than keeping conflicting copies.

## Extensibility boundary

The implementation extends the native menu and does not replace the World Outliner or introduce a second hierarchy model.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
