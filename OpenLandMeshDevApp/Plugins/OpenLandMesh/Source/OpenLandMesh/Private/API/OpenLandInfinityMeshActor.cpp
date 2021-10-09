// Fill out your copyright notice in the Description page of Project Settings.


#include "API/OpenLandInfinityMeshActor.h"
#include "API/OpenLandMeshHash.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

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

TSharedPtr<FVector> AOpenLandInfinityMeshActor::GetPlayerPosition() const
{
	if (!GetWorld()->IsGameWorld())
	{
		return nullptr;
	}
	
	const APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (!Player)
	{
		return nullptr;
	}
	
	const APawn* Pawn = Player->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}
	
	const FVector PlayerPosition = Pawn->GetActorLocation();
	return MakeShared<FVector>(PlayerPosition);
}

TSharedPtr<FVector> AOpenLandInfinityMeshActor::GetCameraPosition() const
{
	const TArray<FVector> ViewLocations = GetWorld()->ViewLocationsRenderedLastFrame;
	if(ViewLocations.Num() == 0)
	{
		return nullptr;
	}
	
	const FVector CameraLocation = ViewLocations[0];
	return MakeShared<FVector>(CameraLocation);
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

	const TSharedPtr<FVector> PlayerPosition = GetPlayerPosition();
	if (PlayerPosition)
	{
		MovingGrid->UpdatePositionAsync(*PlayerPosition.Get());
		return;
	}

	const TSharedPtr<FVector> CameraPosition = GetCameraPosition();
	if (CameraPosition)
	{
		MovingGrid->UpdatePositionAsync(*CameraPosition.Get());
	}
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

