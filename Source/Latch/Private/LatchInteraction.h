// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "SceneOutlinerFwd.h"
#include "SceneOutlinerStandaloneTypes.h"

class ISceneOutliner;
class SExpanderArrow;
class SImage;
class SButton;
class UToolMenu;
template<typename ItemType> class STableRow;

namespace Latch
{
    struct FRowRecord
    {
        TWeakPtr<ISceneOutliner> Outliner;
        TWeakPtr<ISceneOutlinerTreeItem> Item;
        TWeakPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row;
        TWeakPtr<SExpanderArrow> Arrow;
        TWeakPtr<SImage> ArrowImage;
        TWeakPtr<SButton> ArrowButton;
        TOptional<EVisibility> VisibilityBeforeLatch;
        bool bHiddenByLatch = false;
        int32 LastArrowTooltipKind = INDEX_NONE;
    };

    struct FOutlinerRuntime
    {
        TWeakPtr<ISceneOutliner> Outliner;

        // Items physically open only so STreeView can generate a Mixed path.
        // The bool is the logical expansion state that existed before the force.
        // This survives Scene Outliner row/tree rebuilds whose native expansion
        // cache may temporarily write the physical state back into item flags.
        TMap<FSceneOutlinerTreeItemID, bool> ForcedPhysicalExpansion;

        // Derived every update. Never persisted.
        TSet<FSceneOutlinerTreeItemID> RequiredTargets;
        TSet<FSceneOutlinerTreeItemID> MixedItems;
        TSet<FSceneOutlinerTreeItemID> MixedRoots;
        TSet<FSceneOutlinerTreeItemID> AllowedItems;

        // Snapshot of the native tree at the end of the previous Latch update.
        // Used to distinguish Unreal's automatic selection-framing expansion
        // from an explicit user expansion click.
        TMap<FSceneOutlinerTreeItemID, bool> LastPhysicalExpansion;
        TMap<FSceneOutlinerTreeItemID, bool> LastLogicalExpansion;
        TSet<FSceneOutlinerTreeItemID> LastSelectedItems;
        bool bHasExpansionSnapshot = false;

        // Search temporarily owns presentation. Capture the user's ordinary
        // logical expansion when search begins, then restore it when search ends.
        TMap<FSceneOutlinerTreeItemID, bool> SearchLogicalExpansion;
        bool bSearchWasActive = false;
    };

    class FRegistry : public TSharedFromThis<FRegistry>
    {
    public:
        bool bActive = true;
        TArray<FRowRecord> Rows;
        TArray<FOutlinerRuntime> Outliners;

        static bool IsSupportedItem(const FSceneOutlinerTreeItemPtr& Item);

        void RegisterOutliner(ISceneOutliner& View);
        void RegisterRow(ISceneOutliner& View, FSceneOutlinerTreeItemRef Item, const STableRow<FSceneOutlinerTreeItemPtr>& Row);
        void Tick(double CurrentTime);
        void Stop();

        TSharedPtr<ISceneOutliner> FindOutlinerForPath(const FWidgetPath& Path) const;
        FSceneOutlinerTreeItemPtr FindItemForPath(const FWidgetPath& Path, TSharedPtr<ISceneOutliner>& Outliner) const;
        bool PathContainsExpander(const FWidgetPath& Path) const;

        // Context menus must never retain row/tree-item objects across Slate menu
        // construction. Resolve stable IDs back to the current real Outliner items
        // immediately before a command executes.
        TArray<FSceneOutlinerTreeItemPtr> ResolveLiveItems(
            const TSharedPtr<ISceneOutliner>& Outliner,
            const TArray<FSceneOutlinerTreeItemID>& ItemIDs) const;

        // Direct row controls use the current Outliner selection when the clicked
        // row is part of that selection. Clicking a row outside the selection
        // remains a single-item operation, matching native Outliner behavior.
        TArray<FSceneOutlinerTreeItemPtr> GetDirectInteractionItems(
            ISceneOutliner& Outliner,
            const FSceneOutlinerTreeItemPtr& ClickedItem) const;

        void ToggleLatch(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item);
        void ToggleLatchForInteraction(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& ClickedItem);
        void SetLatch(const FSceneOutlinerTreeItemPtr& Item, int32 StateValue);
        void LatchCurrentSelection(const TArray<FSceneOutlinerTreeItemPtr>& Items);
        void SetLatchForItems(const TArray<FSceneOutlinerTreeItemPtr>& Items, int32 StateValue);
        void SetKeepVisibleForItems(const TArray<FSceneOutlinerTreeItemPtr>& Items, bool bKeepVisible);
        void ToggleKeepVisible(const FSceneOutlinerTreeItemPtr& Item);
        void ToggleKeepVisibleForInteraction(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& ClickedItem);
        void ClearBranch(const FSceneOutlinerTreeItemPtr& Item);
        void ClearBranches(const TArray<FSceneOutlinerTreeItemPtr>& Items);
        void ClearAllExpansion();
        void ClearAllKeepVisible();
        void ClearAll();
        void ToggleSuspend();

        // Implements Shift+Click recursively inside Latch so Red descendants keep
        // authority instead of being briefly overwritten by native recursion.
        void HandleRecursiveExpanderClick(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item);

        // Returns true when Latch handled an ordinary left click and native
        // SExpanderArrow should not receive it.
        bool HandleNormalExpanderClick(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item);
        bool HasRequiredTargetDescendant(ISceneOutliner& Outliner, const FOutlinerRuntime& Runtime, const FSceneOutlinerTreeItemPtr& Item) const;

        void PopulateContextMenu(UToolMenu& Menu, const TSharedPtr<ISceneOutliner>& Outliner, const FSceneOutlinerTreeItemPtr& Item);
        void PopulateContextMenu(UToolMenu& Menu, const TSharedPtr<ISceneOutliner>& Outliner, const TArray<FSceneOutlinerTreeItemPtr>& Items);

    private:
        void RefreshRows();
        void UpdateOutliner(FOutlinerRuntime& Runtime);
        void RestoreRecord(FRowRecord& Record, bool bClearTooltip = false);
        void RestoreRowsForOutliner(const TSharedPtr<ISceneOutliner>& Outliner);
        void RestorePhysicalOverrides(FOutlinerRuntime& Runtime, bool bRespectLatchState);
    };

    class FInputProcessor final : public IInputProcessor
    {
    public:
        explicit FInputProcessor(const TSharedRef<FRegistry>& InRegistry)
            : Registry(InRegistry)
        {
        }

        virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
        virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual const TCHAR* GetDebugName() const override { return TEXT("Latch World Outliner Expansion"); }

    private:
        TWeakPtr<FRegistry> Registry;
    };
}
