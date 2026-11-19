# Mixed View — Design

**Latch v2.0.3 · User guide**

## Why a yellow triangle appears

Mixed View occurs when a parent is **logically collapsed** but something below it still needs to be accessible. Latch makes the minimum ancestor chain visible and marks its bridging triangles yellow.

Consider `Folder → Actor A → Actor B → Actor C`. If C becomes selected while the ancestors are collapsed, the necessary path can appear as a series of yellow rows leading to C. Unrelated siblings remain hidden.

## What can require exposure?

- Selecting a descendant, including through the viewport.
- A descendant with **Keep Visible** enabled.
- A latched-open descendant beneath a logically collapsed ancestor.

## What clicking yellow does

Clicking the **yellow triangle** commits ordinary full expansion for that branch rather than abruptly collapsing the visible path. The triangle returns to grey when no longer in a Mixed condition.

## Practical workflow

Pin one Actor inside a large folder; collapse its ancestors and observe the narrow yellow path. Remove its pin and deselect it. If no other requirement exists, the logical collapse can be presented normally again.

**Mixed View is not a third kind of latch.** You cannot save yellow as an independent mode.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
