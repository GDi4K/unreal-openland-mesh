#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::RenderGrid()
{
	const float UVCellWidth = RootGrid->GetCellWidth() / CurrentBuildOptions.UnitUVLenght;
	MeshInfo = FOpenLandMeshInfo::New();
	
	for(FVector2D Cell: RootGrid->GetAllCells())
	{
		FVector CellPos = FVector(Cell.X, Cell.Y, 0) * RootGrid->GetCellWidth();
		
		FVector A = CellPos + FVector(0, 0, 0) * RootGrid->GetCellWidth();
		FVector B = CellPos + FVector(0, 1, 0) * RootGrid->GetCellWidth();
		FVector C = CellPos + FVector(1, 1, 0) * RootGrid->GetCellWidth();
		FVector D = CellPos + FVector(1, 0, 0) * RootGrid->GetCellWidth();

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

	MeshInfo->BoundingBox = RootGrid->GetBoundingBox();

	// Render It
	if (MeshSectionIndex < 0)
	{
		MeshSectionIndex = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(MeshSectionIndex, MeshInfo);
	} else
	{
		MeshComponent->ReplaceMeshSection(MeshSectionIndex, MeshInfo);
	}

	MeshComponent->SetupCollisions(true);
	MeshComponent->InvalidateRendering();
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;
	RootGrid = MakeShared<FOpenLandGrid>();

	RootGrid->Build({0, 0}, { 50, 50 }, CurrentBuildOptions.CellWidth,  CurrentBuildOptions.CellWidth*2);
	RootGrid->ReCenter({0, 0, 0});

	RenderGrid();
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter)
{
	if (MeshSectionIndex < 0)
	{
		return;
	}
	
	if (MeshInfo->IsLocked())
	{
		return;
	}

	const FOpenLandGridChangedCells Result = RootGrid->ReCenter(NewCenter);
	UE_LOG(LogTemp, Warning, TEXT("Result: Added: %d, Removed: %d"), Result.CellsToAdd.Num(), Result.CellsToRemove.Num())

	if (Result.CellsToAdd.Num() == 0 && Result.CellsToRemove.Num() == 0)
	{
		return;
	}

	RenderGrid();
}
