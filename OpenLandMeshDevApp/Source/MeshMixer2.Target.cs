// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MeshMixer2Target : TargetRules
{
	public MeshMixer2Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("MeshMixer2");
	}
}