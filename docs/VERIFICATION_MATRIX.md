# Latch v2.0.3 Verification Matrix

**Target:** UE 5.8.x, Win64, Latch v2.0.3. All unexecuted tests are **Not run**. Record exact engine/build, project, World Partition/OFPA setup, source-control state, tester, date and logs when executing.

| ID | Scenario | Expected result | Status |
| --- | --- | --- | --- |
| LAT-01 | Enable plugin; open World Outliner | Latch controls appear in native rows/menus | Not run |
| LAT-02 | Normal click and hover unlocked expander | Normal grey/white native behavior | Not run |
| LAT-03 | Alt-click expanded/collapsed parents | Both become red with correct protected logical state | Not run |
| LAT-04 | Alt-click red expander again | Unlatched; native expansion resumes | Not run |
| LAT-05 | Select deeply nested descendant under collapsed parents | Yellow minimum Mixed View path; siblings hidden | Not run |
| LAT-06 | Click yellow Mixed triangle | Commits full expansion without unexpected collapse | Not run |
| LAT-07 | Pin and unpin a descendant | White pin and minimum path appear/clear correctly | Not run |
| LAT-08 | Pin root-level Actor | Pin saved without inventing an ancestor path | Not run |
| LAT-09 | Rename/reparent/move Actor and Folder | Stable pin identity preserved where supported | Not run |
| LAT-10 | Duplicate/delete/Undo Actor | No unexpected copied or restored latch state | Not run |
| LAT-11 | Remove final child then add a new one | Parent latch lifecycle matches actual source | Not run |
| LAT-12 | Batch Alt-click on selected parent rows with leaf | Parents update, leaf ignored | Not run |
| LAT-13 | Multi-selection pin with mixed/all-pinned values | Consistent all-on/all-off behavior | Not run |
| LAT-14 | Act on unselected row with unrelated selected rows | Only clicked row is changed | Not run |
| LAT-15 | Shift-click unlocked parent with red descendants | Recursion respects existing red latches | Not run |
| LAT-16 | Outliner context submenu and viewport submenu | Available commands match item kind | Not run |
| LAT-17 | Search with red/yellow/white states then clear | Saved intent restored, Mixed recalculated | Not run |
| LAT-18 | Suspend, modify visible tree, resume | Native behavior then restored latch intent | Not run |
| LAT-19 | Restart with persistence on/off | Correct config behavior, level remains clean | Not run |
| LAT-20 | Clear branch, expansion only, pins only, all state | Only chosen presentation state removed | Not run |
| LAT-21 | Large Outliner with nested folders and frequent row refresh | Correct rendering with acceptable editor performance | Not run |
| LAT-22 | Module shutdown/restart | No stale Slate delegates/input state | Not run |
| WEB-01 | Open home and all five category indexes | Navigation and category counts correct | Not run |
| WEB-02 | Switch Design/Technical and use source links | Pages and breadcrumbs resolve | Not run |
| WEB-03 | Search body-only term then navigate | Full-text result/excerpt correct | Not run |
| WEB-04 | Mobile 320–400px and desktop | Media, code and tables fit the article | Not run |
| WEB-05 | Inspect every planned image/GIF placeholder | Appropriate type, reader caption, capture directions | Not run |
| WEB-06 | Verify Pages deployment and browser keyboard use | Live site, navigation and search accessible | Not run |

Do not change a status to Passed without the observed result and evidence.

[Documentation Audit](DOCUMENTATION_AUDIT.md)
