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

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMeshInfoPtr MeshInfoInner = nullptr;
	FOpenLandMeshInfoPtr MeshInfoOuter = nullptr;
	int32 MeshSectionIndexInner = -1;
	int32 MeshSectionIndexOuter = -1;
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;
	
	FOpenLandGridPtr GridInner = nullptr;
	FOpenLandGridPtr GridOuter = nullptr;
	FOpenLandGridRendererPtr GridRendererInner = nullptr;
	FOpenLandGridRendererPtr GridRendererOuter = nullptr;

public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePosition(FVector NewCenter) const;
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;