// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class WildOmissionCore : ModuleRules
{
	public WildOmissionCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"InputCore", "EnhancedInput", "UMG", 
			"Niagara", "NavigationSystem", "MetasoundEngine", "Achievements", "Animals", "Animation", "Cheats", "Clothing", "Crafting",
			"CreativeMode", "CustomUI", "DedicatedServer", "Deployables", "Despawner", "Durability",
			"EquipableItems", "GameChat", "GameMusic", "GameSettings", "GatherableResources",
			"Interaction", "Inventory", "Locks", "MenuSystem", "Maps", "Monsters", "Notifications", "Ragdoll", "SavableObject",
			"SaveSystem", "SurfaceProperties", "TimeOfDay", "Vitals", "Weapons", "Weather",
			"WildOmissionAI", "WorldGeneration"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "OnlineSubsystem", "OnlineSubsystemSteam" });
    }
}
