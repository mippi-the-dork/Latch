// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#include "LatchInteraction.h"

#include "LatchSubsystem.h"
#include "LatchSettings.h"
#include "ActorTreeItem.h"
#include "FolderTreeItem.h"
#include "WorldTreeItem.h"
#include "ISceneOutliner.h"
#include "ISceneOutlinerTreeItem.h"
#include "SceneOutlinerPublicTypes.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SWidget.h"
#include "Layout/Children.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"
#include "ToolMenus.h"

namespace Latch
{
    namespace
    {
        // Expensive hierarchy/row reconciliation is event-driven. Keep a slow
        // safety pass for native and third-party Outliner changes that have no
        // reliable shared invalidation event (Index, Chroma, Stem, Focus, Origin).
        // A zero deadline requests reconciliation on the next Slate tick.
        double NextRefreshTime = 0.0;
        // The fallback is a compatibility audit, not the main update path.
        // Selection, Latch actions and row expansion still reconcile immediately.
        constexpr double FallbackRefreshSeconds = 3.0;
        // Native STreeView builds descendant rows lazily after an expansion change.
        // Permit a few consecutive reconciliation passes while those rows settle,
        // rather than postponing nested Mixed-path processing for half a second.
        int32 HierarchySettleFrames = 0;
        constexpr int32 MaxHierarchySettleFrames = 3;
        void ScheduleHierarchySettle()
        {
            HierarchySettleFrames = FMath::Max(HierarchySettleFrames, MaxHierarchySettleFrames);
        }

        ULatchSubsystem* State()
        {
            return GEditor ? GEditor->GetEditorSubsystem<ULatchSubsystem>() : nullptr;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& MutableTree(ISceneOutliner& Outliner)
        {
            return const_cast<STreeView<FSceneOutlinerTreeItemPtr>&>(Outliner.GetTree());
        }

        void GatherTree(const FSceneOutlinerTreeItemPtr& Item, TArray<FSceneOutlinerTreeItemPtr>& Out, TSet<const ISceneOutlinerTreeItem*>& Seen)
        {
            if (!Item || Seen.Contains(Item.Get()))
            {
                return;
            }

            Seen.Add(Item.Get());
            Out.Add(Item);
            for (const TWeakPtr<ISceneOutlinerTreeItem>& WeakChild : Item->GetChildren())
            {
                GatherTree(WeakChild.Pin(), Out, Seen);
            }
        }

        TArray<FSceneOutlinerTreeItemPtr> AllItems(ISceneOutliner& Outliner)
        {
            TArray<FSceneOutlinerTreeItemPtr> Result;
            TSet<const ISceneOutlinerTreeItem*> Seen;
            for (const FSceneOutlinerTreeItemPtr& Root : MutableTree(Outliner).GetRootItems())
            {
                GatherTree(Root, Result, Seen);
            }
            return Result;
        }

        bool IsDescendantOf(const FSceneOutlinerTreeItemPtr& Item, const FSceneOutlinerTreeItemPtr& Ancestor)
        {
            if (!Item || !Ancestor || Item == Ancestor)
            {
                return false;
            }

            TSet<const ISceneOutlinerTreeItem*> Seen;
            FSceneOutlinerTreeItemPtr Cursor = Item->GetParent();
            while (Cursor && !Seen.Contains(Cursor.Get()))
            {
                if (Cursor == Ancestor)
                {
                    return true;
                }
                Seen.Add(Cursor.Get());
                Cursor = Cursor->GetParent();
            }
            return false;
        }

        bool HasAncestorID(const FSceneOutlinerTreeItemPtr& Item, const TSet<FSceneOutlinerTreeItemID>& IDs)
        {
            if (!Item)
            {
                return false;
            }

            TSet<const ISceneOutlinerTreeItem*> Seen;
            FSceneOutlinerTreeItemPtr Cursor = Item->GetParent();
            while (Cursor && !Seen.Contains(Cursor.Get()))
            {
                if (IDs.Contains(Cursor->GetID()))
                {
                    return true;
                }
                Seen.Add(Cursor.Get());
                Cursor = Cursor->GetParent();
            }
            return false;
        }

        template<typename WidgetType>
        TSharedPtr<WidgetType> FindWidgetOfType(const TSharedRef<SWidget>& Widget, const FName& WantedType, int32 Depth = 0)
        {
            if (Depth > 16)
            {
                return nullptr;
            }

            if (Widget->GetType() == WantedType)
            {
                return StaticCastSharedRef<WidgetType>(Widget);
            }

            if (FChildren* Children = Widget->GetChildren())
            {
                for (int32 Index = 0; Index < Children->Num(); ++Index)
                {
                    if (TSharedPtr<WidgetType> Found = FindWidgetOfType<WidgetType>(Children->GetChildAt(Index), WantedType, Depth + 1))
                    {
                        return Found;
                    }
                }
            }
            return nullptr;
        }

        TSharedPtr<SExpanderArrow> FindExpander(const TSharedRef<SWidget>& Widget, int32 Depth = 0)
        {
            if (Depth > 16)
            {
                return nullptr;
            }

            const FName Type = Widget->GetType();
            if (Type == FName(TEXT("SExpanderArrow")) || Type == FName(TEXT("SGuideArrow")))
            {
                return StaticCastSharedRef<SExpanderArrow>(Widget);
            }

            if (FChildren* Children = Widget->GetChildren())
            {
                for (int32 Index = 0; Index < Children->Num(); ++Index)
                {
                    if (TSharedPtr<SExpanderArrow> Found = FindExpander(Children->GetChildAt(Index), Depth + 1))
                    {
                        return Found;
                    }
                }
            }
            return nullptr;
        }

        bool IsSearchActive(ISceneOutliner& Outliner)
        {
            return !Outliner.GetFilterHighlightText().Get().IsEmpty();
        }

        bool HasChildren(const FSceneOutlinerTreeItemPtr& Item)
        {
            return Item && !Item->GetChildren().IsEmpty();
        }

        bool LogicalExpansion(const FSceneOutlinerTreeItemPtr& Item)
        {
            if (!Item)
            {
                return false;
            }

            if (const ULatchSubsystem* Subsystem = State())
            {
                const ELatchExpansionState LatchState = Subsystem->GetExpansionState(Item);
                if (LatchState == ELatchExpansionState::LatchedOpen)
                {
                    return true;
                }
                if (LatchState == ELatchExpansionState::LatchedClosed)
                {
                    return false;
                }
            }

            // Scene Outliner already owns the user's intended expansion here.
            // Mixed State may make STreeView physically disagree temporarily.
            return Item->Flags.bIsExpanded;
        }

        void SetLogicalExpansion(const FSceneOutlinerTreeItemPtr& Item, bool bExpanded)
        {
            if (!Item || Item->Flags.bIsExpanded == bExpanded)
            {
                return;
            }

            Item->Flags.bIsExpanded = bExpanded;
            Item->OnExpansionChanged();
        }

        void SetPhysicalExpansionPreservingLogical(
            STreeView<FSceneOutlinerTreeItemPtr>& Tree,
            const FSceneOutlinerTreeItemPtr& Item,
            bool bPhysicalExpanded,
            bool bLogicalExpanded)
        {
            if (!Item)
            {
                return;
            }

            if (Tree.IsItemExpanded(Item) != bPhysicalExpanded)
            {
                Tree.SetItemExpansion(Item, bPhysicalExpanded);
                ScheduleHierarchySettle();
            }

            // STreeView's OnExpansionChanged callback writes back into the item and,
            // for actor folders, persists that value in the editor's folder state.
            // Restore the logical value immediately so a temporary Mixed bridge never
            // leaks into reloads or becomes the user's real expansion state.
            if (Item->Flags.bIsExpanded != bLogicalExpanded)
            {
                Item->Flags.bIsExpanded = bLogicalExpanded;
                Item->OnExpansionChanged();
            }
        }

        FText TooltipFor(bool bLatched, bool bMixed, bool bLogicalExpanded, bool bSuspendedOrSearching)
        {
            if (bSuspendedOrSearching)
            {
                return NSLOCTEXT("Latch", "TooltipInactive", "Expand or collapse this item. Latch is temporarily inactive here.");
            }
            if (bLatched)
            {
                return bLogicalExpanded
                    ? NSLOCTEXT("Latch", "TooltipLatchedOpen", "Latched expanded. Alt+Click to unlatch.")
                    : NSLOCTEXT("Latch", "TooltipLatchedClosed", "Latched collapsed. Alt+Click to unlatch.");
            }
            if (bMixed)
            {
                return NSLOCTEXT("Latch", "TooltipMixed", "Mixed view. Some descendants are visible through this collapsed item. Click to expand normally. Alt+Click to latch it collapsed.");
            }
            return bLogicalExpanded
                ? NSLOCTEXT("Latch", "TooltipExpanded", "Collapse. Alt+Click to latch this item expanded.")
                : NSLOCTEXT("Latch", "TooltipCollapsed", "Expand. Alt+Click to latch this item collapsed.");
        }

        int32 TooltipKindFor(bool bLatched, bool bMixed, bool bLogicalExpanded, bool bSuspendedOrSearching)
        {
            if (bSuspendedOrSearching)
            {
                return 0;
            }
            if (bLatched)
            {
                return bLogicalExpanded ? 1 : 2;
            }
            if (bMixed)
            {
                return 3;
            }
            return bLogicalExpanded ? 4 : 5;
        }

        FOutlinerRuntime* FindRuntime(FRegistry& Registry, ISceneOutliner& Outliner)
        {
            for (FOutlinerRuntime& Runtime : Registry.Outliners)
            {
                if (Runtime.Outliner.Pin().Get() == &Outliner)
                {
                    return &Runtime;
                }
            }
            return nullptr;
        }

        void AddExpandedVisibleSubtree(
            const FSceneOutlinerTreeItemPtr& Root,
            TSet<FSceneOutlinerTreeItemID>& Allowed,
            TSet<const ISceneOutlinerTreeItem*>& Seen)
        {
            if (!Root || Seen.Contains(Root.Get()) || !LogicalExpansion(Root))
            {
                return;
            }

            Seen.Add(Root.Get());
            for (const TWeakPtr<ISceneOutlinerTreeItem>& WeakChild : Root->GetChildren())
            {
                const FSceneOutlinerTreeItemPtr Child = WeakChild.Pin();
                if (!Child)
                {
                    continue;
                }

                Allowed.Add(Child->GetID());
                AddExpandedVisibleSubtree(Child, Allowed, Seen);
            }
        }
    }

