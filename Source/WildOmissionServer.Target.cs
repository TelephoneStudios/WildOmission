// Copyright Telephone Studios. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WildOmissionServerTarget : TargetRules
{
	public WildOmissionServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        
		bUseLoggingInShipping = true;
	
		ExtraModuleNames.AddRange( new string[] { "WildOmissionCore" } );

        // The name of your game as you set it on the Steamworks webpage.
        GlobalDefinitions.Add("UE_PROJECT_STEAMPRODUCTNAME=\"Wild Omission\"");

        // This is what will appear under the "Game" tab in the Steam server browser. Steamworks list this under "Server browser name" in the Dedicated Servers section of your app.
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=\"Wild Omission\"");

        // I am not entirely sure what this is used for. Changing it seemed to not effect game searches.
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=\"Wild Omission\"");

        // The steam appID of your project.
        GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=2348700");
    }
}
