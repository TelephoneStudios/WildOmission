// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class CustomUI : ModuleRules
{
    public CustomUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Slate", "SlateCore", "Synthesis" });
    }
}