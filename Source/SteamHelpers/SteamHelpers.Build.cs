// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class SteamHelpers : ModuleRules
{
    public SteamHelpers (ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "OnlineSubsystem", "OnlineSubsystemUtils", "Steamworks" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });

        DynamicallyLoadedModuleNames.AddRange(new string[] { "OnlineSubsystemSteam" });
    }
}