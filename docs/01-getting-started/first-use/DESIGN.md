# First Use — Design

**Latch v2.0.3 · User guide**

## Try Latch on a test hierarchy

<!-- Media target: GIF | first-use | docs/assets/media/first-use.gif -->
<figure class="doc-media" data-kind="gif" data-media-id="first-use">
  <img src="../../assets/media-placeholder.png" alt="Animation of the first expansion latch, Keep Visible pin and resulting Mixed View" loading="lazy">
  <figcaption>The first-use workflow moves from a normal expander to a red latch and then a pinned descendant exposed through a yellow parent.<span class="media-capture-instructions"><strong>Capture instructions · GIF:</strong> Record a compact Unreal Editor sequence with one folder and children: expand, Alt-click to turn red, unlatch, pin a child using the white pin, collapse the ancestor and show its yellow Mixed triangle. Display mouse modifier labels unobtrusively.</span></figcaption>
</figure>


Create or open a disposable level containing a folder with a few Actors.

1. **Expand** the folder normally using its grey triangle.
2. **Alt-click** its triangle. It turns **red** and now holds the expanded state.
3. Alt-click the red triangle again to **unlatch**. Collapse the folder and Alt-click again to latch it closed.
4. Unlatch it, then hover the reserved pin slot beside a child Actor's label. Click the grey pin so it becomes **white**.
5. Collapse the parent. Latch exposes the minimum required path to that pinned Actor and marks the logically collapsed ancestor **yellow**.
6. Click the yellow triangle to make the expansion normal, and click the white pin to remove Keep Visible.

## Understand the gestures

**Normal click**: native expansion when unlocked. **Alt-click**: toggle current expansion latch on a parent. **Shift-click**: recursively expand/collapse unlocked descendants. **Pin click**: toggle Keep Visible for the eligible Actor/Folder. Right-click opens the native menu with a **Latch** submenu.

## Important distinction

A white pin does not render an Actor in the viewport. It only ensures that the Actor's **Outliner row** can remain reachable through collapsed ancestors.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
