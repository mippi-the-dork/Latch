# Workspace Persistence — Technical

**Latch v2.0.3 · Source-backed reference**

## Config storage

<!-- Media target: IMAGE | persistence-config | docs/assets/media/persistence-config.png -->
<figure class="doc-media" data-kind="image" data-media-id="persistence-config">
  <img src="../../assets/media-placeholder.png" alt="Diagram of Latch.State.V3 expansion, KeepVisible and Suspended records" loading="lazy">
  <figcaption>Latch stores stable keyed expansion and pin information in per-project editor configuration rather than level packages.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Create a technical diagram showing Section Latch.State.V3 with Expansion array, KeepVisible array, and Suspended boolean written to GEditorPerProjectIni; distinguish it from .umap and external Actor .uasset packages.</span></figcaption>
</figure>


`ULatchSubsystem::LoadState` and `SaveState` use section `Latch.State.V3` in `GEditorPerProjectIni`. Arrays `Expansion` and `KeepVisible` encode item keys and `ELatchExpansionState` values; the `Suspended` boolean is saved separately.

## Save conditions

`ULatchSubsystem::MarkDirty` saves when `ULatchSettings::bPersistStateAcrossEditorRestarts` is enabled. `Initialize` conditionally loads, and `Deinitialize` saves. The saved arrays are sorted for stable file output.

## Disabled persistence

`ApplyPersistenceSetting` calls `ClearSavedStateFromDisk` when persistence turns off, removing the config section while retaining current in-memory state. When enabled, current state is written.

## No Actor packages

`ULatchSubsystem` stores item identities based on Actor GUID + world package or Folder GUID/path and does not call `AActor::Modify`, change transforms or save external Actor packages merely to record a latch.

## Test boundary

Verify project-specific config-file permissions and read-only behavior with actual editor/source-control testing. The C++ storage design does not guarantee all third-party integrations and editor interruption scenarios.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
