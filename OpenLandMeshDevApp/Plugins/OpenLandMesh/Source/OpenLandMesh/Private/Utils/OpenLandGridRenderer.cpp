#include "Utils/OpenLandGridRenderer.h"

#include "Compute/OpenLandThreading.h"
#include "Core/OpenLandPolygonMesh.h"

FOpenLandGridRenderer::FOpenLandGridRenderer()
{
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildCell(FOpenLandGridCell Cell) const
{
	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	
	FVector CellPos = FVector(Cell.X, Cell.Y, 0) * Grid->GetCellWidth();
		
	FVector A = CellPos + FVector(0, 0, 0) * Grid->GetCellWidth();
	FVector B = CellPos + FVector(0, 1, 0) * Grid->GetCellWidth();
	FVector C = CellPos + FVector(1, 1, 0) * Grid->GetCellWidth();
	FVector D = CellPos + FVector(1, 0, 0) * Grid->GetCellWidth();

	FVector2D UVRoot = Cell.ToVector2D() * UVCellWidth;
			
	FOpenLandMeshVertex MA = {ApplyVertexModifier(Cell, A), UVRoot + FVector2D(0, 0) * UVCellWidth};
	FOpenLandMeshVertex MB = {ApplyVertexModifier(Cell, B), UVRoot + FVector2D(0, 1) * UVCellWidth};
	FOpenLandMeshVertex MC = {ApplyVertexModifier(Cell, C), UVRoot + FVector2D(1, 1) * UVCellWidth};
	FOpenLandMeshVertex MD = {ApplyVertexModifier(Cell, D), UVRoot + FVector2D(1, 0) * UVCellWidth};

	FOpenLandMeshVertex T0_1 = MA;
	FOpenLandMeshVertex T0_2 = MB;
	FOpenLandMeshVertex T0_3 = MC;
	FOpenLandPolygonMesh::BuildFaceTangents(T0_1, T0_2, T0_3);

	FOpenLandMeshVertex T1_1 = MA;
	FOpenLandMeshVertex T1_2 = MC;
	FOpenLandMeshVertex T1_3 = MD;
	FOpenLandPolygonMesh::BuildFaceTangents(T1_1, T1_2, T1_3);

	return {
		T0_1, T0_2, T0_3,
		T1_1, T1_2, T1_3
	};
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildEdgeCell(FOpenLandGridCell Cell) const
{
	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	const FVector2D VectorFromHoleMid = Cell.ToVector2D() - (Grid->GetHoleRootCell().ToVector2D() + Grid->GetHoleSize().ToVector2D() / 2.0f);
	// TODO: Try to get rid of Atan and simply use the tangent value.
	const float Angle = FMath::Atan2(VectorFromHoleMid.Y, VectorFromHoleMid.X) + PI;

	TOpenLandArray<FOpenLandMeshVertex> Vertices;

	const auto AddTri = [&Vertices](FOpenLandMeshVertex A, FOpenLandMeshVertex B, FOpenLandMeshVertex C)
	{
		FOpenLandPolygonMesh::BuildFaceTangents(A, B, C);
		Vertices.Push(A);
		Vertices.Push(B);
		Vertices.Push(C);
	};
	
	FVector CellPos = FVector(Cell.X, Cell.Y, 0) * Grid->GetCellWidth();
		
	FVector A = CellPos + FVector(0, 0, 0) * Grid->GetCellWidth();
	FVector B = CellPos + FVector(0, 1, 0) * Grid->GetCellWidth();
	FVector C = CellPos + FVector(1, 1, 0) * Grid->GetCellWidth();
	FVector D = CellPos + FVector(1, 0, 0) * Grid->GetCellWidth();
	

	FVector2D UVRoot = Cell.ToVector2D() * UVCellWidth;
			
	FOpenLandMeshVertex MA = {ApplyVertexModifier(Cell, A), UVRoot + FVector2D(0, 0) * UVCellWidth};
	FOpenLandMeshVertex MB = {ApplyVertexModifier(Cell, B), UVRoot + FVector2D(0, 1) * UVCellWidth};
	FOpenLandMeshVertex MC = {ApplyVertexModifier(Cell, C), UVRoot + FVector2D(1, 1) * UVCellWidth};
	FOpenLandMeshVertex MD = {ApplyVertexModifier(Cell, D), UVRoot + FVector2D(1, 0) * UVCellWidth};

	const FOpenLandGridCell EdgeRoot = Grid->GetHoleRootCell() - FOpenLandGridCell(1, 1);
	const FOpenLandGridCell EdgeRoot2 = EdgeRoot + FOpenLandGridCell(0, Grid->GetHoleSize().Y + 1);
	const FOpenLandGridCell EdgeRoot3 = EdgeRoot + FOpenLandGridCell(Grid->GetHoleSize().X + 1, Grid->GetHoleSize().Y + 1);
	const FOpenLandGridCell EdgeRoot4 = EdgeRoot + FOpenLandGridCell(Grid->GetHoleSize().X + 1, 0);

	if (Cell == EdgeRoot || Cell == EdgeRoot2 || Cell == EdgeRoot3 || Cell == EdgeRoot4)
	{
		AddTri(MA, MB, MC);
		AddTri(MA, MC, MD);
		// This is just to make the computing easier and all edge cell has 3 triangles.
		AddTri(MA, MC, MB);
		return Vertices;
	}

	if (Angle > PI + PI/4 && Angle <= 2*PI - PI/4)
	{
		FVector E = CellPos + FVector(0.5, 0, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(0.5, 0) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MC, MD);
	}
	else if(Angle > 2*PI - PI/4 || Angle <= + PI/4)
	{
		FVector E = CellPos + FVector(1, 0.5, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(1, 0.5) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MD, MA);
	} else if (Angle > PI/4 && Angle < PI - PI/4)
	{
		FVector E = CellPos + FVector(0.5, 1, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(0.5, 1) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MD, MA);
		AddTri(ME, MC, MD);
	} else
	{
		FVector E = CellPos + FVector(0, 0.5, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(0, 0.5) * UVCellWidth};

		AddTri(MA, ME, MD);
		AddTri(ME, MB, MC);
		AddTri(ME, MC, MD);
	}
	
	return Vertices;
}

void FOpenLandGridRenderer::ApplyCellChangesAsync(FOpenLandGridChangedCells ChangedCells)
{
	// Apply Cell Changes
	
	CurrentGridChangedCells = ChangedCells.Clone();
	CellGenInfo.Reset(
		ChangedCells.CellsToAdd.Num(),
		ChangedCells.EdgeCellsToAdd.Num(),
		ChangedCells.ExistingEdgeCells.Num()
	);

	FinishCellGeneration();
	
	MeshInfo->BoundingBox = Grid->GetBoundingBox();
}

void FOpenLandGridRenderer::FinishCellGeneration()
{
	FOpenLandThreading::RunOnAnyBackgroundThread([this]()
	{
		
		for (int32 Index = 0; Index < CurrentGridChangedCells->CellsToRemove.Num(); Index++)
		{
			SwapCell(Index);
		}

		// Apply Edge Cell Changes
		for (int32 Index = 0; Index < CurrentGridChangedCells->EdgeCellsToRemove.Num(); Index++)
		{
			SwapEdgeCell(Index);
		}
		
		// Go through existing edge cells & regenerate cells
		for (int32 Index = 0; Index < CurrentGridChangedCells->ExistingEdgeCells.Num(); Index++)
		{
			RegenerateEdgeCell(Index);
		}
		
		CurrentOperation->bCompleted = true;
	});
}

void FOpenLandGridRenderer::SwapCell(int32 Index)
{
	const FOpenLandGridCell RemovingCellPos = CurrentGridChangedCells->CellsToRemove[Index];
	const FOpenLandGridCell AddingCellPos = CurrentGridChangedCells->CellsToAdd[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(AddingCellPos);

	const FOpenLandGridRendererCell RenderCell = Cells[GetTypeHash(RemovingCellPos)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));

	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));

	Cells.Remove(GetTypeHash(RemovingCellPos));
	Cells.Add(GetTypeHash(AddingCellPos), {
		AddingCellPos,
		RenderCell.IndexT0,
		RenderCell.IndexT1
	});

	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

void FOpenLandGridRenderer::SwapEdgeCell(int32 Index)
{
	const FOpenLandGridCell RemovingCellPos = CurrentGridChangedCells->EdgeCellsToRemove[Index];
	const FOpenLandGridCell AddingCellPos = CurrentGridChangedCells->EdgeCellsToAdd[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildEdgeCell(AddingCellPos);
	
	const FOpenLandGridRendererEdgeCell RenderCell = EdgeCells[GetTypeHash(RemovingCellPos)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));
	
	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));
	
	const FOpenLandMeshTriangle T2 = MeshInfo->Triangles.Get(RenderCell.IndexT2);
	MeshInfo->Vertices.Set(T2.T0, CellVertices.Get(6));
	MeshInfo->Vertices.Set(T2.T1, CellVertices.Get(7));
	MeshInfo->Vertices.Set(T2.T2, CellVertices.Get(8));
	
	EdgeCells.Remove(GetTypeHash(RemovingCellPos));
	EdgeCells.Add(GetTypeHash(AddingCellPos), {
		AddingCellPos,
		RenderCell.IndexT0,
		RenderCell.IndexT1,
		RenderCell.IndexT2
	}); 
	
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT2);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

