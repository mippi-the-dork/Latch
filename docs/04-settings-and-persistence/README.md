# 04. Settings and Persistence

Latch's saved state belongs to editor configuration, not scene packages. Saving the map is unnecessary to preserve an Outliner layout.

## How to use this section

Saved editor layout. Choose **Design** for controls, expected results and examples; choose **Technical** for native implementation details, data ownership and limitations. These pages complement each other rather than duplicating the README.

## Topics

| Topic | What it explains | Design | Technical |
| --- | --- | --- | --- |
| **Project Settings** | Spacing, colors, hover, persistence | [Design](settings/DESIGN.md) | [Technical](settings/TECHNICAL.md) |
| **Workspace Persistence** | Per-project state, session-only choices | [Design](persistence/DESIGN.md) | [Technical](persistence/TECHNICAL.md) |
| **Identity and Parent Lifecycle** | Renames, moves, childless branches | [Design](identity/DESIGN.md) | [Technical](identity/TECHNICAL.md) |
| **Clearing Latch State** | Branch and saved-state cleanup | [Design](clearing/DESIGN.md) | [Technical](clearing/TECHNICAL.md) |

## Things to remember

Latch's saved state belongs to editor configuration, not scene packages. Saving the map is unnecessary to preserve an Outliner layout.

[Documentation home](../README.md).
