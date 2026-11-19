# Clearing Latch State — Design

**Latch v2.0.3 · User guide**

## Clear only what you intend

Latch provides several clearing commands through the **World Outliner → Latch** submenu. These remove stored hierarchy presentation choices, not Actors, Folders or their scene properties.

| You want | Use |
| --- | --- |
| Remove one row's expansion protection | Unlatch Expansion |
| Remove one pin | Stop Keeping Visible |
| Reset the chosen branch | Clear Latch State in Branch |
| Reset several selected branches | Clear Latch State in Selected Branches |
| Reset expansion latches everywhere | Clear All Expansion Latches |
| Reset pins everywhere | Clear All Keep Visible States |
| Reset everything in memory | Clear All Latch State |

## Project Settings utility

**Project Settings → Plugins → Latch → Clear Saved Latch State** removes saved editor Latch records after confirmation. It does not delete Actors or undo normal level edits.

## Suspend isn't clearing

If you only want native Unreal expansion behavior temporarily, choose **Suspend Latch**, not Clear All. Resuming restores saved intent. Clearing deliberately discards it.

## Example

A previous task left too many pinned Actors across a level. Use Clear All Keep Visible States to remove the pins while leaving deliberate red expansion latches intact.

## Before bulk cleanup

Check the scope carefully: a branch clear can affect descendants, while a global clear affects all saved Latch items in the project. There is no benefit to deleting level files to reset Latch.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
