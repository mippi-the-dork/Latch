# Project Settings — Technical

**Latch v2.0.3 · Source-backed reference**

## Settings class

`ULatchSettings` extends `UDeveloperSettings` with `UCLASS(Config=Latch,DefaultConfig)` and display name Latch. The category is **Plugins** under Project Settings.

## Default fields

`ExpanderContentSpacing = 2.0f` with clamp 0–10; `ExpansionLatchColor = (1,0,0,1)`; `MixedStateColor = (1,1,0,1)`; `bShowKeepVisibleHoverControl = true`; and `bPersistStateAcrossEditorRestarts = true`. These defaults appear in `Config/DefaultLatch.ini`.

## Presentation consumers

The label column reads pin hover configuration, while row widget decoration and expansion controls use spacing and color settings. None of these settings modifies Actor render visibility or property data.

## Storage distinction

`ULatchSettings` stores configuration options in the `Latch` config domain, while per-item expansion/pin/suspend state is saved by `ULatchSubsystem` to the editor per-project user configuration when enabled. The two are not the same file/ownership path.

## Utility callback

The Clear Saved Latch State setting delegates to subsystem cleanup of persisted records. Avoid confusing that utility with merely unchecking persistence.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Described from v2.0.3 source. Live UE 5.8 testing is tracked in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for user workflows.
