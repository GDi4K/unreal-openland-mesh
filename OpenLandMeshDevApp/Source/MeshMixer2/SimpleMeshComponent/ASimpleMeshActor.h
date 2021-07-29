// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Core/OpenLandMeshComponent.h"
#include "GameFramework/Actor.h"
#include "ASimpleMeshActor.generated.h"

UCLASS()
class MESHMIXER2_API ASimpleMeshActor : public AActor
{
	GENERATED_BODY()

	bool bMeshGenerated = false;

public:
	// Sets default values for this actor's properties
	ASimpleMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UOpenLandMeshComponent* MeshComponent;

	// Called every frame
	virtual bool ShouldTickIfViewportsOnly() const override;

	UFUNCTION(CallInEditor, BlueprintCallable)
	virtual void GenerateMesh();
};
