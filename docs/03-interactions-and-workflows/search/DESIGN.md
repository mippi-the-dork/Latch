# Search-Aware Presentation — Design

**Latch v2.0.3 · User guide**

## Search temporarily gets control

When you type into the World Outliner search box, Unreal may show matching rows and open paths to them in ways that differ from your saved Latch arrangement. Latch allows that presentation so search remains useful.

It preserves the prior **logical expansion**. Red latches and white pins are not deleted simply because a search reveals more rows.

## What happens on clearing search?

<!-- Media target: GIF | search-restore | docs/assets/media/search-restore.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="search-restore">
  <img src="../../assets/media-placeholder.png" alt="Animation of World Outliner search temporarily changing hierarchy presentation and restoring Latch state" loading="lazy">
  <figcaption>Search can reveal matching rows without permanently replacing the saved expansion and Keep Visible choices.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a red latched-closed folder and white pinned descendant, then type a search for an unrelated Actor, inspect results, and clear the search. Show the red and white state returning and the correct yellow paths recalculating.</span></figcaption>
</figure>


When you remove the query, Latch restores the pre-search logical expansion state, reapplies expansion latches and Keep Visible requirements, then calculates any necessary yellow Mixed View paths.

## Try it

Latch one folder collapsed, keep a descendant pinned, and search for a different Actor. Observe search results. Clear the query: the previously latched folder is still latched, and the pinned item's minimal path is again exposed.

## Common misconception

Seeing a previously collapsed folder expanded during search does **not** mean the red latch was lost. Check its state after clearing the query. Search owns temporary presentation, not persistent intent.

Suspending Latch is a separate operation that deliberately returns more control to normal Unreal expansion until resumed.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
