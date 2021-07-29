// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "API/OpenLandMeshActor.h"
#include "GameFramework/Actor.h"
#include "OpenLandMeshHelloWorldActor.generated.h"

UCLASS()
class MESHMIXER2_API AOpenLandMeshHelloWorldActor : public AOpenLandMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOpenLandMeshHelloWorldActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FVertexModifierResult OnModifyVertex_Implementation(FVertexModifierPayload Payload);
	UOpenLandMeshPolygonMeshProxy* GetPolygonMesh_Implementation();
};
