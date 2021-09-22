#pragma once
#include "Math/Box.h"

struct FOpenLandGridChangedCells
{
	TArray<FVector2D> CellsToAdd;
	TArray<FVector2D> CellsToRemove;
};

class FOpenLandGrid
{
	FVector2D RootCell;
	FVector2D Size;
	int32 CellWidth;
	int32 UpperCellWidth;

	static FVector ToVector3D(FVector2D Vector);

public:
	FOpenLandGrid();
	void Build(FVector2D RootCell, FVector2D Size, int32 CellWidth, int32 UpperCellWidth);
	int32 GetCellWidth() const { return CellWidth; }
	FBox GetBoundingBox() const;
	
	FOpenLandGridChangedCells ReCenter(FVector NewCenter);
	TArray<FVector2D> GetAllCells() const;
};

typedef TSharedPtr<FOpenLandGrid> FOpenLandGridPtr;