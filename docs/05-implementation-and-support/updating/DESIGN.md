# Updates and Support — Design

**Latch v2.0.3 · User guide**

## Updating a precompiled installation

<!-- Media target: IMAGE | updating-folder | docs/assets/media/updating-folder.png -->
<figure class="doc-media" data-kind="image" data-media-id="updating-folder">
  <img src="../../assets/media-placeholder.png" alt="Latch plugin directory in a project's Plugins folder after an update" loading="lazy">
  <figcaption>Replacing the complete Latch folder avoids mixing source and binary files from incompatible releases.<span class="media-capture-instructions"><strong>Capture instructions · Image:</strong> Capture Windows File Explorer showing YourProject/Plugins/Latch with Latch.uplugin directly inside and Source/Resources/Config beneath it. Include a clear version reference for the downloaded release and exclude irrelevant project folders.</span></figcaption>
</figure>


1. Close Unreal Editor.
2. Download the release matching your Unreal Engine and Win64 platform from the [Latch repository](https://github.com/mippi-the-dork/Latch).
3. Replace the complete `YourProject/Plugins/Latch/` folder. Do not merge old source or binaries into a new release.
4. Reopen the project, verify Latch is enabled and confirm the native controls load in the World Outliner.

## Updating a source checkout

Pull the intended Latch commit/tag in your Plugins directory, regenerate project files if needed, and rebuild the Unreal **Editor** target. Don't assume source from a different engine version is compatible.

## Current compatibility

The descriptor reports **Latch v2.0.3**, Unreal Engine **5.8.0** target and **Win64 Editor** support. The README describes UE 5.8.0–5.8.3, but other engine/platform combinations have not been established here.

## Getting help

Open a [GitHub issue](https://github.com/mippi-the-dork/Latch/issues) for bugs or feature requests. State the problem, expected outcome, actual outcome, exact Latch/UE version, an Outliner screenshot/GIF, reproduction steps, and any relevant Unreal Output Log lines.

## What not to delete

Never delete level or External Actor packages to reset Latch. Its persistent presentation state lives in editor configuration and can be managed through its own clear commands and Project Settings utility.

---

[Read Technical](TECHNICAL.md) for implementation and engine constraints.