    bool FRegistry::IsSupportedItem(const FSceneOutlinerTreeItemPtr& Item)
    {
        if (!Item)
        {
            return false;
        }

        if (const FActorTreeItem* ActorItem = Item->CastTo<FActorTreeItem>())
        {
            const AActor* Actor = ActorItem->Actor.Get();
            const UWorld* World = IsValid(Actor) ? Actor->GetWorld() : nullptr;
            return World && (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview);
        }

        if (const FWorldTreeItem* WorldItem = Item->CastTo<FWorldTreeItem>())
        {
            const UWorld* World = WorldItem->World.Get();
            return World && (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview);
        }

        // Folder rows are created by the editor Actor Browser Outliner. They do not
        // carry a direct UWorld pointer, so their containing Outliner supplies the
        // editor-world boundary.
        return Item->IsA<FFolderTreeItem>();
    }

    void FRegistry::RegisterOutliner(ISceneOutliner& View)
    {
        Outliners.RemoveAll([](const FOutlinerRuntime& Runtime)
        {
            return !Runtime.Outliner.IsValid();
        });

        for (const FOutlinerRuntime& Runtime : Outliners)
        {
            if (Runtime.Outliner.Pin().Get() == &View)
            {
                return;
            }
        }

        FOutlinerRuntime Runtime;
        Runtime.Outliner = StaticCastSharedRef<ISceneOutliner>(View.AsShared());
        Outliners.Add(MoveTemp(Runtime));
        bPendingRefresh = true;
        ScheduleHierarchySettle();
    }

    void FRegistry::RegisterRow(ISceneOutliner& View, FSceneOutlinerTreeItemRef Item, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
    {
        RegisterOutliner(View);

        const TSharedRef<STableRow<FSceneOutlinerTreeItemPtr>> RowRef =
            ConstCastSharedRef<STableRow<FSceneOutlinerTreeItemPtr>>(StaticCastSharedRef<const STableRow<FSceneOutlinerTreeItemPtr>>(Row.AsShared()));

        for (FRowRecord& Existing : Rows)
        {
            if (Existing.Row.Pin() == RowRef)
            {
                // A recycled SListView row may now represent a different item. Never
                // carry Latch visibility or arrow styling across that association.
                if (Existing.Item.Pin() != Item)
                {
                    RestoreRecord(Existing);
                }
                // Do not restart the multi-frame settle window just because a
                // column asks to register an already-known row again. Repeated
                // registrations can otherwise perpetuate tree/layout work while
                // Unreal is scrolling a newly selected actor into view.
                const bool bAssociationChanged = Existing.Item.Pin() != Item ||
                    Existing.Outliner.Pin().Get() != &View;
                Existing.Item = Item;
                Existing.Outliner = StaticCastSharedRef<ISceneOutliner>(View.AsShared());
                if (bAssociationChanged)
                {
                    ApplyCachedRowVisibility(Existing);
                    // Widget reassociation is not a hierarchy mutation. The
                    // cached Mixed state is applied before Slate measures it.
                    bPendingRowRefresh = true;
                }
                return;
            }
        }

        FRowRecord Record;
        Record.Outliner = StaticCastSharedRef<ISceneOutliner>(View.AsShared());
        Record.Item = Item;
        Record.Row = RowRef;
        Rows.Add(MoveTemp(Record));
        // SListView measures the row immediately after generation. Apply the
        // last committed Mixed visibility before its first layout prepass.
        ApplyCachedRowVisibility(Rows.Last());
        // Discover the native arrow on the next tick. No hierarchy traversal
        // is necessary just because virtualized Slate generated a row.
        bPendingRowRefresh = true;
    }

    void FRegistry::ApplyCachedRowVisibility(FRowRecord& Record)
    {
        const TSharedPtr<ISceneOutliner> Outliner = Record.Outliner.Pin();
        const FSceneOutlinerTreeItemPtr Item = Record.Item.Pin();
        const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin();
        if (!Outliner || !Item || !Row || !bActive)
        {
            return;
        }

        const ULatchSubsystem* Subsystem = State();
        if (!Subsystem || Subsystem->IsSuspended() || IsSearchActive(*Outliner))
        {
            return;
        }

        for (const FOutlinerRuntime& Runtime : Outliners)
        {
            if (Runtime.Outliner.Pin() != Outliner)
            {
                continue;
            }
            const bool bInsideMixed = Runtime.MixedRoots.Contains(Item->GetID()) ||
                HasAncestorID(Item, Runtime.MixedRoots);
            const bool bHide = bInsideMixed && !Runtime.AllowedItems.Contains(Item->GetID());
            if (bHide && !Record.bHiddenByLatch)
            {
                Record.VisibilityBeforeLatch = Row->GetVisibility();
                Row->SetVisibility(EVisibility::Collapsed);
                Record.bHiddenByLatch = true;
            }
            else if (!bHide && Record.bHiddenByLatch)
            {
                Row->SetVisibility(Record.VisibilityBeforeLatch.Get(EVisibility::Visible));
                Record.VisibilityBeforeLatch.Reset();
                Record.bHiddenByLatch = false;
            }
            return;
        }
    }

    void FRegistry::RestoreRecord(FRowRecord& Record, bool bClearTooltip)
    {
        if (const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin())
        {
            if (Record.bHiddenByLatch)
            {
                Row->SetVisibility(Record.VisibilityBeforeLatch.Get(EVisibility::Visible));
            }
        }
        Record.bHiddenByLatch = false;
        Record.VisibilityBeforeLatch.Reset();

        const TSharedPtr<SExpanderArrow> Arrow = Record.Arrow.Pin();
        const TSharedPtr<SImage> Image = Record.ArrowImage.Pin();
        const TSharedPtr<SButton> Button = Record.ArrowButton.Pin();
        if (Arrow && Image)
        {
            const bool bExpanded = Record.Row.IsValid() ? Record.Row.Pin()->IsItemExpanded() : false;
            Image->SetColorAndOpacity(TAttribute<FSlateColor>(FSlateColor::UseSubduedForeground()));
            if (bClearTooltip)
            {
                Arrow->SetToolTipText(FText::GetEmpty());
                Image->SetToolTipText(FText::GetEmpty());
                if (Button)
                {
                    Button->SetToolTipText(FText::GetEmpty());
                }
            }
        }

        Record.LastArrowTooltipKind = INDEX_NONE;
        Record.Arrow.Reset();
        Record.ArrowImage.Reset();
        Record.ArrowButton.Reset();
    }

    FSlateColor FRegistry::GetArrowTint(
        const TSharedPtr<ISceneOutliner>& Outliner,
        const FSceneOutlinerTreeItemPtr& Item,
        const TSharedPtr<SButton>& Button) const
    {
        if (!bActive || !Outliner || !Item)
        {
            return FSlateColor::UseSubduedForeground();
        }

        const ULatchSubsystem* Subsystem = State();
        if (!Subsystem || Subsystem->IsSuspended() || IsSearchActive(*Outliner))
        {
            return FSlateColor::UseSubduedForeground();
        }

        const ULatchSettings* Settings = GetDefault<ULatchSettings>();
        if (!Settings)
        {
            return FSlateColor::UseSubduedForeground();
        }
        if (Subsystem->GetExpansionState(Item) != ELatchExpansionState::Unlatched)
        {
            FLinearColor Tint = Settings->ExpansionLatchColor;
            Tint.A = 1.0f;
            return FSlateColor(Tint);
        }
        for (const FOutlinerRuntime& Runtime : Outliners)
        {
            if (Runtime.Outliner.Pin() == Outliner && Runtime.MixedItems.Contains(Item->GetID()))
            {
                FLinearColor Tint = Settings->MixedStateColor;
                Tint.A = 1.0f;
                return FSlateColor(Tint);
            }
        }
        return (Button && Button->IsHovered())
            ? FSlateColor(FLinearColor::White)
            : FSlateColor::UseSubduedForeground();
    }

    void FRegistry::RefreshRows()
    {
        for (FRowRecord& Record : Rows)
        {
            const TSharedPtr<ISceneOutliner> Outliner = Record.Outliner.Pin();
            const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin();
            if (!Outliner || !Row)
            {
                continue;
            }

            STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(*Outliner);
            const FSceneOutlinerTreeItemPtr* CurrentItemPtr = Tree.ItemFromWidget(Row.Get());
            const FSceneOutlinerTreeItemPtr CurrentItem = CurrentItemPtr ? *CurrentItemPtr : nullptr;

            if (Record.Item.Pin() != CurrentItem)
            {
                RestoreRecord(Record);
                Record.Item = CurrentItem;
            }

            if (!CurrentItem)
            {
                continue;
            }

            // Re-discover just the expander widget subtree during reconciliation
            // (normally twice a second), not on every Slate frame. An expander
            // may survive while its button/image child is reconstructed. Find the
            // actual glyph under the button rather than an arbitrary SImage in
            // the expander subtree.
            const TSharedPtr<SExpanderArrow> CurrentArrow = FindExpander(Row.ToSharedRef());
            const TSharedPtr<SButton> CurrentButton = CurrentArrow
                ? FindWidgetOfType<SButton>(CurrentArrow.ToSharedRef(), FName(TEXT("SButton")))
                : nullptr;
            // Native UE 5.8 Scene Outliner uses SExpanderArrow > SButton >
            // SImage. Retain a fallback for custom Outliner row layouts.
            TSharedPtr<SImage> CurrentImage = CurrentButton
                ? FindWidgetOfType<SImage>(CurrentButton.ToSharedRef(), FName(TEXT("SImage")))
                : nullptr;
            if (!CurrentImage && CurrentArrow)
            {
                CurrentImage = FindWidgetOfType<SImage>(CurrentArrow.ToSharedRef(), FName(TEXT("SImage")));
            }

            if (Record.Arrow.Pin() != CurrentArrow
                || Record.ArrowButton.Pin() != CurrentButton
                || Record.ArrowImage.Pin() != CurrentImage)
            {
                // Restore the previous glyph before replacing its weak widget
                // references so a recycled row never retains another item's tint.
                RestoreRecord(Record);
                Record.Item = CurrentItem;
                Record.Arrow = CurrentArrow;
                Record.ArrowButton = CurrentButton;
                Record.ArrowImage = CurrentImage;

                // UE 5.8 builds the native arrow as SExpanderArrow > SButton >
                // SImage. Keep its native GetExpanderImage() brush attribute
                // intact. Bind only the tint, evaluated at paint time against
                // Latch's cached Mixed state and persisted expansion latch.
                // Weak captures avoid retaining recycled rows or Outliners.
                if (CurrentImage)
                {
                    TWeakPtr<FRegistry> WeakRegistry = AsShared();
                    TWeakPtr<ISceneOutliner> WeakOutliner = Record.Outliner;
                    TWeakPtr<ISceneOutlinerTreeItem> WeakItem = Record.Item;
                    TWeakPtr<SButton> WeakButton = CurrentButton;
                    CurrentImage->SetColorAndOpacity(TAttribute<FSlateColor>::CreateLambda(
                        [WeakRegistry, WeakOutliner, WeakItem, WeakButton]() -> FSlateColor
                        {
                            const TSharedPtr<FRegistry> Registry = WeakRegistry.Pin();
                            const TSharedPtr<ISceneOutliner> Outliner = WeakOutliner.Pin();
                            const FSceneOutlinerTreeItemPtr Item = WeakItem.Pin();
                            if (!Registry || !Registry->bActive || !Outliner || !Item)
                            {
                                return FSlateColor::UseSubduedForeground();
                            }
                            return Registry->GetArrowTint(Outliner, Item, WeakButton.Pin());
                        }));
                }
            }
        }

        Rows.RemoveAll([](const FRowRecord& Record)
        {
            return !Record.Outliner.IsValid() || !Record.Row.IsValid();
        });
    }

    void FRegistry::RestoreRowsForOutliner(const TSharedPtr<ISceneOutliner>& Outliner)
    {
        for (FRowRecord& Record : Rows)
        {
            if (Record.Outliner.Pin() == Outliner)
            {
                if (Record.bHiddenByLatch)
                {
                    if (const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin())
                    {
                        Row->SetVisibility(Record.VisibilityBeforeLatch.Get(EVisibility::Visible));
                    }
                    Record.bHiddenByLatch = false;
                    Record.VisibilityBeforeLatch.Reset();
                }
            }
        }
    }

    void FRegistry::RestorePhysicalOverrides(FOutlinerRuntime& Runtime, bool bRespectLatchState)
    {
        const TSharedPtr<ISceneOutliner> Outliner = Runtime.Outliner.Pin();
        if (!Outliner)
        {
            Runtime.ForcedPhysicalExpansion.Reset();
            return;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(*Outliner);
        const TArray<FSceneOutlinerTreeItemPtr> Items = AllItems(*Outliner);

        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!Item)
            {
                continue;
            }

            const bool* StoredLogical = Runtime.ForcedPhysicalExpansion.Find(Item->GetID());
            if (!StoredLogical)
            {
                continue;
            }

            bool bLogical = *StoredLogical;
            if (bRespectLatchState)
            {
                const ELatchExpansionState LatchState = State()
                    ? State()->GetExpansionState(Item)
                    : ELatchExpansionState::Unlatched;
                if (LatchState == ELatchExpansionState::LatchedOpen)
                {
                    bLogical = true;
                }
                else if (LatchState == ELatchExpansionState::LatchedClosed)
                {
                    bLogical = false;
                }
            }
            SetPhysicalExpansionPreservingLogical(Tree, Item, bLogical, bLogical);
        }

        Runtime.ForcedPhysicalExpansion.Reset();
        Runtime.RequiredTargets.Reset();
        Runtime.MixedItems.Reset();
        Runtime.MixedRoots.Reset();
        Runtime.AllowedItems.Reset();
    }

