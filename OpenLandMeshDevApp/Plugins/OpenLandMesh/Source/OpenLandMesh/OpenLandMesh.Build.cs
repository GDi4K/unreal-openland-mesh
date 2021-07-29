// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

using UnrealBuildTool;

public class OpenLandMesh : ModuleRules
{
	public OpenLandMesh(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

		PublicIncludePaths.Add(ModuleDirectory + "/Public");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private");

		PublicDependencyModuleNames.AddRange(new[]
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