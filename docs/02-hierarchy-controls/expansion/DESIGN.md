# Expansion Latches — Design

**Latch v2.0.3 · User guide**

## Three expansion modes

| Visual state | Meaning | How to activate |
| --- | --- | --- |
| Grey triangle | Normal native expansion | Unlatched |
| Red + expanded | Keep this parent expanded | Expand, then Alt-click |
| Red + collapsed | Keep this parent collapsed | Collapse, then Alt-click |

Alt-clicking a **red** triangle removes the latch. Latches apply only when a row has actual children.

## What a red latch protects

A latched-open parent resists ordinary attempts to collapse it and is protected during recursive collapse. A latched-closed parent resists normal expansion and is protected during recursive expansion. Global native expand/collapse commands cannot permanently override saved latched intent while Latch is active.

## Try it

Make two sibling folders. Latch one expanded and the other collapsed. Use Shift-click on a common unlocked ancestor and observe that each red child's own latch remains authoritative.

## Red versus yellow

Red denotes **the row's saved latch**. Yellow denotes **a temporary Mixed View** when a logically collapsed ancestor must expose a required descendant. Red takes precedence over yellow; yellow is not another saved expansion mode.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
