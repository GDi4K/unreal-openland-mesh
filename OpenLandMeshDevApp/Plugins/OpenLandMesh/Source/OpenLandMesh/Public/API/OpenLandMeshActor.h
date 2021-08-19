// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "OpenLandMeshPolygonMeshProxy.h"
#include "Core/OpenLandMeshComponent.h"
#include "Compute/Types/ComputeMaterial.h"

#include "OpenLandMeshActor.generated.h"

struct FLODInfo
{
	FOpenLandPolygonMeshBuildResultPtr MeshBuildResult = nullptr;
	int32 MeshComponentIndex = 0;
	int32 LODIndex = 0;
};

typedef TSharedPtr<FLODInfo> FLODInfoPtr;

UCLASS()
class OPENLANDMESH_API AOpenLandMeshActor : public AActor
{
	GENERATED_BODY()

	bool bMeshGenerated = false;
	bool bModifyMeshIsInProgress = false;
	bool bNeedToModifyMesh = true;

	TArray<FLODInfoPtr> LODList;
	FLODInfoPtr CurrentLOD = nullptr;
	bool bNeedLODVisibilityChange = false;
	
	bool SwitchLODs();
	void EnsureLODVisibility();

public:
	// Sets default values for this actor's properties
	AOpenLandMeshActor();
	~AOpenLandMeshActor();

protected:
	UPROPERTY(Transient)
	UOpenLandMeshPolygonMeshProxy* PolygonMesh;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
	UOpenLandMeshPolygonMeshProxy* GetPolygonMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
	FVertexModifierResult OnModifyVertex(FVertexModifierPayload Payload);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
    void OnAfterAnimations();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void SetMaterial(UMaterialInterface* Material);
	virtual bool ShouldTickIfViewportsOnly() const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void BuildMeshAsync(TFunction<void()> Callback = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rendering", Transient)
	UOpenLandMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int SubDivisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	float SmoothNormalAngle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bRunCpuVertexModifiers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	FComputeMaterial GpuVertexModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bRunGpuVertexModifiers = false;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bAnimate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bDisableGPUVertexModifiersOnAnimate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncBuildMeshOnGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncCollisionCooking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncAnimations = true;

	UPROPERTY(VisibleAnywhere, Category=OpenLandMesh)
	int32 CurrentLODIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int32 MaximumLODCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int32 LODStepUnits = 3000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	float LODStepPower = 1.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int32 LODIndexForCollisions = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	UMaterialInterface* Material;

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void BuildMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void ModifyMesh();

	//UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	//TODO: Get rid of this callback or do something, because this is a useful API
	void ModifyMeshAsync(TFunction<void()> Callback = nullptr);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	void SetGPUScalarParameter(FName Name, float Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    float GetGPUScalarParameter(FName Name);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    void SetGPUVectorParameter(FName Name, FVector Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    FVector GetGPUVectorParameter(FName Name);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    void SetGPUTextureParameter(FName Name, UTexture2D* Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    UTexture2D* GetGPUTextureParameter(FName Name);
};
