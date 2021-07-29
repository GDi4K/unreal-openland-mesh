// Fill out your copyright notice in the Description page of Project Settings.


#include "ASimpleMeshActor.h"


// Sets default values
ASimpleMeshActor::ASimpleMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	MeshComponent = CreateDefaultSubobject<UOpenLandMeshComponent>(TEXT("MeshComponent"));

	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

// Called when the game starts or when spawned
void ASimpleMeshActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASimpleMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bMeshGenerated)
	{
		GenerateMesh();
		bMeshGenerated = true;
	}
}

bool ASimpleMeshActor::ShouldTickIfViewportsOnly() const
{
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
		return true;
	return false;
}

void ASimpleMeshActor::GenerateMesh()
{
}