    void FRegistry::UpdateOutliner(FOutlinerRuntime& Runtime)
    {
        const TSharedPtr<ISceneOutliner> Outliner = Runtime.Outliner.Pin();
        ULatchSubsystem* Subsystem = State();
        if (!Outliner || !Subsystem)
        {
            return;
        }

        // Do not inspect or correct the Outliner while Undo/Redo is replaying.
        // Hierarchy and row associations are transient during a transaction.
        if (GIsTransacting)
        {
            return;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(*Outliner);
        const TArray<FSceneOutlinerTreeItemPtr> Items = AllItems(*Outliner);
        const bool bSearchActive = IsSearchActive(*Outliner);

        // A Scene Outliner rebuild can reapply its cached physical expansion to
        // newly-created tree items. Reassert the logical side of every Mixed bridge
        // before deriving state. The runtime map is authoritative while forced.
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!Item)
            {
                continue;
            }
            if (const bool* StoredLogical = Runtime.ForcedPhysicalExpansion.Find(Item->GetID()))
            {
                if (Item->Flags.bIsExpanded != *StoredLogical)
                {
                    Item->Flags.bIsExpanded = *StoredLogical;
                    Item->OnExpansionChanged();
                }
            }
        }

        auto PresentInactive = [&]()
        {
            RestoreRowsForOutliner(Outliner);

            for (FRowRecord& Record : Rows)
            {
                if (Record.Outliner.Pin() != Outliner)
                {
                    continue;
                }
                const TSharedPtr<SExpanderArrow> Arrow = Record.Arrow.Pin();
                const TSharedPtr<SImage> Image = Record.ArrowImage.Pin();
                const TSharedPtr<SButton> Button = Record.ArrowButton.Pin();
                const FSceneOutlinerTreeItemPtr Item = Record.Item.Pin();
                if (!Arrow || !Image || !Item)
                {
                    continue;
                }
                const bool bExpanded = Tree.IsItemExpanded(Item);
                Image->Invalidate(EInvalidateWidgetReason::Paint);
                const int32 TipKind = TooltipKindFor(false, false, bExpanded, true);
                if (Record.LastArrowTooltipKind != TipKind)
                {
                    const FText Tip = TooltipFor(false, false, bExpanded, true);
                    Arrow->SetToolTipText(Tip);
                    Image->SetToolTipText(Tip);
                    if (Button)
                    {
                        Button->SetToolTipText(Tip);
                    }
                    Record.LastArrowTooltipKind = TipKind;
                }
            }

            Runtime.LastPhysicalExpansion.Reset();
            Runtime.LastSelectedItems.Reset();
            Runtime.bHasExpansionSnapshot = false;
        };

        // Suspension deliberately gives ordinary Unreal expansion full authority for
        // the session. Do not restore a search snapshot while Latch is suspended.
        if (Subsystem->IsSuspended())
        {
            RestorePhysicalOverrides(Runtime, false);
            Runtime.SearchLogicalExpansion.Reset();
            Runtime.bSearchWasActive = false;
            Runtime.LastLogicalExpansion.Reset();
            PresentInactive();
            return;
        }

        // Search temporarily owns physical presentation, but it must not permanently
        // rewrite the user's pre-search logical expansion. Capture once on entry and
        // restore on exit. Persistent Red/Keep Visible state is never modified here.
        if (bSearchActive)
        {
            if (!Runtime.bSearchWasActive)
            {
                RestorePhysicalOverrides(Runtime, false);
                Runtime.SearchLogicalExpansion = Runtime.LastLogicalExpansion;
                for (const FSceneOutlinerTreeItemPtr& Item : Items)
                {
                    if (Item && !Runtime.SearchLogicalExpansion.Contains(Item->GetID()))
                    {
                        Runtime.SearchLogicalExpansion.Add(Item->GetID(), Item->Flags.bIsExpanded);
                    }
                }
                Runtime.bSearchWasActive = true;
            }

            PresentInactive();
            return;
        }

        if (Runtime.bSearchWasActive)
        {
            for (const FSceneOutlinerTreeItemPtr& Item : Items)
            {
                if (!Item)
                {
                    continue;
                }

                if (const bool* StoredLogical = Runtime.SearchLogicalExpansion.Find(Item->GetID()))
                {
                    bool bLogical = *StoredLogical;
                    const ELatchExpansionState LatchState = Subsystem->GetExpansionState(Item);
                    if (LatchState == ELatchExpansionState::LatchedOpen)
                    {
                        bLogical = true;
                    }
                    else if (LatchState == ELatchExpansionState::LatchedClosed)
                    {
                        bLogical = false;
                    }
                    SetPhysicalExpansionPreservingLogical(Tree, Item, bLogical, bLogical);
                }
            }
            Runtime.SearchLogicalExpansion.Reset();
            Runtime.bSearchWasActive = false;
        }

