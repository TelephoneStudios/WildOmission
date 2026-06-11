// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Deployables : ModuleRules
{
    public Deployables(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "Durability", "Inventory", "SavableObject", "Weather" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "InputCore", "SlateCore", "NavigationSystem",
            "Niagara", "Crafting", "Interaction", "Locks", "Despawner", "WorldGeneration" });

        CircularlyReferencedDependentModules.AddRange(new string[] { "Crafting", "WorldGeneration" });
    }
}