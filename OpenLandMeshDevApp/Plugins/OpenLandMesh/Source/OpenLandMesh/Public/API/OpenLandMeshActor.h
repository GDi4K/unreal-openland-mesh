// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "OpenLandMeshPolygonMeshProxy.h"
#include "Core/OpenLandMeshComponent.h"
#include "Compute/Types/ComputeMaterial.h"


#include "OpenLandMeshActor.generated.h"

UCLASS()
class OPENLANDMESH_API AOpenLandMeshActor : public AActor
{
	GENERATED_BODY()

	bool bMeshGenerated = false;

public:
	// Sets default values for this actor's properties
	AOpenLandMeshActor();
	~AOpenLandMeshActor();

protected:
	UPROPERTY()
	UOpenLandMeshPolygonMeshProxy* PolygonMesh;

	FSimpleMeshInfoPtr OriginalMeshInfo = nullptr;
	FSimpleMeshInfoPtr RenderingMeshInfo = nullptr;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UOpenLandMeshPolygonMeshProxy* GetPolygonMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVertexModifierResult OnModifyVertex(FVertexModifierPayload Payload);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void OnAfterAnimations();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void SetMaterial(UMaterialInterface* Material);
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void BuildMeshAsync(TFunction<void()> Callback = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	UOpenLandMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int SubDivisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int SmoothNormalAngle = 0;

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
	bool bUseAsyncCollisionCooking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncAnimations = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	UMaterialInterface* Material;

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void BuildMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void ModifyMesh();
};
