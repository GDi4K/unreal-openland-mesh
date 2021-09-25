#include "Utils/OpenLandGrid.h"

FVector FOpenLandGrid::ToVector3D(FVector2D Vector)
{
	return { Vector.X, Vector.Y, 0};
}

void FOpenLandGrid::Build(FOpenLandGridBuildInfo InputBuildInfo)
{
	BuildInfo = InputBuildInfo;
	
	check(BuildInfo.UpperCellWidth % BuildInfo.CellWidth == 0);
	const int32 UpperCellWidthRatio = BuildInfo.UpperCellWidth / BuildInfo.CellWidth;
	check(FMath::RoundToInt(BuildInfo.Size.X) % UpperCellWidthRatio == 0);
	check(FMath::RoundToInt(BuildInfo.Size.Y) % UpperCellWidthRatio == 0);

	if (BuildInfo.HasHole())
	{
		check(BuildInfo.HoleSize.X < BuildInfo.Size.X);
		check(BuildInfo.HoleSize.Y < BuildInfo.Size.Y);
	}
}

FOpenLandGrid::FOpenLandGrid()
{
	
}

FBox FOpenLandGrid::GetBoundingBox() const
{
	FBox BoundingBox;
	BoundingBox.Init();

	BoundingBox += ToVector3D((BuildInfo.RootCell + FVector2D(0, 0)) * BuildInfo.CellWidth);
	BoundingBox += ToVector3D((BuildInfo.RootCell + FVector2D(0, BuildInfo.Size.Y)) * BuildInfo.CellWidth);
	BoundingBox += ToVector3D((BuildInfo.RootCell + FVector2D(BuildInfo.Size.X, BuildInfo.Size.Y)) * BuildInfo.CellWidth);
	BoundingBox += ToVector3D((BuildInfo.RootCell + FVector2D(BuildInfo.Size.X, 0)) * BuildInfo.CellWidth);

	return BoundingBox;
}

FOpenLandGridChangedCells FOpenLandGrid::ReCenter(FVector NewCenter)
{
	const FVector2D NewCenter2D = {NewCenter.X, NewCenter.Y};
	const FVector2D CurrentCenter = (BuildInfo.RootCell + (BuildInfo.Size * 0.5)) * BuildInfo.CellWidth;
	const FVector2D Diff = NewCenter2D - CurrentCenter;

	const int32 CellFactor = BuildInfo.UpperCellWidth/BuildInfo.CellWidth;

	const int32 XShift = FMath::RoundToInt(Diff.X / BuildInfo.UpperCellWidth) * CellFactor;
	const int32 YShift = FMath::RoundToInt(Diff.Y / BuildInfo.UpperCellWidth) * CellFactor;

	if (XShift == 0 && YShift == 0)
	{
		return {};
	}

	const FVector2D NewRootCell = BuildInfo.RootCell + FVector2D(XShift, YShift);

	FOpenLandGridChangedCells ChangedCells;

	// Loop the Old Cells to find what to remove
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			const int32 XPos = BuildInfo.RootCell.X + X;
			const int32 YPos = BuildInfo.RootCell.Y + Y;

			if (XPos < NewRootCell.X || XPos >= NewRootCell.X + BuildInfo.Size.X)
			{
				ChangedCells.CellsToRemove.Push(FVector2D(X, Y) + BuildInfo.RootCell);
				continue;
			}

			if (YPos < NewRootCell.Y || YPos >= NewRootCell.Y + BuildInfo.Size.Y)
			{
				ChangedCells.CellsToRemove.Push(FVector2D(XPos, YPos));
				continue;
			}

			ChangedCells.ExistingCells.Push(FVector2D(XPos, YPos));
		}
	}

	// Loop the NewCells to find out what to add
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			const int32 XPos = NewRootCell.X + X;
			const int32 YPos = NewRootCell.Y + Y;

			if (XPos < BuildInfo.RootCell.X || XPos >= BuildInfo.RootCell.X + BuildInfo.Size.X)
			{
				ChangedCells.CellsToAdd.Push(FVector2D(XPos, YPos));
				continue;
			}

			if (YPos < BuildInfo.RootCell.Y || YPos >= BuildInfo.RootCell.Y + BuildInfo.Size.Y)
			{
				ChangedCells.CellsToAdd.Push(FVector2D(XPos, YPos));
				continue;
			}
			
		}
	}

	BuildInfo.RootCell = NewRootCell;

	return ChangedCells;
}

TArray<FVector2D> FOpenLandGrid::GetAllCells() const
{
	TArray<FVector2D> Cells;
	
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			Cells.Push(FVector2D(X, Y) + BuildInfo.RootCell);
		}
	}

	return Cells;
}
