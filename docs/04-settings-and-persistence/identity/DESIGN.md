# Identity and Parent Lifecycle — Design

**Latch v2.0.3 · User guide**

## Keep Visible follows the item

<!-- Media target: GIF | identity-move | docs/assets/media/identity-move.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="identity-move">
  <img src="../../assets/media-placeholder.png" alt="Animation moving a pinned Actor between folders while retaining its pin" loading="lazy">
  <figcaption>Keep Visible remains associated with the Actor's identity when it is moved or reparented in the Outliner.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a white-pinned Actor in Folder A, drag it to Folder B or attach it under another Actor, then collapse the new ancestor and show its white pin and required Mixed path still functioning. Include both before and after states.</span></figcaption>
</figure>


Latch associates a pin with the Actor or Folder identity, not simply with the row's current screen position. Supported renames, attachment/reparent changes and folder moves preserve the pin where a stable ID is available.

## Duplicates, deletion and Undo

A **newly duplicated Actor** starts with no inherited Latch pin or expansion latch. Deleting an Actor clears the associated saved Latch state; Undo of deletion is not designed to resurrect the deleted Actor's previous Latch state automatically.

## When a parent becomes a leaf

Expansion Latches only make sense for rows that have children. If the final real child is removed, the row can no longer show its expansion triangle and the registry/subsystem may clear its prior latch state. If children return, do not assume it automatically regains its earlier red latch; verify and reapply if needed.

Keep Visible is independent of this rule. A leaf Actor or Folder may be pinned, although root-level items have no collapsed ancestors to bridge.

## Why a moved folder may look different

The visible row may be rebuilt after hierarchy changes, while Latch re-associates state by stable item identity. Its physical expansion presentation can change even when the underlying pin or logical latch persists. Distinguish a moved item from a duplicated one before reporting missing state.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
