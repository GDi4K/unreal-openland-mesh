// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <functional>

#include "Types/OpenLandMeshInfo.h"
#include "Core/OpenLandPolygonMesh.h"
#include "UObject/Object.h"
#include "OpenLandMeshPolygonMeshProxy.generated.h"

/**
 * 
 */
UCLASS()
class OPENLANDMESH_API UOpenLandMeshPolygonMeshProxy : public UObject
{
	GENERATED_BODY()

	FOpenLandPolygonMesh* PolygonMesh;

public:
	UOpenLandMeshPolygonMeshProxy();
	~UOpenLandMeshPolygonMeshProxy();
	
	FSimpleMeshInfoPtr BuildMesh(UObject* WorldContext, int SubDivisions = 0, float CuspAngle = 0) const;
	void BuildMeshAsync(UObject* WorldContext, int SubDivisions, float CuspAngle, std::function<void(FSimpleMeshInfoPtr)> Callback) const;
	void ModifyVertices(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target, float RealTimeSeconds,
	                    float CuspAngle = 0) const;
	// Here we do vertex modifications outside of the game thread
	// The return boolean value indicates whether we should render the Target MeshInfo or not
	// Note: It's very important to pass the same Target all the time because the return value is related to something happens earlier.
	bool ModifyVerticesAsync(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target, float RealTimeSeconds,
	                         float CuspAngle = 0);

	void RegisterVertexModifier(function<FVertexModifierResult(FVertexModifierPayload)> Callback);
	FGpuComputeMaterialStatus RegisterGpuVertexModifier(FComputeMaterial VertexModifier);

	// Blueprint Methods
	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	UOpenLandMeshPolygonMeshProxy* AddTriFace(const FVector A, const FVector B, const FVector C);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	UOpenLandMeshPolygonMeshProxy* AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    UOpenLandMeshPolygonMeshProxy* Transform(const FTransform Tranformer);

	// Static Methods
	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	static UOpenLandMeshPolygonMeshProxy* MakeEmptyPolygonMesh();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    static UOpenLandMeshPolygonMeshProxy* MakePlaneMesh();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    static UOpenLandMeshPolygonMeshProxy* MakeCubeMesh();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    static UOpenLandMeshPolygonMeshProxy* MakePyramidMesh();
};
