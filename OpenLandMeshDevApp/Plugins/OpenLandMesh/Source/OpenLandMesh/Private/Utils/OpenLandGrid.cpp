#include "Utils/OpenLandGrid.h"

FVector FOpenLandGrid::ToVector3D(FVector2D Vector)
{
	return { Vector.X, Vector.Y, 0};
}

bool FOpenLandGrid::IsPointInsideRect(FVector2D RectRoot, FVector2D RectSize, FVector2D PointToCheck)
{
	const int32 XPos = PointToCheck.X;
	const int32 YPos = PointToCheck.Y;

	if (XPos < RectRoot.X || XPos >= RectRoot.X + RectSize.X)
	{
		return false;
	}

	if (YPos < RectRoot.Y || YPos >= RectRoot.Y + RectSize.Y)
	{
		return false;
	}

	return true;
}

bool FOpenLandGrid::IsRectInsideRect(FVector2D RectOuterRoot, FVector2D RectOuterSize, FVector2D RectInnerRoot, FVector2D RectInnerSize)
{
	if (RectInnerRoot.X < RectOuterRoot.X)
	{
		return false;
	}

	if (RectInnerRoot.X + RectInnerSize.X > RectOuterRoot.X + RectOuterSize.X)
	{
		return false;
	}

	if (RectInnerRoot.Y < RectOuterRoot.Y)
	{
		return false;
	}

	if (RectInnerRoot.Y + RectInnerSize.Y > RectOuterRoot.Y + RectOuterSize.Y)
	{
		return false;
	}

	return true;
}

TSet<FVector2D> FOpenLandGrid::GetAllCellsSet() const
{
	TSet<FVector2D> Cells;
	
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			const FVector2D CurrentCell = FVector2D(X, Y) + BuildInfo.RootCell;
			if (BuildInfo.HasHole())
			{
				if (!IsPointInsideRect(BuildInfo.HoleRootCell, BuildInfo.HoleSize, CurrentCell))
				{
					Cells.Add(CurrentCell);
				}
			} else
			{
				Cells.Add(CurrentCell);
			}
		}
	}

	return Cells;
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
		check(IsRectInsideRect(BuildInfo.RootCell, BuildInfo.Size, BuildInfo.HoleRootCell, BuildInfo.HoleSize))
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

FVector2D FOpenLandGrid::FindClosestCellRoot(FVector Position) const
{
	const float XPos = FMath::Floor(Position.X / BuildInfo.CellWidth);
	const float YPos = FMath::Floor(Position.Y / BuildInfo.CellWidth);
	return {XPos, YPos};
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

	if (BuildInfo.HasHole())
	{
		if(!IsRectInsideRect(NewRootCell, BuildInfo.Size, BuildInfo.HoleRootCell, BuildInfo.HoleSize))
		{
			return {};
		}	
	}
	
	FOpenLandGridChangedCells ChangedCells;

	TSet<FVector2D> OldCells = GetAllCellsSet();
	
	// Loop Over New Cells
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			const FVector2D CurrentCell = FVector2D(X, Y) + NewRootCell;
			if (BuildInfo.HasHole())
			{
				if (IsPointInsideRect(BuildInfo.HoleRootCell, BuildInfo.HoleSize, CurrentCell))
				{
					// There's no such cell
					continue;
				}
			}

			// Existing cell
			if (OldCells.Find(CurrentCell) != nullptr)
			{
				OldCells.Remove(CurrentCell);
				continue;
			}

			// This is a new cell
			ChangedCells.CellsToAdd.Push(CurrentCell);
		}
	}

	ChangedCells.CellsToRemove = OldCells.Array();

	BuildInfo.RootCell = NewRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::ReCenter(FVector NewCenter, FVector2D NewHoleRootCell)
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

	if (BuildInfo.HasHole())
	{
		if(!IsRectInsideRect(NewRootCell, BuildInfo.Size, NewHoleRootCell, BuildInfo.HoleSize))
		{
			return {};
		}	
	}
	
	FOpenLandGridChangedCells ChangedCells;

	TSet<FVector2D> OldCells = GetAllCellsSet();
	
	// Loop Over New Cells
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			const FVector2D CurrentCell = FVector2D(X, Y) + NewRootCell;
			if (BuildInfo.HasHole())
			{
				if (IsPointInsideRect(NewHoleRootCell, BuildInfo.HoleSize, CurrentCell))
				{
					// There's no such cell
					continue;
				}
			}

			// Existing cell
			if (OldCells.Find(CurrentCell) != nullptr)
			{
				OldCells.Remove(CurrentCell);
				continue;
			}

			// This is a new cell
			ChangedCells.CellsToAdd.Push(CurrentCell);
		}
	}

	ChangedCells.CellsToRemove = OldCells.Array();

	BuildInfo.RootCell = NewRootCell;
	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::ChangeHoleRootCell(FVector2D NewHoleRootCell)
{
	if(!IsRectInsideRect(BuildInfo.RootCell, BuildInfo.Size, NewHoleRootCell, BuildInfo.HoleSize))
	{
		return {};
	}

	FOpenLandGridChangedCells ChangedCells;
	
	// Loop the Old Cells to find whether it needs to add back to the mesh
	for (int32 X=0; X<BuildInfo.HoleSize.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.HoleSize.Y; Y++)
		{
			const FVector2D CellPoint = BuildInfo.HoleRootCell + FVector2D(X, Y);
			if (!IsPointInsideRect(NewHoleRootCell, BuildInfo.HoleSize, CellPoint))
			{
				ChangedCells.CellsToAdd.Push(CellPoint);
			}
			
		}
	}

	// Loop the NewCells to find whether it needs to remove from the mesh
	for (int32 X=0; X<BuildInfo.HoleSize.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.HoleSize.Y; Y++)
		{
			const FVector2D CellPoint = NewHoleRootCell + FVector2D(X, Y);
			if (!IsPointInsideRect(BuildInfo.HoleRootCell, BuildInfo.HoleSize, CellPoint))
			{
				ChangedCells.CellsToRemove.Push(CellPoint);
			}
			
		}
	}

	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

TArray<FVector2D> FOpenLandGrid::GetAllCells() const
{
	return GetAllCellsSet().Array();
}
