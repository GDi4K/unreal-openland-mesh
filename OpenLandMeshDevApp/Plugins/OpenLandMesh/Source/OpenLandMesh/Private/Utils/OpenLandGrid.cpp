#include "Utils/OpenLandGrid.h"

FVector FOpenLandGrid::ToVector3D(FOpenLandGridCell Vector)
{
	return { static_cast<float>(Vector.X), static_cast<float>(Vector.Y), 0};
}

bool FOpenLandGrid::IsPointInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell PointToCheck)
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

bool FOpenLandGrid::IsRectInsideRect(FOpenLandGridCell RectOuterRoot, FOpenLandGridCell RectOuterSize, FOpenLandGridCell RectInnerRoot, FOpenLandGridCell RectInnerSize)
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

bool FOpenLandGrid::IsHoleInsideRect(FOpenLandGridCell RectRoot, FOpenLandGridCell RectSize, FOpenLandGridCell HoleRoot, FOpenLandGridCell HoleSize)
{
	return IsRectInsideRect(RectRoot + FVector2D(1, 1), RectSize - FVector2D(2, 2), HoleRoot, HoleSize);
}

TSet<FOpenLandGridCell> FOpenLandGrid::GetAllCellsSet() const
{
	TSet<FOpenLandGridCell> Cells;
	
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			FOpenLandGridCell CurrentCell = FOpenLandGridCell(X, Y) + BuildInfo.RootCell;
			if (BuildInfo.HasHole())
			{
				if (!IsPointInsideRect(BuildInfo.HoleRootCell, BuildInfo.HoleSize, CurrentCell))
				{
					CurrentCell.bHoleEdge = IsHoleEdge(BuildInfo.HoleRootCell, CurrentCell);
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


TArray<FOpenLandGridCell> FOpenLandGrid::ApplyEdgeModifications(FOpenLandGridEdgeModificationOptions Options) const
{
	TSet<FOpenLandGridCell> EdgeCells;
	const auto AddCell = [&EdgeCells](FOpenLandGridCell Cell)
	{
		if (EdgeCells.Find(Cell))
		{
			EdgeCells.Remove(Cell);
		}
		EdgeCells.Add(Cell);
	};

	const auto IsPointRemoved = [Options, this](FOpenLandGridCell Cell)
	{
		if (IsPointInsideRect(Options.NewRootCell, BuildInfo.Size, Cell))
		{
			if (!IsPointInsideRect(Options.NewHoleRootCell, BuildInfo.HoleSize, Cell))
			{
				return false;
			}
		}

		return true;
	};

	const auto IsPointAdded = [Options, this](FOpenLandGridCell Cell)
	{
		if (IsPointInsideRect(Options.OldRootCell, BuildInfo.Size, Cell))
		{
			if (!IsPointInsideRect(Options.OldHoleRootCell, BuildInfo.HoleSize, Cell))
			{
				return false;
			}
		}

		return true;
	};

	// Look at the OLD Edge & remove the edge hole modification
	const FOpenLandGridCell OldHoleEdgeRoot = Options.OldHoleRootCell - FOpenLandGridCell(1, 1);
	for (int32 X=0; X<BuildInfo.HoleSize.X + 2; X++)
	{
		for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
		{
			FOpenLandGridCell EdgeCell = OldHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointRemoved(EdgeCell))
			{
				continue;
			}
			
			EdgeCell.bHoleEdge = false;
			AddCell(EdgeCell);
		}
		X += BuildInfo.HoleSize.X;
	}

	for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
	{
		for (int32 X=1; X<BuildInfo.HoleSize.X + 1; X++)
		{
			FOpenLandGridCell EdgeCell = OldHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointRemoved(EdgeCell))
			{
				continue;
			}
			
			EdgeCell.bHoleEdge = false;
			AddCell(EdgeCell);
		}
		Y += BuildInfo.HoleSize.Y;
	}

	// Look at the New Edge & add the edge hole modification
	const FOpenLandGridCell NewHoleEdgeRoot = Options.NewHoleRootCell - FOpenLandGridCell(1, 1);
	for (int32 X=0; X<BuildInfo.HoleSize.X + 2; X++)
	{
		for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
		{
			FOpenLandGridCell EdgeCell = NewHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointAdded(EdgeCell))
			{
				continue;
			}
			
			EdgeCell.bHoleEdge = true;
			AddCell(EdgeCell);
		}
		X += BuildInfo.HoleSize.X;
	}

	for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
	{
		for (int32 X=1; X<BuildInfo.HoleSize.X + 1; X++)
		{
			FOpenLandGridCell EdgeCell = NewHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointAdded(EdgeCell))
			{
				continue;
			}
			
			EdgeCell.bHoleEdge = true;
			AddCell(EdgeCell);
		}
		Y += BuildInfo.HoleSize.Y;
	}

	return EdgeCells.Array();
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
		check(IsHoleInsideRect(BuildInfo.RootCell, BuildInfo.Size, BuildInfo.HoleRootCell, BuildInfo.HoleSize))
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

