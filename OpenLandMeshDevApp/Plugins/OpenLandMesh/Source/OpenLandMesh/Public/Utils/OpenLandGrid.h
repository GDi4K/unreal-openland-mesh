﻿#pragma once
#include "OpenLandGridCell.h"
#include "Math/Box.h"

struct FOpenLandGridChangedCells
{
	TArray<FOpenLandGridCell> CellsToAdd;
	TArray<FOpenLandGridCell> CellsToRemove;
	TArray<FOpenLandGridCell> ExistingCells;
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

class FOpenLandGrid
{
	FOpenLandGridBuildInfo BuildInfo;

	static FVector ToVector3D(FOpenLandGridCell Vector);
	static bool IsPointInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell PointToCheck);
	static bool IsRectInsideRect(FOpenLandGridCell RectOuterRoot, FOpenLandGridCell RectOuterSize, FOpenLandGridCell RectInnerRoot, FOpenLandGridCell RectInnerSize);
	static bool IsHoleInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell HoleRoot, FOpenLandGridCell HoleSize);
	TSet<FOpenLandGridCell> GetAllCellsSet() const;

public:
	FOpenLandGrid();
	void Build(FOpenLandGridBuildInfo InputBuildInfo);
	int32 GetCellWidth() const { return BuildInfo.CellWidth; }
	FOpenLandGridCell GetRootCell() const { return BuildInfo.RootCell; }
	FOpenLandGridCell GetSize() const { return BuildInfo.Size; }
	
	FBox GetBoundingBox() const;
	FOpenLandGridCell FindClosestCellRoot(FVector Position) const;
	FOpenLandGridChangedCells ReCenter(FVector NewCenter);
	FOpenLandGridChangedCells ReCenter(FVector NewCenter, FVector2D NewHoleRootCell);
	FOpenLandGridChangedCells ChangeHoleRootCell(FVector2D NewHoleRootCell);
	TArray<FOpenLandGridCell> GetAllCells() const;
};

typedef TSharedPtr<FOpenLandGrid> FOpenLandGridPtr;