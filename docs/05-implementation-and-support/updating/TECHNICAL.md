# Updates and Support — Technical

**Latch v2.0.3 · Source-backed reference**

## Current module target

`Latch.uplugin` specifies `Version:43`, `VersionName:"2.0.3"`, `EngineVersion:"5.8.0"`, an Editor module loaded PostEngineInit, and a Win64 platform allowlist.

## Upgrade-sensitive integration

`FRegistry`, `FInputProcessor`, label/observer column hooks and Scene Outliner menu modification depend on UE editor APIs. Source builds must link matching editor modules from `Latch.Build.cs`. Upgrades across Unreal versions require compiling and testing against those APIs.

## Persistence ownership

`ULatchSubsystem` stores expansion/pin/suspended state in `Latch.State.V3` editor per-project configuration. Upgrading the plugin does not require rewriting level Actor packages; do not advise users to delete their level to recover Outliner layout.

## Cleanup

`FLatchModule::ShutdownModule` removes selection and Actor lifecycle delegates, registered input preprocessor and columns, then stops the registry and unregisters menu callbacks. Dynamic reloading is not supported by this module.

## Support evidence

For editor integration bugs, collect exact plugin source commit, engine build, Outliner item kind, mouse modifier sequence, current search/suspended state, and source control/config permissions. Troubleshooting should distinguish incorrect presentation from actual Actor hierarchy modifications.

---

## Source references

- [LatchModule.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchModule.cpp)
- [LatchInteraction.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchInteraction.cpp)
- [LatchSubsystem.cpp](https://github.com/mippi-the-dork/Latch/blob/main/Source/Latch/Private/LatchSubsystem.cpp)

[Read Design](DESIGN.md) for user workflow. **QA:** Tests remain [Not run](../../VERIFICATION_MATRIX.md) until executed.
