// Fill out your copyright notice in the Description page of Project Settings.


#include "API/OpenLandStaticMeshActor.h"


// Sets default values
AOpenLandStaticMeshActor::AOpenLandStaticMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	StaticMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

// Called when the game starts or when spawned
void AOpenLandStaticMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOpenLandStaticMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

