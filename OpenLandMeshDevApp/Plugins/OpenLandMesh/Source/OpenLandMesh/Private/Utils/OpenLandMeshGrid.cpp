#include "Utils/OpenLandMeshGrid.h"

FOpenLandMeshGrid::FOpenLandMeshGrid(float InputCellWidth)
{
	CellWidth = InputCellWidth;
}

void FOpenLandMeshGrid::AddPoint(FVector Point)
{
	check(bBoundsCreated == false)
	
	if (Lowest.X > Point.X)
	{
		Lowest.X = Point.X;
	}

	if (Lowest.Y > Point.Y)
	{
		Lowest.Y = Point.Y;
	}

	if (Highest.X < Point.X)
	{
		Highest.X = Point.X;
	}

	if (Highest.Y < Point.Y)
	{
		Highest.Y = Point.Y;
	}
}

TArray<FVector> FOpenLandMeshGrid::GetBoundingPoints() const
{
	TArray<FVector> BoxPoints;

	BoxPoints.Push({Lowest.X, Lowest.Y, 0});
	BoxPoints.Push({Highest.X, Lowest.Y, 0});
	BoxPoints.Push({Highest.X, Highest.Y, 0});
	BoxPoints.Push({Lowest.X, Highest.Y, 0});

	return BoxPoints;
}

void FOpenLandMeshGrid::MakeBounds()
{
	bBoundsCreated = true;
	TotalCells.X = FMath::CeilToInt((Highest.X - Lowest.X)/CellWidth);
	TotalCells.Y = FMath::CeilToInt((Highest.Y - Lowest.Y)/CellWidth);
}

int32 FOpenLandMeshGrid::FindCellId(FVector Point) const
{
	check(bBoundsCreated == true)

	FOpenLandMeshGridCell Cell;
	const int32 StepsX = FMath::CeilToInt((Point.X - Lowest.X)/CellWidth);
	const int32 StepsY = FMath::CeilToInt((Point.Y - Lowest.Y)/CellWidth);
	Cell.X =  FMath::Max(StepsX -1, 0);
	Cell.Y =  FMath::Max(StepsY -1, 0);

	checkf(Cell.X >= 0 && Cell.Y >= 0, TEXT("Given point is not inside within the bounds"))
	checkf(Cell.X < TotalCells.X && Cell.Y < TotalCells.Y, TEXT("Given point is not inside within the bounds"))

	return CellToCellId(Cell);
}

FOpenLandMeshGridCell FOpenLandMeshGrid::CellIdToCell(int32 CellId) const
{
	FOpenLandMeshGridCell Cell;
	Cell.Y = FMath::CeilToInt(CellId/TotalCells.X);
	Cell.X = CellId - (TotalCells.X * Cell.Y);

	check(Cell.X < TotalCells.X);
	check(Cell.Y < TotalCells.Y);

	return Cell;
}

int32 FOpenLandMeshGrid::CellToCellId(FOpenLandMeshGridCell Cell) const
{
	return Cell.Y * TotalCells.X + Cell.X;
}

FVector FOpenLandMeshGrid::GetCellMidPoint(int32 CellId) const
{
	const FOpenLandMeshGridCell Cell = CellIdToCell(CellId);
	const FVector MidPoint = {
		(Cell.X * CellWidth) + Lowest.X,
		(Cell.Y * CellWidth) + Lowest.Y,
		0.0f
	};

	return MidPoint;
}

TArray<int32> FOpenLandMeshGrid::FindCellsAround(int32 CellId, int32 CellRadius) const
{
	const FOpenLandMeshGridCell Center = CellIdToCell(CellId);
	TArray<int32> Cells;

	for (int32 XChange=-CellRadius; XChange <= CellRadius; XChange ++)
	{
		for (int32 YChange=-CellRadius; YChange <= CellRadius; YChange ++)
		{
			if (XChange == 0 && YChange == 0)
			{
				continue;
			}

			if (FMath::Abs(XChange) == CellRadius && FMath::Abs(YChange) == CellRadius)
			{
				continue;
			}
			
			const FOpenLandMeshGridCell NewCell = {
				Center.X + XChange,
				Center.Y + YChange
			};

			if (NewCell.X < 0 || NewCell.Y < 0 || NewCell.X >= TotalCells.X || NewCell.Y >= TotalCells.Y)
			{
				continue;
			}

			Cells.Push(CellToCellId(NewCell));
		}
	}

	return Cells;
}
