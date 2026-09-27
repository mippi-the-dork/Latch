// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LatchSubsystem.generated.h"

struct ISceneOutlinerTreeItem;
class AActor;

UENUM()
enum class ELatchExpansionState : uint8
{
    Unlatched,
    LatchedOpen,
    LatchedClosed
};

/**
 * Persistent editor-presentation state for Latch.
 *
 * Latch never serializes hierarchy, visibility, transform, attachment, folder
 * membership, or gameplay state. This subsystem stores only Outliner intent.
 */
UCLASS()
class LATCH_API ULatchSubsystem final : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    FString MakeItemKey(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const;

    ELatchExpansionState GetExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const;
    ELatchExpansionState GetExpansionStateByKey(const FString& Key) const;
    void SetExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item, ELatchExpansionState State);
    void ClearExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item);
    void ToggleCurrentExpansionLatch(const TSharedPtr<ISceneOutlinerTreeItem>& Item, bool bCurrentlyExpanded);

    bool IsExposureLatched(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const;
    bool IsExposureLatchedByKey(const FString& Key) const;
    void SetExposureLatched(const TSharedPtr<ISceneOutlinerTreeItem>& Item, bool bLatched);

    void ClearItem(const TSharedPtr<ISceneOutlinerTreeItem>& Item);
    void ClearActorState(const AActor* Actor);
    void ClearAllExpansionStates();
    void ClearAllExposureStates();
    void ClearAll();
    void ClearSavedStateFromDisk();
    void ApplyPersistenceSetting();

    bool IsSuspended() const { return bSuspended; }
    void SetSuspended(bool bInSuspended);

private:
    void LoadState();
    void SaveState() const;
    void MarkDirty();

    TMap<FString, ELatchExpansionState> ExpansionStates;
    TSet<FString> ExposureKeys;
    bool bSuspended = false;
};
