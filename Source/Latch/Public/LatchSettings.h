// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LatchSettings.generated.h"

struct FPropertyChangedEvent;

/** Project-wide presentation preferences for the Latch editor plugin. */
UCLASS(Config=Latch, DefaultConfig, meta=(DisplayName="Latch"))
class LATCH_API ULatchSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category="Appearance", meta=(DisplayName="Expander Content Spacing", ToolTip="Adds horizontal spacing, in Slate units, between the hierarchy expander and the Actor or Folder content. Set to 0 to match Unreal's default spacing.", ClampMin="0.0", ClampMax="10.0", UIMin="0.0", UIMax="10.0"))
    float ExpanderContentSpacing = 2.0f;

    UPROPERTY(Config, EditAnywhere, Category="Appearance", meta=(DisplayName="Expansion Latch Color", ToolTip="Color used for expansion-latched hierarchy triangles.", HideAlphaChannel))
    FLinearColor ExpansionLatchColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Appearance", meta=(DisplayName="Mixed State Color", ToolTip="Color used when a logically collapsed item is selectively exposing descendants.", HideAlphaChannel))
    FLinearColor MixedStateColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category="Appearance", meta=(DisplayName="Show Keep Visible Hover Control", ToolTip="Show the inactive Keep Visible pin when hovering its reserved slot. Active pins remain visible regardless of this setting."))
    bool bShowKeepVisibleHoverControl = true;

    UPROPERTY(Config, EditAnywhere, Category="Persistence", meta=(DisplayName="Persist State Across Editor Restarts", ToolTip="Preserve Expansion Latches, Keep Visible state, and Suspend state across editor restarts. When disabled, Latch state remains available for the current editor session only."))
    bool bPersistStateAcrossEditorRestarts = true;

    UFUNCTION(CallInEditor, Category="Utilities", meta=(DisplayName="Clear Saved Latch State"))
    void ClearSavedLatchState();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    virtual FName GetSectionName() const override { return TEXT("Latch"); }
    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
