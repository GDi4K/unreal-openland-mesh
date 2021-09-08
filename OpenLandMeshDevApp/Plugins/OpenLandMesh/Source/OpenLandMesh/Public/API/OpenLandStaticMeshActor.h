// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "OpenLandStaticMeshActor.generated.h"

UCLASS()
class OPENLANDMESH_API AOpenLandStaticMeshActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOpenLandStaticMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* StaticMesh;
};
