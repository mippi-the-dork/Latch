// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#include "LatchSubsystem.h"

#include "LatchSettings.h"

#include "ActorTreeItem.h"
#include "FolderTreeItem.h"
#include "WorldTreeItem.h"
#include "ISceneOutlinerTreeItem.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/ConfigCacheIni.h"

namespace
{
    // V3 deliberately starts clean. Earlier prototype builds could leave stale
    // presentation state after Mixed-State experiments, so do not import it.
    static const TCHAR* SectionName = TEXT("Latch.State.V3");
    static const TCHAR* ExpansionKeyName = TEXT("Expansion");
    static const TCHAR* ExposureKeyName = TEXT("KeepVisible");
    static const TCHAR* SuspendedKeyName = TEXT("Suspended");

    FString WorldKey(const UWorld* World)
    {
        if (World && World->GetOutermost())
        {
            return World->GetOutermost()->GetName();
        }

        if (GEditor)
        {
            if (const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
            {
                if (const UPackage* Package = EditorWorld->GetOutermost())
                {
                    return Package->GetName();
                }
            }
        }

        return TEXT("<NoWorld>");
    }

    FString ActorKey(const AActor* Actor)
    {
        // Deletion notifications can arrive after the Actor has entered pending-kill
        // state. The pointer and GUID are still usable for identity cleanup.
        if (!Actor)
        {
            return FString();
        }

        const FGuid Guid = Actor->GetActorGuid();
        if (Guid.IsValid())
        {
            return FString::Printf(TEXT("Actor|%s|%s"),
                *WorldKey(Actor->GetWorld()),
                *Guid.ToString(EGuidFormats::DigitsWithHyphensLower));
        }

        return FString::Printf(TEXT("ActorPath|%s"), *Actor->GetPathName());
    }
}

void ULatchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (GetDefault<ULatchSettings>()->bPersistStateAcrossEditorRestarts)
    {
        LoadState();
    }
}

void ULatchSubsystem::Deinitialize()
{
    if (GetDefault<ULatchSettings>()->bPersistStateAcrossEditorRestarts)
    {
        SaveState();
    }
    Super::Deinitialize();
}

FString ULatchSubsystem::MakeItemKey(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const
{
    if (!Item)
    {
        return FString();
    }

    if (const FActorTreeItem* ActorItem = Item->CastTo<FActorTreeItem>())
    {
        const AActor* Actor = ActorItem->Actor.Get();
        if (!IsValid(Actor))
        {
            return FString();
        }

        return ActorKey(Actor);
    }

    if (const FFolderTreeItem* FolderItem = Item->CastTo<FFolderTreeItem>())
    {
        const FFolder Folder = FolderItem->GetFolder();
        const FGuid FolderGuid = Folder.GetActorFolderGuid();
        if (FolderGuid.IsValid())
        {
            return FString::Printf(TEXT("FolderGuid|%s|%s"),
                *WorldKey(nullptr),
                *FolderGuid.ToString(EGuidFormats::DigitsWithHyphensLower));
        }

        return FString::Printf(TEXT("FolderPath|%s|%s"),
            *WorldKey(nullptr),
            *Folder.GetPath().ToString());
    }

    if (const FWorldTreeItem* WorldItem = Item->CastTo<FWorldTreeItem>())
    {
        if (const UWorld* World = WorldItem->World.Get())
        {
            return FString::Printf(TEXT("World|%s"), *WorldKey(World));
        }
    }

    return FString();
}

ELatchExpansionState ULatchSubsystem::GetExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const
{
    return GetExpansionStateByKey(MakeItemKey(Item));
}

ELatchExpansionState ULatchSubsystem::GetExpansionStateByKey(const FString& Key) const
{
    if (Key.IsEmpty())
    {
        return ELatchExpansionState::Unlatched;
    }

    if (const ELatchExpansionState* Found = ExpansionStates.Find(Key))
    {
        return *Found;
    }

    return ELatchExpansionState::Unlatched;
}

void ULatchSubsystem::SetExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item, ELatchExpansionState State)
{
    const FString Key = MakeItemKey(Item);
    if (Key.IsEmpty())
    {
        return;
    }

    if (State == ELatchExpansionState::Unlatched)
    {
        ExpansionStates.Remove(Key);
    }
    else
    {
        ExpansionStates.Add(Key, State);
    }

    MarkDirty();
}

void ULatchSubsystem::ClearExpansionState(const TSharedPtr<ISceneOutlinerTreeItem>& Item)
{
    const FString Key = MakeItemKey(Item);
    if (!Key.IsEmpty() && ExpansionStates.Remove(Key) > 0)
    {
        MarkDirty();
    }
}

void ULatchSubsystem::ToggleCurrentExpansionLatch(const TSharedPtr<ISceneOutlinerTreeItem>& Item, bool bCurrentlyExpanded)
{
    if (GetExpansionState(Item) != ELatchExpansionState::Unlatched)
    {
        SetExpansionState(Item, ELatchExpansionState::Unlatched);
        return;
    }

    SetExpansionState(Item, bCurrentlyExpanded
        ? ELatchExpansionState::LatchedOpen
        : ELatchExpansionState::LatchedClosed);
}

bool ULatchSubsystem::IsExposureLatched(const TSharedPtr<ISceneOutlinerTreeItem>& Item) const
{
    return IsExposureLatchedByKey(MakeItemKey(Item));
}

