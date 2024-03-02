// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Deployables : ModuleRules
{
    public Deployables(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "EquipableItems", "Durability", "Inventory", "SavableObject" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "InputCore", "SlateCore", "NavigationSystem",
            "Niagara", "Crafting", "Despawner", "GatherableResources", "Interaction", "Locks", "Weather", "WorldGeneration" });

        SetupIrisSupport(Target);
    }
}