// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WildOmissionClientTarget : TargetRules
{
	public WildOmissionClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "WildOmissionCore" } );

        GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=2348700");
        GlobalDefinitions.Add("UE_PROJECT_STEAMPRODUCTNAME=\"Wild Omission\"");
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=\"Wild Omission\"");
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=\"Wild Omission\"");
    }
}
