# Expansion Latches — Design

**Latch v2.0.3 · User guide**

## Three expansion modes

<!-- Media target: IMAGE | expansion-three-states | docs/assets/media/expansion-three-states.png -->
<figure class="doc-media" data-kind="image" data-media-id="expansion-three-states">
  <img src="../../assets/media-placeholder.png" alt="Comparison of normal, latched-open and latched-closed expansion triangles" loading="lazy">
  <figcaption>Grey triangles follow normal Outliner expansion, while red triangles hold their parent's chosen open or closed state.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Capture three representative parent rows in the same Unreal Outliner: unlocked grey, red latched-expanded with children showing, and red latched-collapsed with children hidden. Include labels naming each state.</span></figcaption>
</figure>


| Visual state | Meaning | How to activate |
| --- | --- | --- |
| Grey triangle | Normal native expansion | Unlatched |
| Red + expanded | Keep this parent expanded | Expand, then Alt-click |
| Red + collapsed | Keep this parent collapsed | Collapse, then Alt-click |

Alt-clicking a **red** triangle removes the latch. Latches apply only when a row has actual children.

## What a red latch protects

<!-- Media target: GIF | expansion-latch-gesture | docs/assets/media/expansion-latch-gesture.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="expansion-latch-gesture">
  <img src="../../assets/media-placeholder.png" alt="Animation of Alt-clicking a parent to latch it and attempting ordinary expansion changes" loading="lazy">
  <figcaption>An expansion latch resists normal clicks until Alt-click removes its protected state.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record Alt-clicking an expanded parent into red, clicking normally without collapsing it, Alt-clicking again to unlatch, then demonstrating the same interaction on a collapsed parent. Keep the triangle and row text in frame.</span></figcaption>
</figure>


A latched-open parent resists ordinary attempts to collapse it and is protected during recursive collapse. A latched-closed parent resists normal expansion and is protected during recursive expansion. Global native expand/collapse commands cannot permanently override saved latched intent while Latch is active.

## Try it

Make two sibling folders. Latch one expanded and the other collapsed. Use Shift-click on a common unlocked ancestor and observe that each red child's own latch remains authoritative.

## Red versus yellow

Red denotes **the row's saved latch**. Yellow denotes **a temporary Mixed View** when a logically collapsed ancestor must expose a required descendant. Red takes precedence over yellow; yellow is not another saved expansion mode.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
