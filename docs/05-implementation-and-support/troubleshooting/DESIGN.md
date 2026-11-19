# Troubleshooting and FAQ — Design

**Latch v2.0.3 · User guide**

## My Alt-click does nothing

An Expansion Latch requires an **eligible parent with real children**. Try a folder that visibly contains Actors and Alt-click its expansion triangle, not the row label or Keep Visible pin.

## Why are ancestors yellow?

<!-- Media target: IMAGE | troubleshooting-mixed | docs/assets/media/troubleshooting-mixed.png -->
<figure class="doc-media" data-kind="image" data-media-id="troubleshooting-mixed">
  <img src="../../assets/media-placeholder.png" alt="Comparison of a normal collapsed folder and a yellow Mixed View ancestor" loading="lazy">
  <figcaption>A yellow ancestor is logically collapsed but temporarily exposes the necessary path to a selected or pinned descendant.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Capture two nearby nested folders in the Outliner: one ordinary grey collapsed, the other yellow showing only a pinned or selected child path. Include a visible white pin or selection highlight to explain the exposure requirement.</span></figcaption>
</figure>


A selected, pinned or latched-open descendant needs exposure through logically collapsed ancestors. Yellow Mixed View is expected. Click the yellow triangle for normal full expansion, or remove the underlying selection/pin requirement.

## Why isn't a white pin making my Actor visible in the viewport?

Keep Visible affects **Outliner rows only**. It does not unhide an Actor or change Hidden in Game.

## My latch disappeared after restarting

Check **Project Settings → Plugins → Latch → Persist State Across Editor Restarts**. When disabled, saved state is cleared and the current session's choices do not intentionally carry over.

## A red latch disappeared when the parent lost its children

A childless row cannot display an expansion triangle. If the item becomes a parent again, check and reapply the latch as needed. The stored-state cleanup path depends on current lifecycle events; do not assume the former latch will automatically return.

## Search changed my arrangement

Clear the World Outliner search. Latch should restore the pre-search logical expansion and recalculate Mixed paths. Also check whether **Suspend Latch** is currently enabled.

## Why did a batch change more than one row?

If you activate a control on a row that belongs to your current Outliner selection, Latch may apply it across eligible selected rows. If you activate an unselected row, only that clicked row should change.

## Report with enough detail

Include exact Unreal/Latch version, Actor vs Folder, whether the row had children, pin/latch states, current selection, search state, any third-party Outliner plugins, and a screenshot or recording of the unexpected behavior.

---

[Read Technical](TECHNICAL.md) for implementation and engine constraints.
