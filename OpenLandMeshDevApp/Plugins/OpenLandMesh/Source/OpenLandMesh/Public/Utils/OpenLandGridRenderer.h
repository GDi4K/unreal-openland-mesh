#pragma once
#include "Utils/OpenLandGrid.h"
#include "Types/OpenLandMeshInfo.h"

struct FOpenLandGridRendererChangedInfo
{
	TArray<int32> ChangedTriangles;
};

struct FOpenLandGridRendererCell
{
	FVector2D GridCell;
	int32 IndexT0;
	int32 IndexT1;
};

class FOpenLandGridRenderer
{
	FOpenLandGridPtr Grid = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	TMap<FString, FOpenLandGridRendererCell> Cells;
	bool bInitialized = false;

	TOpenLandArray<FOpenLandMeshVertex> BuildCell(FVector2D Cell) const;

public:
	FOpenLandGridRenderer();
	FOpenLandMeshInfoPtr Initialize(FOpenLandGridPtr SourceGrid);
	FOpenLandGridRendererChangedInfo ReCenter(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandGridRenderer> FOpenLandGridRendererPtr;