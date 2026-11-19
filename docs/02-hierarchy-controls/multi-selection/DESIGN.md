# Multi-Selection — Design

**Latch v2.0.3 · User guide**

## Interacting with selected rows

When several Outliner rows are selected, acting on **one of the selected rows** can apply the relevant Latch action to all eligible selected rows. Acting on an unselected row changes that row alone—your unrelated selection isn't changed accidentally.

## Expansion batch

Alt-click an eligible selected parent. If the clicked parent is unlatched, its **current logical expanded/collapsed state** determines the target latch state for other eligible selected parents. If the clicked parent is already red, the action unlatches eligible selected parents. Leaf rows are ignored.

## Keep Visible batch

Click a selected pin. If the eligible selection contains any unpinned items, the batch enables pins for all. If all are pinned, it disables them all. Mixed and all-off therefore become all-on.

## Example

Select three folders and an Actor leaf. Alt-click the expansion triangle of a selected expanded folder: the eligible parent folders latch expanded, while the leaf is ignored. Then select several Actors and click a selected pin to make their Keep Visible state consistent.

## Scope caution

Confirm the clicked row is part of the selection before performing a bulk action; otherwise the operation only affects that clicked item.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
