# Hierarchy Reconciliation and Performance — Design

**Latch v2.0.3 · User guide**

## Designed for large Outliners

Latch needs to maintain expansion and selective row exposure as a World Outliner changes. Doing a full hierarchy scan every mouse movement or render frame would make deep levels unpleasant to navigate.

The current design uses **event-driven invalidation**. Selection, Actor addition/deletion, hierarchy changes and row lifecycle events can schedule a reconciliation. A slower fallback audit catches editor or third-party changes that don't reliably announce themselves.

## What you'll notice

Normal scrolling should not require rebuilding the entire hierarchy on every frame. Mixed paths should be recalculated when a pin, selection or latch changes. During an active search, the native Outliner temporarily owns presentation rather than Latch constantly fighting it.

## A demanding workflow to test

<!-- Media target: GIF | performance-outliner | docs/assets/media/performance-outliner.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="performance-outliner">
  <img src="../../assets/media-placeholder.png" alt="Animation of Latch maintaining a large Outliner during scrolling, selection and search" loading="lazy">
  <figcaption>Latch's event-driven hierarchy updates are designed to avoid continuous full-tree work during ordinary row interaction.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a realistically large UE Outliner with many nested folders: scroll, pin an item, select a viewport Actor, search and clear, then resume. Include an FPS/stat overlay only if measured honestly and don't invent a performance comparison.</span></figcaption>
</figure>


Open a level with deep nested folders and many Actor rows. Pin distant descendants, latch some intermediate parents, scroll quickly, select Actors in the viewport, search and clear the query, then suspend and resume. Check that the visible paths remain correct and the Outliner doesn't develop disruptive hitches.

## Troubleshooting refresh issues

If a yellow path remains after removing the last pinned/selected requirement, inspect whether the Actor remains selected elsewhere or whether an active search/suspension is affecting presentation. A delayed refresh should be reported with exact steps and engine version.

The architecture aims for responsiveness, but source review is not a benchmark or a guarantee of performance in every project.

---

[Read Technical](TECHNICAL.md) for implementation and engine constraints.
