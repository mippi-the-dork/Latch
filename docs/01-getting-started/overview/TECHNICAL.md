# What Latch Does — Technical

**Latch v2.0.3 · Source-backed reference**

## Module architecture

<!-- Media target: IMAGE | overview-architecture | docs/assets/media/overview-architecture.png -->
<figure class="doc-media" data-kind="image" data-media-id="overview-architecture">
  <img src="../../assets/media-placeholder.png" alt="Diagram connecting the Latch editor module to subsystem, registry and row widgets" loading="lazy">
  <figcaption>Latch separates saved presentation intent from live Outliner row interaction and reconciliation.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw a source-labeled diagram with FLatchModule registering the Scene Outliner label/observer columns and input processor, FRegistry reconciling UI state, and ULatchSubsystem holding per-project expansion/pin/suspend data.</span></figcaption>
</figure>


`Latch.uplugin` declares one **Editor** module loaded at `PostEngineInit`, restricted to **Win64** and targeting UE 5.8.0. `FLatchModule` registers Outliner integration; `Latch::FRegistry` reconciles live row widgets and selection, and `ULatchSubsystem` owns persisted expansion and Keep Visible state.

## Logical versus rendered state

`LatchInteraction.cpp` distinguishes a row's logical expansion flag from its physical `STreeView` expansion. When a logically collapsed ancestor must expose a descendant, the registry can keep a physical bridge open and suppress unrelated child rows. That transient **Mixed View** is calculated, not saved as a third `ELatchExpansionState`.

## Storage and scope

`ULatchSubsystem` stores state keyed by Actor, Folder and World identity in `GEditorPerProjectIni`. There is no runtime module and no Actor/component field to replicate. Refer to `Source/Latch/Private/LatchInteraction.cpp` and `Source/Latch/Private/LatchSubsystem.cpp` for the current implementation.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
