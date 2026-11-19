# Project Settings — Technical

**Latch v2.0.3 · Source-backed reference**

## Settings class

<!-- Media target: IMAGE | settings-ownership | docs/assets/media/settings-ownership.png -->
<figure class="doc-media" data-kind="image" data-media-id="settings-ownership">
  <img src="../../assets/media-placeholder.png" alt="Diagram distinguishing Latch Project Settings from per-project editor state" loading="lazy">
  <figcaption>Appearance preferences live in Latch configuration, while saved item latches and pins live in editor workspace configuration.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Diagram ULatchSettings reading Config/DefaultLatch.ini and ULatchSubsystem saving section Latch.State.V3 to GEditorPerProjectIni. Label spacing/color/hover/persist preference separately from per-item expansion, pin and Suspend state.</span></figcaption>
</figure>


`ULatchSettings` extends `UDeveloperSettings` with `UCLASS(Config=Latch,DefaultConfig)` and display name Latch. The category is **Plugins** under Project Settings.

## Default fields

`ExpanderContentSpacing = 2.0f` with clamp 0–10; `ExpansionLatchColor = (1,0,0,1)`; `MixedStateColor = (1,1,0,1)`; `bShowKeepVisibleHoverControl = true`; and `bPersistStateAcrossEditorRestarts = true`. These defaults appear in `Config/DefaultLatch.ini`.

## Presentation consumers

The label column reads pin hover configuration, while row widget decoration and expansion controls use spacing and color settings. None of these settings modifies Actor render visibility or property data.

## Storage distinction

`ULatchSettings` stores configuration options in the `Latch` config domain, while per-item expansion/pin/suspend state is saved by `ULatchSubsystem` to the editor per-project user configuration when enabled. The two are not the same file/ownership path.

## Utility callback

The Clear Saved Latch State setting delegates to subsystem cleanup of persisted records. Avoid confusing that utility with merely unchecking persistence.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
