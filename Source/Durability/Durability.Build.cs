// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Durability : ModuleRules
{
    public Durability(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });
    }
}