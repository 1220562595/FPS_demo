// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPS_demo : ModuleRules
{
	public FPS_demo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"GameplayAbilities", 
			"GameplayTasks", 
			"GameplayTags", 
			"MultiplayerSessions", 
			"OnlineSubsystem", 
			"OnlineSubsystemSteam",
			"GameplayTasks"
		});
	}
}
