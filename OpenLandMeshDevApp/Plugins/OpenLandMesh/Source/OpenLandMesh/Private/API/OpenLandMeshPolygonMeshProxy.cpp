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

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddTriFace(const FOpenLandMeshVertex A,
	const FOpenLandMeshVertex B, const FOpenLandMeshVertex C)
{
	PolygonMesh->AddTriFace(A, B, C);
	return this;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::AddQuadFace(const FOpenLandMeshVertex A,
	const FOpenLandMeshVertex B, const FOpenLandMeshVertex C, const FOpenLandMeshVertex D)
{
	PolygonMesh->AddQuadFace(A, B, C, D);
	return this;
}

FVector2D UOpenLandMeshPolygonMeshProxy::RegularPolygonPositionToUV(FVector Position, float Radius)
{
	const FVector2D Position2D = {Position[0], Position[1]};
	return (Position2D / (Radius * 2)) + FVector2D(0.5, 0.5);
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeRegularPolygonMesh(int32 NoOfSides, float Radius)
{
	if (NoOfSides < 3)
	{
		NoOfSides = 3;
	}

	const FVector ZVector = {0.0f, 0.0f, 1.0f};
	const FVector Center = {0.0f, 0.0f, 0.0f};
	const float RotateAngle = 360.0f / NoOfSides;
	TArray<FVector> EdgePoints;

	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		FVector BasePoint = {Radius, 0.0f, 0.0f};
		EdgePoints.Push(BasePoint.RotateAngleAxis(RotateAngle*PointIndex, ZVector));
	}
	
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		if (PointIndex == NoOfSides - 1)
		{
			P->AddTriFace(
				FOpenLandMeshVertex(Center, RegularPolygonPositionToUV(Center, Radius)),
				FOpenLandMeshVertex(EdgePoints[0], RegularPolygonPositionToUV(EdgePoints[0], Radius)),
				FOpenLandMeshVertex(EdgePoints[PointIndex], RegularPolygonPositionToUV(EdgePoints[PointIndex], Radius))
			);
		} else
		{
			P->AddTriFace(
				FOpenLandMeshVertex(Center, RegularPolygonPositionToUV(Center, Radius)),
				FOpenLandMeshVertex(EdgePoints[PointIndex+1], RegularPolygonPositionToUV(EdgePoints[PointIndex+1], Radius)),
				FOpenLandMeshVertex(EdgePoints[PointIndex], RegularPolygonPositionToUV(EdgePoints[PointIndex], Radius))
			);
		}
	}
	
	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeRegularPrismMesh(int32 NoOfSides, int32 NoOfRows, float Height, float RadiusTop, float RadiusBottom, bool bAddTop, bool bAddBottom)
{
	if (NoOfSides < 3)
	{
		NoOfSides = 3;
	}

	const FVector ZVector = {0.0f, 0.0f, 1.0f};
	const FVector CenterTop = {0.0f, 0.0f, Height};
	const FVector CenterBottom = {0.0f, 0.0f, 0.0f};
	const float RotateAngle = 360.0f / NoOfSides;

	TArray<FVector> PointsTop;
	TArray<FVector> PointsBottom;

	// Make points
	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		PointsTop.Push(FVector(RadiusTop, 0.0f, Height).RotateAngleAxis(RotateAngle*PointIndex, ZVector));
		PointsBottom.Push(FVector(RadiusBottom, 0.0f, 0.0f).RotateAngleAxis(RotateAngle*PointIndex, ZVector));
	}
	
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	// Add top bottom
	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		if (PointIndex == NoOfSides - 1)
		{
			if (bAddTop)
			{
				P->AddTriFace(
					FOpenLandMeshVertex(CenterTop, RegularPolygonPositionToUV(CenterTop, RadiusTop)),
					FOpenLandMeshVertex(PointsTop[0], RegularPolygonPositionToUV(PointsTop[0], RadiusTop)),
					FOpenLandMeshVertex(PointsTop[PointIndex], RegularPolygonPositionToUV(PointsTop[PointIndex], RadiusTop))
				);
			}

			if (bAddBottom)
			{
				P->AddTriFace(
					FOpenLandMeshVertex(CenterBottom, RegularPolygonPositionToUV(CenterBottom, RadiusBottom)),
					FOpenLandMeshVertex(PointsBottom[PointIndex], RegularPolygonPositionToUV(PointsBottom[PointIndex], RadiusBottom)),
					FOpenLandMeshVertex(PointsBottom[0], RegularPolygonPositionToUV(PointsBottom[0], RadiusBottom))
				);
			}
		} else
		{
			if (bAddTop)
			{
				P->AddTriFace(
					FOpenLandMeshVertex(CenterTop, RegularPolygonPositionToUV(CenterTop, RadiusTop)),
					FOpenLandMeshVertex(PointsTop[PointIndex+1], RegularPolygonPositionToUV(PointsTop[PointIndex+1], RadiusTop)),
					FOpenLandMeshVertex(PointsTop[PointIndex], RegularPolygonPositionToUV(PointsTop[PointIndex], RadiusTop))
				);
			}

			if (bAddBottom)
			{
				P->AddTriFace(
					FOpenLandMeshVertex(CenterBottom, RegularPolygonPositionToUV(CenterBottom, RadiusBottom)),
					FOpenLandMeshVertex(PointsBottom[PointIndex], RegularPolygonPositionToUV(PointsBottom[PointIndex], RadiusBottom)),
					FOpenLandMeshVertex(PointsBottom[PointIndex+1], RegularPolygonPositionToUV(PointsBottom[PointIndex+1], RadiusBottom))
				);
			}
		}
	}

	const float UVHeight = Height / 100.0f;
	const float UVWidth = 1.0 / NoOfSides;
	
	
	// Add sides
	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		int32 StartIndex = PointIndex;
		int32 EndIndex = PointIndex == NoOfSides - 1 ? 0 : PointIndex + 1;

		float RowHeight = FVector::Dist(PointsBottom[StartIndex], PointsTop[StartIndex]) / NoOfRows;
		FVector RowDirectionStart = (PointsTop[StartIndex] - PointsBottom[StartIndex]).GetSafeNormal();
		FVector RowDirectionEnd = (PointsTop[EndIndex] - PointsBottom[EndIndex]).GetSafeNormal();

		// Add rows
		for (int32 RowIndex=0; RowIndex<NoOfRows; RowIndex++)
		{
			const FVector PosA = PointsBottom[EndIndex] + RowDirectionEnd * RowHeight * (RowIndex);
			const FVector PosB = PointsBottom[StartIndex] + RowDirectionStart * RowHeight * (RowIndex);
			const FVector PosC = PointsBottom[StartIndex] + RowDirectionStart * RowHeight * (RowIndex + 1);
			const FVector PosD = PointsBottom[EndIndex] + RowDirectionEnd * RowHeight * (RowIndex + 1);
			
			P->AddQuadFace(
				FOpenLandMeshVertex(PosA, FVector2D(1, 0)),
				FOpenLandMeshVertex(PosB, FVector2D(0, 0)),
				FOpenLandMeshVertex(PosC, FVector2D(0, 1)),
				FOpenLandMeshVertex(PosD, FVector2D(1, 1))
			);
		}

	}
	
	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeGridMesh(float CellWidth, int32 Rows, int32 Cols)
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();
	
	for(int32 CellX=0; CellX<Rows; CellX++)
	{
		for(int32 CellY=0; CellY<Cols; CellY++)
		{
			const FVector A(CellX * CellWidth, CellY * CellWidth, 0);
			const FVector B(CellX * CellWidth, (CellY + 1) * CellWidth, 0);
			const FVector C((CellX + 1) * CellWidth, (CellY + 1) * CellWidth, 0);
			const FVector D((CellX + 1) * CellWidth, CellY * CellWidth, 0);

			const float UVCellWidth = CellWidth/100.0;
			
			P->AddQuadFace(
				FOpenLandMeshVertex(A, FVector2D(CellX * UVCellWidth, CellY * UVCellWidth)),
				FOpenLandMeshVertex(B, FVector2D(CellX * UVCellWidth, (CellY + 1) * UVCellWidth)),
				FOpenLandMeshVertex(C, FVector2D((CellX + 1) * UVCellWidth, (CellY + 1) * UVCellWidth)),
				FOpenLandMeshVertex(D, FVector2D((CellX + 1) * UVCellWidth, CellY * UVCellWidth))
			);
		}
	}

	return P;
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

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeCubeMesh(bool bAddTop, bool bAddBottom, bool bInvert)
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
	{
		Vertices[Index] *= 100 * (bInvert? -1 : 1);
	}

	if (bAddTop)
	{
		P->AddQuadFace(Vertices[4], Vertices[5], Vertices[6], Vertices[7]);
	}

	P->AddQuadFace(Vertices[0], Vertices[1], Vertices[5], Vertices[4]);
	P->AddQuadFace(Vertices[1], Vertices[2], Vertices[6], Vertices[5]);
	P->AddQuadFace(Vertices[2], Vertices[3], Vertices[7], Vertices[6]);
	P->AddQuadFace(Vertices[3], Vertices[0], Vertices[4], Vertices[7]);

	if (bAddBottom)
	{
		P->AddQuadFace(Vertices[0], Vertices[3], Vertices[2], Vertices[1]);
	}

	return P;
}

