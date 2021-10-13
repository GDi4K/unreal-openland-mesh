﻿// Fill out your copyright notice in the Description page of Project Settings.


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

	MovingGrid = MakeShared<FOpenLandMovingGrid>(MeshComponent, this);
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
	Rebuild();
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
	if (!bBuiltWithUserParameters)
	{
		Rebuild();
		bBuiltWithUserParameters = true;
	}
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

bool AOpenLandInfinityMeshActor::Rebuild()
{
	if (GetWorld() == nullptr)
	{
		return false;
	}
	
	// We cannot do instances inside the blueprint editor
	if (GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		return false;
	}
	
	// Here we detect the preview actor created when dragging it from the content browser
	// Even in that case, we should create any instance
	if (HasAnyFlags(RF_Transient))
	{
		return false;
	}
	
	FOpenLandMovingGridBuildOptions BuildOptions = {};
	BuildOptions.CuspAngle = 0;
	BuildOptions.CellWidth = CellWidth;
	BuildOptions.CellCount = CellCount;
	BuildOptions.UnitUVLenght = UnitUVLenght;
	BuildOptions.MaxUVs = MaxUVs;
	MovingGrid->SetVertexModifier(VertexModifier);
	MovingGrid->Build(BuildOptions);

	return true;
}

