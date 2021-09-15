// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OpenLandMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Utils/OpenLandPointsBuilder.h"
#include "OpenLandInstancingController.generated.h"

enum EOpenLandSpawningRegistrationAction
{
	OSC_SET_POINTS,
	OSC_UNREGISTER,
	OSC_UPDATE_TRANSFORM
};

USTRUCT()
struct FOpenLandInstancingRequestPoint
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FVector Normal;

	UPROPERTY()
	FVector TangentX;

	UPROPERTY()
	UClass* ActorClass = nullptr;

	UPROPERTY()
	UStaticMesh* StaticMesh = nullptr;

	UPROPERTY()
	bool bEnableCollisions = true;

	UPROPERTY();
	FVector RandomScale = {1.0f, 1.0f, 1.0f};

	UPROPERTY();
	FVector RandomRotation = {0.0f, 0.0f, 0.0f};

	UPROPERTY();
	bool bUseLocalRandomRotation = true;

	UPROPERTY();
	FVector RandomDisplacement = {0.0f, 0.0f, 0.0f};

	UPROPERTY();
	bool bUseLocalRandomDisplacement = true;
};

struct FOpenLandInstancingRequest
{
	FString OwnerId;
	TArray<FOpenLandInstancingRequestPoint> NewPoints;
	FTransform ComponentTransform;
};

struct FOpenLandInstancingRequestPayload
{
	FString OwnerId;
	EOpenLandSpawningRegistrationAction Action;
};

USTRUCT()
struct FOpenLandInstancedActorInfo
{
	GENERATED_BODY()

	UPROPERTY();
	AActor* Actor;

	UPROPERTY();
	FOpenLandInstancingRequestPoint OriginalPoint;
};

USTRUCT()
struct FOpenLandInstancedActorGroup
{
	GENERATED_BODY()

	UPROPERTY();
	TArray<FOpenLandInstancedActorInfo> SpawnedActors;

	UPROPERTY();
	bool bAllowCleaning = true;
};

struct AOpenLandInstancingTransformedInfo
{
	FVector Position;
	FRotator Rotator;
	FVector Scale;
};

UCLASS()
class OPENLANDMESH_API AOpenLandInstancingController : public AActor
{
	GENERATED_BODY()

	static TMap<FString, FOpenLandInstancingRequest> RequestsRegistry;
	static TArray<FOpenLandInstancingRequestPayload> RequestsToUpdate;
	static AOpenLandInstancingController* Singleton;

	float RemainingTimeToCleanUp = 0.0f;

	void SetPoints(FOpenLandInstancingRequest& Registration);
	void RemovePoints(FString OwnerId);
	void UpdatePointTransform(FOpenLandInstancingRequest& Registration);
	static AOpenLandInstancingTransformedInfo ApplyTransformation(FOpenLandInstancingRequestPoint Point, FTransform Transform);
	
public:
	// Sets default values for this actor's properties
	AOpenLandInstancingController();
	static void EnsureSpawnController(AOpenLandMeshActor* Owner);
	static void CreateInstances(AOpenLandMeshActor* Owner, TArray<FOpenLandInstancingRequestPoint> Points);
	static void UpdateTransforms(AOpenLandMeshActor* Owner);
	static void Unregister(AOpenLandMeshActor* Owner);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY()
	TMap<FString, FOpenLandInstancedActorGroup> InstancedGroupsMap;

	UPROPERTY()
	TMap<FString, AOpenLandMeshActor*> ChildMeshActors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="OpenLandMesh Instancing")
	float InstanceCleaningInterval = 2;
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void CleanUnlinkedInstances();
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void RemoveAllInstances();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void DontRunInstancingAfterBuildMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void RunInstancingAfterBuildMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void RemoveChildMeshActors();

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh Instancing")
	static TArray<AActor*> GetInstancesForOwner(AOpenLandMeshActor* OwnerMesh);
};
