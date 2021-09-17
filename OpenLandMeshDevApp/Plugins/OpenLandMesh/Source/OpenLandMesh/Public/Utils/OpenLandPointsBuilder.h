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
	static TArray<FOpenLandMeshPoint> BuildPointsModifiedPoisson2D(FOpenLandMeshInfoPtr MeshInfo, float Density, float MinRadius);
	static TArray<FOpenLandMeshPoint> BuildPointsUseOrigin(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsPickVertices(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsPickCentroids(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToZAxis(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToXAxis(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsMoveToYAxis(FOpenLandMeshInfoPtr MeshInfo);
	static TArray<FOpenLandMeshPoint> BuildPointsOnTriangle(FOpenLandMeshInfoPtr MeshInfo, int32 TriangleIndex, int32 Count, float MinRadius);
};