FOpenLandGridCell FOpenLandGrid::FindClosestCellRoot(FVector Position) const
{
	const float XPos = FMath::Floor(Position.X / BuildInfo.CellWidth);
	const float YPos = FMath::Floor(Position.Y / BuildInfo.CellWidth);
	return {static_cast<int32>(XPos), static_cast<int32>(YPos)};
}

FOpenLandGridChangedCells FOpenLandGrid::ReCenter(FVector NewCenter)
{
	const FVector2D NewCenter2D = {NewCenter.X, NewCenter.Y};
	const FVector2D CurrentCenter = (BuildInfo.RootCell.ToVector2D() + (BuildInfo.Size.ToVector2D() * 0.5)) * BuildInfo.CellWidth;
	const FVector2D Diff = NewCenter2D - CurrentCenter;

	const int32 CellFactor = BuildInfo.UpperCellWidth/BuildInfo.CellWidth;

	const int32 XShift = FMath::RoundToInt(Diff.X / BuildInfo.UpperCellWidth) * CellFactor;
	const int32 YShift = FMath::RoundToInt(Diff.Y / BuildInfo.UpperCellWidth) * CellFactor;

	if (XShift == 0 && YShift == 0)
	{
		return {};
	}

	const FOpenLandGridCell NewRootCell = BuildInfo.RootCell+ FOpenLandGridCell(XShift, YShift);

	if (BuildInfo.HasHole())
	{
		if(!IsHoleInsideRect(NewRootCell, BuildInfo.Size, BuildInfo.HoleRootCell, BuildInfo.HoleSize))
		{
			return {};
		}	
	}
	
	FOpenLandGridChangedCells ChangedCells;

	TSet<FOpenLandGridCell> OldCells = GetAllCellsSet();
	
	// Loop Over New Cells
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			FOpenLandGridCell CurrentCell = FOpenLandGridCell(X, Y) + NewRootCell;
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
			CurrentCell.bHoleEdge = IsHoleEdge(BuildInfo.HoleRootCell, CurrentCell);
			ChangedCells.CellsToAdd.Push(CurrentCell);
		}
	}

	ChangedCells.CellsToRemove = OldCells.Array();

	BuildInfo.RootCell = NewRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::ReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell)
{
	const FVector2D NewCenter2D = {NewCenter.X, NewCenter.Y};
	const FVector2D CurrentCenter = (BuildInfo.RootCell.ToVector2D() + (BuildInfo.Size.ToVector2D() * 0.5)) * BuildInfo.CellWidth;
	const FVector2D Diff = NewCenter2D - CurrentCenter;

	const int32 CellFactor = BuildInfo.UpperCellWidth/BuildInfo.CellWidth;

	const int32 XShift = FMath::RoundToInt(Diff.X / BuildInfo.UpperCellWidth) * CellFactor;
	const int32 YShift = FMath::RoundToInt(Diff.Y / BuildInfo.UpperCellWidth) * CellFactor;

	if (XShift == 0 && YShift == 0)
	{
		return {};
	}

	const FOpenLandGridCell NewRootCell = BuildInfo.RootCell + FOpenLandGridCell(XShift, YShift);

	if (BuildInfo.HasHole())
	{
		if(!IsHoleInsideRect(NewRootCell, BuildInfo.Size, NewHoleRootCell, BuildInfo.HoleSize))
		{
			return {};
		}	
	}
	
	FOpenLandGridChangedCells ChangedCells;

	TSet<FOpenLandGridCell> OldCells = GetAllCellsSet();
	
	// Loop Over New Cells
	for (int32 X=0; X<BuildInfo.Size.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.Size.Y; Y++)
		{
			FOpenLandGridCell CurrentCell = FOpenLandGridCell(X, Y) + NewRootCell;
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
			CurrentCell.bHoleEdge = IsHoleEdge(NewHoleRootCell, CurrentCell);
			ChangedCells.CellsToAdd.Push(CurrentCell);
		}
	}

	FOpenLandGridEdgeModificationOptions EdgeModificationOptions;
	EdgeModificationOptions.OldHoleRootCell = BuildInfo.HoleRootCell;
	EdgeModificationOptions.NewHoleRootCell = NewHoleRootCell;
	EdgeModificationOptions.OldRootCell = BuildInfo.RootCell;
	EdgeModificationOptions.NewRootCell = NewRootCell;
	
	ChangedCells.ModifiedCells = ApplyEdgeModifications(EdgeModificationOptions);

	ChangedCells.CellsToRemove = OldCells.Array();
	
	BuildInfo.RootCell = NewRootCell;
	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::ChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell)
{
	if(!IsHoleInsideRect(BuildInfo.RootCell, BuildInfo.Size, NewHoleRootCell, BuildInfo.HoleSize))
	{
		return {};
	}

	FOpenLandGridChangedCells ChangedCells;
	
	// Loop the Old Cells to find whether it needs to add back to the mesh
	for (int32 X=0; X<BuildInfo.HoleSize.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.HoleSize.Y; Y++)
		{
			FOpenLandGridCell CellPoint = BuildInfo.HoleRootCell + FOpenLandGridCell(X, Y);
			if (!IsPointInsideRect(NewHoleRootCell, BuildInfo.HoleSize, CellPoint))
			{
				CellPoint.bHoleEdge = IsHoleEdge(NewHoleRootCell, CellPoint);
				ChangedCells.CellsToAdd.Push(CellPoint);
			}
			
		}
	}

	// Loop the NewCells to find whether it needs to remove from the mesh
	for (int32 X=0; X<BuildInfo.HoleSize.X; X++)
	{
		for(int32 Y=0; Y<BuildInfo.HoleSize.Y; Y++)
		{
			const FOpenLandGridCell CellPoint = NewHoleRootCell + FOpenLandGridCell(X, Y);
			if (!IsPointInsideRect(BuildInfo.HoleRootCell, BuildInfo.HoleSize, CellPoint))
			{
				ChangedCells.CellsToRemove.Push(CellPoint);
			}
			
		}
	}

	FOpenLandGridEdgeModificationOptions EdgeModificationOptions;
	EdgeModificationOptions.OldHoleRootCell = BuildInfo.HoleRootCell;
	EdgeModificationOptions.NewHoleRootCell = NewHoleRootCell;
	EdgeModificationOptions.OldRootCell = BuildInfo.RootCell;
	EdgeModificationOptions.NewRootCell = BuildInfo.RootCell;
	
	ChangedCells.ModifiedCells = ApplyEdgeModifications(EdgeModificationOptions);
	
	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

TArray<FOpenLandGridCell> FOpenLandGrid::GetAllCells() const
{
	return GetAllCellsSet().Array();
}

bool FOpenLandGrid::IsHoleEdge(FOpenLandGridCell HoleCell, FOpenLandGridCell Cell) const
{
	if (!BuildInfo.HasHole())
	{
		return false;
	}
	
	if(IsPointInsideRect(HoleCell - FOpenLandGridCell(1, 1), BuildInfo.HoleSize + FOpenLandGridCell(2, 2), Cell))
	{
		return true;
	}

	return false;
}
