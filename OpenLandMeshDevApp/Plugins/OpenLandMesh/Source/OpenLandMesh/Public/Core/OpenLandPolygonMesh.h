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
	// If this is non-zero, the result data texture
	// will contain a texture width as mentioned below
	int32 ForcedTextureWidth = 0;
};

struct FOpenLandPolygonMeshModifyOptions
{
	float RealTimeSeconds = 0;
	float CuspAngle = 0;
};

struct FOpenLandPolygonMeshBuildResult
{
	FSimpleMeshInfoPtr Original = nullptr;
	FSimpleMeshInfoPtr Target = nullptr;
	int32 TextureWidth = 0;
	TArray<FGpuComputeVertexDataTextureItem> DataTextures;
};

// Above TArray contains a runtime texture
// If we pass FOpenLandPolygonMeshBuildResult by value
// Unreal may need to create the array data many times
// In those cases, Unreal might try to delete the texture
// So, using a pointer will fix that issue
typedef TSharedPtr<FOpenLandPolygonMeshBuildResult> FOpenLandPolygonMeshBuildResultPtr;

class OPENLANDMESH_API FOpenLandPolygonMesh
{
	// For the delete delete schedular
	static bool bIsDeleteSchedulerRunning;
	static TArray<FOpenLandPolygonMesh*> PolygonMeshesToDelete;

	FOpenLandMeshInfo SourceMeshInfo;
	function<FVertexModifierResult(FVertexModifierPayload)> VertexModifier = nullptr;
	TArray<bool> AsyncCompletions;
	FTransform SourceTransformer;
	TArray<TSharedPtr<FGpuComputeVertex>> OldGpuComputeEngines;

	TSharedPtr<FGpuComputeVertex> GpuComputeEngine = nullptr;
	FComputeMaterial GpuVertexModifier;

	static void ApplyNormalSmoothing(FOpenLandMeshInfo* MeshInfo, float CuspAngle);
	static FOpenLandMeshInfo SubDivide(FOpenLandMeshInfo SourceMeshInfo, int Depth);
	static void AddFace(FOpenLandMeshInfo* MeshInfo, TOpenLandArray<FOpenLandMeshVertex> Vertices);
	static void BuildFaceTangents(FOpenLandMeshVertex& T0, FOpenLandMeshVertex& T1, FOpenLandMeshVertex& T2);
	static void ApplyVertexModifiers(function<FVertexModifierResult(FVertexModifierPayload)> VertexModifier, FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target, int RangeStart, int RangeEnd,
	                          float RealTimeSeconds);
	static void BuildDataTextures(FOpenLandPolygonMeshBuildResultPtr Result, int32 ForcedTextureWidth);
	void EnsureGpuComputeEngine(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult);
	void ApplyGpuVertexModifers(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
	                            TArray<FComputeMaterialParameter> AdditionalMaterialParameters);
	void ApplyGpuVertexModifersAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
								TArray<FComputeMaterialParameter> AdditionalMaterialParameters, std::function<void()> Callback);
	static TArray<FComputeMaterialParameter> MakeParameters(float Time);

public:
	~FOpenLandPolygonMesh();
	void RegisterVertexModifier(std::function<FVertexModifierResult(FVertexModifierPayload)> Callback);
	FGpuComputeMaterialStatus RegisterGpuVertexModifier(FComputeMaterial ComputeMaterial);
	
	FOpenLandPolygonMeshBuildResultPtr BuildMesh(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options);
	void BuildMeshAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildOptions Options,
	                    std::function<void(FOpenLandPolygonMeshBuildResultPtr)> Callback);
	
	void ModifyVertices(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
	                    FOpenLandPolygonMeshModifyOptions Options);
	// Here we do vertex modifications outside of the game thread
	// The return boolean value indicates whether we should render the Target MeshInfo or not
	// Note: It's very important to pass the same Target all the time because the return value is related to something happens earlier.
	// TODO: Get rid of this callback & implement the logic via the Tick.
	bool ModifyVerticesAsync(UObject* WorldContext, FOpenLandPolygonMeshBuildResultPtr MeshBuildResult,
	                         FOpenLandPolygonMeshModifyOptions Options, function<void()> Callback=nullptr);
	
	void AddTriFace(const FVector A, const FVector B, const FVector C);
	void AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D);
	void Transform(FTransform Transformer);
	bool IsThereAnyAsyncTask() const;

	// Methods for delete schedular
	static void RunDeleteScheduler();
	static void DeletePolygonMesh(FOpenLandPolygonMesh* PolygonMesh);
};
