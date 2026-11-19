# Latch Roadmap

**Current documented release:** Latch v2.0.3 · Unreal Engine 5.8.x · Win64 · Editor only.  
**Future release scope:** Not yet announced or committed.

## Where Latch is today

<!-- Media target: IMAGE | roadmap-current-future | docs/assets/media/roadmap-current-future.png -->
<figure class="doc-media" data-kind="image" data-media-id="roadmap-current-future">
  <img src="assets/media-placeholder.png" alt="Timeline separating Latch v2.0.3 features from uncommitted future research" loading="lazy">
  <figcaption>The current release supports hierarchy latches, Mixed View, pins and editor persistence; future ideas have not been assigned a release.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Create a clean timeline showing Current: Latch v2.0.3 with expansion, Mixed View, Keep Visible and persistence. Show Future Review as explicitly uncommitted, containing compatibility, usability, performance and testing, without an invented v3.0 milestone.</span></figcaption>
</figure>


| Area | Current implementation |
| --- | --- |
| Expansion | Red Latched Expanded and Latched Collapsed parent states |
| Mixed View | Yellow minimum ancestor paths to required descendants |
| Keep Visible | White pins for eligible Actors and Folders |
| Interactions | Alt-click, Shift-click, multi-selection and context menus |
| Presentation | Search-aware restoration, Suspend/Resume |
| Persistence | Per-project editor configuration, no Actor/level data modification |
| Settings | Colors, spacing, pin hover and persistence options |

## Future directions — Under consideration, not committed

No v3.0 feature list, dates, or specific new capabilities were confirmed in the inspected README or C++ source. This roadmap therefore records areas worth evaluating, **not promises**:

1. **Compatibility:** Maintain native World Outliner integration as Unreal's Slate and editor APIs evolve.
2. **Hierarchy UX:** Continue reviewing unusual tree rebuilds, selection-driven Mixed View and identity behavior.
3. **Performance:** Benchmark large Outliners and frequent search/rebuild/selection events against the existing event-driven reconciliation.
4. **Documentation and tests:** Complete the published verification matrix and real UI media examples.

## Historical-release accuracy

The current plugin descriptor states **v2.0.3**. One older compatibility section in README says **v1.0.18**, whereas the README header also says 2.0.3. A full dated release history was not found in the inspected source, so the roadmap does not invent which feature shipped in v1.0 or v2.0.

A future historical timeline should be grounded in actual tags, release notes or the author's confirmed milestones.

## When plans become approved

Record version, user problem, expected behavior, delivery state, tests and documentation links before moving a future item into the current-feature navigation.

[Documentation Audit](DOCUMENTATION_AUDIT.md) · [Hierarchy Controls](02-hierarchy-controls/README.md)
