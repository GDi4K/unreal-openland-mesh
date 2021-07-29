// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MeshMixer2 : ModuleRules
{
	public MeshMixer2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OpenLandMesh"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "ProceduralMeshComponent",
			"InteractiveToolsFramework",
			"MeshDescription",
			"RenderCore",
			"RHI",
			"StaticMeshDescription",
			"PhysicsCore",
			"OpenLandMesh"
		});
	}
}