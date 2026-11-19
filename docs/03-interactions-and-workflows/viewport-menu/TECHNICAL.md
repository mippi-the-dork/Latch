# Viewport Context Menu — Technical

**Latch v2.0.3 · Source-backed reference**

## Registration

`FLatchModule::RegisterViewportActorMenu` adds Actor-oriented commands through `UToolMenus` in the Level Editor viewport menu. The module manages the menu registration and removes hooks at shutdown.

## Selection source

The viewport is an Actor selection path; it does not provide Folder tree items. The registry resolves selected Actors to supported Outliner items when available, then invokes the same action methods used by the Outliner context submenu.

## Why this matters

The menu is an editor convenience, not a second store of Latch state. `ULatchSubsystem` remains authoritative for red expansion latches and white Keep Visible pins, while `FRegistry` updates visual row presentation.

## Edge cases

A selected Actor without a valid current Outliner row or expandable children might not be eligible for every command. The source's selected-Actor restriction should not be presented as a general-purpose viewport editing mode.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
