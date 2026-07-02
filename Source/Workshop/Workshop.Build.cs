// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Workshop : ModuleRules
{
    public Workshop(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG" });
        PrivateDependencyModuleNames.AddRange(new string[] { 
            "Core", "CoreUObject", "InputCore", "Slate", "SlateCore",
             "CustomUI", "SaveSystem",
            "OnlineSubsystem", "OnlineSubsystemSteam", "Steamworks", 
            "SaveSystem", "HTTP", "ImageWrapper", "SteamHelpers" });
    }
}