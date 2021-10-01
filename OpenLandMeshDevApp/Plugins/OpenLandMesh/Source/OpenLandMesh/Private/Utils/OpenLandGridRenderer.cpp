#include "Utils/OpenLandGridRenderer.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandGridRenderer::FOpenLandGridRenderer()
{
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildCell(FOpenLandGridCell Cell) const
{
	if (Cell.bHoleEdge)
	{
		return BuildEdgeCell(Cell);
	}
	
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
	const float angle = FMath::Atan2(Cell.Y, Cell.X) + PI;
	UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), angle)

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
		// This is just to make the computing easier
		AddTri(MA, MC, MD);
		return Vertices;
	}

	if (angle > PI + PI/4 && angle <= 2*PI - PI/4)
	{
		FVector E = CellPos + FVector(0.5, 0, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(0.5, 0) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MC, MD);
	}
	else if(angle > 2*PI - PI/4 || angle <= + PI/4)
	{
		FVector E = CellPos + FVector(1, 0.5, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyVertexModifier(Cell, E), UVRoot + FVector2D(1, 0.5) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MD, MA);
	} else if (angle > PI/4 && angle < PI - PI/4)
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

FOpenLandGridRendererChangedInfo FOpenLandGridRenderer::ApplyCellChanges(FOpenLandGridChangedCells ChangedCells)
{
	FOpenLandGridRendererChangedInfo ChangedInfo;

	for (int32 Index = 0; Index < ChangedCells.CellsToRemove.Num(); Index++)
	{
		const FOpenLandGridCell RemovingCellPos = ChangedCells.CellsToRemove[Index];
		const FOpenLandGridCell AddingCellPos = ChangedCells.CellsToAdd[Index];
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

		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
	}

	for (const FOpenLandGridCell ModifiedCell: ChangedCells.ModifiedCells)
	{
		TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(ModifiedCell);
		const FOpenLandGridRendererCell RenderCell = Cells[GetTypeHash(ModifiedCell)];
		const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);

		MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
		MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
		MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));

		const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
		MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
		MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
		MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));

		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
	}

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return ChangedInfo;
}

FVector FOpenLandGridRenderer::ApplyVertexModifier(FOpenLandGridCell Cell, FVector Source)
{
	//return Source;	
	// if (Cell.bHoleEdge)
	// {
	// 	return Source + FVector(0, 0, 200);
	// } else
	// {
	// 	return  Source;
	// }
	const float Distance = FVector::Distance(Source, FVector(0, 0, 0));
	constexpr float Divider = 2000.0f;
	constexpr float Height = 300.0f;
	
	const float SinInput = (FMath::CeilToInt( Distance/Divider* 100) % 314 * 2) / 100.0f;
	
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
	
	for(const FOpenLandGridCell Cell: Grid->GetAllCells())
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

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return  MeshInfo;
}

FOpenLandGridRendererChangedInfo FOpenLandGridRenderer::ReCenter(FVector NewCenter)
{
	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());

	return ApplyCellChanges(ChangedCells);
}

FOpenLandGridRendererChangedInfo FOpenLandGridRenderer::ReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell)
{
	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter, NewHoleRootCell);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());

	return ApplyCellChanges(ChangedCells);
}

FOpenLandGridRendererChangedInfo FOpenLandGridRenderer::ChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell)
{
	const FOpenLandGridChangedCells ChangedCells = Grid->ChangeHoleRootCell(NewHoleRootCell);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());

	return ApplyCellChanges(ChangedCells);
}
