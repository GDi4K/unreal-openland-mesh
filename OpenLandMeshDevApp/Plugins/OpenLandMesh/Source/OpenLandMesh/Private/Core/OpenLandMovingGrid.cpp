#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

void FOpenLandMovingGrid::BuildMesh(float CuspAngle) const
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

void FOpenLandMovingGrid::Build()
{
	MeshInfo = FOpenLandMeshInfo::New();

	// Build Geometry
	TArray<FVector> _Vertices = {
		FVector{-0.5, -0.5, 0.0},
		FVector{-0.5, 0.5, 0.0},
		FVector{0.5, 0.5, 0.0},
		FVector{0.5, -0.5, 0.0}
	};

	for (int32 Index = 0; Index < _Vertices.Num(); Index++)
	{
		_Vertices[Index] *= 1000;
	}
	
	const TOpenLandArray<FOpenLandMeshVertex> InputVertices = {
		FOpenLandMeshVertex(_Vertices[0], FVector2D(0, 1)),
		FOpenLandMeshVertex(_Vertices[1], FVector2D(1, 1)),
		FOpenLandMeshVertex(_Vertices[2], FVector2D(1, 0)),

		FOpenLandMeshVertex(_Vertices[0], FVector2D(0, 1)),
		FOpenLandMeshVertex(_Vertices[2], FVector2D(1, 0)),
		FOpenLandMeshVertex(_Vertices[3], FVector2D(0, 0))
	};

	FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), InputVertices);
	MeshInfo = FOpenLandPolygonMesh::SubDivide(*MeshInfo.Get(), 6).Clone();
	BuildMesh(60.0);

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
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter)
{
	if (MeshInfo->IsLocked())
	{
		return;
	}
	
	NewCenter.Z = 0;
	const FVector Displacement = NewCenter - CenterPosition;
	for (size_t VertexId = 0; VertexId<MeshInfo->Vertices.Length(); VertexId++)
	{
		FOpenLandMeshVertex& T0 = MeshInfo->Vertices.GetRef(VertexId);
		T0.Position += Displacement;
	}

	BuildMesh(60);

	CenterPosition = NewCenter;
	MeshComponent->UpdateMeshSection(MeshSectionIndex, {0, -1});
}
