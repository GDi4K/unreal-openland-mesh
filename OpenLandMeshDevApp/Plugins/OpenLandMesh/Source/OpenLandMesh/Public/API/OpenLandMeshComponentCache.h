// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLandMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Types/OpenLandMeshInfo.h"
#include "Materials/MaterialInterface.h"
#include "OpenLandMeshComponentCache.generated.h"

UCLASS()
class OPENLANDMESH_API AOpenLandMeshComponentCache : public AActor
{
	GENERATED_BODY()

	static TArray<FSimpleMeshInfoPtr> MeshSections;
	static TArray<UOpenLandMeshComponent*> MeshComponents;
	
	static TArray<int32> MeshSectionsToCreate;
	static TArray<int32> MeshSectionsToReplace;
	static TArray<int32> MeshSectionsToUpdate;
	static TArray<int32> MeshSectionsToChangeVisibility;
	static TMap<int32, UMaterialInterface*> MaterialsToSet;
	static TMap<int32, FTransform> PositionsToChange;

	UOpenLandMeshComponent* MeshComponent;

public:
	// Sets default values for this actor's properties
	AOpenLandMeshComponentCache();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

	// PUBLIC API
	static int32 CreateMeshSection(FSimpleMeshInfoPtr MeshInfo);
	static void ReplaceMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo);
	static void UpdateMeshSection(int32 SectionIndex, FOpenLandMeshComponentUpdateRange UpdateRange);
	static void RemoveMeshSection(int32 SectionIndex);
	static int32 NumMeshSections();
	static void UpdateMeshSectionVisibility(int32 SectionIndex);
	static void SetMaterial(int32 SectionIndex, UMaterialInterface* Material);
	static void SetTransform(int32 SectionIndex, FTransform NewPosition);

	UFUNCTION(CallInEditor)
	void Start();
};
