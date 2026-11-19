# Mixed View — Design

**Latch v2.0.3 · User guide**

## Why a yellow triangle appears

<!-- Media target: GIF | mixed-view-selective-path | docs/assets/media/mixed-view-selective-path.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="mixed-view-selective-path">
  <img src="../../assets/media-placeholder.png" alt="Animation revealing the minimum path to a selected Actor inside collapsed ancestors" loading="lazy">
  <figcaption>Yellow Mixed View exposes only the ancestor rows needed for a selected or pinned descendant, leaving unrelated siblings collapsed.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a deeply nested Actor C beneath Folder/Actor A/Actor B with the ancestors collapsed; select C from the viewport and show the yellow path appearing while unrelated sibling rows remain hidden. Deselect or unpin to show the path reversing.</span></figcaption>
</figure>


Mixed View occurs when a parent is **logically collapsed** but something below it still needs to be accessible. Latch makes the minimum ancestor chain visible and marks its bridging triangles yellow.

Consider `Folder → Actor A → Actor B → Actor C`. If C becomes selected while the ancestors are collapsed, the necessary path can appear as a series of yellow rows leading to C. Unrelated siblings remain hidden.

## What can require exposure?

- Selecting a descendant, including through the viewport.
- A descendant with **Keep Visible** enabled.
- A latched-open descendant beneath a logically collapsed ancestor.

## What clicking yellow does

<!-- Media target: GIF | mixed-view-yellow-click | docs/assets/media/mixed-view-yellow-click.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="mixed-view-yellow-click">
  <img src="../../assets/media-placeholder.png" alt="Animation of clicking a yellow Mixed triangle to commit a normal expansion" loading="lazy">
  <figcaption>Clicking a Mixed ancestor changes the logical branch to fully expanded instead of collapsing the temporary exposure path.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Start with a yellow ancestor exposing one pinned child. Click its yellow triangle once and show all regular children expanding; the indicator becomes grey if no other Mixed condition remains. Keep the same Outliner framing.</span></figcaption>
</figure>


Clicking the **yellow triangle** commits ordinary full expansion for that branch rather than abruptly collapsing the visible path. The triangle returns to grey when no longer in a Mixed condition.

## Practical workflow

Pin one Actor inside a large folder; collapse its ancestors and observe the narrow yellow path. Remove its pin and deselect it. If no other requirement exists, the logical collapse can be presented normally again.

**Mixed View is not a third kind of latch.** You cannot save yellow as an independent mode.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
