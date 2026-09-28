// Copyright Mippithedork 2026, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"

#include "LatchInteraction.h"
#include "LatchObserverColumn.h"
#include "LatchLabelColumn.h"
#include "LatchSubsystem.h"
#include "SSceneOutliner.h"
#include "SceneOutlinerMenuContext.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "LevelEditorMenuContext.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "ToolMenus.h"

namespace
{
    TSet<FName> LatchExtendedContextMenus;
}

class FLatchModule final : public IModuleInterface
{
    void HandleLevelActorDeleted(AActor* Actor)
    {
        if (GEditor)
        {
            if (ULatchSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULatchSubsystem>())
            {
                Subsystem->ClearActorState(Actor);
            }
        }
    }

    void RegisterViewportActorMenu()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.ActorContextMenu"));
        const TWeakPtr<Latch::FRegistry> WeakRegistry = Registry;

        Menu->AddDynamicSection(TEXT("Latch.ViewportActorMenu"), FNewToolMenuDelegate::CreateLambda(
            [WeakRegistry](UToolMenu* InMenu)
            {
                const TSharedPtr<Latch::FRegistry> RegistryPin = WeakRegistry.Pin();
                const ULevelEditorContextMenuContext* Context = InMenu->FindContext<ULevelEditorContextMenuContext>();
                if (!RegistryPin || !GEditor || GEditor->PlayWorld || !Context || Context->ContextType != ELevelEditorMenuContext::Viewport)
                {
                    return;
                }

                const TWeakObjectPtr<AActor> TargetActor = Context->HitProxyActor;
                AActor* Actor = TargetActor.Get();
                if (!IsValid(Actor))
                {
                    return;
                }

                TSharedPtr<ISceneOutliner> Outliner;
                if (const TSharedPtr<ILevelEditor> LevelEditor = Context->LevelEditor.Pin())
                {
                    Outliner = LevelEditor->GetMostRecentlyUsedSceneOutliner();
                }

                auto ContainsActor = [&RegistryPin](const TSharedPtr<ISceneOutliner>& Candidate, AActor* CandidateActor) -> bool
                {
                    if (!Candidate || !IsValid(CandidateActor))
                    {
                        return false;
                    }

                    TArray<FSceneOutlinerTreeItemID> IDs;
                    IDs.Add(FSceneOutlinerTreeItemID(CandidateActor));
                    return !RegistryPin->ResolveLiveItems(Candidate, IDs).IsEmpty();
                };

                // Prefer an Outliner that can resolve the actor actually right-clicked.
                if (!ContainsActor(Outliner, Actor))
                {
                    Outliner.Reset();
                    for (const Latch::FOutlinerRuntime& Runtime : RegistryPin->Outliners)
                    {
                        const TSharedPtr<ISceneOutliner> Candidate = Runtime.Outliner.Pin();
                        if (ContainsActor(Candidate, Actor))
                        {
                            Outliner = Candidate;
                            break;
                        }
                    }
                }

                if (!Outliner)
                {
                    return;
                }

                FToolMenuSection& Section = InMenu->AddSection(TEXT("Latch"), NSLOCTEXT("Latch", "ViewportContextSection", "Latch"));
                Section.AddSubMenu(
                    TEXT("Latch.Controls"),
                    NSLOCTEXT("Latch", "ViewportContextSubmenu", "Latch"),
                    NSLOCTEXT("Latch", "ViewportContextSubmenuTip", "Control this Actor's persistent World Outliner expansion and selective visibility."),
                    FNewToolMenuDelegate::CreateLambda(
                        [WeakRegistry, WeakOutliner = TWeakPtr<ISceneOutliner>(Outliner), TargetActor](UToolMenu* SubMenu)
                        {
                            const TSharedPtr<Latch::FRegistry> SubRegistry = WeakRegistry.Pin();
                            const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                            AActor* HitActor = TargetActor.Get();
                            if (!SubRegistry || !OutlinerPin || !GEditor || !IsValid(HitActor))
                            {
                                return;
                            }

                            // Resolve viewport selection only when the Latch submenu
                            // opens. Unreal may update selection as part of opening the
                            // viewport context menu, so doing this in the outer dynamic
                            // section can observe the previous selection set.
                            TArray<FSceneOutlinerTreeItemID> TargetIDs;
                            USelection* SelectedActors = GEditor->GetSelectedActors();
                            if (SelectedActors && SelectedActors->IsSelected(HitActor))
                            {
                                for (FSelectionIterator It(*SelectedActors); It; ++It)
                                {
                                    if (AActor* SelectedActor = Cast<AActor>(*It); IsValid(SelectedActor))
                                    {
                                        TargetIDs.AddUnique(FSceneOutlinerTreeItemID(SelectedActor));
                                    }
                                }
                            }
                            else
                            {
                                TargetIDs.Add(FSceneOutlinerTreeItemID(HitActor));
                            }

                            const TArray<FSceneOutlinerTreeItemPtr> LiveItems = SubRegistry->ResolveLiveItems(OutlinerPin, TargetIDs);
                            if (!LiveItems.IsEmpty())
                            {
                                SubRegistry->PopulateContextMenu(*SubMenu, OutlinerPin, LiveItems);
                            }
                        }));
            }));
    }

