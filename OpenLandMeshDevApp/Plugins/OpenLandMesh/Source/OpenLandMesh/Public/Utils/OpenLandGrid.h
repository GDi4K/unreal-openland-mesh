#pragma once
#include "OpenLandGridCell.h"
#include "Math/Box.h"

struct FOpenLandGridChangedCells
{
	TArray<FOpenLandGridCell> CellsToAdd;
	TArray<FOpenLandGridCell> CellsToRemove;
	TArray<FOpenLandGridCell> EdgeCellsToAdd;
	TArray<FOpenLandGridCell> EdgeCellsToRemove;
	TArray<FOpenLandGridCell> ModifiedEdgeCells;

	TSharedPtr<FOpenLandGridChangedCells> Clone() const
	{
		TSharedPtr<FOpenLandGridChangedCells> ClonedMe = MakeShared<FOpenLandGridChangedCells>();
		ClonedMe->CellsToAdd = CellsToAdd;
		ClonedMe->CellsToRemove = CellsToRemove;
		ClonedMe->EdgeCellsToAdd = EdgeCellsToAdd;
		ClonedMe->EdgeCellsToRemove = EdgeCellsToRemove;
		ClonedMe->ModifiedEdgeCells = ModifiedEdgeCells;

		return ClonedMe;
	}
};

struct FOpenLandGridBuildInfo
{
	FOpenLandGridCell RootCell = {0, 0};
	FOpenLandGridCell Size = {0, 0};
	int32 CellWidth = 0;
	int32 UpperCellWidth = 0;

	FOpenLandGridCell HoleRootCell = {0, 0};
	FOpenLandGridCell HoleSize = {0, 0 };

	bool HasHole() const
	{
		return !HoleSize.ToVector2D().IsNearlyZero();
	}
};

struct FOpenLandGridEdgeModificationOptions
{
	FOpenLandGridCell OldHoleRootCell;
	FOpenLandGridCell NewHoleRootCell;
	FOpenLandGridCell OldRootCell;
	FOpenLandGridCell NewRootCell;
};

class FOpenLandGrid
{
	FOpenLandGridBuildInfo BuildInfo;

	static FVector ToVector3D(FOpenLandGridCell Vector);
	static bool IsPointInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell PointToCheck);
	static bool IsRectInsideRect(FOpenLandGridCell RectOuterRoot, FOpenLandGridCell RectOuterSize, FOpenLandGridCell RectInnerRoot, FOpenLandGridCell RectInnerSize);
	static bool IsHoleInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell HoleRoot, FOpenLandGridCell HoleSize);
	TSet<FOpenLandGridCell> GetAllCellsSet() const;
	void ApplyEdgeModifications(FOpenLandGridChangedCells &ChangedCells, FOpenLandGridEdgeModificationOptions Options) const;

public:
	FOpenLandGrid();
	void Build(FOpenLandGridBuildInfo InputBuildInfo);
	int32 GetCellWidth() const { return BuildInfo.CellWidth; }
	FOpenLandGridCell GetRootCell() const { return BuildInfo.RootCell; }
	FOpenLandGridCell GetHoleRootCell() const { return BuildInfo.HoleRootCell; }
	FOpenLandGridCell GetSize() const { return BuildInfo.Size; }
	FOpenLandGridCell GetHoleSize() const { return BuildInfo.HoleSize; }
	
	FBox GetBoundingBox() const;
	FOpenLandGridCell FindClosestCellRoot(FVector Position) const;
	FOpenLandGridChangedCells ReCenter(FVector NewCenter);
	FOpenLandGridChangedCells ReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell);
	FOpenLandGridChangedCells ChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell);
	FOpenLandGridChangedCells GetAllCells() const;
	bool IsHoleEdge(FOpenLandGridCell HoleRoot, FOpenLandGridCell Cell) const;
};

typedef TSharedPtr<FOpenLandGrid> FOpenLandGridPtr;