#pragma once
#include "Utils/OpenLandGrid.h"
#include "Types/OpenLandMeshInfo.h"

struct FOpenLandGridRendererChangedInfo
{
	TArray<int32> ChangedTriangles;
};

struct FOpenLandGridRendererChangedInfoStatus
{
	bool bCompleted = false;
	FOpenLandGridRendererChangedInfo ChangedInfo = {};
};

struct FOpenLandGridRendererCell
{
	FOpenLandGridCell GridCell;
	int32 IndexT0;
	int32 IndexT1;
};

struct FOpenLandGridRendererEdgeCell
{
	FOpenLandGridCell GridCell;
	int32 IndexT0;
	int32 IndexT1;
	int32 IndexT2;
};

class FOpenLandGridRenderer
{
	FOpenLandGridPtr Grid = nullptr;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	TMap<uint32, FOpenLandGridRendererCell> Cells;
	TMap<uint32, FOpenLandGridRendererEdgeCell> EdgeCells;
	bool bInitialized = false;
	
	TSharedPtr<FOpenLandGridRendererChangedInfoStatus> CurrentOperation = nullptr;
	TSharedPtr<FOpenLandGridChangedCells> CurrentGridChangedCells = nullptr;
	FThreadSafeCounter CompletedCells = 0;

	TOpenLandArray<FOpenLandMeshVertex> BuildCell(FOpenLandGridCell Cell) const;
	TOpenLandArray<FOpenLandMeshVertex> BuildEdgeCell(FOpenLandGridCell Cell) const;
	void ApplyCellChangesAsync(FOpenLandGridChangedCells ChangedCells);
	static FVector ApplyVertexModifier(FOpenLandGridCell Cell, FVector Source);

	void SwapCell(int32 Index);
	void SwapEdgeCell(int32 Index);
	void RegenerateEdgeCell(int32 Index);

	void StartSwapCell();

public:
	FOpenLandGridRenderer();
	FOpenLandMeshInfoPtr Initialize(FOpenLandGridPtr SourceGrid);
	bool StartReCenter(FVector NewCenter);
	bool StartReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell);
	bool StartChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell);
	TSharedPtr<FOpenLandGridRendererChangedInfo> CheckStatus();
};

typedef TSharedPtr<FOpenLandGridRenderer> FOpenLandGridRendererPtr;