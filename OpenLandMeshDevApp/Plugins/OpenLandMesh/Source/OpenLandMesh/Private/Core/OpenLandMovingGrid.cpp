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

FVector2D FOpenLandMovingGrid::PositionToUV(FVector Position, int32 VertexPosition)
{
	float U = FMath::Frac(Position.X / 100.0f);
	float V = FMath::Frac(Position.Y / 100.0f);

	constexpr bool XStartByPosition[] = {true, true, false, false};
	constexpr bool YStartByPosition[] = {true, false, false, true};

	if (!XStartByPosition[VertexPosition] && U == 0.0)
	{
		U = 1.0f;
	}

	if (!YStartByPosition[VertexPosition] && V == 0.0)
	{
		V = 1.0f;
	}

	return {U, V};
}

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;
	MeshInfo = FOpenLandMeshInfo::New();

	const float CellWidth = CurrentBuildOptions.CellWidth;
	const int32 CellCount = CurrentBuildOptions.CellCount;

	const float UnitUVLength = CurrentBuildOptions.UnitUVLenght;
	FVector PosRoot = CenterPosition - FVector(CellWidth * CellCount /2, CellWidth * CellCount /2, 0);
	FVector PosCell = {CellWidth, CellWidth, 0};
	FVector2D UVRoot = { PosRoot.X/UnitUVLength, PosRoot.Y/UnitUVLength };
	FVector2D UVCell = {CellWidth / UnitUVLength, CellWidth / UnitUVLength};

	UVRoot.X -= FMath::Floor(UVRoot.X / CurrentBuildOptions.MaxUVs) * CurrentBuildOptions.MaxUVs;
	UVRoot.Y -= FMath::Floor(UVRoot.Y / CurrentBuildOptions.MaxUVs) * CurrentBuildOptions.MaxUVs;

	UE_LOG(LogTemp, Warning, TEXT("UVRoot: %s"), *UVRoot.ToString())

	for (int32 CellX=0; CellX<CellCount; CellX++)
	{
		for (int32 CellY=0; CellY<CellCount; CellY++)
		{
			FVector A = PosRoot + PosCell * FVector(CellX, CellY, 0);
			FVector B = PosRoot + PosCell * FVector(CellX, CellY + 1, 0);
			FVector C = PosRoot + PosCell * FVector(CellX + 1, CellY + 1, 0);
			FVector D = PosRoot + PosCell * FVector(CellX + 1, CellY, 0);
			
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
	if (MeshInfo->IsLocked())
	{
		return;
	}
	
	NewCenter.Z = 0;
	CenterPosition = NewCenter;

	Build(CurrentBuildOptions);
}
