// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class GameChat: ModuleRules
{
    public GameChat(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Slate", "SlateCore", "CustomUI", "DedicatedServer" });

        SetupIrisSupport(Target);
    }
}