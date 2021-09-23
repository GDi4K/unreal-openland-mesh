#include "Utils/OpenLandGridRenderer.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandGridRenderer::FOpenLandGridRenderer()
{
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildCell(FVector2D Cell) const
{
	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	
	FVector CellPos = FVector(Cell.X, Cell.Y, 0) * Grid->GetCellWidth();
		
	FVector A = CellPos + FVector(0, 0, 0) * Grid->GetCellWidth();
	FVector B = CellPos + FVector(0, 1, 0) * Grid->GetCellWidth();
	FVector C = CellPos + FVector(1, 1, 0) * Grid->GetCellWidth();
	FVector D = CellPos + FVector(1, 0, 0) * Grid->GetCellWidth();

	FVector2D UVRoot = Cell * UVCellWidth;
			
	FOpenLandMeshVertex MA = {A, UVRoot + FVector2D(0, 0) * UVCellWidth};
	FOpenLandMeshVertex MB = {B, UVRoot + FVector2D(0, 1) * UVCellWidth};
	FOpenLandMeshVertex MC = {C, UVRoot + FVector2D(1, 1) * UVCellWidth};
	FOpenLandMeshVertex MD = {D, UVRoot + FVector2D(1, 0) * UVCellWidth};

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

FOpenLandMeshInfoPtr FOpenLandGridRenderer::Initialize(FOpenLandGridPtr SourceGrid)
{
	check(bInitialized == false);
	bInitialized = true;
	Grid = SourceGrid;
	MeshInfo = FOpenLandMeshInfo::New();
	Cells = {};
	
	for(const FVector2D Cell: Grid->GetAllCells())
	{
		const TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(Cell);
		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), CellVertices);
		
		const int32 TotalTriangles = MeshInfo->Triangles.Length();
		Cells.Add(Cell.ToString(), {
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
	FOpenLandGridChangedCells Result = Grid->ReCenter(NewCenter);
	check(Result.CellsToAdd.Num() == Result.CellsToRemove.Num());

	FOpenLandGridRendererChangedInfo ChangedInfo;

	for (int32 Index = 0; Index < Result.CellsToRemove.Num(); Index++)
	{
		const FVector2D RemovingCellPos = Result.CellsToRemove[Index];
		const FVector2D AddingCellPos = Result.CellsToAdd[Index];
		TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(AddingCellPos);

		const FOpenLandGridRendererCell RenderCell = Cells[RemovingCellPos.ToString()];
		const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
		
		MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
		MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
		MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));

		const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
		MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
		MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
		MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));

		Cells.Remove(RemovingCellPos.ToString());
		Cells.Add(AddingCellPos.ToString(), {
			AddingCellPos,
			RenderCell.IndexT0,
			RenderCell.IndexT1
		});

		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
		ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
	}

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return ChangedInfo;
}