        // Keep the previous Mixed state for targeted arrow repainting. Row tint
        // reads the current state at paint time, so unchanged ancestors never
        // need another explicit invalidation.
        const TSet<FSceneOutlinerTreeItemID> PreviousMixedItems = Runtime.MixedItems;
        Runtime.RequiredTargets.Reset();
        Runtime.MixedItems.Reset();
        Runtime.MixedRoots.Reset();
        Runtime.AllowedItems.Reset();

        // Never delete persistent expansion latches just because an Outliner tree
        // item temporarily reports no children. The Scene Outliner can rebuild
        // nested folders and their child links independently from its visible rows.
        // In particular, an interim empty child list is NOT proof that the folder
        // permanently became childless. Explicit user actions still clear latches.

        // Actor selection is authoritative in the editor. Immediately after a
        // viewport pick, STreeView may still report actors from its previous
        // selection. Unioning both sets briefly exposes BOTH paths in Mixed View,
        // followed by a second collapse when Scene Outliner catches up. Use the
        // editor's live actor selection for actor rows, and preserve the tree's
        // selection for non-actor items such as folders.
        TSet<FSceneOutlinerTreeItemID> SelectedIDs;
        const bool bHasEditorActorSelection = GEditor && GEditor->GetSelectedActors();
        for (const FSceneOutlinerTreeItemPtr& SelectedItem : Tree.GetSelectedItems())
        {
            if (SelectedItem && (!bHasEditorActorSelection || !SelectedItem->CastTo<FActorTreeItem>()))
            {
                SelectedIDs.Add(SelectedItem->GetID());
            }
        }

        if (bHasEditorActorSelection)
        {
            for (const FSceneOutlinerTreeItemPtr& Item : Items)
            {
                if (const FActorTreeItem* ActorItem = Item ? Item->CastTo<FActorTreeItem>() : nullptr)
                {
                    if (AActor* Actor = ActorItem->Actor.Get(); Actor && GEditor->GetSelectedActors()->IsSelected(Actor))
                    {
                        SelectedIDs.Add(Item->GetID());
                    }
                }
            }
        }

        // SSceneOutliner::ScrollItemIntoView expands every ancestor when Unreal
        // frames a viewport selection. That is useful vanilla behavior, but Latch's
        // selection exposure contract is different: preserve the user's collapsed
        // state and expose only the minimum path as Yellow Mixed view.
        //
        // Compare against the previous-frame physical snapshot so we only undo
        // expansion that happened alongside a newly selected item. Explicit user
        // expansion clicks remain authoritative.
        if (Runtime.bHasExpansionSnapshot)
        {
            TSet<FSceneOutlinerTreeItemID> NewlySelected;
            for (const FSceneOutlinerTreeItemID& SelectedID : SelectedIDs)
            {
                if (!Runtime.LastSelectedItems.Contains(SelectedID))
                {
                    NewlySelected.Add(SelectedID);
                }
            }

            for (const FSceneOutlinerTreeItemPtr& Item : Items)
            {
                if (!Item || !NewlySelected.Contains(Item->GetID()))
                {
                    continue;
                }

                FSceneOutlinerTreeItemPtr Ancestor = Item->GetParent();
                while (Ancestor)
                {
                    const FSceneOutlinerTreeItemID AncestorID = Ancestor->GetID();
                    const bool* bWasExpanded = Runtime.LastPhysicalExpansion.Find(AncestorID);
                    const bool bIsExpandedNow = Tree.IsItemExpanded(Ancestor);

                    if (bWasExpanded && !*bWasExpanded && bIsExpandedNow &&
                        !Runtime.ForcedPhysicalExpansion.Contains(AncestorID) &&
                        Subsystem->GetExpansionState(Ancestor) == ELatchExpansionState::Unlatched)
                    {
                        // Restore only logical intent here. Leave the physical tree
                        // open so this same update can turn it into a stable Mixed
                        // bridge without a collapse/re-expand flash.
                        SetLogicalExpansion(Ancestor, false);
                    }

                    Ancestor = Ancestor->GetParent();
                }
            }
        }

        TArray<FSceneOutlinerTreeItemPtr> Targets;
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!IsSupportedItem(Item))
            {
                continue;
            }

            const bool bTarget = SelectedIDs.Contains(Item->GetID()) ||
                Subsystem->IsExposureLatched(Item) ||
                Subsystem->GetExpansionState(Item) == ELatchExpansionState::LatchedOpen;

