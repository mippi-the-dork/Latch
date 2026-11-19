# Suspend and Resume — Design

**Latch v2.0.3 · User guide**

## Temporary normal Unreal behavior

Use **Latch → Suspend Latch** when you want to inspect or reorganize the visible hierarchy using the usual native Unreal expansion behavior without deleting the latches and pins you previously established.

While suspended, the special red and yellow hierarchy presentation is removed, ordinary expansion behavior is available, and saved latch/Keep Visible state remains recorded.

## Resume

Use **Latch → Resume Latch** to reapply the recorded expansion and pin rules and rebuild any required yellow Mixed View paths.

## When to use it

If a complex level is temporarily difficult to navigate because many branches are protected, Suspend gives you ordinary Outliner traversal. Return to Resume when finished. Do **not** choose Clear All Latch State unless you want to discard your stored arrangement.

## Persistent suspension

Suspend state can itself be preserved across restarts when the general persistence setting is enabled. If you reopen the project and do not see red indicators, check whether Latch is still suspended rather than assuming saved data is gone.

---

[Read Technical](TECHNICAL.md) for exact editor implementation.
