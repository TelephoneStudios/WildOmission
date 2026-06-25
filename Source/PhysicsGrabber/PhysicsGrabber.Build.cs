// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class PhysicsGrabber : ModuleRules
{
    public PhysicsGrabber(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });
    }
}