bool ULatchSubsystem::IsExposureLatchedByKey(const FString& Key) const
{
    return !Key.IsEmpty() && ExposureKeys.Contains(Key);
}

void ULatchSubsystem::SetExposureLatched(const TSharedPtr<ISceneOutlinerTreeItem>& Item, bool bLatched)
{
    const FString Key = MakeItemKey(Item);
    if (Key.IsEmpty())
    {
        return;
    }

    if (bLatched)
    {
        ExposureKeys.Add(Key);
    }
    else
    {
        ExposureKeys.Remove(Key);
    }

    MarkDirty();
}

void ULatchSubsystem::ClearItem(const TSharedPtr<ISceneOutlinerTreeItem>& Item)
{
    const FString Key = MakeItemKey(Item);
    if (Key.IsEmpty())
    {
        return;
    }

    const int32 RemovedExpansion = ExpansionStates.Remove(Key);
    const int32 RemovedExposure = ExposureKeys.Remove(Key);
    if (RemovedExpansion > 0 || RemovedExposure > 0)
    {
        MarkDirty();
    }
}

void ULatchSubsystem::ClearActorState(const AActor* Actor)
{
    const FString Key = ActorKey(Actor);
    if (Key.IsEmpty())
    {
        return;
    }

    const int32 RemovedExpansion = ExpansionStates.Remove(Key);
    const int32 RemovedExposure = ExposureKeys.Remove(Key);
    if (RemovedExpansion > 0 || RemovedExposure > 0)
    {
        MarkDirty();
    }
}

void ULatchSubsystem::ClearAllExpansionStates()
{
    if (!ExpansionStates.IsEmpty())
    {
        ExpansionStates.Reset();
        MarkDirty();
    }
}

void ULatchSubsystem::ClearAllExposureStates()
{
    if (!ExposureKeys.IsEmpty())
    {
        ExposureKeys.Reset();
        MarkDirty();
    }
}

void ULatchSubsystem::ClearAll()
{
    if (ExpansionStates.IsEmpty() && ExposureKeys.IsEmpty())
    {
        return;
    }

    ExpansionStates.Reset();
    ExposureKeys.Reset();
    MarkDirty();
}


void ULatchSubsystem::ClearSavedStateFromDisk()
{
    GConfig->EmptySection(SectionName, GEditorPerProjectIni);
    GConfig->Flush(false, GEditorPerProjectIni);
}

void ULatchSubsystem::ApplyPersistenceSetting()
{
    if (GetDefault<ULatchSettings>()->bPersistStateAcrossEditorRestarts)
    {
        SaveState();
    }
    else
    {
        // Keep the current in-memory session state, but remove persisted state so
        // the next editor session starts clean.
        ClearSavedStateFromDisk();
    }
}

void ULatchSubsystem::SetSuspended(bool bInSuspended)
{
    if (bSuspended == bInSuspended)
    {
        return;
    }

    bSuspended = bInSuspended;
    MarkDirty();
}

void ULatchSubsystem::LoadState()
{
    ExpansionStates.Reset();
    ExposureKeys.Reset();

    TArray<FString> ExpansionEntries;
    GConfig->GetArray(SectionName, ExpansionKeyName, ExpansionEntries, GEditorPerProjectIni);
    for (const FString& Entry : ExpansionEntries)
    {
        FString Key;
        FString Value;
        if (!Entry.Split(TEXT("="), &Key, &Value, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
        {
            continue;
        }

        const int32 StateValue = FCString::Atoi(*Value);
        if (StateValue == static_cast<int32>(ELatchExpansionState::LatchedOpen) ||
            StateValue == static_cast<int32>(ELatchExpansionState::LatchedClosed))
        {
            ExpansionStates.Add(Key, static_cast<ELatchExpansionState>(StateValue));
        }
    }

    TArray<FString> ExposureEntries;
    GConfig->GetArray(SectionName, ExposureKeyName, ExposureEntries, GEditorPerProjectIni);
    for (const FString& Entry : ExposureEntries)
    {
        if (!Entry.IsEmpty())
        {
            ExposureKeys.Add(Entry);
        }
    }

    GConfig->GetBool(SectionName, SuspendedKeyName, bSuspended, GEditorPerProjectIni);
}

void ULatchSubsystem::SaveState() const
{
    if (!GetDefault<ULatchSettings>()->bPersistStateAcrossEditorRestarts)
    {
        return;
    }

    TArray<FString> ExpansionEntries;
    ExpansionEntries.Reserve(ExpansionStates.Num());
    for (const TPair<FString, ELatchExpansionState>& Pair : ExpansionStates)
    {
        ExpansionEntries.Add(FString::Printf(TEXT("%s=%d"), *Pair.Key, static_cast<int32>(Pair.Value)));
    }
    ExpansionEntries.Sort();

    TArray<FString> ExposureEntries = ExposureKeys.Array();
    ExposureEntries.Sort();

    GConfig->SetArray(SectionName, ExpansionKeyName, ExpansionEntries, GEditorPerProjectIni);
    GConfig->SetArray(SectionName, ExposureKeyName, ExposureEntries, GEditorPerProjectIni);
    GConfig->SetBool(SectionName, SuspendedKeyName, bSuspended, GEditorPerProjectIni);
    GConfig->Flush(false, GEditorPerProjectIni);
}

void ULatchSubsystem::MarkDirty()
{
    if (GetDefault<ULatchSettings>()->bPersistStateAcrossEditorRestarts)
    {
        SaveState();
    }
}
