// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISceneOutlinerColumn.h"
#include "ISceneOutliner.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SNullWidget.h"
#include "LatchInteraction.h"

namespace Latch
{
    class FObserverColumn final : public ISceneOutlinerColumn
    {
    public:
        FObserverColumn(ISceneOutliner& InOutliner, const TSharedRef<FRegistry>& InRegistry)
            : Outliner(StaticCastSharedRef<ISceneOutliner>(InOutliner.AsShared()))
            , Registry(InRegistry)
        {
            InRegistry->RegisterOutliner(InOutliner);
        }

        static FName GetID()
        {
            static const FName ColumnID(TEXT("LatchObserverColumn"));
            return ColumnID;
        }

        virtual FName GetColumnID() override { return GetID(); }

        virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override
        {
            return SHeaderRow::Column(GetID())
                .DefaultLabel(FText::GetEmpty())
                .ManualWidth(1.0f)
                .Visibility(EVisibility::Hidden);
        }

        virtual const TSharedRef<SWidget> ConstructRowWidget(
            FSceneOutlinerTreeItemRef Item,
            const STableRow<FSceneOutlinerTreeItemPtr>& Row) override
        {
            const TSharedPtr<ISceneOutliner> View = Outliner.Pin();
            const TSharedPtr<FRegistry> State = Registry.Pin();
            if (View && State && Row.GetType() == FName(TEXT("SSceneOutlinerTreeRow")))
            {
                State->RegisterRow(*View, Item, Row);
            }
            return SNullWidget::NullWidget;
        }

    private:
        TWeakPtr<ISceneOutliner> Outliner;
        TWeakPtr<FRegistry> Registry;
    };
}
