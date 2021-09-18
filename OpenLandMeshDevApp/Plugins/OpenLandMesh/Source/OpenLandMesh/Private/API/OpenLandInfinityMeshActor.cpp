// Fill out your copyright notice in the Description page of Project Settings.


#include "API/OpenLandInfinityMeshActor.h"

#include "API/OpenLandMeshHash.h"


// Sets default values
AOpenLandInfinityMeshActor::AOpenLandInfinityMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	MeshComponent = CreateDefaultSubobject<UOpenLandMeshComponent>(TEXT("MeshComponent"));

	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	UOpenLandMeshHash* HashGen = NewObject<UOpenLandMeshHash>();
	HashGen->AddFloat(FMath::Rand());
	ObjectId = HashGen->Generate();

	MovingGrid = MakeShared<FOpenLandMovingGrid>(MeshComponent);
	FOpenLandMovingGridBuildOptions BuildOptions = {};
	BuildOptions.CuspAngle = 0;
	BuildOptions.CellWidth = 20.0f;
	BuildOptions.CellCount = 20;
	MovingGrid->Build(BuildOptions);
}

// Called when the game starts or when spawned
void AOpenLandInfinityMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOpenLandInfinityMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld()->IsGameWorld())
	{
		return;
	}
	
	const FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	MovingGrid->UpdatePosition(PlayerPosition);
}

void AOpenLandInfinityMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

bool AOpenLandInfinityMeshActor::ShouldTickIfViewportsOnly() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	return World->WorldType == EWorldType::Editor;
}

