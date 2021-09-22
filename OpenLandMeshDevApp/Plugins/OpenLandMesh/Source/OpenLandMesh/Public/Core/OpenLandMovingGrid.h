#pragma once
#include "OpenLandMeshComponent.h"
#include "Utils/OpenLandGrid.h"

struct FOpenLandMovingGridBuildOptions
{
	float CuspAngle = 60.0f;
	float CellWidth = 10.0f;
	int32 CellCount = 10;
	float UnitUVLenght = 100.0f;
	int32 MaxUVs = 10;
};

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	int32 MeshSectionIndex = -1;
	FVector2D RootCell = {0, 0};
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;
	FOpenLandGridPtr RootGrid = nullptr;

public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void RenderGrid();
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePosition(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;