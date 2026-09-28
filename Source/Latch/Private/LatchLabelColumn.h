// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"
#include "ISceneOutliner.h"
#include "ISceneOutlinerTreeItem.h"
#include "SceneOutlinerPublicTypes.h"
#include "ActorTreeItem.h"
#include "FolderTreeItem.h"
#include "LatchSettings.h"
#include "LatchSubsystem.h"
#include "LatchInteraction.h"
#include "Editor.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Views/STableRow.h"

namespace Latch
{
    /**
     * Native-looking Keep Visible control inserted between the normal Actor/Folder
     * icon and label. Its slot always exists so row labels never shift.
     *
     * Inactive: hidden until the pointer enters the pin slot, then subdued grey.
     * Active: native pinned glyph in pure white at all times.
     */
    class SKeepVisiblePin final : public SButton
    {
    public:
        SLATE_BEGIN_ARGS(SKeepVisiblePin) {}
        SLATE_END_ARGS()

        void Construct(
            const FArguments& InArgs,
            TWeakPtr<ISceneOutlinerTreeItem> InItem,
            TWeakPtr<ISceneOutliner> InOutliner,
            TWeakPtr<FRegistry> InRegistry)
        {
            Item = MoveTemp(InItem);
            Outliner = MoveTemp(InOutliner);
            Registry = MoveTemp(InRegistry);
            PinnedBrush = FAppStyle::Get().GetBrush(TEXT("Icons.Pinned"));
            UnpinnedBrush = FAppStyle::Get().GetBrush(TEXT("Icons.Unpinned"));

            SButton::Construct(
                SButton::FArguments()
                .ButtonStyle(FCoreStyle::Get(), TEXT("NoBorder"))
                .ClickMethod(EButtonClickMethod::MouseDown)
                .ContentPadding(0.0f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .IsFocusable(false)
                .Visibility(this, &SKeepVisiblePin::GetPinVisibility)
                .ToolTipText(this, &SKeepVisiblePin::GetPinTooltip)
                .OnClicked(this, &SKeepVisiblePin::TogglePin)
                [
                    SNew(SImage)
                    .Image(this, &SKeepVisiblePin::GetPinBrush)
                    .ColorAndOpacity(this, &SKeepVisiblePin::GetPinColor)
                    .ToolTipText(this, &SKeepVisiblePin::GetPinTooltip)
                ]);
        }

    private:
        bool IsEditorItem() const
        {
            const TSharedPtr<ISceneOutlinerTreeItem> ItemPin = Item.Pin();
            if (const FActorTreeItem* ActorItem = ItemPin ? ItemPin->CastTo<FActorTreeItem>() : nullptr)
            {
                const AActor* Actor = ActorItem->Actor.Get();
                const UWorld* World = IsValid(Actor) ? Actor->GetWorld() : nullptr;
                return World && (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview);
            }
            return ItemPin.IsValid();
        }

        ULatchSubsystem* GetState() const
        {
            return GEditor ? GEditor->GetEditorSubsystem<ULatchSubsystem>() : nullptr;
        }

        bool IsPinned() const
        {
            const TSharedPtr<ISceneOutlinerTreeItem> ItemPin = Item.Pin();
            if (const ULatchSubsystem* Subsystem = GetState())
            {
                return IsEditorItem() && ItemPin && Subsystem->IsExposureLatched(ItemPin);
            }
            return false;
        }

        FReply TogglePin()
        {
            const TSharedPtr<ISceneOutlinerTreeItem> ItemPin = Item.Pin();
            ULatchSubsystem* Subsystem = GetState();
            if (!ItemPin || !Subsystem || !IsEditorItem())
            {
                return FReply::Unhandled();
            }

            const TSharedPtr<ISceneOutliner> OutlinerPin = Outliner.Pin();
            const TSharedPtr<FRegistry> RegistryPin = Registry.Pin();
            if (OutlinerPin && RegistryPin)
            {
                RegistryPin->ToggleKeepVisibleForInteraction(*OutlinerPin, ItemPin);
            }
            else
            {
                // Safe single-item fallback during an Outliner rebuild/teardown.
                Subsystem->SetExposureLatched(ItemPin, !Subsystem->IsExposureLatched(ItemPin));
            }
            return FReply::Handled().PreventThrottling();
        }

        virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
        {
            if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
            {
                // MouseDown click method already toggled on the first click. Consume
                // the second down without toggling or selecting the row.
                return FReply::Handled().PreventThrottling();
            }
            return FReply::Unhandled();
        }

        const FSlateBrush* GetPinBrush() const
        {
            return IsPinned() ? PinnedBrush : UnpinnedBrush;
        }

        FSlateColor GetPinColor() const
        {
            if (IsPinned())
            {
                return FSlateColor(FLinearColor::White);
            }

            return IsHovered()
                ? FSlateColor::UseSubduedForeground()
                : FSlateColor(FLinearColor::Transparent);
        }

        EVisibility GetPinVisibility() const
        {
            if (IsPinned() || GetDefault<ULatchSettings>()->bShowKeepVisibleHoverControl)
            {
                return EVisibility::Visible;
            }

            // Hidden preserves the reserved slot while removing an invisible click target.
            return EVisibility::Hidden;
        }

        FText GetPinTooltip() const
        {
            return IsPinned()
                ? NSLOCTEXT("Latch", "KeepVisiblePinOn", "Keep Visible: On\nThis item remains visible in the World Outliner when its ancestors are collapsed. Click to disable.")
                : NSLOCTEXT("Latch", "KeepVisiblePinOff", "Keep Visible\nKeep this item visible in the World Outliner when its ancestors are collapsed.");
        }

        TWeakPtr<ISceneOutlinerTreeItem> Item;
        TWeakPtr<ISceneOutliner> Outliner;
        TWeakPtr<FRegistry> Registry;
        const FSlateBrush* PinnedBrush = nullptr;
        const FSlateBrush* UnpinnedBrush = nullptr;
    };

