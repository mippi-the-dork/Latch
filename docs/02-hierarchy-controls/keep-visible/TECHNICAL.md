# Keep Visible — Technical

**Latch v2.0.3 · Source-backed reference**

## Widget ownership

`LatchLabelColumn.h` implements `SKeepVisiblePin` within the normal Actor/Folder label area. The reserved slot prevents label jitter; hover visibility is configurable, while enabled pins remain white.

## Data ownership

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
