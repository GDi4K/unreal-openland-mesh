// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OpenLandMesh : ModuleRules
{
	public OpenLandMesh(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;
		
		PublicIncludePaths.Add(ModuleDirectory +  "/Public");
        PrivateIncludePaths.Add(ModuleDirectory +  "/Private");

        PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine",
			"MeshDescription",
			"RenderCore",
			"RHI",
			"StaticMeshDescription",
			"PhysicsCore"
		});
	}
}