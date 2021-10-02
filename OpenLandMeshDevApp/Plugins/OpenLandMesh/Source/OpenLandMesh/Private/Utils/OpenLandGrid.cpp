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

void FOpenLandGrid::ApplyEdgeModifications(FOpenLandGridChangedCells &ChangedCells, FOpenLandGridEdgeModificationOptions Options) const
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

	const auto HandleOldEdgeCell = [this, &ChangedCells, Options](FOpenLandGridCell Cell)
	{
		const bool bInInsideNewEdge = IsHoleEdge(Options.NewHoleRootCell, Cell);
		if (bInInsideNewEdge)
		{
			return;
		}

		ChangedCells.EdgeCellsToRemove.Push(Cell);
		ChangedCells.CellsToAdd.Push(Cell);
	};

	const auto HandleNewEdgeCell = [this, &ChangedCells, Options](FOpenLandGridCell Cell)
	{
		const bool bInInsideOldEdge = IsHoleEdge(Options.OldHoleRootCell, Cell);
		if (bInInsideOldEdge)
		{
			return;
		}

		ChangedCells.EdgeCellsToAdd.Push(Cell);
		ChangedCells.CellsToRemove.Push(Cell);
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
			const FOpenLandGridCell EdgeCell = OldHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointRemoved(EdgeCell))
			{
				continue;
			}
			
			HandleOldEdgeCell(EdgeCell);
		}
		X += BuildInfo.HoleSize.X;
	}

	for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
	{
		for (int32 X=1; X<BuildInfo.HoleSize.X + 1; X++)
		{
			const FOpenLandGridCell EdgeCell = OldHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointRemoved(EdgeCell))
			{
				continue;
			}
			
			HandleOldEdgeCell(EdgeCell);
		}
		Y += BuildInfo.HoleSize.Y;
	}

	// Look at the New Edge & add the edge hole modification
	const FOpenLandGridCell NewHoleEdgeRoot = Options.NewHoleRootCell - FOpenLandGridCell(1, 1);
	for (int32 X=0; X<BuildInfo.HoleSize.X + 2; X++)
	{
		for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
		{
			const FOpenLandGridCell EdgeCell = NewHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointAdded(EdgeCell))
			{
				continue;
			}

			HandleNewEdgeCell(EdgeCell);
		}
		X += BuildInfo.HoleSize.X;
	}

	for (int32 Y=0; Y<BuildInfo.HoleSize.Y + 2; Y++)
	{
		for (int32 X=1; X<BuildInfo.HoleSize.X + 1; X++)
		{
			const FOpenLandGridCell EdgeCell = NewHoleEdgeRoot + FOpenLandGridCell(X, Y);
			if (IsPointAdded(EdgeCell))
			{
				continue;
			}
			
			HandleNewEdgeCell(EdgeCell);
		}
		Y += BuildInfo.HoleSize.Y;
	}
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
			const bool bIsHoleEdge = IsHoleEdge(NewHoleRootCell, CurrentCell);
			if (bIsHoleEdge)
			{
				ChangedCells.EdgeCellsToAdd.Push(CurrentCell);
			}
			else
			{
				ChangedCells.CellsToAdd.Push(CurrentCell);
			}
		}
	}

	// Loop over Old Cells
	for (const FOpenLandGridCell OldCell: OldCells.Array())
	{
		if (!BuildInfo.HasHole())
		{
			ChangedCells.CellsToRemove.Push(OldCell);
			continue;
		}
		
		bool bWasInHoleEdge = IsHoleEdge(BuildInfo.HoleRootCell, OldCell);
		if (bWasInHoleEdge)
		{
			ChangedCells.EdgeCellsToRemove.Push(OldCell);
		}
		else
		{
			ChangedCells.CellsToRemove.Push(OldCell);
		}
	}

	// Finding Modified Cells
	FOpenLandGridEdgeModificationOptions EdgeModificationOptions;
	EdgeModificationOptions.OldHoleRootCell = BuildInfo.HoleRootCell;
	EdgeModificationOptions.NewHoleRootCell = NewHoleRootCell;
	EdgeModificationOptions.OldRootCell = BuildInfo.RootCell;
	EdgeModificationOptions.NewRootCell = NewRootCell;
	
	ApplyEdgeModifications(ChangedCells, EdgeModificationOptions);
	
	// Update the New State
	BuildInfo.RootCell = NewRootCell;
	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::ChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell)
{
	if (BuildInfo.HoleRootCell == NewHoleRootCell)
	{
		return {};
	}
	
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
				const bool bIsHoleEdge = IsHoleEdge(NewHoleRootCell, CellPoint);
				if (bIsHoleEdge)
				{
					ChangedCells.EdgeCellsToAdd.Push(CellPoint);
				}
				else
				{
					ChangedCells.CellsToAdd.Push(CellPoint);
				}
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
				const bool bWasInHoleEdge = IsHoleEdge(BuildInfo.HoleRootCell, CellPoint);
				if (bWasInHoleEdge)
				{
					ChangedCells.EdgeCellsToRemove.Push(CellPoint);
				}
				else
				{
					ChangedCells.CellsToRemove.Push(CellPoint);
				}
			}
			
		}
	}

	// Finding Modified Cells
	FOpenLandGridEdgeModificationOptions EdgeModificationOptions;
	EdgeModificationOptions.OldHoleRootCell = BuildInfo.HoleRootCell;
	EdgeModificationOptions.NewHoleRootCell = NewHoleRootCell;
	EdgeModificationOptions.OldRootCell = BuildInfo.RootCell;
	EdgeModificationOptions.NewRootCell = BuildInfo.RootCell;
	
	ApplyEdgeModifications(ChangedCells, EdgeModificationOptions);
	
	BuildInfo.HoleRootCell = NewHoleRootCell;

	return ChangedCells;
}

FOpenLandGridChangedCells FOpenLandGrid::GetAllCells() const
{
	FOpenLandGridChangedCells ChangedCells;
	
	for (const FOpenLandGridCell Cell: GetAllCellsSet())
	{
		if (BuildInfo.HasHole() && IsHoleEdge(BuildInfo.HoleRootCell, Cell))
		{
			ChangedCells.EdgeCellsToAdd.Push(Cell);
			continue;
		}

		ChangedCells.CellsToAdd.Push(Cell);
	}

	return ChangedCells;
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
