// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Latch : ModuleRules
{
    public Latch(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "DeveloperSettings",
            "EditorSubsystem"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "InputCore",
            "UnrealEd",
            "LevelEditor",
            "SceneOutliner",
            "ToolMenus",
            "Projects",
            "AppFramework",
            "ApplicationCore"
        });
    }
}
