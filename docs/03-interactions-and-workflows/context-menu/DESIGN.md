# World Outliner Context Menu — Design

**Latch v2.0.3 · User guide**

## Where the commands live

Right-click a supported Actor or Folder in the World Outliner and open **Latch**. The submenu provides operations beyond the triangle and pin, including **Latch Current Expansion**, **Latch Expanded**, **Latch Collapsed**, **Unlatch Expansion**, **Keep Visible / Stop Keeping Visible**, branch clearing, suspension, and global clearing.

## Which command should I choose?

| Goal | Command |
| --- | --- |
| Keep the current open/closed setting | Latch Current Expansion |
| Set a specific protected state | Latch Expanded / Latch Collapsed |
| Return to native expansion | Unlatch Expansion |
| Expose selected item through collapsed parents | Keep Visible |
| Remove the selected item's pin | Stop Keeping Visible |
| Remove stored state for part of a tree | Clear Latch State in Branch |
| Temporarily use native expansion everywhere | Suspend Latch |

## Selection behavior

The relevant commands can apply to several selected rows. Commands requiring an expandable parent are unavailable or skipped for childless entries. Right-clicking a row outside the selection should not change other selected rows.

## Destructive scope

The menu also contains **Clear All Expansion Latches**, **Clear All Keep Visible States**, and **Clear All Latch State**. These affect editor presentation state, not Actor/level content. Check the exact command before clearing many items.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