UOpenLandMeshPolygonMeshProxy* UOpenLandMeshPolygonMeshProxy::MakeRegularPyramidMesh(int32 NoOfSides, float Radius, float Height, bool bAddBottom)
{
	if (NoOfSides < 3)
	{
		NoOfSides = 3;
	}

	const FVector ZVector = {0.0f, 0.0f, 1.0f};
	const FVector Center = {0.0f, 0.0f, 0.0f};
	const FVector Top = {0.0f, 0.0f, Height};
	const float RotateAngle = 360.0f / NoOfSides;
	TArray<FVector> EdgePoints;

	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		FVector BasePoint = {Radius, 0.0f, 0.0f};
		EdgePoints.Push(BasePoint.RotateAngleAxis(RotateAngle*PointIndex, ZVector));
	}
	
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();

	for (int32 PointIndex=0; PointIndex<NoOfSides; PointIndex++)
	{
		if (PointIndex == NoOfSides - 1)
		{
			// Add bottom face
			if (bAddBottom)
			{
				P->AddTriFace(
					FOpenLandMeshVertex(Center, RegularPolygonPositionToUV(Center, Radius)),
					FOpenLandMeshVertex(EdgePoints[PointIndex], RegularPolygonPositionToUV(EdgePoints[PointIndex], Radius)),
					FOpenLandMeshVertex(EdgePoints[0], RegularPolygonPositionToUV(EdgePoints[0], Radius))
				);
			}

			// Add side face
			P->AddTriFace(EdgePoints[0], EdgePoints[PointIndex], Top);
		} else
		{
			if (bAddBottom)
			{
				// Add bottom face
				P->AddTriFace(
					FOpenLandMeshVertex(Center, RegularPolygonPositionToUV(Center, Radius)),
					FOpenLandMeshVertex(EdgePoints[PointIndex], RegularPolygonPositionToUV(EdgePoints[PointIndex], Radius)),
					FOpenLandMeshVertex(EdgePoints[PointIndex + 1], RegularPolygonPositionToUV(EdgePoints[PointIndex + 1], Radius))
				);
			}

			// Add side face
			P->AddTriFace(EdgePoints[PointIndex + 1], EdgePoints[PointIndex], Top);
		}
	}
	
	return P;
}
