// Fill out your copyright notice in the Description page of Project Settings.


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

FSimpleMeshInfoPtr UOpenLandMeshPolygonMeshProxy::BuildMesh(UObject* WorldContext, int SubDivisions, float CuspAngle) const
{
	return PolygonMesh->BuildMesh(WorldContext, SubDivisions, CuspAngle);
}

void UOpenLandMeshPolygonMeshProxy::BuildMeshAsync(UObject* WorldContext, int SubDivisions, float CuspAngle,
	std::function<void(FSimpleMeshInfoPtr)> Callback) const
{
	return PolygonMesh->BuildMeshAsync(WorldContext, SubDivisions, CuspAngle, Callback);
}

void UOpenLandMeshPolygonMeshProxy::ModifyVertices(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target, float RealTimeSeconds,
                                              float CuspAngle) const
{
	return PolygonMesh->ModifyVertices(WorldContext, Original, Target, RealTimeSeconds, CuspAngle);
}

bool UOpenLandMeshPolygonMeshProxy::ModifyVerticesAsync(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target,
	float RealTimeSeconds, float CuspAngle)
{
	return PolygonMesh->ModifyVerticesAsync(WorldContext, Original, Target, RealTimeSeconds, CuspAngle);
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddTriFace(const FVector A, const FVector B, const FVector C)
{
	PolygonMesh->AddTriFace(A, B, C);
	return this;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D)
{
	PolygonMesh->AddQuadFace(A, B, C, D);
	return this;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::Transform(const FTransform Tranformer)
{
	PolygonMesh->Transform(Tranformer);
	return this;
}

void UOpenLandMeshPolygonMeshProxy::RegisterVertexModifier(function<FVertexModifierResult(FVertexModifierPayload)> Callback)
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
	
	for (int32 Index=0; Index<Vertices.Num(); Index++)
	{
		Vertices[Index] *= 100;
	}

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
	
	for (int32 Index=0; Index<Vertices.Num(); Index++)
	{
		Vertices[Index] *= 100;
	}

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
	
	for (int32 Index=0; Index<Vertices.Num(); Index++)
	{
		Vertices[Index] *= 100;
	}

	P->AddTriFace(Vertices[0], Vertices[1], Vertices[4]);
	P->AddTriFace(Vertices[1], Vertices[2], Vertices[4]);
	P->AddTriFace(Vertices[2], Vertices[3], Vertices[4]);
	P->AddTriFace(Vertices[3], Vertices[0], Vertices[4]);
	
	P->AddQuadFace(Vertices[0], Vertices[3], Vertices[2], Vertices[1]); // bottom

	return P;
}

