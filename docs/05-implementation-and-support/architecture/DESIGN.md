# Editor Architecture — Design

**Latch v2.0.3 · User guide**

## How Latch fits into Unreal

Latch extends the **standard World Outliner**. It doesn't spawn an Actor, replace Scene Outliner, or introduce a second hierarchy source. Its controls live in the existing expander and label areas plus native context menus.

## Three responsibilities

<!-- Media target: IMAGE | architecture-user-flow | docs/assets/media/architecture-user-flow.png -->
<figure class="doc-media" data-kind="image" data-media-id="architecture-user-flow">
  <img src="../../assets/media-placeholder.png" alt="Simple diagram explaining how Latch stores intent and updates World Outliner presentation" loading="lazy">
  <figcaption>The subsystem stores workspace choices, the registry reconciles hierarchy state, and row widgets display the result.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Draw three labeled blocks: ULatchSubsystem Saved Intent → FRegistry Hierarchy Reconciliation → World Outliner Rows/Controls. Include a selection/pin event input and illustrate that the actual level hierarchy is unchanged.</span></figcaption>
</figure>


**The subsystem** remembers which Actor/Folder rows are latched or pinned, and whether the tool is suspended.

**The registry** watches live tree items, selection and hierarchy changes, reconciles logical versus physical expansion, and determines which rows belong in the minimum Mixed View path.

**The columns and input processor** make the controls feel native: label-adjacent pins, expansion triangle interaction, context menu actions and modifier-key gestures.

## A practical example

After a viewport selection, a collapsed ancestor might need to become yellow. Selection invalidates registry state, the registry finds the required path, and the UI exposes the minimum rows. Nothing about that action moves or expands the actual Actor hierarchy in the world.

## Why separate storage from presentation?

A tree row widget can be recreated when the Outliner refreshes. Storing a latch only in the widget would lose it. Latch instead stores item identity and logical intent independently, then reapplies it to the current UI.

---

[Read Technical](TECHNICAL.md) for implementation and engine constraints.