void FOpenLandGridRenderer::RegenerateEdgeCell(int32 Index)
{
	const FOpenLandGridCell Cell = CurrentGridChangedCells->ExistingEdgeCells[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildEdgeCell(Cell);
	
	const FOpenLandGridRendererEdgeCell RenderCell = EdgeCells[GetTypeHash(Cell)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));
	
	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));
	
	const FOpenLandMeshTriangle T2 = MeshInfo->Triangles.Get(RenderCell.IndexT2);
	MeshInfo->Vertices.Set(T2.T0, CellVertices.Get(6));
	MeshInfo->Vertices.Set(T2.T1, CellVertices.Get(7));
	MeshInfo->Vertices.Set(T2.T2, CellVertices.Get(8));
		
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT2);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

FVector FOpenLandGridRenderer::ApplyVertexModifier(FOpenLandGridCell Cell, FVector Source)
{
	const float Distance = FVector::Distance(Source, FVector(0, 0, 0));
	constexpr float Divider = 2000.0f;
	constexpr float Height = 300.0f;
	
	float NewHeight = FMath::Sin(Distance/Divider) * Height;
	return Source + FVector(0, 0, NewHeight);
}

FOpenLandMeshInfoPtr FOpenLandGridRenderer::Initialize(FOpenLandGridPtr SourceGrid)
{
	check(bInitialized == false);
	bInitialized = true;
	Grid = SourceGrid;
	MeshInfo = FOpenLandMeshInfo::New();
	Cells = {};

	FOpenLandGridChangedCells InitialCells = Grid->GetAllCells();
	
	for(const FOpenLandGridCell Cell: InitialCells.CellsToAdd)
	{
		const TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(Cell);
		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), CellVertices);
		
		const int32 TotalTriangles = MeshInfo->Triangles.Length();
		Cells.Add(GetTypeHash(Cell), {
			Cell,
			TotalTriangles - 2,
			TotalTriangles - 1,
		});
	}

	for(const FOpenLandGridCell Cell: InitialCells.EdgeCellsToAdd)
	{
		const TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildEdgeCell(Cell);
		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), CellVertices);
		
		const int32 TotalTriangles = MeshInfo->Triangles.Length();
		EdgeCells.Add(GetTypeHash(Cell), {
			Cell,
			TotalTriangles - 3,
			TotalTriangles - 2,
			TotalTriangles - 1,
		});
	}

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return  MeshInfo;
}

