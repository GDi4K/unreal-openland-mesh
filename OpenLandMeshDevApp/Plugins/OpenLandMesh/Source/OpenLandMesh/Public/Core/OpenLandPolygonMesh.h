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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	bool bOnAnimating;
};

USTRUCT(BlueprintType)
struct FVertexModifierResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLandMesh)
	FVector Position = {0, 0, 0};
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
	void ApplyVertexModifiers(FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target, int RangeStart, int RangeEnd, float RealTimeSeconds,  bool bOnBuilding) const;
	void EnsureGpuComputeEngine(UObject* WorldContext, FOpenLandMeshInfo* MeshInfo);
	void ApplyGpuVertexModifers(UObject* WorldContext, FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target, TArray<FComputeMaterialParameter> AdditionalMaterialParameters);
	static TArray<FComputeMaterialParameter> MakeParameters(float Time, bool bOnBuilding);

public:
	~FOpenLandPolygonMesh();
	FSimpleMeshInfoPtr BuildMesh(UObject* WorldContext, int SubDivisions = 0, float CuspAngle = 0);
	void BuildMeshAsync(UObject* WorldContext, int SubDivisions, float CuspAngle, std::function<void(FSimpleMeshInfoPtr)> Callback);
	void ModifyVertices(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target, float RealTimeSeconds, float CuspAngle = 0, bool bOnBuilding=false);
	void RegisterVertexModifier(std::function<FVertexModifierResult(FVertexModifierPayload)> Callback);
	FGpuComputeMaterialStatus RegisterGpuVertexModifier(FComputeMaterial ComputeMaterial);
	
	// Here we do vertex modifications outside of the game thread
	// The return boolean value indicates whether we should render the Target MeshInfo or not
	// Note: It's very important to pass the same Target all the time because the return value is related to something happens earlier.
	bool ModifyVerticesAsync(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target, float RealTimeSeconds, float CuspAngle = 0);
	void AddTriFace(const FVector A, const FVector B, const FVector C);
	void AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D);
	void Transform(FTransform Transformer);
	bool IsThereAnyAsyncTask() const;

	// Methods for delete schedular
	static void RunDeleteScheduler();
	static void DeletePolygonMesh(FOpenLandPolygonMesh* PolygonMesh);
};