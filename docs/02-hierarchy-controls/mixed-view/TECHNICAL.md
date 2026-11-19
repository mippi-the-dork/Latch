# Mixed View — Technical

**Latch v2.0.3 · Source-backed reference**

## Logical/physical split

<!-- Media target: IMAGE | mixed-view-bridge | docs/assets/media/mixed-view-bridge.png -->
<figure class="doc-media" data-kind="image" data-media-id="mixed-view-bridge">
  <img src="../../assets/media-placeholder.png" alt="Diagram contrasting a logically collapsed branch with its physically visible Mixed View bridge" loading="lazy">
  <figcaption>Latch preserves logical collapse while temporarily expanding a physical path to a required descendant.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw side-by-side logical hierarchy and physical STreeView representation: collapsed flag false for an ancestor, an exposed required descendant path, suppressed siblings, and FOutlinerRuntime ForcedPhysicalExpansion preserving the original logical value.</span></figcaption>
</figure>


`FRegistry::UpdateOutliner` calculates `RequiredTargets` from selection, pinned keys and latched-open descendant needs. A logically collapsed branch can stay physically expanded in `STreeView` to expose those targets while unrelated sibling rows are suppressed.

## Reversible bridge

`FOutlinerRuntime::ForcedPhysicalExpansion` tracks physical expansion overrides with their original logical values. On reconciliation, the original logical `Flags.bIsExpanded` is restored when rebuilding tree items; this prevents temporary presentation from overwriting the desired expansion.

## Yellow click path

`FRegistry::HandleNormalExpanderClick` checks `MixedItems`, commits `SetLogicalExpansion(Item,true)` and removes the forced physical override. When a required descendant exists under an expanded parent that the user clicks to collapse, it can commit the logical collapse while leaving the physical bridge for the next Mixed calculation.

## Serialization boundary

Only red expansion states and white Keep Visible keys are stored. The yellow Mixed set is derived at runtime and not serialized as a separate expansion mode.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
