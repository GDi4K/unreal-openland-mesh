// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

struct FOpenLandMeshGridCell
{
	int X;
	int Y;

	FString ToString() const
	{
		return "{" + FString::FromInt(X) + ", " + FString::FromInt(Y) + "}";
	}
};

class OPENLANDMESH_API FOpenLandMeshGrid
{
	bool bBoundsCreated = false;
	FVector2D Lowest = {0, 0};
	FVector2D Highest = { 0, 0};
	float CellWidth;
	FOpenLandMeshGridCell TotalCells;
	
public:
	FOpenLandMeshGrid(float InputCellWidth);
	void AddPoint(FVector Point);
	void MakeBounds();
	TArray<FVector> GetBoundingPoints() const;
	int32 FindCellId(FVector Point) const;
	FOpenLandMeshGridCell CellIdToCell(int32 CellId) const;
	int32 CellToCellId(FOpenLandMeshGridCell Cell) const;
	FVector GetCellMidPoint(int32 CellId) const;
	TArray<int32> FindCellsAround(int32 CellId, int32 CellRadius) const;
};
