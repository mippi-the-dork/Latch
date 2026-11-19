# Search-Aware Presentation — Technical

**Latch v2.0.3 · Source-backed reference**

## Detection

`FRegistry::UpdateOutliner` calls `IsSearchActive` and treats search as a temporary authority change. On entry, `Runtime.SearchLogicalExpansion` captures the pre-search logical flags and a `bSearchWasActive` marker.

## While searching

`RestorePhysicalOverrides` and `PresentInactive` remove temporary Mixed row suppression/semantic coloring and let Unreal's search tree present itself. The saved expansion and exposure maps are not cleared.

## On search exit

The registry reconciles the saved snapshot with current tree item IDs and rebuilds required exposure targets. This reintroduces red latch presentation and the derived yellow bridges without treating search-driven physical expansion as a new persistent state.

## Distinguish suspension

The suspended branch of `UpdateOutliner` explicitly avoids restoring a search snapshot while suspended. This prevents search cleanup from overriding deliberate temporary native expansion during suspended mode.

## Practical limit

Third-party Outliner extensions and dynamic tree rebuilds can interact with these timing paths. The source uses coalesced invalidation and a slower safety audit; live browser or editor tests are not established by source review.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
