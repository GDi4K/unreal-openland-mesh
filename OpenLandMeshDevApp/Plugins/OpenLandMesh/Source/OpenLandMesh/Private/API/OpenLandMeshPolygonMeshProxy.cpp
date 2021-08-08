// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "API/OpenLandMeshPolygonMeshProxy.h"

UOpenLandMeshPolygonMeshProxy::UOpenLandMeshPolygonMeshProxy()
{
	PolygonMesh = new FOpenLandPolygonMesh();
}

UOpenLandMeshPolygonMeshProxy::~UOpenLandMeshPolygonMeshProxy()
{
	UE_LOG(LogTemp, Warning, TEXT("Deleting UOpenLandMeshPolygonMesh"))
	FOpenLandPolygonMesh::DeletePolygonMesh(PolygonMesh);
}

FOpenLandPolygonMeshBuildResult UOpenLandMeshPolygonMeshProxy::BuildMesh(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options) const
{
	return PolygonMesh->BuildMesh(WorldContext, Options);
}

void UOpenLandMeshPolygonMeshProxy::BuildMeshAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options,
                                                   std::function<void(FOpenLandPolygonMeshBuildResult)> Callback) const
{
	return PolygonMesh->BuildMeshAsync(WorldContext, Options, Callback);
}

void UOpenLandMeshPolygonMeshProxy::ModifyVertices(UObject* WorldContext, FOpenLandPolygonMeshBuildResult MeshBuildResult,
                                                   FOpenLandPolygonMeshModifyOptions Options) const
                                                   
{
	return PolygonMesh->ModifyVertices(WorldContext, MeshBuildResult, Options);
}

bool UOpenLandMeshPolygonMeshProxy::ModifyVerticesAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildResult MeshBuildResult,
                                                        FOpenLandPolygonMeshModifyOptions Options, function<void()> Callback) const
{
	return PolygonMesh->ModifyVerticesAsync(WorldContext, MeshBuildResult, Options, Callback);
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddTriFace(
	const FVector A, const FVector B, const FVector C)
{
	PolygonMesh->AddTriFace(A, B, C);
	return this;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddQuadFace(
	const FVector A, const FVector B, const FVector C, const FVector D)
{
	PolygonMesh->AddQuadFace(A, B, C, D);
	return this;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::Transform(const FTransform Tranformer)
{
	PolygonMesh->Transform(Tranformer);
	return this;
}

void UOpenLandMeshPolygonMeshProxy::RegisterVertexModifier(
	function<FVertexModifierResult(FVertexModifierPayload)> Callback)
{
	PolygonMesh->RegisterVertexModifier(Callback);
}

FGpuComputeMaterialStatus UOpenLandMeshPolygonMeshProxy::RegisterGpuVertexModifier(FComputeMaterial VertexModifier)
{
	return PolygonMesh->RegisterGpuVertexModifier(VertexModifier);
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeEmptyPolygonMesh()
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();
	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakePlaneMesh()
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	TArray<FVector> Vertices = {
		FVector{-0.5, 0, -0.5},
		FVector{0.5, 0, -0.5},
		FVector{0.5, 0, 0.5},
		FVector{-0.5, 0, 0.5}
	};

	for (int32 Index = 0; Index < Vertices.Num(); Index++)
		Vertices[Index] *= 100;

	P->AddQuadFace(Vertices[0], Vertices[1], Vertices[2], Vertices[3]);

	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeCubeMesh()
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	TArray<FVector> Vertices = {
		FVector{-0.5, 0.5, -0.5},
		FVector{0.5, 0.5, -0.5},
		FVector{0.5, -0.5, -0.5},
		FVector{-0.5, -0.5, -0.5},

		FVector{-0.5, 0.5, 0.5},
		FVector{0.5, 0.5, 0.5},
		FVector{0.5, -0.5, 0.5},
		FVector{-0.5, -0.5, 0.5},
	};

	for (int32 Index = 0; Index < Vertices.Num(); Index++)
		Vertices[Index] *= 100;

	P->AddQuadFace(Vertices[4], Vertices[5], Vertices[6], Vertices[7]);

	P->AddQuadFace(Vertices[0], Vertices[1], Vertices[5], Vertices[4]);
	P->AddQuadFace(Vertices[1], Vertices[2], Vertices[6], Vertices[5]);
	P->AddQuadFace(Vertices[2], Vertices[3], Vertices[7], Vertices[6]);
	P->AddQuadFace(Vertices[3], Vertices[0], Vertices[4], Vertices[7]);

	P->AddQuadFace(Vertices[0], Vertices[3], Vertices[2], Vertices[1]);

	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakePyramidMesh()
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	TArray<FVector> Vertices = {
		FVector{-0.5, 0.5, 0},
		FVector{0.5, 0.5, 0},
		FVector{0.5, -0.5, 0},
		FVector{-0.5, -0.5, 0},

		FVector{0, 0, 0.75},
	};

	for (int32 Index = 0; Index < Vertices.Num(); Index++)
		Vertices[Index] *= 100;

	P->AddTriFace(Vertices[0], Vertices[1], Vertices[4]);
	P->AddTriFace(Vertices[1], Vertices[2], Vertices[4]);
	P->AddTriFace(Vertices[2], Vertices[3], Vertices[4]);
	P->AddTriFace(Vertices[3], Vertices[0], Vertices[4]);

	P->AddQuadFace(Vertices[0], Vertices[3], Vertices[2], Vertices[1]); // bottom

	return P;
}
