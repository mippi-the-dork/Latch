# Workspace Persistence — Design

**Latch v2.0.3 · User guide**

## Your layout belongs to the editor, not the level

By default, Latch saves **Expansion Latches, Keep Visible pins, and Suspend state** across level reloads and Unreal Editor restarts. It stores its intent in per-project editor configuration.

**You do not need to save the level** to preserve a Latch arrangement. Simply clicking a red triangle or pin does not write a new Actor/Level property or force an OFPA Actor package checkout.

## When persistence is disabled

Go to **Project Settings → Plugins → Latch → Persist State Across Editor Restarts** and disable the setting. Your current session's latches and pins remain usable, but saved persisted Latch state is cleared so the next editor session begins without that previous arrangement.

## Example

You are debugging a level and want an unusual temporary Outliner layout that should not appear the next time you start Unreal. Disable persistence first, organize the tree and work normally. When you close and restart, the temporary arrangement is not intentionally restored.

## What isn't saved?

Yellow Mixed View is not serialized as a separate latch mode; it is derived from the selected/pinned/latched-open descendants. The actual Outliner hierarchy remains owned by Unreal, not by Latch. Team members may therefore have different Latch arrangements without changing the shared scene.

## Source-control distinction

Source-control tools can still report changes caused by unrelated level edits. Latch's per-editor configuration is not a guarantee that no other tool will dirty an Actor package.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
