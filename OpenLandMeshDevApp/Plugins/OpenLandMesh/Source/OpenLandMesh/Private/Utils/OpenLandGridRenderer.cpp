#include "Utils/OpenLandGridRenderer.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandGridRenderer::FOpenLandGridRenderer()
{
}

FOpenLandMeshInfoPtr FOpenLandGridRenderer::Initialize(FOpenLandGridPtr SourceGrid)
{
	check(bInitialized == false);
	bInitialized = true;
	Grid = SourceGrid;
	MeshInfo = FOpenLandMeshInfo::New();

	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	for(FVector2D Cell: Grid->GetAllCells())
	{
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
		MeshInfo->BoundingBox += T0_1.Position;
		MeshInfo->BoundingBox += T0_2.Position;
		MeshInfo->BoundingBox += T0_3.Position;

		FOpenLandMeshVertex T1_1 = MA;
		FOpenLandMeshVertex T1_2 = MC;
		FOpenLandMeshVertex T1_3 = MD;
		FOpenLandPolygonMesh::BuildFaceTangents(T1_1, T1_2, T1_3);
		MeshInfo->BoundingBox += T1_1.Position;
		MeshInfo->BoundingBox += T1_2.Position;
		MeshInfo->BoundingBox += T1_3.Position;

		const TOpenLandArray<FOpenLandMeshVertex> InputVertices = {
			T0_1,
			T0_2,
			T0_3,

			T1_1,
			T1_2,
			T1_3
		};

		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), InputVertices);
	}

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return  MeshInfo;
}

FOpenLandGridRendererChangedInfo FOpenLandGridRenderer::ReCenter(FVector NewCenter) const
{
	const FOpenLandGridChangedCells Result= Grid->ReCenter(NewCenter);
	check(Result.CellsToAdd.Num() == Result.CellsToRemove.Num());

	FOpenLandGridRendererChangedInfo ChangedInfo;
	ChangedInfo.bInvalidateRendering = Result.CellsToAdd.Num() > 0;
	
	return ChangedInfo;
}