    /**
     * Decorates Unreal's existing Item Label column. It does not replace label
     * generation, rename, search, icon coloring, or sorting. This deliberately
     * composes with wrappers such as Chroma by always delegating to Original first.
     */
    class FLabelColumn final : public ISceneOutlinerColumn
    {
    public:
        FLabelColumn(
            TSharedRef<ISceneOutlinerColumn> InOriginal,
            ISceneOutliner& InOutliner,
            TSharedRef<FRegistry> InRegistry)
            : Original(MoveTemp(InOriginal))
            , Outliner(StaticCastSharedRef<ISceneOutliner>(InOutliner.AsShared()))
            , Registry(InRegistry)
        {
        }

        virtual FName GetColumnID() override { return Original->GetColumnID(); }
        virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override { return Original->ConstructHeaderRowColumn(); }

        virtual const TSharedRef<SWidget> ConstructRowWidget(
            FSceneOutlinerTreeItemRef Item,
            const STableRow<FSceneOutlinerTreeItemPtr>& Row) override
        {
            const TSharedRef<SWidget> Widget = Original->ConstructRowWidget(Item, Row);

            // Keep Visible is meaningful for actual Actors/Folders. The world root
            // intentionally has no pin slot because it has no ancestor to expose through.
            const bool bActor = Item->IsA<FActorTreeItem>();
            const bool bFolder = Item->IsA<FFolderTreeItem>();
            if (bActor)
            {
                const FActorTreeItem* ActorItem = Item->CastTo<FActorTreeItem>();
                const AActor* Actor = ActorItem ? ActorItem->Actor.Get() : nullptr;
                const UWorld* World = IsValid(Actor) ? Actor->GetWorld() : nullptr;
                if (!World || (World->WorldType != EWorldType::Editor && World->WorldType != EWorldType::EditorPreview))
                {
                    return Widget;
                }
            }
            if (!bActor && !bFolder)
            {
                return Widget;
            }

            const FName WidgetType = Widget->GetType();
            if ((bActor && WidgetType != FName(TEXT("SActorTreeLabel"))) ||
                (bFolder && WidgetType != FName(TEXT("SActorFolderTreeLabel"))))
            {
                // Unknown/custom labels are left exactly as supplied by Unreal or
                // another plugin rather than risking rename/focus behavior.
                return Widget;
            }

            if (TSharedPtr<SHorizontalBox> LabelBox = FindOuterLabelBox(Widget))
            {
                // Pin is always between the native type icon and the item label.
                LabelBox->InsertSlot(1)
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(0.0f, 0.0f, 2.0f, 0.0f)
                [
                    SNew(SBox)
                    .WidthOverride(16.0f)
                    .HeightOverride(16.0f)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    [
                        SNew(SKeepVisiblePin,
                            TWeakPtr<ISceneOutlinerTreeItem>(Item),
                            Outliner,
                            Registry)
                    ]
                ];

                // A small fixed slot before the native Actor/Folder icon controls the
                // distance between the expander area and row content. The attribute is
                // live, so Project Settings changes apply without rebuilding the Outliner.
                LabelBox->InsertSlot(0)
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride_Lambda([]() -> FOptionalSize
                    {
                        return FOptionalSize(FMath::Clamp(GetDefault<ULatchSettings>()->ExpanderContentSpacing, 0.0f, 10.0f));
                    })
                ];
            }

            return Widget;
        }

        virtual void Tick(double CurrentTime, float DeltaTime) override { Original->Tick(CurrentTime, DeltaTime); }
        virtual void PopulateSearchStrings(const ISceneOutlinerTreeItem& Item, TArray<FString>& OutSearchStrings) const override { Original->PopulateSearchStrings(Item, OutSearchStrings); }
        virtual bool SupportsSorting() const override { return Original->SupportsSorting(); }
        virtual void SortItems(TArray<FSceneOutlinerTreeItemPtr>& Items, EColumnSortMode::Type Mode) const override { Original->SortItems(Items, Mode); }
        virtual void OnSortRequested(EColumnSortPriority::Type Priority, EColumnSortMode::Type Mode) override { Original->OnSortRequested(Priority, Mode); }
        virtual bool IsSortReady() override { return Original->IsSortReady(); }

    private:
        static TSharedPtr<SHorizontalBox> FindOuterLabelBox(const TSharedRef<SWidget>& Widget)
        {
            FChildren* Children = Widget->GetChildren();
            if (!Children || Children->Num() == 0)
            {
                return nullptr;
            }

            const TSharedRef<SWidget> Child = Children->GetChildAt(0);
            if (Child->GetType() == FName(TEXT("SHorizontalBox")))
            {
                return StaticCastSharedRef<SHorizontalBox>(Child);
            }
            return nullptr;
        }

        TSharedRef<ISceneOutlinerColumn> Original;
        TWeakPtr<ISceneOutliner> Outliner;
        TWeakPtr<FRegistry> Registry;
    };
}
