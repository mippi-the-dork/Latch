# Hierarchy Reconciliation and Performance — Technical

**Latch v2.0.3 · Source-backed reference**

## Event-driven scheduling

`LatchInteraction.cpp` documents row lifecycle maintenance separately from hierarchy reconciliation. `NextRefreshTime`, `ScheduleHierarchySettle`, and `FRegistry::Invalidate` coalesce requests so expensive work is not needed on every Slate paint.

## Safety pass

The source declares `FallbackRefreshSeconds = 3.0`. This slow fallback audits cases in which native or third-party Outliner changes lack a shared invalidation event. It should not be described as a three-second fixed delay on every normal action.

## Registry lifetime

`RegisterOutliner` and `RegisterRow` track live Outliner and row widgets. `RefreshRows` and `UpdateOutliner` update semantic expansion presentation; `RestoreRecord` and `RestoreRowsForOutliner` clean up transient row modifications.

## Hierarchy reconciliation cost

`UpdateOutliner` gathers current tree items and required targets when reconciling. It also avoids attempting hierarchy repairs while `GIsTransacting` because Undo/Redo can temporarily invalidate associations.

## Verification

No benchmark results were available in the source inspected. The [Verification Matrix](../../VERIFICATION_MATRIX.md) includes large-Outliner tests covering frame-time behavior, selection, search, rebuilding rows and third-party integration. Do not present a performance target or fixed FPS without measured evidence.

---

## Source references

- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)
- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)

[Read Design](DESIGN.md) for user workflow. **QA:** Tests remain [Not run](../../VERIFICATION_MATRIX.md) until executed.
