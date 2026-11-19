# Identity and Parent Lifecycle — Technical

**Latch v2.0.3 · Source-backed reference**

## Identity implementation

<!-- Media target: IMAGE | identity-keys | docs/assets/media/identity-keys.png -->
<figure class="doc-media" data-kind="image" data-media-id="identity-keys">
  <img src="../../assets/media-placeholder.png" alt="Diagram of Actor GUID and Folder GUID/path identity keys" loading="lazy">
  <figcaption>Latch associates editor presentation state with stable Actor or Folder identifiers when available.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Create an example of Actor|WorldPackage|GUID, FolderGuid|WorldPackage|GUID, and the fallback ActorPath/FolderPath key families. Show those keys mapping to expansion and pin state inside ULatchSubsystem.</span></figcaption>
</figure>


`ULatchSubsystem::MakeItemKey` uses `AActor::GetActorGuid()` and the world package for Actors with valid GUIDs, falling back to Actor object path. For Folders it prefers `FFolder::GetActorFolderGuid`, then uses folder path and world package. World items use a world package key.

## Deletion

`FLatchModule` registers `OnLevelActorDeleted` and calls `ULatchSubsystem::ClearActorState` for removed Actors. The subsystem removes matching Actor keys rather than storing a permanent tombstone.

## Parenthood changes

`FRegistry::HasLiveChildren` and `HasChildren` determine expansion eligibility; `ToggleLatch`, `SetLatchForItems` and recursive operations reject childless rows. A disappearing final child can invalidate a parent expansion latch.

## Restoration caveat

The README contains conflicting descriptions of childless-row latch cleanup. Prefer the actual current source and verify the final-child removal/readdition path in UE before promising whether an old latch survives in every tree rebuild. This documentation deliberately does **not** guarantee a resurrected red latch.

## Persistence boundary

Identity keys live in editor configuration and do not alter Actor GUIDs, hierarchy parentage, Folders, Level Packages or source-control semantics.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
