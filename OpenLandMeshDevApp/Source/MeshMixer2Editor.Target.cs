// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MeshMixer2EditorTarget : TargetRules
{
	public MeshMixer2EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("MeshMixer2");
	}
}