// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#include "LatchSettings.h"

#include "LatchSubsystem.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "UObject/UnrealType.h"

void ULatchSettings::ClearSavedLatchState()
{
#if WITH_EDITOR
    const EAppReturnType::Type Result = FMessageDialog::Open(
        EAppMsgType::YesNo,
        NSLOCTEXT("Latch", "ClearSavedStateConfirm", "Clear all saved Expansion Latches and Keep Visible states for this project?\n\nThis does not modify any levels or Actors."),
        NSLOCTEXT("Latch", "ClearSavedStateTitle", "Clear Saved Latch State"));

    if (Result != EAppReturnType::Yes)
    {
        return;
    }

    if (GEditor)
    {
        if (ULatchSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULatchSubsystem>())
        {
            Subsystem->ClearAll();
            Subsystem->ClearSavedStateFromDisk();
        }
    }
#endif
}

#if WITH_EDITOR
void ULatchSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    ExpansionLatchColor.A = 1.0f;
    MixedStateColor.A = 1.0f;
    ExpanderContentSpacing = FMath::Clamp(ExpanderContentSpacing, 0.0f, 10.0f);

    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ULatchSettings, bPersistStateAcrossEditorRestarts))
    {
        if (GEditor)
        {
            if (ULatchSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULatchSubsystem>())
            {
                Subsystem->ApplyPersistenceSetting();
            }
        }
    }
}
#endif
