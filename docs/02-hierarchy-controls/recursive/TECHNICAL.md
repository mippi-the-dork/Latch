# Recursive Expansion — Technical

**Latch v2.0.3 · Source-backed reference**

## Native hierarchy interaction

`FRegistry::HandleRecursiveExpanderClick` enumerates the Outliner tree and applies expansion changes while checking each item's stored latch state. `ELatchExpansionState` and live child checks determine protected branches.

## Logical versus visible tree

The method operates on the intended logical hierarchy. When the resulting logical collapse conflicts with pinned/selected targets, `UpdateOutliner` can derive Mixed bridges. This is why a visually exposed path does not prove a recursive collapse was ignored.

## Limits

Recursive expansion can touch many loaded Outliner items. Latch avoids turning this into level modification: the operation affects tree presentation. The registry's event-driven invalidation and fallback audit handle subsequent tree refreshes. Large nested hierarchies need actual editor performance measurement rather than a source-only guarantee.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