            if (bTarget)
            {
                Targets.Add(Item);
                Runtime.RequiredTargets.Add(Item->GetID());
            }
        }

        TSet<FSceneOutlinerTreeItemID> ForceExpanded;

        for (const FSceneOutlinerTreeItemPtr& Target : Targets)
        {
            TArray<FSceneOutlinerTreeItemPtr> Chain;
            FSceneOutlinerTreeItemPtr Cursor = Target ? Target->GetParent() : nullptr;
            while (Cursor)
            {
                Chain.Add(Cursor);
                Cursor = Cursor->GetParent();
            }

            int32 HighestCollapsedIndex = INDEX_NONE;
            for (int32 Index = 0; Index < Chain.Num(); ++Index)
            {
                if (!LogicalExpansion(Chain[Index]))
                {
                    HighestCollapsedIndex = Index;
                }
            }

            if (HighestCollapsedIndex == INDEX_NONE)
            {
                continue;
            }

            Runtime.AllowedItems.Add(Target->GetID());

            // Chain order is target-parent toward root. Keep the exact minimum
            // path through the highest logically-collapsed ancestor.
            for (int32 Index = 0; Index <= HighestCollapsedIndex; ++Index)
            {
                const FSceneOutlinerTreeItemPtr& Bridge = Chain[Index];
                if (!Bridge)
                {
                    continue;
                }

                Runtime.AllowedItems.Add(Bridge->GetID());
                if (!LogicalExpansion(Bridge))
                {
                    Runtime.MixedItems.Add(Bridge->GetID());
                    ForceExpanded.Add(Bridge->GetID());
                }
            }

            Runtime.MixedRoots.Add(Chain[HighestCollapsedIndex]->GetID());
        }

        // Once a path reaches a genuinely expanded node, Unreal semantics resume:
        // all normally-visible children under that expanded node stay visible.
        TSet<const ISceneOutlinerTreeItem*> ExpandedTraversalSeen;
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (Item && Runtime.AllowedItems.Contains(Item->GetID()))
            {
                AddExpandedVisibleSubtree(Item, Runtime.AllowedItems, ExpandedTraversalSeen);
            }
        }

        // Apply the complete desired expansion state in one traversal. In
        // particular, do not first collapse an old Mixed bridge and then reopen
        // it when the selection switches to another descendant of that branch.
        // Root-first traversal makes each ancestor available before its children.
        const TMap<FSceneOutlinerTreeItemID, bool> PreviouslyForced = Runtime.ForcedPhysicalExpansion;
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!IsSupportedItem(Item))
            {
                continue;
            }

            const FSceneOutlinerTreeItemID ID = Item->GetID();
            const ELatchExpansionState LatchState = Subsystem->GetExpansionState(Item);
            const bool bShouldForce = ForceExpanded.Contains(ID);
            const bool* PreviousLogical = PreviouslyForced.Find(ID);

            // A temporary physical bridge must never become the persisted
            // expansion preference, even after Unreal rebuilds the row.
            bool bLogical = PreviousLogical ? *PreviousLogical : LogicalExpansion(Item);
            if (LatchState == ELatchExpansionState::LatchedOpen)
            {
                bLogical = true;
            }
            else if (LatchState == ELatchExpansionState::LatchedClosed)
            {
                bLogical = false;
            }

            if (bShouldForce)
            {
                if (!Runtime.ForcedPhysicalExpansion.Contains(ID))
                {
                    Runtime.ForcedPhysicalExpansion.Add(ID, bLogical);
                }
                SetPhysicalExpansionPreservingLogical(Tree, Item, true, bLogical);
            }
            else
            {
                // An ordinary unlatched branch remains under Unreal's native
                // control. Only restore a previous Mixed bridge or enforce an
                // explicit latch; do not churn every other row's expansion.
                if (PreviousLogical || LatchState != ELatchExpansionState::Unlatched)
                {
                    SetLogicalExpansion(Item, bLogical);
                    SetPhysicalExpansionPreservingLogical(Tree, Item, bLogical, bLogical);
                }
                Runtime.ForcedPhysicalExpansion.Remove(ID);
            }
        }

        // A viewport selection can leave the native virtualized list scrolled to
        // rows that are about to be collapsed by Mixed View. Track transitions so
        // we repair only an invalid offset, not the user's normal scrolling.
        bool bMixedRowVisibilityChanged = false;

        // Suppress only real rows that lie inside a Mixed root but are not part of
        // the allowed real hierarchy. Track the transition, rather than forcing
        // visibility writes and layout invalidations on every reconciliation.
        for (FRowRecord& Record : Rows)
        {
            if (Record.Outliner.Pin() != Outliner)
            {
                continue;
            }

            const FSceneOutlinerTreeItemPtr Item = Record.Item.Pin();
            const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin();
            if (!Item || !Row)
            {
                continue;
            }

            const bool bInsideMixedRoot = Runtime.MixedRoots.Contains(Item->GetID()) || HasAncestorID(Item, Runtime.MixedRoots);
            const bool bAllowed = Runtime.AllowedItems.Contains(Item->GetID());
            const bool bHide = bInsideMixedRoot && !bAllowed;

            if (bHide && !Record.bHiddenByLatch)
            {
                Record.VisibilityBeforeLatch = Row->GetVisibility();
                Row->SetVisibility(EVisibility::Collapsed);
                Record.bHiddenByLatch = true;
                bMixedRowVisibilityChanged = true;
            }
            else if (!bHide && Record.bHiddenByLatch)
            {
                Row->SetVisibility(Record.VisibilityBeforeLatch.Get(EVisibility::Visible));
                Record.VisibilityBeforeLatch.Reset();
                Record.bHiddenByLatch = false;
                bMixedRowVisibilityChanged = true;
            }
        }

        // Changed row visibility changes the measured height of already
        // generated SListView widgets. A single native refresh remeasures the
        // list with the correct initial visibility applied in RegisterRow.
        // Without it the list may show a cut-off region until manual scrolling.
        // Only enqueue this when the committed presentation actually changed.
        if (bMixedRowVisibilityChanged)
        {
            Tree.RequestListRefresh();
        }

        // The tint attribute installed at row discovery reads cached state at
        // paint time. Only request a glyph repaint here: never take over the
        // native expander brush or replace its color binding.
        for (FRowRecord& Record : Rows)
        {
            if (Record.Outliner.Pin() != Outliner)
            {
                continue;
            }
            const FSceneOutlinerTreeItemPtr Item = Record.Item.Pin();
            const TSharedPtr<SExpanderArrow> Arrow = Record.Arrow.Pin();
            const TSharedPtr<SImage> Image = Record.ArrowImage.Pin();
            const TSharedPtr<SButton> Button = Record.ArrowButton.Pin();
            if (!Item || !Arrow || !Image)
            {
                continue;
            }

            const bool bLogical = LogicalExpansion(Item);
            const bool bLatched = Subsystem->GetExpansionState(Item) != ELatchExpansionState::Unlatched;
            const bool bMixed = !bLatched && Runtime.MixedItems.Contains(Item->GetID());
            const int32 TipKind = TooltipKindFor(bLatched, bMixed, bLogical, false);
            const bool bMixedChanged = PreviousMixedItems.Contains(Item->GetID()) != Runtime.MixedItems.Contains(Item->GetID());
            if (Record.LastArrowTooltipKind != TipKind || bMixedChanged)
            {
                Image->Invalidate(EInvalidateWidgetReason::Paint);
                const FText Tip = TooltipFor(bLatched, bMixed, bLogical, false);
                Arrow->SetToolTipText(Tip);
                Image->SetToolTipText(Tip);
                if (Button)
                {
                    Button->SetToolTipText(Tip);
                }
                Record.LastArrowTooltipKind = TipKind;
            }
        }

        // Snapshot the final native physical state after Latch has applied this
        // frame's presentation. On the next update this lets us identify expansion
        // performed automatically by selection framing without second-guessing
        // ordinary user expansion.
        Runtime.LastPhysicalExpansion.Reset();
        Runtime.LastLogicalExpansion.Reset();
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (Item)
            {
                Runtime.LastPhysicalExpansion.Add(Item->GetID(), Tree.IsItemExpanded(Item));
                Runtime.LastLogicalExpansion.Add(Item->GetID(), LogicalExpansion(Item));
            }
        }
        Runtime.LastSelectedItems = MoveTemp(SelectedIDs);
        Runtime.bHasExpansionSnapshot = true;
    }

    void FRegistry::Tick(double CurrentTime)
    {
        if (!bActive)
        {
            return;
        }

        // The native Outliner can expand a parent to frame a viewport selection
        // before broadcasting the Actor-selection notification Latch expects.
        // Observe only the small editor selection set here, never the Outliner
        // hierarchy. A changed selection schedules reconciliation this tick,
        // rather than waiting for the three-second external compatibility audit.
        TSet<TWeakObjectPtr<AActor>> CurrentSelectedActors;
        if (GEditor && GEditor->GetSelectedActors())
        {
            for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
            {
                if (AActor* Actor = Cast<AActor>(*It))
                {
                    CurrentSelectedActors.Add(Actor);
                }
            }
        }
        if (!bObservedActorSelection || CurrentSelectedActors.Num() != ObservedSelectedActors.Num() ||
            CurrentSelectedActors.Difference(ObservedSelectedActors).Num() > 0)
        {
            ObservedSelectedActors = MoveTemp(CurrentSelectedActors);
            bObservedActorSelection = true;
            bPendingRefresh = true;
            // Native framing can run after this tick. Recheck at bounded
            // offsets rather than waiting for the three-second safety audit.
            SelectionAuditStartedAt = CurrentTime;
            NextSelectionAudit = 0;
        }

        // Selection framing is asynchronous relative to the editor selection
        // notification. Scene Outliner may expand an ancestor AFTER our first
        // reconciliation, while the selection set itself stays unchanged.
        // Inspect only already-generated, visible row widgets against the last
        // committed native expansion snapshot. This is O(visible rows), not a
        // walk of the Outliner hierarchy, and catches a late native expansion
        // on the next Slate tick rather than on the 3-second safety audit.
        if (!bPendingRefresh && HierarchySettleFrames == 0)
        {
            for (const FRowRecord& Record : Rows)
            {
                const TSharedPtr<ISceneOutliner> RowOutliner = Record.Outliner.Pin();
                const FSceneOutlinerTreeItemPtr Item = Record.Item.Pin();
                if (!RowOutliner || !Item || !Record.Row.IsValid())
                {
                    continue;
                }
                for (const FOutlinerRuntime& Runtime : Outliners)
                {
                    if (Runtime.Outliner.Pin() != RowOutliner || !Runtime.bHasExpansionSnapshot)
                    {
                        continue;
                    }
                    if (const bool* Previous = Runtime.LastPhysicalExpansion.Find(Item->GetID()))
                    {
                        if (*Previous != MutableTree(*RowOutliner).IsItemExpanded(Item))
                        {
                            bPendingRefresh = true;
                        }
                    }
                    break;
                }
                if (bPendingRefresh)
                {
                    break;
                }
            }
        }

        // Each selection schedules at most four additional hierarchy passes,
        // spread across Slate frames. Between passes the fast path only checks
        // the clock, not the complete tree. This is independent of whether the
        // native expansion affected a currently registered row widget.
        static constexpr double SelectionAuditDelays[] = { 0.05, 0.15, 0.35, 0.75 };
        if (NextSelectionAudit < UE_ARRAY_COUNT(SelectionAuditDelays) &&
            CurrentTime - SelectionAuditStartedAt >= SelectionAuditDelays[NextSelectionAudit])
        {
            ++NextSelectionAudit;
            bPendingRefresh = true;
        }

        const bool bHierarchyDirty = bPendingRefresh || HierarchySettleFrames > 0 || CurrentTime >= NextRefreshTime;
        const bool bRowsDirty = bPendingRowRefresh;
        if (!bHierarchyDirty && !bRowsDirty)
        {
            return;
        }

        // Slate frequently generates or recycles rows during normal scrolling.
        // Maintain glyph bindings for those rows without traversing the tree or
        // calculating Mixed paths. Hierarchy work remains event driven, with a
        // slower compatibility audit for other plugins and engine-side changes.
        bPendingRowRefresh = false;
        RefreshRows();
        if (!bHierarchyDirty)
        {
            return;
        }

        bPendingRefresh = false;
        if (HierarchySettleFrames > 0)
        {
            --HierarchySettleFrames;
        }
        NextRefreshTime = CurrentTime + FallbackRefreshSeconds;
        Outliners.RemoveAll([](const FOutlinerRuntime& Runtime)
        {
            return !Runtime.Outliner.IsValid();
        });
        for (FOutlinerRuntime& Runtime : Outliners)
        {
            UpdateOutliner(Runtime);
        }
    }

    void FRegistry::Stop()
    {
        bActive = false;

        for (FOutlinerRuntime& Runtime : Outliners)
        {
            RestorePhysicalOverrides(Runtime, false);
        }

        for (FRowRecord& Record : Rows)
        {
            RestoreRecord(Record, true);
        }

        Rows.Reset();
        Outliners.Reset();
        HierarchySettleFrames = 0;
        bPendingRowRefresh = false;
        NextSelectionAudit = 4;
    }

    TSharedPtr<ISceneOutliner> FRegistry::FindOutlinerForPath(const FWidgetPath& Path) const
    {
        for (int32 Index = 0; Index < Path.Widgets.Num(); ++Index)
        {
            const TSharedRef<SWidget> Widget = Path.Widgets[Index].Widget;
            if (Widget->GetType() == FName(TEXT("SSceneOutliner")))
            {
                const TSharedPtr<ISceneOutliner> Candidate = StaticCastSharedRef<ISceneOutliner>(Widget);
                for (const FOutlinerRuntime& Runtime : Outliners)
                {
                    if (Runtime.Outliner.Pin() == Candidate)
                    {
                        return Candidate;
                    }
                }
            }
        }
        return nullptr;
    }

    FSceneOutlinerTreeItemPtr FRegistry::FindItemForPath(const FWidgetPath& Path, TSharedPtr<ISceneOutliner>& Outliner)
    {
        Outliner = FindOutlinerForPath(Path);
        if (!Outliner)
        {
            return nullptr;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(*Outliner);

        // Resolve the native row on the live Slate hit-test path, not through
        // Latch's observer-column cache. Rows for nested/recycled branches may
        // be created after the last reconciliation or may not have reached the
        // observer column when the first pointer event arrives.
        for (int32 Index = Path.Widgets.Num() - 1; Index >= 0; --Index)
        {
            const TSharedRef<SWidget> Widget = Path.Widgets[Index].Widget;
            if (Widget->GetType() != FName(TEXT("SSceneOutlinerTreeRow")))
            {
                continue;
            }

            const TSharedRef<STableRow<FSceneOutlinerTreeItemPtr>> Row =
                StaticCastSharedRef<STableRow<FSceneOutlinerTreeItemPtr>>(Widget);
            if (const FSceneOutlinerTreeItemPtr* Item = Tree.ItemFromWidget(&Row.Get()))
            {
                if (*Item)
                {
                    RegisterRow(*Outliner, (*Item).ToSharedRef(), *Row);
                    return *Item;
                }
            }
        }

        // Compatibility fallback for custom Outliner row implementations.
        for (const FRowRecord& Record : Rows)
        {
            if (Record.Outliner.Pin() != Outliner)
            {
                continue;
            }
            const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin();
            if (!Row)
            {
                continue;
            }
            for (int32 WidgetIndex = 0; WidgetIndex < Path.Widgets.Num(); ++WidgetIndex)
            {
                if (Path.Widgets[WidgetIndex].Widget == Row)
                {
                    if (const FSceneOutlinerTreeItemPtr* Item = Tree.ItemFromWidget(Row.Get()))
                    {
                        return *Item;
                    }
                    break;
                }
            }
        }
        return nullptr;
    }

    bool FRegistry::PathContainsExpander(const FWidgetPath& Path) const
    {
        for (int32 Index = 0; Index < Path.Widgets.Num(); ++Index)
        {
            const FName Type = Path.Widgets[Index].Widget->GetType();
            if (Type == FName(TEXT("SExpanderArrow")) || Type == FName(TEXT("SGuideArrow")))
            {
                return true;
            }
        }
        return false;
    }

    TArray<FSceneOutlinerTreeItemPtr> FRegistry::ResolveLiveItems(
        const TSharedPtr<ISceneOutliner>& Outliner,
        const TArray<FSceneOutlinerTreeItemID>& ItemIDs) const
    {
        TArray<FSceneOutlinerTreeItemPtr> Result;
        if (!Outliner || ItemIDs.IsEmpty())
        {
            return Result;
        }

        TSet<FSceneOutlinerTreeItemID> WantedIDs;
        for (const FSceneOutlinerTreeItemID& ID : ItemIDs)
        {
            WantedIDs.Add(ID);
        }

        // Traverse the Outliner's real current hierarchy instead of asking Scene
        // Outliner to manufacture/return an item from a possibly changing cache.
        // This is important for context menus opened while viewport selection is
        // synchronizing or while Mixed presentation is rebuilding rows.
        for (const FSceneOutlinerTreeItemPtr& Candidate : AllItems(*Outliner))
        {
            if (Candidate && WantedIDs.Contains(Candidate->GetID()) && IsSupportedItem(Candidate))
            {
                Result.AddUnique(Candidate);
            }
        }
        return Result;
    }

    TArray<FSceneOutlinerTreeItemPtr> FRegistry::GetDirectInteractionItems(
        ISceneOutliner& Outliner,
        const FSceneOutlinerTreeItemPtr& ClickedItem) const
    {
        TArray<FSceneOutlinerTreeItemPtr> Result;
        if (!IsSupportedItem(ClickedItem))
        {
            return Result;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(Outliner);
        const TArray<FSceneOutlinerTreeItemPtr> SelectedItems = Tree.GetSelectedItems();

        bool bClickedIsSelected = false;
        for (const FSceneOutlinerTreeItemPtr& SelectedItem : SelectedItems)
        {
            if (SelectedItem && SelectedItem->GetID() == ClickedItem->GetID())
            {
                bClickedIsSelected = true;
                break;
            }
        }

        if (bClickedIsSelected && SelectedItems.Num() > 1)
        {
            for (const FSceneOutlinerTreeItemPtr& SelectedItem : SelectedItems)
            {
                if (IsSupportedItem(SelectedItem))
                {
                    Result.AddUnique(SelectedItem);
                }
            }
        }
        else
        {
            Result.Add(ClickedItem);
        }

        return Result;
    }

    bool FRegistry::HasLiveChildren(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item) const
    {
        if (!Item)
        {
            return false;
        }

        // Native row geometry is the same source Unreal uses to decide whether
        // the expander triangle is visible. The item child list can temporarily be
        // empty for nested folders while Outliner rows are reconstructed.
        for (const FRowRecord& Record : Rows)
        {
            if (Record.Outliner.Pin().Get() != &Outliner)
            {
                continue;
            }
            const TSharedPtr<STableRow<FSceneOutlinerTreeItemPtr>> Row = Record.Row.Pin();
            if (!Row)
            {
                continue;
            }
            const FSceneOutlinerTreeItemPtr* LiveItem = MutableTree(Outliner).ItemFromWidget(Row.Get());
            if (LiveItem && *LiveItem && (*LiveItem)->GetID() == Item->GetID())
            {
                return Row->DoesItemHaveChildren() || HasChildren(Item);
            }
        }
        return HasChildren(Item);
    }

    void FRegistry::ToggleLatch(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem || !IsSupportedItem(Item) || !HasLiveChildren(Outliner, Item))
        {
            return;
        }

        Subsystem->ToggleCurrentExpansionLatch(Item, LogicalExpansion(Item));
        NextRefreshTime = 0.0;
    }

    void FRegistry::ToggleLatchForInteraction(
        ISceneOutliner& Outliner,
        const FSceneOutlinerTreeItemPtr& ClickedItem)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem || !IsSupportedItem(ClickedItem) || !HasLiveChildren(Outliner, ClickedItem))
        {
            return;
        }

        const TArray<FSceneOutlinerTreeItemPtr> Items = GetDirectInteractionItems(Outliner, ClickedItem);
        if (Items.Num() <= 1)
        {
            ToggleLatch(Outliner, ClickedItem);
            return;
        }

        // The clicked row defines the direct-control intent for the whole current
        // selection. Red means unlatch the selected parents. Otherwise the clicked
        // row's logical expansion chooses Latched Open or Latched Closed for every
        // eligible selected parent. Childless selected items are ignored.
        const ELatchExpansionState ClickedState = Subsystem->GetExpansionState(ClickedItem);
        if (ClickedState != ELatchExpansionState::Unlatched)
        {
            SetLatchForItems(Items, static_cast<int32>(ELatchExpansionState::Unlatched));
        }
        else
        {
            const ELatchExpansionState TargetState = LogicalExpansion(ClickedItem)
                ? ELatchExpansionState::LatchedOpen
                : ELatchExpansionState::LatchedClosed;
            SetLatchForItems(Items, static_cast<int32>(TargetState));
        }
    }

    void FRegistry::SetLatch(const FSceneOutlinerTreeItemPtr& Item, int32 StateValue)
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            const ELatchExpansionState NewState = static_cast<ELatchExpansionState>(StateValue);
            if (NewState != ELatchExpansionState::Unlatched && !HasChildren(Item))
            {
                return;
            }
            Subsystem->SetExpansionState(Item, NewState);
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::LatchCurrentSelection(const TArray<FSceneOutlinerTreeItemPtr>& Items)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem)
        {
            return;
        }

        bool bAnyEligible = false;
        bool bAllEligibleAlreadyLatched = true;
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!IsSupportedItem(Item) || !HasChildren(Item))
            {
                continue;
            }
            bAnyEligible = true;
            if (Subsystem->GetExpansionState(Item) == ELatchExpansionState::Unlatched)
            {
                bAllEligibleAlreadyLatched = false;
            }
        }

        if (!bAnyEligible)
        {
            return;
        }

        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!IsSupportedItem(Item) || !HasChildren(Item))
            {
                continue;
            }

            if (bAllEligibleAlreadyLatched)
            {
                Subsystem->SetExpansionState(Item, ELatchExpansionState::Unlatched);
            }
            else
            {
                Subsystem->SetExpansionState(Item, LogicalExpansion(Item)
                    ? ELatchExpansionState::LatchedOpen
                    : ELatchExpansionState::LatchedClosed);
            }
        }
        NextRefreshTime = 0.0;
    }

    void FRegistry::SetLatchForItems(const TArray<FSceneOutlinerTreeItemPtr>& Items, int32 StateValue)
    {
        const ELatchExpansionState NewState = static_cast<ELatchExpansionState>(StateValue);
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!IsSupportedItem(Item))
            {
                continue;
            }
            if (NewState != ELatchExpansionState::Unlatched && !HasChildren(Item))
            {
                continue;
            }
            SetLatch(Item, StateValue);
        }
    }

    void FRegistry::SetKeepVisibleForItems(const TArray<FSceneOutlinerTreeItemPtr>& Items, bool bKeepVisible)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem)
        {
            return;
        }

        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!Item || (!Item->IsA<FActorTreeItem>() && !Item->IsA<FFolderTreeItem>()))
            {
                continue;
            }
            Subsystem->SetExposureLatched(Item, bKeepVisible);
        }
        NextRefreshTime = 0.0;
    }

    void FRegistry::ToggleKeepVisible(const FSceneOutlinerTreeItemPtr& Item)
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            Subsystem->SetExposureLatched(Item, !Subsystem->IsExposureLatched(Item));
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::ToggleKeepVisibleForInteraction(
        ISceneOutliner& Outliner,
        const FSceneOutlinerTreeItemPtr& ClickedItem)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem || !ClickedItem)
        {
            return;
        }

        const TArray<FSceneOutlinerTreeItemPtr> Items = GetDirectInteractionItems(Outliner, ClickedItem);
        bool bAnyEligible = false;
        bool bAllEligiblePinned = true;

        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (!Item || (!Item->IsA<FActorTreeItem>() && !Item->IsA<FFolderTreeItem>()))
            {
                continue;
            }

            bAnyEligible = true;
            if (!Subsystem->IsExposureLatched(Item))
            {
                bAllEligiblePinned = false;
            }
        }

        if (!bAnyEligible)
        {
            return;
        }

        // Match the context-menu batch rule. A mixed selection becomes fully
        // pinned; only an all-pinned selection toggles the whole set off.
        SetKeepVisibleForItems(Items, !bAllEligiblePinned);
    }

    void FRegistry::ClearBranch(const FSceneOutlinerTreeItemPtr& Item)
    {
        if (!Item)
        {
            return;
        }
        TArray<FSceneOutlinerTreeItemPtr> Items;
        Items.Add(Item);
        ClearBranches(Items);
    }

    void FRegistry::ClearBranches(const TArray<FSceneOutlinerTreeItemPtr>& Items)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem)
        {
            return;
        }

        TArray<FSceneOutlinerTreeItemPtr> Pending = Items;
        TSet<const ISceneOutlinerTreeItem*> Seen;
        while (!Pending.IsEmpty())
        {
            const FSceneOutlinerTreeItemPtr Current = Pending.Pop(EAllowShrinking::No);
            if (!Current || Seen.Contains(Current.Get()))
            {
                continue;
            }
            Seen.Add(Current.Get());
            Subsystem->ClearItem(Current);
            for (const TWeakPtr<ISceneOutlinerTreeItem>& WeakChild : Current->GetChildren())
            {
                Pending.Add(WeakChild.Pin());
            }
        }
        NextRefreshTime = 0.0;
    }

    void FRegistry::ClearAllExpansion()
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            Subsystem->ClearAllExpansionStates();
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::ClearAllKeepVisible()
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            Subsystem->ClearAllExposureStates();
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::ClearAll()
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            Subsystem->ClearAll();
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::ToggleSuspend()
    {
        if (ULatchSubsystem* Subsystem = State())
        {
            Subsystem->SetSuspended(!Subsystem->IsSuspended());
            NextRefreshTime = 0.0;
        }
    }

    void FRegistry::HandleRecursiveExpanderClick(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem || !Item || !HasLiveChildren(Outliner, Item))
        {
            return;
        }

        // Match native recursive intent using the clicked item's logical state. Red
        // latches keep their own state, but recursion continues through them so an
        // unlocked descendant still participates in the recursive operation.
        const bool bExpand = !LogicalExpansion(Item);
        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(Outliner);

        TArray<FSceneOutlinerTreeItemPtr> Pending;
        Pending.Add(Item);
        TSet<const ISceneOutlinerTreeItem*> Seen;
        while (!Pending.IsEmpty())
        {
            const FSceneOutlinerTreeItemPtr Current = Pending.Pop(EAllowShrinking::No);
            if (!Current || Seen.Contains(Current.Get()))
            {
                continue;
            }
            Seen.Add(Current.Get());

            for (const TWeakPtr<ISceneOutlinerTreeItem>& WeakChild : Current->GetChildren())
            {
                Pending.Add(WeakChild.Pin());
            }

            if (!HasChildren(Current))
            {
                continue;
            }

            const ELatchExpansionState LatchState = Subsystem->GetExpansionState(Current);
            if (LatchState != ELatchExpansionState::Unlatched)
            {
                continue;
            }

            // Recursive Shift+Click is an explicit user expansion decision. A Mixed
            // item may still have a ForcedPhysicalExpansion entry recording the
            // logical state from before it was temporarily opened as a bridge. If
            // that stale presentation record survives this click, the next refresh
            // can restore the old collapsed state and make recursive expansion look
            // like it partially reverted. Retire the temporary Mixed ownership
            // before committing the new logical state. If a selected or Keep Visible
            // descendant still requires a Mixed bridge after a recursive collapse,
            // UpdateOutliner will derive and register a fresh bridge from the new
            // logical state on the next refresh.
            if (FOutlinerRuntime* Runtime = FindRuntime(*this, Outliner))
            {
                Runtime->ForcedPhysicalExpansion.Remove(Current->GetID());
                Runtime->MixedItems.Remove(Current->GetID());
                Runtime->MixedRoots.Remove(Current->GetID());
            }

            SetLogicalExpansion(Current, bExpand);
            SetPhysicalExpansionPreservingLogical(Tree, Current, bExpand, bExpand);
        }

        NextRefreshTime = 0.0;
    }

    bool FRegistry::HasRequiredTargetDescendant(ISceneOutliner& Outliner, const FOutlinerRuntime& Runtime, const FSceneOutlinerTreeItemPtr& Item) const
    {
        if (!Item)
        {
            return false;
        }

        for (const FSceneOutlinerTreeItemPtr& Candidate : AllItems(Outliner))
        {
            if (Candidate && Runtime.RequiredTargets.Contains(Candidate->GetID()) && IsDescendantOf(Candidate, Item))
            {
                return true;
            }
        }
        return false;
    }

    bool FRegistry::HandleNormalExpanderClick(ISceneOutliner& Outliner, const FSceneOutlinerTreeItemPtr& Item)
    {
        if (!Item)
        {
            return false;
        }

        FOutlinerRuntime* Runtime = FindRuntime(*this, Outliner);
        if (!Runtime)
        {
            return false;
        }

        // Yellow means logically collapsed but physically open. Clicking it should
        // simply commit a normal expanded state, not toggle the physical tree closed.
        if (Runtime->MixedItems.Contains(Item->GetID()))
        {
            SetLogicalExpansion(Item, true);
            Runtime->ForcedPhysicalExpansion.Remove(Item->GetID());
            NextRefreshTime = 0.0;
            return true;
        }

        STreeView<FSceneOutlinerTreeItemPtr>& Tree = MutableTree(Outliner);
        if (Tree.IsItemExpanded(Item) && HasRequiredTargetDescendant(Outliner, *Runtime, Item))
        {
            // Collapse logically but leave it physically open. The next frame derives
            // the exact Mixed path without a native collapse/re-expand flash.
            SetLogicalExpansion(Item, false);
            Runtime->ForcedPhysicalExpansion.Add(Item->GetID(), false);
            NextRefreshTime = 0.0;
            return true;
        }

        return false;
    }

    void FRegistry::PopulateContextMenu(UToolMenu& Menu, const TSharedPtr<ISceneOutliner>& Outliner, const FSceneOutlinerTreeItemPtr& Item)
    {
        TArray<FSceneOutlinerTreeItemPtr> Items;
        if (Item)
        {
            Items.Add(Item);
        }
        PopulateContextMenu(Menu, Outliner, Items);
    }

    void FRegistry::PopulateContextMenu(UToolMenu& Menu, const TSharedPtr<ISceneOutliner>& Outliner, const TArray<FSceneOutlinerTreeItemPtr>& InItems)
    {
        ULatchSubsystem* Subsystem = State();
        if (!Subsystem || !Outliner)
        {
            return;
        }

        TArray<FSceneOutlinerTreeItemPtr> Items;
        for (const FSceneOutlinerTreeItemPtr& Item : InItems)
        {
            if (IsSupportedItem(Item))
            {
                Items.AddUnique(Item);
            }
        }
        if (Items.IsEmpty())
        {
            return;
        }

        TArray<FSceneOutlinerTreeItemID> ItemIDs;
        ItemIDs.Reserve(Items.Num());
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            ItemIDs.AddUnique(Item->GetID());
        }
        const TWeakPtr<ISceneOutliner> WeakOutliner = Outliner;

        bool bAnyHasChildren = false;
        bool bAnyLatched = false;
        bool bAnyKeepEligible = false;
        bool bAllKeepEligiblePinned = true;
        for (const FSceneOutlinerTreeItemPtr& Item : Items)
        {
            if (HasChildren(Item))
            {
                bAnyHasChildren = true;
            }
            if (Subsystem->GetExpansionState(Item) != ELatchExpansionState::Unlatched)
            {
                bAnyLatched = true;
            }
            if (Item->IsA<FActorTreeItem>() || Item->IsA<FFolderTreeItem>())
            {
                bAnyKeepEligible = true;
                if (!Subsystem->IsExposureLatched(Item))
                {
                    bAllKeepEligiblePinned = false;
                }
            }
        }

        const bool bEnableStopKeepingVisible = bAnyKeepEligible && bAllKeepEligiblePinned;
        const TWeakPtr<FRegistry> WeakRegistry = AsShared();

        FToolMenuSection& Expansion = Menu.AddSection(TEXT("LatchExpansion"), NSLOCTEXT("Latch", "ExpansionSection", "Expansion"));
        Expansion.AddMenuEntry(
            TEXT("LatchCurrent"),
            NSLOCTEXT("Latch", "LatchCurrent", "Latch Current Expansion"),
            NSLOCTEXT("Latch", "LatchCurrentTip", "Latch each selected parent in its current expanded or collapsed state. If every eligible selected item is already latched, this unlatches them."),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs]
                {
                    const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                    const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                    if (RegistryPin && OutlinerPin)
                    {
                        RegistryPin->LatchCurrentSelection(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs));
                    }
                }),
                FCanExecuteAction::CreateLambda([bAnyHasChildren]() { return bAnyHasChildren; })));

        Expansion.AddMenuEntry(
            TEXT("LatchExpanded"),
            NSLOCTEXT("Latch", "LatchExpanded", "Latch Expanded"),
            NSLOCTEXT("Latch", "LatchExpandedTip", "Latch every eligible selected parent expanded until it is unlatched."),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs]
                {
                    const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                    const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                    if (RegistryPin && OutlinerPin)
                    {
                        RegistryPin->SetLatchForItems(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs), static_cast<int32>(ELatchExpansionState::LatchedOpen));
                    }
                }),
                FCanExecuteAction::CreateLambda([bAnyHasChildren]() { return bAnyHasChildren; })));

        Expansion.AddMenuEntry(
            TEXT("LatchCollapsed"),
            NSLOCTEXT("Latch", "LatchCollapsed", "Latch Collapsed"),
            NSLOCTEXT("Latch", "LatchCollapsedTip", "Latch every eligible selected parent logically collapsed. Required descendants can still appear through them as Mixed view."),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs]
                {
                    const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                    const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                    if (RegistryPin && OutlinerPin)
                    {
                        RegistryPin->SetLatchForItems(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs), static_cast<int32>(ELatchExpansionState::LatchedClosed));
                    }
                }),
                FCanExecuteAction::CreateLambda([bAnyHasChildren]() { return bAnyHasChildren; })));

        Expansion.AddMenuEntry(
            TEXT("UnlatchExpansion"),
            NSLOCTEXT("Latch", "UnlatchExpansion", "Unlatch Expansion"),
            NSLOCTEXT("Latch", "UnlatchExpansionTip", "Return every selected item's expansion control to normal Unreal behavior."),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs]
                {
                    const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                    const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                    if (RegistryPin && OutlinerPin)
                    {
                        RegistryPin->SetLatchForItems(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs), static_cast<int32>(ELatchExpansionState::Unlatched));
                    }
                }),
                FCanExecuteAction::CreateLambda([bAnyLatched]() { return bAnyLatched; })));

        FToolMenuSection& Visibility = Menu.AddSection(TEXT("LatchVisibility"), NSLOCTEXT("Latch", "VisibilitySection", "Visibility"));
        Visibility.AddMenuEntry(
            TEXT("KeepVisible"),
            bEnableStopKeepingVisible ? NSLOCTEXT("Latch", "StopKeepingVisible", "Stop Keeping Visible") : NSLOCTEXT("Latch", "KeepVisible", "Keep Visible"),
            NSLOCTEXT("Latch", "KeepVisibleTip", "Keep selected Actors/Folders visible in the Outliner when ancestors are collapsed. Does not affect actors, levels, or gameplay."),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs, bEnableStopKeepingVisible]
                {
                    const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                    const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                    if (RegistryPin && OutlinerPin)
                    {
                        RegistryPin->SetKeepVisibleForItems(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs), !bEnableStopKeepingVisible);
                    }
                }),
                FCanExecuteAction::CreateLambda([bAnyKeepEligible]() { return bAnyKeepEligible; })));

        FToolMenuSection& Maintenance = Menu.AddSection(TEXT("LatchMaintenance"), NSLOCTEXT("Latch", "MaintenanceSection", "Latch"));
        Maintenance.AddMenuEntry(
            TEXT("ClearBranch"),
            Items.Num() > 1 ? NSLOCTEXT("Latch", "ClearBranches", "Clear Latch State in Selected Branches") : NSLOCTEXT("Latch", "ClearBranch", "Clear Latch State in Branch"),
            NSLOCTEXT("Latch", "ClearBranchTip", "Remove Expansion Latches and Keep Visible state from the selected item(s) and their descendants."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([WeakRegistry, WeakOutliner, ItemIDs]
            {
                const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin();
                const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                if (RegistryPin && OutlinerPin)
                {
                    RegistryPin->ClearBranches(RegistryPin->ResolveLiveItems(OutlinerPin, ItemIDs));
                }
            })));

        Maintenance.AddMenuEntry(
            TEXT("SuspendLatch"),
            Subsystem->IsSuspended() ? NSLOCTEXT("Latch", "Resume", "Resume Latch") : NSLOCTEXT("Latch", "Suspend", "Suspend Latch"),
            NSLOCTEXT("Latch", "SuspendTip", "Temporarily return the Outliner to normal Unreal expansion behavior without deleting saved Latch state."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([WeakRegistry]
            {
                if (const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin())
                {
                    RegistryPin->ToggleSuspend();
                }
            })));

        Maintenance.AddSeparator(TEXT("LatchGlobalClearSeparator"));

        Maintenance.AddMenuEntry(
            TEXT("ClearAllExpansion"),
            NSLOCTEXT("Latch", "ClearAllExpansion", "Clear All Expansion Latches"),
            NSLOCTEXT("Latch", "ClearAllExpansionTip", "Remove every Red Expansion Latch while preserving Keep Visible state."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([WeakRegistry]
            {
                if (const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin())
                {
                    RegistryPin->ClearAllExpansion();
                }
            })));

        Maintenance.AddMenuEntry(
            TEXT("ClearAllKeepVisible"),
            NSLOCTEXT("Latch", "ClearAllKeepVisible", "Clear All Keep Visible States"),
            NSLOCTEXT("Latch", "ClearAllKeepVisibleTip", "Remove every White Keep Visible pin while preserving Expansion Latches."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([WeakRegistry]
            {
                if (const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin())
                {
                    RegistryPin->ClearAllKeepVisible();
                }
            })));

        Maintenance.AddMenuEntry(
            TEXT("ClearAll"),
            NSLOCTEXT("Latch", "ClearAll", "Clear All Latch State"),
            NSLOCTEXT("Latch", "ClearAllTip", "Remove every saved Expansion Latch and Keep Visible state. Current ordinary expansion is left as-is."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([WeakRegistry]
            {
                if (const TSharedPtr<FRegistry> RegistryPin = WeakRegistry.Pin())
                {
                    RegistryPin->ClearAll();
                }
            })));
    }

    void FInputProcessor::Tick(float, FSlateApplication&, TSharedRef<ICursor>)
    {
        if (const TSharedPtr<FRegistry> StateRegistry = Registry.Pin())
        {
            StateRegistry->Tick(FPlatformTime::Seconds());
        }
    }

    bool FInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
    {
        if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
        {
            return false;
        }

        const TSharedPtr<FRegistry> StateRegistry = Registry.Pin();
        ULatchSubsystem* Subsystem = State();
        if (!StateRegistry || !StateRegistry->bActive || !Subsystem || Subsystem->IsSuspended())
        {
            return false;
        }

        const FWidgetPath Path = SlateApp.LocateWindowUnderMouse(
            MouseEvent.GetScreenSpacePosition(),
            SlateApp.GetInteractiveTopLevelWindows(),
            false,
            MouseEvent.GetUserIndex());

        if (!StateRegistry->PathContainsExpander(Path))
        {
            return false;
        }

        TSharedPtr<ISceneOutliner> Outliner;
        const FSceneOutlinerTreeItemPtr Item = StateRegistry->FindItemForPath(Path, Outliner);
        if (!Outliner || !FRegistry::IsSupportedItem(Item) || IsSearchActive(*Outliner))
        {
            return false;
        }

        // Match Unreal's native expander convention: Shift remains recursive.
        // Alt is Latch's only modifier and is scoped to the expander hit target.
        if (MouseEvent.IsAltDown() && !MouseEvent.IsShiftDown())
        {
            StateRegistry->ToggleLatchForInteraction(*Outliner, Item);
            return true;
        }

        if (Subsystem->GetExpansionState(Item) != ELatchExpansionState::Unlatched)
        {
            return true;
        }

        // Latch owns recursive Shift+Click so Red descendants never lose their
        // authority, even for a single frame. Unlatched descendants still follow
        // the native recursive expand/collapse intent.
        if (MouseEvent.IsShiftDown())
        {
            StateRegistry->HandleRecursiveExpanderClick(*Outliner, Item);
            return true;
        }

        return StateRegistry->HandleNormalExpanderClick(*Outliner, Item);
    }

    bool FInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
    {
        if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
        {
            return false;
        }

        const TSharedPtr<FRegistry> StateRegistry = Registry.Pin();
        ULatchSubsystem* Subsystem = State();
        if (!StateRegistry || !StateRegistry->bActive || !Subsystem || Subsystem->IsSuspended())
        {
            return false;
        }

        const FWidgetPath Path = SlateApp.LocateWindowUnderMouse(
            MouseEvent.GetScreenSpacePosition(),
            SlateApp.GetInteractiveTopLevelWindows(),
            false,
            MouseEvent.GetUserIndex());

        if (!StateRegistry->PathContainsExpander(Path))
        {
            return false;
        }

        TSharedPtr<ISceneOutliner> Outliner;
        const FSceneOutlinerTreeItemPtr Item = StateRegistry->FindItemForPath(Path, Outliner);
        if (!Outliner || !FRegistry::IsSupportedItem(Item) || IsSearchActive(*Outliner))
        {
            return false;
        }

        // SButton treats double-click as another MouseDown. If the first click was
        // consumed by Latch, consume the double-click too rather than repeating it.
        if ((MouseEvent.IsAltDown() && !MouseEvent.IsShiftDown()) ||
            Subsystem->GetExpansionState(Item) != ELatchExpansionState::Unlatched)
        {
            return true;
        }

        if (MouseEvent.IsShiftDown())
        {
            return true;
        }

        if (FOutlinerRuntime* Runtime = FindRuntime(*StateRegistry, *Outliner))
        {
            return Runtime->MixedItems.Contains(Item->GetID()) ||
                (MutableTree(*Outliner).IsItemExpanded(Item) && StateRegistry->HasRequiredTargetDescendant(*Outliner, *Runtime, Item));
        }

        return false;
    }
}
