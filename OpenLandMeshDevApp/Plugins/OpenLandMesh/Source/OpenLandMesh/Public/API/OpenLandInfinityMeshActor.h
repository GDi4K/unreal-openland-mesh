// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLandMeshComponent.h"
#include "Core/OpenLandMovingGrid.h"
#include "GameFramework/Actor.h"

#include "OpenLandInfinityMeshActor.generated.h"

UCLASS()
class OPENLANDMESH_API AOpenLandInfinityMeshActor : public AActor
{
	GENERATED_BODY()

	FString ObjectId;
	FOpenLandMovingGridPtr MovingGrid;

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AOpenLandInfinityMeshActor();
	FString GetObjectId() const { return ObjectId; }
	
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rendering", Transient)
	UOpenLandMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLand Infinity Mesh")
	float CellWidth = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLand Infinity Mesh")
	int32 CellCount = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLand Infinity Mesh")
	float UnitUVLenght = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLand Infinity Mesh")
	int32 MaxUVs = 5;
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLand Infinity Mesh")
	void Rebuild();
};
