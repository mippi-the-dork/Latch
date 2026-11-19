# Installation and Compatibility — Technical

**Latch v2.0.3 · Source-backed reference**

## Descriptor truth

`Latch.uplugin` declares `VersionName: "2.0.3"`, `Version: 43`, `EngineVersion: "5.8.0"`, module `Latch` with `Type: "Editor"`, `LoadingPhase: "PostEngineInit"`, and `PlatformAllowList: ["Win64"]`.

## Build dependencies

`Source/Latch/Latch.Build.cs` references editor and Scene Outliner/Slate modules needed for native widget integration. Compiling a project from source requires a matching UE 5.8 editor C++ environment; a packaged game should not depend on this editor module.

## Startup behavior

`FLatchModule` registers a label column and a hidden observer column, an input processor, menu extensions and editor delegates. These hooks are tied to the actual World Outliner and Level Editor lifecycle.

## Documentation discrepancy

The README header and plugin descriptor identify v2.0.3. One compatibility table still names v1.0.18; that line is stale relative to the current descriptor, not evidence that the installed module is v1.0.18.

---

## Source references

- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)
- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)

**Verification:** Implementation source reviewed for v2.0.3; live Unreal Editor checks remain in the [Verification Matrix](../../VERIFICATION_MATRIX.md).

[Read Design](DESIGN.md) for practical workflows.
