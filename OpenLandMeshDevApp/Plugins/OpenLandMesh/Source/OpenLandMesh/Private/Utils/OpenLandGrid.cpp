#include "Utils/OpenLandGrid.h"

FVector FOpenLandGrid::ToVector3D(FVector2D Vector)
{
	return { Vector.X, Vector.Y, 0};
}

FOpenLandGrid::FOpenLandGrid(FVector2D _RootCell, FVector2D _Size, int32 _CellWidth, int32 _UpperCellWidth)
{
	RootCell = _RootCell;
	Size = _Size;
	CellWidth = _CellWidth;
	UpperCellWidth = _UpperCellWidth;
	
	check(UpperCellWidth % CellWidth == 0);
	check(FMath::RoundToInt(Size.X) % UpperCellWidth == 0);
	check(FMath::RoundToInt(Size.Y) % UpperCellWidth == 0);
}

FOpenLandGridChangedCells FOpenLandGrid::ReCenter(FVector NewCenter)
{
	const FVector2D NewCenter2D = {NewCenter.X, NewCenter.Y};
	const FVector2D CurrentCenter = (RootCell + (Size * 0.5)) * CellWidth;
	const FVector2D Diff = NewCenter2D - CurrentCenter;

	const int32 CellFactor = UpperCellWidth/CellWidth;

	const int32 XShift = FMath::RoundToInt(Diff.X / UpperCellWidth) * CellFactor;
	const int32 YShift = FMath::RoundToInt(Diff.Y / UpperCellWidth) * CellFactor;

	if (XShift == 0 && YShift == 0)
	{
		return {};
	}

	const FVector2D NewRootCell = RootCell + FVector2D(XShift, YShift);

	FOpenLandGridChangedCells ChangedCells;

	// Loop the Old Cells to find what to remove
	for (int32 X=0; X<Size.X; X++)
	{
		for(int32 Y=0; Y<Size.Y; Y++)
		{
			const int32 XPos = RootCell.X + X;
			const int32 YPos = RootCell.Y + Y;

			if (XPos < NewRootCell.X || XPos >= NewRootCell.X + Size.X)
			{
				ChangedCells.CellsToRemove.Push(FVector2D(X, Y) + RootCell);
				continue;
			}

			if (YPos < NewRootCell.Y || YPos >= NewRootCell.Y + Size.Y)
			{
				ChangedCells.CellsToRemove.Push(FVector2D(X, Y) + RootCell);
				continue;
			}
		}
	}

	// Loop the NewCells to find out what to add
	for (int32 X=0; X<Size.X; X++)
	{
		for(int32 Y=0; Y<Size.Y; Y++)
		{
			const int32 XPos = NewRootCell.X + X;
			const int32 YPos = NewRootCell.Y + Y;

			if (XPos < RootCell.X || XPos >= RootCell.X + Size.X)
			{
				ChangedCells.CellsToAdd.Push(FVector2D(X, Y) + RootCell);
				continue;
			}

			if (YPos < RootCell.Y || YPos >= RootCell.Y + Size.Y)
			{
				ChangedCells.CellsToAdd.Push(FVector2D(X, Y) + RootCell);
				continue;
			}
		}
	}

	RootCell = NewRootCell;

	return ChangedCells;
}

TArray<FVector2D> FOpenLandGrid::GetAllCells() const
{
	TArray<FVector2D> Cells;
	
	for (int32 X=0; X<Size.X; X++)
	{
		for(int32 Y=0; Y<Size.Y; Y++)
		{
			Cells.Push(FVector2D(X, Y) + RootCell);
		}
	}

	return Cells;
}
