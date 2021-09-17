#pragma once
#include "OpenLandMeshComponent.h"

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	int32 MeshSectionIndex = -1;
	FVector CenterPosition = {0, 0, 0};

	void BuildMesh(float CuspAngle) const;
	
public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void Build();
	void UpdatePosition(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;