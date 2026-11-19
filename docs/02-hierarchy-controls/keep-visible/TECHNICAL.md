# Keep Visible — Technical

**Latch v2.0.3 · Source-backed reference**

## Widget ownership

`LatchLabelColumn.h` implements `SKeepVisiblePin` within the normal Actor/Folder label area. The reserved slot prevents label jitter; hover visibility is configurable, while enabled pins remain white.

## Data ownership

<!-- Media target: IMAGE | keep-visible-identity | docs/assets/media/keep-visible-identity.png -->
<figure class="doc-media" data-kind="image" data-media-id="keep-visible-identity">
  <img src="../../assets/media-placeholder.png" alt="Diagram of an item identity key persisting while its hierarchy position changes" loading="lazy">
  <figcaption>A Keep Visible key belongs to an Actor or Folder identity, not the row's current screen position.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Diagram the same Actor GUID/world package identity moving from Folder A to Folder B while its KeepVisible key remains in ULatchSubsystem. Show the Outliner row being recreated without changing the pin's logical state.</span></figcaption>
</figure>


`ULatchSubsystem::IsExposureLatched` and `SetExposureLatched` maintain a set of stable item keys. `MakeItemKey` prefers Actor GUID + world package and Folder GUID where available, with name/path fallbacks. The current config key is `Latch.State.V3` → `KeepVisible`.

## Reconciliation

`FRegistry::UpdateOutliner` includes pinned items in required exposure targets, then derives the minimum visible ancestor paths. A pin does not invoke `AActor::SetActorHiddenInGame`, `SetIsTemporarilyHiddenInEditor`, transforms or hierarchy mutation.

## Scope

Keep Visible is meaningful for Actor and Folder rows, including leaves. Expansion Latches, unlike pins, require actual children. Persistent pins are editor user configuration and are not saved in a cooked map.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
