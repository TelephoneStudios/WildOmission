// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Achievements : ModuleRules
{
    public Achievements(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "OnlineSubsystem", "OnlineSubsystemSteam" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });
    }
}