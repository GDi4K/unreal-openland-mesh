// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once
#include "Types/OpenLandMeshInfo.h"

struct FOpenLandMeshPoint
{
	FVector Position;
	FVector Normal;
	FVector TangentX;
};

class OPENLANDMESH_API FOpenLandPointsBuilder
{

public:
	static TArray<FOpenLandMeshPoint> BuildPointsModifiedPoisson2D(FSimpleMeshInfoPtr MeshInfo, float Density, float MinRadius);
	static TArray<FOpenLandMeshPoint> BuildPointsUseOrigin(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsPickVertices(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsPickCentroids(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToZAxis(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToXAxis(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToYAxis(FSimpleMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsOnTriangle(FSimpleMeshInfoPtr MeshInfo, int32 TriangleIndex, int32 Count, float MinRadius);
};
