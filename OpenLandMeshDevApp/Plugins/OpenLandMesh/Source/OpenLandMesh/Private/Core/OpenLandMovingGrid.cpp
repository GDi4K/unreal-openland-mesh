#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

void FOpenLandMovingGrid::BuildFaces(float CuspAngle) const
{
	MeshInfo->BoundingBox.Init();
	for(size_t Index=0; Index < MeshInfo->Triangles.Length(); Index++)
	{
		const FOpenLandMeshTriangle OTriangle = MeshInfo->Triangles.Get(Index);
		FOpenLandMeshVertex& T0 = MeshInfo->Vertices.GetRef(OTriangle.T0);
		FOpenLandMeshVertex& T1 = MeshInfo->Vertices.GetRef(OTriangle.T1);
		FOpenLandMeshVertex& T2 = MeshInfo->Vertices.GetRef(OTriangle.T2);

		FOpenLandPolygonMesh::BuildFaceTangents(T0, T1, T2);

		// Build Bounding Box
		MeshInfo->BoundingBox += T0.Position;
		MeshInfo->BoundingBox += T1.Position;
		MeshInfo->BoundingBox += T2.Position;
	}

	FOpenLandPolygonMesh::ApplyNormalSmoothing(MeshInfo.Get(), CuspAngle);
}

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::BuildGrid()
{
	const float CellWidth = CurrentBuildOptions.CellWidth;
	const int32 CellCount = CurrentBuildOptions.CellCount;

	FVector PosRoot = FVector(RootCell.X * CellWidth, RootCell.Y * CellWidth, 0);
	PosRoot -= FVector(CellWidth * CellCount /2, CellWidth * CellCount /2, 0);
	FVector PosCell = {CellWidth, CellWidth, 0};
	
	const float UVCellWidth = CellWidth / CurrentBuildOptions.UnitUVLenght;
	FVector2D UVCell = {UVCellWidth, UVCellWidth};
	FVector2D UVRoot = UVCell * RootCell;

	// float DivisionX = UVRoot.X / CurrentBuildOptions.MaxUVs;
	// DivisionX = DivisionX - FMath::Frac(DivisionX);
	// float DivisionY = UVRoot.Y / CurrentBuildOptions.MaxUVs;
	// DivisionY = DivisionY - FMath::Frac(DivisionY);
	//
	// UVRoot.X -= DivisionX * CurrentBuildOptions.MaxUVs;
	// UVRoot.Y -= DivisionY * CurrentBuildOptions.MaxUVs;

	UE_LOG(LogTemp, Warning, TEXT("UVCellWidth: %f, RootCell: %s, UVRoot: %s"), UVCellWidth, *RootCell.ToString(), *UVRoot.ToString())

	for (int32 CellX=0; CellX<CellCount; CellX++)
	{
		for (int32 CellY=0; CellY<CellCount; CellY++)
		{
			FVector A = PosRoot + PosCell * FVector(CellX, CellY, 0);
			FVector B = PosRoot + PosCell * FVector(CellX, CellY + 1, 0);
			FVector C = PosRoot + PosCell * FVector(CellX + 1, CellY + 1, 0);
			FVector D = PosRoot + PosCell * FVector(CellX + 1, CellY, 0);

			if (CellX == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Final Cell: %s"), *(UVRoot + UVCell * FVector2D(CellX, CellY)).ToString())
			}
			
			FOpenLandMeshVertex MA = {A, UVRoot + UVCell * FVector2D(CellX, CellY)};
			FOpenLandMeshVertex MB = {B, UVRoot + UVCell * FVector2D(CellX, CellY + 1)};
			FOpenLandMeshVertex MC = {C, UVRoot + UVCell * FVector2D(CellX + 1, CellY + 1)};
			FOpenLandMeshVertex MD = {D, UVRoot + UVCell * FVector2D(CellX + 1, CellY)};

			const TOpenLandArray<FOpenLandMeshVertex> InputVertices = {
				MA,
				MB,
				MC,

				MA,
				MC,
				MD
			};

			FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), InputVertices);
		}
	}
	
	BuildFaces(CurrentBuildOptions.CuspAngle);
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;
	MeshInfo = FOpenLandMeshInfo::New();

	BuildGrid();

	// Render It
	if (MeshSectionIndex < 0)
	{
		MeshSectionIndex = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(MeshSectionIndex, MeshInfo);
	} else
	{
		MeshComponent->ReplaceMeshSection(MeshSectionIndex, MeshInfo);
	}

	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();
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

	float RootCellX = FMath::Floor(NewCenter.X / CurrentBuildOptions.CellWidth * 100)/100;
	float RootCellY = FMath::Floor(NewCenter.Y / CurrentBuildOptions.CellWidth * 100)/100;

	if (RootCell.X == RootCellX && RootCell.Y == RootCellY)
	{
		return;
	}

	RootCell = {RootCellX, RootCellY};

	Build(CurrentBuildOptions);
	MeshComponent->UpdateMeshSection(MeshSectionIndex, {0, -1});
}
