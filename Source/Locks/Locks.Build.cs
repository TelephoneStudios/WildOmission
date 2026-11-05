// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;

public class Locks : ModuleRules
{
    public Locks(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "UMG", "SavableObject" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "CoreOnline", "EquipableItems", "Interaction", "Inventory" });
    }
}