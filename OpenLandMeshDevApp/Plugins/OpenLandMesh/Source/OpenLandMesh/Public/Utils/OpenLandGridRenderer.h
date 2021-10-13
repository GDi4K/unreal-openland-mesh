#pragma once
#include "Compute/GpuComputeVertex.h"
#include "Compute/Types/ComputeMaterial.h"
#include "Compute/Types/DataTexture.h"
#include "Types/OpenLandGridRenderer.h"
#include "Utils/OpenLandGrid.h"
#include "Types/OpenLandMeshInfo.h"
#include "Materials/MaterialInterface.h"

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

struct FOpenLandGridRendererGeneratedCells
{
	TOpenLandArray<FOpenLandGridRendererCellBuildResult> GeneratedCells = {};
	TOpenLandArray<FOpenLandGridRendererCellBuildResult> GeneratedEdgeCells = {};
	TOpenLandArray<FOpenLandGridRendererCellBuildResult> ReGeneratedEdgeCells = {};

	FThreadSafeCounter GeneratedCellCount;
	FThreadSafeCounter GeneratedEdgeCellCount;
	FThreadSafeCounter ReGeneratedEdgeCellCount;
	FThreadSafeCounter CallFinishedCounter;

	int32 GetNextGeneratedCellIndex()
	{
		const size_t NewValue = GeneratedCellCount.Increment();
		if (NewValue > GeneratedCells.Length())
		{
			return -1;
		}

		return NewValue - 1;
	}

	int32 GetNextGeneratedEdgeCellIndex()
	{
		const size_t NewValue = GeneratedEdgeCellCount.Increment();
		if (NewValue > GeneratedEdgeCells.Length())
		{
			return -1;
		}

		return NewValue - 1;
	}

	int32 GetNextReGeneratedEdgeCellIndex()
	{
		const size_t NewValue = ReGeneratedEdgeCellCount.Increment();
		if (NewValue > ReGeneratedEdgeCells.Length())
		{
			return -1;
		}

		return NewValue - 1;
	}

	bool CanFinish()
	{
		return CallFinishedCounter.Increment() == 1;
	}

	void Reset(
		int32 NumGeneratedCells,
		int32 NumEdgeGeneratedCells,
		int32 NumRegeneratedEdgeGeneratedCells
	)
	{
		GeneratedCells.SetLength(NumGeneratedCells);
		
		GeneratedEdgeCells.SetLength(NumEdgeGeneratedCells);
		
		ReGeneratedEdgeCells.SetLength(NumRegeneratedEdgeGeneratedCells);
		
		GeneratedCellCount.Set(0);
		GeneratedEdgeCellCount.Set(0);
		ReGeneratedEdgeCellCount.Set(0);
		CallFinishedCounter.Set(0);
	}

	static TSharedPtr<FOpenLandGridRendererGeneratedCells> New(
		int32 NumGeneratedCells,
		int32 NumEdgeGeneratedCells,
		int32 NumRegeneratedEdgeGeneratedCells
	)
	{
		TSharedPtr<FOpenLandGridRendererGeneratedCells> Ptr = MakeShared<FOpenLandGridRendererGeneratedCells>();
		Ptr->Reset(NumGeneratedCells, NumEdgeGeneratedCells, NumRegeneratedEdgeGeneratedCells);
		return Ptr;
	}
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
	TSharedPtr<FOpenLandGridRendererGeneratedCells> CellGenInfo = nullptr;

	TOpenLandArray<FOpenLandMeshVertex> BuildCell(FOpenLandGridCell Cell) const;
	TOpenLandArray<FOpenLandMeshVertex> BuildEdgeCell(FOpenLandGridCell Cell) const;
	void ApplyCellChangesAsync(FOpenLandGridChangedCells ChangedCells);
	void ApplyVertexModifiersAsync(FOpenLandGridChangedCells ChangedCells);
	static FVector ApplyCpuVertexModifier(FVector Source);
	static void BuildTangents(TOpenLandArray<FOpenLandMeshVertex>& Vertices);
	
	TSharedPtr<FDataTexture> DataTextureX = nullptr;
	TSharedPtr<FDataTexture> DataTextureY = nullptr;
	TSharedPtr<FDataTexture> DataTextureZ = nullptr;
	TSharedPtr<FGpuComputeVertex> GpuComputeEngine = nullptr;

	FComputeMaterial GpuVertexModifier = {};
	UObject* WorldContext = nullptr;
	
	void SwapCell(int32 Index);
	void SwapEdgeCell(int32 Index);
	void RegenerateEdgeCell(int32 Index);

	bool GenerateCellTangentsAsync() const;
	void FinishCellGeneration();

public:
	FOpenLandGridRenderer();
	FOpenLandMeshInfoPtr Initialize(FOpenLandGridPtr SourceGrid);
	bool StartReCenter(FVector NewCenter);
	bool StartReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell);
	bool StartChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell);
	TSharedPtr<FOpenLandGridRendererChangedInfo> CheckStatus();
	void SetGpuVertexModifier(FComputeMaterial ComputeMaterial, UObject* InputWorldContext);
};

typedef TSharedPtr<FOpenLandGridRenderer> FOpenLandGridRendererPtr;
