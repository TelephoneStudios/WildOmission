// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class CreativeMode : ModuleRules
{
    public CreativeMode(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "Inventory" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "CustomUI" });
    }
}