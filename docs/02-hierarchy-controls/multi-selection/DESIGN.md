# Multi-Selection — Design

**Latch v2.0.3 · User guide**

## Interacting with selected rows

When several Outliner rows are selected, acting on **one of the selected rows** can apply the relevant Latch action to all eligible selected rows. Acting on an unselected row changes that row alone—your unrelated selection isn't changed accidentally.

## Expansion batch

<!-- Media target: GIF | multi-latch | docs/assets/media/multi-latch.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="multi-latch">
  <img src="../../assets/media-placeholder.png" alt="Animation of Alt-clicking one selected parent to latch several selected parents" loading="lazy">
  <figcaption>A selected parent determines the batch's expansion-latch intent while childless selected rows are ignored.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record three selected folders with children plus one selected leaf Actor. Alt-click the expansion triangle on one selected folder, show red states applied to eligible parents and the leaf unchanged, then Alt-click again to remove the latches.</span></figcaption>
</figure>


Alt-click an eligible selected parent. If the clicked parent is unlatched, its **current logical expanded/collapsed state** determines the target latch state for other eligible selected parents. If the clicked parent is already red, the action unlatches eligible selected parents. Leaf rows are ignored.

## Keep Visible batch

<!-- Media target: GIF | multi-pin | docs/assets/media/multi-pin.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="multi-pin">
  <img src="../../assets/media-placeholder.png" alt="Animation of batch pin toggling on a mixed Outliner selection" loading="lazy">
  <figcaption>A Keep Visible batch enables all eligible pins when any are unpinned, and disables them when all are pinned.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Select three Actors with only one white pin active. Click the pin of a selected row to make all white, then click again to clear all pins. Keep selection highlighting and the pin states clearly visible.</span></figcaption>
</figure>


Click a selected pin. If the eligible selection contains any unpinned items, the batch enables pins for all. If all are pinned, it disables them all. Mixed and all-off therefore become all-on.

## Example

Select three folders and an Actor leaf. Alt-click the expansion triangle of a selected expanded folder: the eligible parent folders latch expanded, while the leaf is ignored. Then select several Actors and click a selected pin to make their Keep Visible state consistent.

## Scope caution

Confirm the clicked row is part of the selection before performing a bulk action; otherwise the operation only affects that clicked item.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
