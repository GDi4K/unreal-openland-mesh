#pragma once
#include "OpenLandMeshComponent.h"
#include "Utils/OpenLandGrid.h"
#include "Utils/OpenLandGridRenderer.h"

struct FOpenLandMovingGridBuildOptions
{
	float CuspAngle = 60.0f;
	float CellWidth = 10.0f;
	int32 CellCount = 10;
	float UnitUVLenght = 100.0f;
	int32 MaxUVs = 10;
};

struct FOpenLandMovingGridLOD
{
	int32 Index = 0;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	int32 MeshSectionIndex = -1;
	FOpenLandGridPtr Grid = nullptr;
	FOpenLandGridRendererPtr GridRenderer = nullptr;

	static FOpenLandMovingGridLOD New()
	{
		FOpenLandMovingGridLOD LOD;
		LOD.Grid = MakeShared<FOpenLandGrid>();
		LOD.GridRenderer = MakeShared<FOpenLandGridRenderer>();
		return LOD;
	}
};

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;
	
	FOpenLandMovingGridLOD LOD0 = FOpenLandMovingGridLOD::New();
	FOpenLandMovingGridLOD LOD1 = FOpenLandMovingGridLOD::New();

public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePosition(FVector NewCenter) const;
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;