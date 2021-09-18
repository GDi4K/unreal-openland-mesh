#pragma once
#include "OpenLandMeshComponent.h"

struct FOpenLandMovingGridBuildOptions
{
	float CuspAngle = 60.0f;
	float CellWidth = 10.0f;
	int32 CellCount = 10;
};

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	int32 MeshSectionIndex = -1;
	FVector CenterPosition = {0, 0, 0};
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;

	void BuildFaces(float CuspAngle) const;
	static FVector2D PositionToUV(FVector Position, int32 VertexPosition);
	
public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePosition(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;