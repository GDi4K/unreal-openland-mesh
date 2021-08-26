// Fill out your copyright notice in the Description page of Project Settings.


#include "API/OpenLandMeshComponentCache.h"

TArray<FSimpleMeshInfoPtr> AOpenLandMeshComponentCache::MeshSections = {};
TArray<UOpenLandMeshComponent*> AOpenLandMeshComponentCache::MeshComponents = {};
	
TArray<int32> AOpenLandMeshComponentCache::MeshSectionsToCreate = {};
TArray<int32> AOpenLandMeshComponentCache::MeshSectionsToReplace = {};
TArray<int32> AOpenLandMeshComponentCache::MeshSectionsToUpdate = {};
TArray<int32> AOpenLandMeshComponentCache::MeshSectionsToChangeVisibility = {};
TMap<int32, UMaterialInterface*> AOpenLandMeshComponentCache::MaterialsToSet = {};
TMap<int32, FTransform> AOpenLandMeshComponentCache::PositionsToChange = {};

// Sets default values
AOpenLandMeshComponentCache::AOpenLandMeshComponentCache()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	MeshComponent = CreateDefaultSubobject<UOpenLandMeshComponent>(FName("MeshComponent"));
	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

// Called when the game starts or when spawned
void AOpenLandMeshComponentCache::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AOpenLandMeshComponentCache::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Start();
}

void AOpenLandMeshComponentCache::Start()
{
	if (MeshComponent->NumMeshSections() > 0)
	{
		const FTransform* NewPosition = PositionsToChange.Find(0);
		if (NewPosition)
		{
			SetActorTransform(*NewPosition);
		}
	}
	
	// Mesh Sections to Create
	for (const int32 SectionIndex: MeshSectionsToCreate)
	{
		if (MeshComponent->NumMeshSections() > 0)
		{
			MeshComponent->ReplaceMeshSection(0, MeshSections[SectionIndex]);
		}
		else
		{
			MeshComponent->CreateMeshSection(MeshSections[SectionIndex]);
		}
		
		MeshComponent->InvalidateRendering();
	}
	MeshSectionsToCreate = {};

	// Mesh Sections to Replace
	for (const int32 SectionIndex: MeshSectionsToReplace)
	{
		if (MeshComponent->NumMeshSections() > 0)
		{
			MeshComponent->ReplaceMeshSection(0, MeshSections[SectionIndex]);
		}
		else
		{
			MeshComponent->CreateMeshSection(MeshSections[SectionIndex]);
		}
		
		MeshComponent->InvalidateRendering();
	}
	MeshSectionsToReplace = {};

	// MeshSections to Update
	for (const int32 SectionIndex: MeshSectionsToUpdate)
	{
		MeshComponent->UpdateMeshSection(0, {0, -1});
	}
	MeshSectionsToUpdate = {};

	// MeshSections to Change Visibility
	for (const int32 SectionIndex: MeshSectionsToChangeVisibility)
	{
		MeshComponent->UpdateMeshSection(0, {0, -1});
	}
	MeshSectionsToChangeVisibility = {};

	// Update Materials
	auto Material = MaterialsToSet.Find(0);
	if (Material != nullptr)
	{
		MeshComponent->SetMaterial(0, *Material);
	}
	MaterialsToSet = {};
}


bool AOpenLandMeshComponentCache::ShouldTickIfViewportsOnly() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	return World->WorldType == EWorldType::Editor;
}

int32 AOpenLandMeshComponentCache::CreateMeshSection(FSimpleMeshInfoPtr MeshInfo)
{
	MeshSections.SetNum(1);
	MeshSections[0] = MeshInfo;
	MeshSectionsToCreate.Push(0);
	
	return 0;
}

void AOpenLandMeshComponentCache::ReplaceMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo)
{
	MeshSections[SectionIndex] = MeshInfo;
	MeshSectionsToReplace.Push(SectionIndex);
}

void AOpenLandMeshComponentCache::UpdateMeshSection(int32 SectionIndex, FOpenLandMeshComponentUpdateRange UpdateRange)
{
	UE_LOG(LogTemp, Warning, TEXT("Asking to Update"))
	MeshSectionsToUpdate.Push(SectionIndex);
}

void AOpenLandMeshComponentCache::RemoveMeshSection(int32 SectionIndex)
{
	
}

int32 AOpenLandMeshComponentCache::NumMeshSections()
{
	return MeshSections.Num();
}

void AOpenLandMeshComponentCache::UpdateMeshSectionVisibility(int32 SectionIndex)
{
	MeshSectionsToChangeVisibility.Push(SectionIndex);
}

void AOpenLandMeshComponentCache::SetMaterial(int32 SectionIndex, UMaterialInterface* Material)
{
	MaterialsToSet.Add(SectionIndex, Material);
}

void AOpenLandMeshComponentCache::SetTransform(int32 SectionIndex, FTransform NewPosition)
{
	PositionsToChange.Add(0, NewPosition);
}

