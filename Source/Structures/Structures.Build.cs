// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Structures : ModuleRules
{
    public Structures(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "SavableObject", "Deployables", "WorldGeneration" });

        // Structures spawns Deployables and is spawned by the WorldGeneration chunk manager.
        CircularlyReferencedDependentModules.AddRange(new string[] { "Deployables", "WorldGeneration" });
    }
}
