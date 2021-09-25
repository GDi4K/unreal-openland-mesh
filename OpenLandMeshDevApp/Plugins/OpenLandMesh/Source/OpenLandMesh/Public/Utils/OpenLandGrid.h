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
		return HoleSize.X == 0 && HoleSize.Y == 0;
	}
};

class FOpenLandGrid
{
	FOpenLandGridBuildInfo BuildInfo;

	static FVector ToVector3D(FVector2D Vector);

public:
	FOpenLandGrid();
	void Build(FOpenLandGridBuildInfo InputBuildInfo);
	int32 GetCellWidth() const { return BuildInfo.CellWidth; }
	FBox GetBoundingBox() const;
	
	FOpenLandGridChangedCells ReCenter(FVector NewCenter);
	FOpenLandGridChangedCells UpdateHoleRoot(FVector2D HoleRootCell);
	TArray<FVector2D> GetAllCells() const;
};

typedef TSharedPtr<FOpenLandGrid> FOpenLandGridPtr;