public:
    virtual void StartupModule() override
    {
        if (IsRunningCommandlet())
        {
            return;
        }

        Registry = MakeShared<Latch::FRegistry>();
        InputProcessor = MakeShared<Latch::FInputProcessor>(Registry.ToSharedRef());

        if (GEngine)
        {
            LevelActorDeletedHandle = GEngine->OnLevelActorDeleted().AddRaw(this, &FLatchModule::HandleLevelActorDeleted);
        }

        if (FSlateApplication::IsInitialized())
        {
            FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);
        }

        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLatchModule::RegisterViewportActorMenu));

        FSceneOutlinerModule& SceneOutlinerModule =
            FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

        const TWeakPtr<Latch::FRegistry> WeakRegistry = Registry;
        ColumnHandle = SceneOutlinerModule.OnCreateActorBrowserColumns().AddLambda(
            [WeakRegistry](FSceneOutlinerInitializationOptions& Options, UWorld*)
            {
                const TSharedPtr<Latch::FRegistry> State = WeakRegistry.Pin();
                if (!State)
                {
                    return;
                }

                Options.ColumnMap.Add(
                    Latch::FObserverColumn::GetID(),
                    FSceneOutlinerColumnInfo(
                        ESceneOutlinerColumnVisibility::Visible,
                        252,
                        FCreateSceneOutlinerColumn::CreateLambda(
                            [WeakRegistry](ISceneOutliner& Outliner) -> TSharedRef<ISceneOutlinerColumn>
                            {
                                const TSharedPtr<Latch::FRegistry> RegistryPin = WeakRegistry.Pin();
                                check(RegistryPin.IsValid());
                                return MakeShared<Latch::FObserverColumn>(Outliner, RegistryPin.ToSharedRef());
                            }),
                        false,
                        TOptional<float>(),
                        NSLOCTEXT("Latch", "ObserverColumnLabel", "Latch")));

                // Decorate the existing Item Label factory rather than replacing it.
                // This preserves native rename/search behavior and composes with
                // other label decorators such as Chroma regardless of load order.
                if (FSceneOutlinerColumnInfo* LabelInfo = Options.ColumnMap.Find(FSceneOutlinerBuiltInColumnTypes::Label()))
                {
                    const FCreateSceneOutlinerColumn PreviousLabelFactory = LabelInfo->Factory;
                    LabelInfo->Factory = FCreateSceneOutlinerColumn::CreateLambda(
                        [PreviousLabelFactory, WeakRegistry](ISceneOutliner& Outliner) -> TSharedRef<ISceneOutlinerColumn>
                        {
                            const TSharedPtr<ISceneOutlinerColumn> Native = PreviousLabelFactory.IsBound()
                                ? TSharedPtr<ISceneOutlinerColumn>(PreviousLabelFactory.Execute(Outliner))
                                : FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"))
                                    .FactoryColumn(FSceneOutlinerBuiltInColumnTypes::Label(), Outliner);
                            check(Native.IsValid());
                            const TSharedPtr<Latch::FRegistry> RegistryPin = WeakRegistry.Pin();
                            if (!RegistryPin)
                            {
                                return Native.ToSharedRef();
                            }
                            return MakeShared<Latch::FLabelColumn>(Native.ToSharedRef(), Outliner, RegistryPin.ToSharedRef());
                        });
                }

                // Use Scene Outliner's public context-menu modifier hook. The engine
                // invokes it after the native menu has been registered, so Latch can
                // extend the real Unreal menu instead of replacing right-click UX.
                const FSceneOutlinerModifyContextMenu PreviousModify = Options.ModifyContextMenu;
                Options.ModifyContextMenu = FSceneOutlinerModifyContextMenu::CreateLambda(
                    [PreviousModify, WeakRegistry](FName& MenuName, FToolMenuContext& MenuContext)
                    {
                        if (PreviousModify.IsBound())
                        {
                            PreviousModify.Execute(MenuName, MenuContext);
                        }

                        if (LatchExtendedContextMenus.Contains(MenuName))
                        {
                            return;
                        }

                        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(MenuName);
                        Menu->AddDynamicSection(TEXT("Latch.Context"), FNewToolMenuDelegate::CreateLambda(
                            [WeakRegistry](UToolMenu* InMenu)
                            {
                                const TSharedPtr<Latch::FRegistry> RegistryPin = WeakRegistry.Pin();
                                const USceneOutlinerMenuContext* Context = InMenu->FindContext<USceneOutlinerMenuContext>();
                                if (!RegistryPin || !Context || Context->NumSelectedItems < 1)
                                {
                                    return;
                                }

                                const TSharedPtr<SSceneOutliner> SceneOutliner = Context->SceneOutliner.Pin();
                                if (!SceneOutliner)
                                {
                                    return;
                                }

                                TArray<FSceneOutlinerTreeItemID> SelectedItemIDs;
                                for (const FSceneOutlinerTreeItemPtr& SelectedItem : SceneOutliner->GetTree().GetSelectedItems())
                                {
                                    if (Latch::FRegistry::IsSupportedItem(SelectedItem))
                                    {
                                        SelectedItemIDs.AddUnique(SelectedItem->GetID());
                                    }
                                }
                                if (SelectedItemIDs.IsEmpty())
                                {
                                    return;
                                }

                                const TSharedPtr<ISceneOutliner> Outliner = SceneOutliner;
                                FToolMenuSection& Section = InMenu->AddSection(TEXT("Latch"), NSLOCTEXT("Latch", "ContextSection", "Latch"));
                                Section.AddSubMenu(
                                    TEXT("Latch.Controls"),
                                    NSLOCTEXT("Latch", "ContextSubmenu", "Latch"),
                                    NSLOCTEXT("Latch", "ContextSubmenuTip", "Control persistent World Outliner expansion and selective visibility."),
                                    FNewToolMenuDelegate::CreateLambda(
                                        [WeakRegistry, WeakOutliner = TWeakPtr<ISceneOutliner>(Outliner), SelectedItemIDs](UToolMenu* SubMenu)
                                        {
                                            const TSharedPtr<Latch::FRegistry> SubRegistry = WeakRegistry.Pin();
                                            const TSharedPtr<ISceneOutliner> OutlinerPin = WeakOutliner.Pin();
                                            if (SubRegistry && OutlinerPin)
                                            {
                                                const TArray<FSceneOutlinerTreeItemPtr> LiveItems = SubRegistry->ResolveLiveItems(OutlinerPin, SelectedItemIDs);
                                                if (!LiveItems.IsEmpty())
                                                {
                                                    SubRegistry->PopulateContextMenu(*SubMenu, OutlinerPin, LiveItems);
                                                }
                                            }
                                        }));
                            }));

                        LatchExtendedContextMenus.Add(MenuName);
                    });
            });
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);

        if (GEngine && LevelActorDeletedHandle.IsValid())
        {
            GEngine->OnLevelActorDeleted().Remove(LevelActorDeletedHandle);
        }

        if (FSlateApplication::IsInitialized() && InputProcessor)
        {
            FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
        }
        InputProcessor.Reset();

        if (FSceneOutlinerModule* SceneOutlinerModule = FModuleManager::GetModulePtr<FSceneOutlinerModule>(TEXT("SceneOutliner")))
        {
            SceneOutlinerModule->OnCreateActorBrowserColumns().Remove(ColumnHandle);
        }

        if (Registry)
        {
            Registry->Stop();
            Registry.Reset();
        }

        LatchExtendedContextMenus.Reset();
    }

    virtual bool SupportsDynamicReloading() override
    {
        return false;
    }

private:
    FDelegateHandle ColumnHandle;
    FDelegateHandle LevelActorDeletedHandle;
    TSharedPtr<Latch::FRegistry> Registry;
    TSharedPtr<Latch::FInputProcessor> InputProcessor;
};

IMPLEMENT_MODULE(FLatchModule, Latch)
