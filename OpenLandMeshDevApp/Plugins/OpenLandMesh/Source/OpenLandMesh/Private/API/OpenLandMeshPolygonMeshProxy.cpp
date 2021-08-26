// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "API/OpenLandMeshPolygonMeshProxy.h"

TMap<FString, FOpenLandBuildMeshResultCacheInfo> UOpenLandMeshPolygonMeshProxy::CachedBuildMesh = {};

UOpenLandMeshPolygonMeshProxy::UOpenLandMeshPolygonMeshProxy()
{
	PolygonMesh = new FOpenLandPolygonMesh();
}

UOpenLandMeshPolygonMeshProxy::~UOpenLandMeshPolygonMeshProxy()
{
	UE_LOG(LogTemp, Warning, TEXT("Deleting UOpenLandMeshPolygonMesh"))
	FOpenLandPolygonMesh::DeletePolygonMesh(PolygonMesh);
}

FOpenLandPolygonMeshBuildResultPtr UOpenLandMeshPolygonMeshProxy::BuildMesh(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options, FString CacheKey) const
{
	if (CacheKey.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No Cache Key"))
		return PolygonMesh->BuildMesh(WorldContext, Options);
	}

	FOpenLandBuildMeshResultCacheInfo* CachedInfo = CachedBuildMesh.Find(CacheKey);
	if (CachedInfo != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cache Hit"))
		CachedInfo->LastCacheHitAt = FDateTime::Now();
		return CachedInfo->MeshBuildResult->ShallowClone();
	}

	UE_LOG(LogTemp, Warning, TEXT("Cache Create"))
	const FOpenLandPolygonMeshBuildResultPtr Result = PolygonMesh->BuildMesh(WorldContext, Options);
	Result->CacheKey = CacheKey;
	
	FOpenLandBuildMeshResultCacheInfo NewCacheInfo = {};
	NewCacheInfo.CacheKey = CacheKey;
	NewCacheInfo.MeshBuildResult = Result;
	NewCacheInfo.CachedAt = FDateTime::Now();
	NewCacheInfo.LastCacheHitAt = NewCacheInfo.CachedAt;
	
	CachedBuildMesh.Add(CacheKey, NewCacheInfo);

	return NewCacheInfo.MeshBuildResult->ShallowClone();
}

void UOpenLandMeshPolygonMeshProxy::BuildMeshAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options,
                                                   std::function<void(FOpenLandPolygonMeshBuildResultPtr)> Callback, FString CacheKey) const
{
	if (CacheKey.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildMeshAsync:No Cache Key"))
		return  PolygonMesh->BuildMeshAsync(WorldContext, Options, Callback);
	}

	FOpenLandBuildMeshResultCacheInfo* CachedInfo = CachedBuildMesh.Find(CacheKey);
	if (CachedInfo != nullptr)
	{
		if (CachedInfo->MeshBuildResult == nullptr || CachedInfo->MeshBuildResult->Target == nullptr)
		{
			
			CachedInfo->AsyncMeshBuildCallbacks.Push(Callback);
			return;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("BuildMeshAsync:Cache Hit, Have Target"))
		CachedInfo->LastCacheHitAt = FDateTime::Now();
		Callback(CachedInfo->MeshBuildResult->ShallowClone());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("BuildMeshAsync:Cache Create"));
	
	FOpenLandBuildMeshResultCacheInfo NewCacheInfo = {};
	NewCacheInfo.CacheKey = CacheKey;
	NewCacheInfo.CachedAt = FDateTime::Now();
	NewCacheInfo.LastCacheHitAt = NewCacheInfo.CachedAt;
	
	CachedBuildMesh.Add(CacheKey, NewCacheInfo);
	
	auto CallbackWrapper = [Callback, CacheKey](FOpenLandPolygonMeshBuildResultPtr Result)
	{
		Result->CacheKey = CacheKey;
		CachedBuildMesh[CacheKey].MeshBuildResult = Result->ShallowClone();
		Callback(Result);
	};
	
	return PolygonMesh->BuildMeshAsync(WorldContext, Options, CallbackWrapper);
}

void UOpenLandMeshPolygonMeshProxy::ModifyVertices(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
                                                   FOpenLandPolygonMeshModifyOptions Options) const
                                                   
{
	FOpenLandBuildMeshResultCacheInfo* CacheInfo = CachedBuildMesh.Find(MeshBuildResult->CacheKey);
	PolygonMesh->ModifyVertices(WorldContext, MeshBuildResult, Options);

	if (CacheInfo && CacheInfo->MeshBuildResult->Target == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ModifyVertices: Processing Callabcks"))
		CacheInfo->MeshBuildResult->Target = MeshBuildResult->Target->Clone();
		for (const auto Callback: CacheInfo->AsyncMeshBuildCallbacks)
		{
			Callback(CacheInfo->MeshBuildResult->ShallowClone());
		}
		CacheInfo->AsyncMeshBuildCallbacks = {};
	}
}

FOpenLandPolygonMeshModifyStatus UOpenLandMeshPolygonMeshProxy::StartModifyVertices(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
                                                        FOpenLandPolygonMeshModifyOptions Options) const
{
	return PolygonMesh->StartModifyVertices(WorldContext, MeshBuildResult, Options);
}

FOpenLandPolygonMeshModifyStatus UOpenLandMeshPolygonMeshProxy::CheckModifyVerticesStatus(FOpenLandPolygonMeshBuildResultPtr MeshBuildResult, float LastFrameTime) const
{
	const FOpenLandPolygonMeshModifyStatus Status = PolygonMesh->CheckModifyVerticesStatus(LastFrameTime);
	if (Status.bCompleted)
	{
		FOpenLandBuildMeshResultCacheInfo* CacheInfo = CachedBuildMesh.Find(MeshBuildResult->CacheKey);
		if (CacheInfo && CacheInfo->MeshBuildResult->Target == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckModifyVerticesStatus: Processing Callabcks"))
			CacheInfo->MeshBuildResult->Target = MeshBuildResult->Target->Clone();
			for (const auto Callback: CacheInfo->AsyncMeshBuildCallbacks)
			{
				Callback(CacheInfo->MeshBuildResult->ShallowClone());
			}
			CacheInfo->AsyncMeshBuildCallbacks = {};
		}
	}

	return Status;
}

int32 UOpenLandMeshPolygonMeshProxy::CalculateVerticesForSubdivision(int32 Subdivision) const
{
	return PolygonMesh->CalculateVerticesForSubdivision(Subdivision);
}

void UOpenLandMeshPolygonMeshProxy::ClearCache()
{
	CachedBuildMesh.Empty();
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
