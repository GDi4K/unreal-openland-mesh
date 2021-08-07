// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once
#include <functional>


#include "Compute/GpuComputeVertex.h"
#include "Types/OpenLandArray.h"
#include "Types/OpenLandMeshInfo.h"
#include "OpenLandPolygonMesh.generated.h"

USTRUCT(BlueprintType)
struct FVertexModifierPayload
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	FVector Position = {0, 0, 0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	FVector PlaneNormal = {0, 0, 0};;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	FVector2D UV0 = {0, 0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	float TimeInSeconds = 0;
};

USTRUCT(BlueprintType)
struct FVertexModifierResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	FVector Position = {0, 0, 0};
};

struct FOpenLandPolygonMeshBuildOptions
{
	int SubDivisions = 0;
	float CuspAngle = 0;
};

struct FOpenLandPolygonMeshBuildResult
{
	FSimpleMeshInfoPtr Original = nullptr;
	FSimpleMeshInfoPtr Target = nullptr;
	int32 TextureWidth = 0;
	TArray<FGpuComputeVertexDataTextureItem> DataTextures;
};

class OPENLANDMESH_API FOpenLandPolygonMesh
{
	// For the delete delete schedular
	static bool bIsDeleteSchedulerRunning;
	static TArray<FOpenLandPolygonMesh*> PolygonMeshesToDelete;

	FOpenLandMeshInfo SourceMeshInfo;
	function<FVertexModifierResult(FVertexModifierPayload)> VertexModifier = nullptr;
	TArray<bool> AsyncCompletions;
	FTransform SourceTransformer;

	TSharedPtr<FGpuComputeVertex> GpuComputeEngine = nullptr;
	FComputeMaterial GpuVertexModifier;

	static void ApplyNormalSmoothing(FOpenLandMeshInfo* MeshInfo, float CuspAngle);
	static FOpenLandMeshInfo SubDivide(FOpenLandMeshInfo SourceMeshInfo, int Depth);
	static void AddFace(FOpenLandMeshInfo* MeshInfo, TOpenLandArray<FOpenLandMeshVertex> Vertices);
	static void BuildFaceTangents(FOpenLandMeshVertex& T0, FOpenLandMeshVertex& T1, FOpenLandMeshVertex& T2);
	static void ApplyVertexModifiers(function<FVertexModifierResult(FVertexModifierPayload)> VertexModifier, FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target, int RangeStart, int RangeEnd,
	                          float RealTimeSeconds);
	static void BuildDataTextures(FOpenLandPolygonMeshBuildResult* Result);
	void EnsureGpuComputeEngine(UObject* WorldContext, FOpenLandPolygonMeshBuildResult MeshBuildResult);
	void ApplyGpuVertexModifers(UObject* WorldContext, FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target,
	                            TArray<FComputeMaterialParameter> AdditionalMaterialParameters);
	static TArray<FComputeMaterialParameter> MakeParameters(float Time);

public:
	~FOpenLandPolygonMesh();
	void RegisterVertexModifier(std::function<FVertexModifierResult(FVertexModifierPayload)> Callback);
	FGpuComputeMaterialStatus RegisterGpuVertexModifier(FComputeMaterial ComputeMaterial);
	
	FOpenLandPolygonMeshBuildResult BuildMesh(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options);
	void BuildMeshAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options,
	                    std::function<void(FOpenLandPolygonMeshBuildResult)> Callback);
	
	void ModifyVertices(UObject* WorldContext, FOpenLandPolygonMeshBuildResult MeshBuildResult,
	                    float RealTimeSeconds, float CuspAngle = 0);
	// Here we do vertex modifications outside of the game thread
	// The return boolean value indicates whether we should render the Target MeshInfo or not
	// Note: It's very important to pass the same Target all the time because the return value is related to something happens earlier.
	bool ModifyVerticesAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildResult MeshBuildResult,
	                         float RealTimeSeconds, float CuspAngle = 0);
	
	void AddTriFace(const FVector A, const FVector B, const FVector C);
	void AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D);
	void Transform(FTransform Transformer);
	bool IsThereAnyAsyncTask() const;

	// Methods for delete schedular
	static void RunDeleteScheduler();
	static void DeletePolygonMesh(FOpenLandPolygonMesh* PolygonMesh);
};
