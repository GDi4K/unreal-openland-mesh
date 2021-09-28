#pragma once
#include "Math/Box.h"

struct FOpenLandGridChangedCells
{
	TArray<FVector2D> CellsToAdd;
	TArray<FVector2D> CellsToRemove;
	TArray<FVector2D> ExistingCells;
};

struct FOpenLandGridBuildInfo
{
	FVector2D RootCell = {0, 0};
	FVector2D Size = {0, 0};
	int32 CellWidth = 0;
	int32 UpperCellWidth = 0;

	FVector2D HoleRootCell = {0, 0};
	FVector2D HoleSize = {0, 0 };

	bool HasHole() const
	{
		return !HoleSize.IsNearlyZero();
	}
};

class FOpenLandGrid
{
	FOpenLandGridBuildInfo BuildInfo;

	static FVector ToVector3D(FVector2D Vector);
	static bool IsPointInsideRect(FVector2D RectRoot, FVector2D RectSize, FVector2D PointToCheck);
	static bool IsRectInsideRect(FVector2D RectOuterRoot, FVector2D RectOuterSize, FVector2D RectInnerRoot, FVector2D RectInnerSize);
	static bool IsHoleInsideRect(FVector2D RectRoot, FVector2D RectSize, FVector2D HoleRoot, FVector2D HoleSize);
	TSet<FVector2D> GetAllCellsSet() const;

public:
	FOpenLandGrid();
	void Build(FOpenLandGridBuildInfo InputBuildInfo);
	int32 GetCellWidth() const { return BuildInfo.CellWidth; }
	FVector2D GetRootCell() const { return BuildInfo.RootCell; }
	FVector2D GetSize() const { return BuildInfo.Size; }
	
	FBox GetBoundingBox() const;
	FVector2D FindClosestCellRoot(FVector Position) const;
	FOpenLandGridChangedCells ReCenter(FVector NewCenter);
	FOpenLandGridChangedCells ReCenter(FVector NewCenter, FVector2D NewHoleRootCell);
	FOpenLandGridChangedCells ChangeHoleRootCell(FVector2D NewHoleRootCell);
	TArray<FVector2D> GetAllCells() const;
};

typedef TSharedPtr<FOpenLandGrid> FOpenLandGridPtr;