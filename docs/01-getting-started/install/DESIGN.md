# Installation and Compatibility — Design

**Latch v2.0.3 · User guide**

## Choose a matching installation

Latch v2.0.3 targets Unreal Engine **5.8.x** on **Windows 64-bit**, running in the Editor. Download a compatible release from the [Latch repository](https://github.com/mippi-the-dork/Latch) or install from Fab if available to your engine installation.

### Project installation

1. Close Unreal Editor.
2. Put the entire **Latch** plugin folder under `YourProject/Plugins/Latch/`.
3. Check that `Latch.uplugin` is directly inside that directory.
4. Launch the project, go to **Edit → Plugins**, enable Latch if required and restart when prompted.
5. Open the standard World Outliner and look for Latch's expansion/pin controls.

### Source installation

Clone the repository into `YourProject/Plugins/Latch`, generate project files as required for your Unreal C++ setup, and compile the Editor target. This is an Editor-only module; do not add it to packaged runtime code.

## If controls are missing

Check the actual project plugin location, version compatibility, whether the plugin loaded, and that you are in the regular World Outliner rather than an unrelated tree picker. Latch does not add a large standalone window.

---

[Read Technical](TECHNICAL.md) for C++ ownership and engine integration.
