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
	BuildOptions.CellWidth = CellWidth;
	BuildOptions.CellCount = CellCount;
	BuildOptions.UnitUVLenght = UnitUVLenght;
	BuildOptions.MaxUVs = MaxUVs;
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

	if (!GetWorld())
	{
		return;
	}

	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	const auto Player = GetWorld()->GetFirstPlayerController();
	if (!Player)
	{
		return;
	}

	const auto Pawn = Player->GetPawn();
	if (!Pawn)
	{
		return;
	}
	
	const FVector PlayerPosition = Pawn->GetActorLocation();
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

void AOpenLandInfinityMeshActor::Rebuild()
{
	FOpenLandMovingGridBuildOptions BuildOptions = {};
	BuildOptions.CuspAngle = 0;
	BuildOptions.CellWidth = CellWidth;
	BuildOptions.CellCount = CellCount;
	BuildOptions.UnitUVLenght = UnitUVLenght;
	BuildOptions.MaxUVs = MaxUVs;
	MovingGrid->Build(BuildOptions);
}

