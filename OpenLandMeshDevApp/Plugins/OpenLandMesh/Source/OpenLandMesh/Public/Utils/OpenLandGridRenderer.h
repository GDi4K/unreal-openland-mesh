#pragma once
#include "Utils/OpenLandGrid.h"
#include "Types/OpenLandMeshInfo.h"

struct FOpenLandGridRendererChangedInfo
{
	bool bInvalidateRendering = false;
	TArray<int32> ChangedTriangles;
};

class FOpenLandGridRenderer
{
	FOpenLandGridPtr Grid = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	bool bInitialized = false;

public:
	FOpenLandGridRenderer();
	FOpenLandMeshInfoPtr Initialize(FOpenLandGridPtr SourceGrid);
	FOpenLandGridRendererChangedInfo ReCenter(FVector NewCenter) const;
};

typedef TSharedPtr<FOpenLandGridRenderer> FOpenLandGridRendererPtr;