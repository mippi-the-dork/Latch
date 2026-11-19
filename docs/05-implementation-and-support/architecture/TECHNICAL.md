# Editor Architecture — Technical

**Latch v2.0.3 · Source-backed reference**

## Module startup and shutdown

<!-- Media target: IMAGE | architecture-components | docs/assets/media/architecture-components.png -->
<figure class="doc-media" data-kind="image" data-media-id="architecture-components">
  <img src="../../assets/media-placeholder.png" alt="Technical architecture diagram of Latch's editor integration components" loading="lazy">
  <figcaption>Latch integrates the Editor module, subsystem, Scene Outliner columns, Slate input processor and menu extensions.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Create a source-class diagram naming FLatchModule, ULatchSubsystem, FRegistry, FObserverColumn, SKeepVisiblePin, FInputProcessor, UToolMenus and STreeView. Use distinct arrows for registration, keyed data reads, invalidation and row presentation.</span></figcaption>
</figure>


`FLatchModule` registers Actor Browser columns (a label extension and hidden observer), native menu extensions, an input processor, selection and Actor lifecycle callbacks. Shutdown unregisters those integrations and stops `FRegistry`. `SupportsDynamicReloading` returns false.

## Storage and source of truth

`ULatchSubsystem` derives from `UEditorSubsystem` and owns the expansion map, Keep Visible key set and suspension flag. It is initialized independently of a particular Outliner widget and uses keyed entries rather than widget pointers.

## Live presentation

`Latch::FRegistry` tracks Outliner row records and required targets, reconciles the native `STreeView` state, and maintains selective row suppression. `LatchLabelColumn.h` implements the stable Keep Visible pin slot. `LatchObserverColumn.h` supplies a hidden column used to register/observe relevant Outliner instances.

## Input and menus

`FInputProcessor` recognizes Alt/Shift gestures on expansion controls; `UToolMenus` adds Latch commands to the Scene Outliner and viewport context menus.

## No runtime module

The descriptor's `Editor` module and code's use of `UnrealEd`/Slate APIs mean the Latch UI and persistence system do not become a runtime gameplay component.

---

## Source references

- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)
- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)

[Read Design](DESIGN.md) for user workflow. **QA:** Tests remain [Not run](../../VERIFICATION_MATRIX.md) until executed.