bool FOpenLandGridRenderer::StartReCenter(FVector NewCenter)
{
	check(CurrentOperation == nullptr);
	
	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter);
	//UE_LOG(LogTemp, Warning, TEXT("CellsToAdd: %d, CellsToRemove: %d, EdgeCellsToAdd: %d, EdgeCellsToRemove: %d"), ChangedCells.CellsToAdd.Num(), ChangedCells.CellsToRemove.Num(), ChangedCells.EdgeCellsToAdd.Num(), ChangedCells.EdgeCellsToRemove.Num())
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());
	check(ChangedCells.EdgeCellsToAdd.Num() == ChangedCells.EdgeCellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}

	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

bool FOpenLandGridRenderer::StartReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell)
{
	check(CurrentOperation == nullptr);

	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter, NewHoleRootCell);
	//UE_LOG(LogTemp, Warning, TEXT("CellsToAdd: %d, CellsToRemove: %d, EdgeCellsToAdd: %d, EdgeCellsToRemove: %d"), ChangedCells.CellsToAdd.Num(), ChangedCells.CellsToRemove.Num(), ChangedCells.EdgeCellsToAdd.Num(), ChangedCells.EdgeCellsToRemove.Num())
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());
	check(ChangedCells.EdgeCellsToAdd.Num() == ChangedCells.EdgeCellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}
	
	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

bool FOpenLandGridRenderer::StartChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell)
{
	check(CurrentOperation == nullptr);

	const FOpenLandGridChangedCells ChangedCells = Grid->ChangeHoleRootCell(NewHoleRootCell);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}

	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

TSharedPtr<FOpenLandGridRendererChangedInfo> FOpenLandGridRenderer::CheckStatus()
{
	if (!CurrentOperation)
	{
		return nullptr;
	}

	if (!CurrentOperation->bCompleted)
	{
		return nullptr;
	}

	const TSharedPtr<FOpenLandGridRendererChangedInfo> ChangedInfo = MakeShared<FOpenLandGridRendererChangedInfo>();
	ChangedInfo->ChangedTriangles = CurrentOperation->ChangedInfo.ChangedTriangles;
	
	CurrentOperation = nullptr;

	return ChangedInfo;
}