# Troubleshooting and FAQ — Technical

**Latch v2.0.3 · Source-backed reference**

## Symptom-to-owner map

<!-- Media target: IMAGE | troubleshooting-flow | docs/assets/media/troubleshooting-flow.png -->
<figure class="doc-media" data-kind="image" data-media-id="troubleshooting-flow">
  <img src="../../assets/media-placeholder.png" alt="Diagnostic flowchart for Latch missing controls, state and refresh issues" loading="lazy">
  <figcaption>Diagnosing Latch begins by identifying whether the issue involves UI registration, keyed persistence or live row reconciliation.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Create a branching diagram with symptom groups Column/Menu Missing → FLatchModule, State Lost → ULatchSubsystem, Yellow Path Wrong → FRegistry MixedItems, Input Gesture Wrong → FInputProcessor, Search Restore Wrong → SearchLogicalExpansion.</span></figcaption>
</figure>


| Symptom | First implementation to inspect |
| --- | --- |
| Missing Outliner controls | `FLatchModule` column and menu registration |
| Input not captured | `FInputProcessor`, expander path handling |
| Wrong red/white saved state | `ULatchSubsystem` and stable identity keys |
| Yellow path persists | `FRegistry::RequiredTargets`, `MixedItems`, forced physical expansion |
| Search undoing layout | `Runtime.SearchLogicalExpansion` snapshot path |
| Suspension confusion | `ULatchSubsystem::bSuspended` and inactive presentation |
| Stale row behavior | `RefreshRows`, `RestoreRecord`, row registration |
| Slow Outliner | `Invalidate`, `Tick`, fallback refresh and full tree scanning |

## Diagnostic method

Reproduce first in a disposable UE 5.8 level with a two-level folder and one Actor. Then test selected descendants, pins and multi-selection separately. Check whether search is active and persistence is enabled before diagnosing missing state.

## Engine boundaries

The plugin uses private native Slate and Scene Outliner behavior in several places, so another engine version or substantial Outliner plugin may alter the available interaction path. A source-level integration diagnosis requires the *actual* engine and plugin commit.

## Verification status

The troubleshooting scenarios are grounded in current code and documentation; the site does not claim all of them have been executed. Refer to the [Verification Matrix](../../VERIFICATION_MATRIX.md).

---

## Source references

- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)
- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)

[Read Design](DESIGN.md) for user workflow. **QA:** Tests remain [Not run](../../VERIFICATION_MATRIX.md) until executed.
