// Fill out your copyright notice in the Description page of Project Settings.


#include "API/OpenLandInstancingController.h"

#include <Actor.h>

#include "API/OpenLandStaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


TMap<FString, FOpenLandInstancingRequest> AOpenLandInstancingController::RequestsRegistry;
TArray<FOpenLandInstancingRequestPayload> AOpenLandInstancingController::RequestsToUpdate;
AOpenLandInstancingController* AOpenLandInstancingController::Singleton = nullptr;

// Sets default values
AOpenLandInstancingController::AOpenLandInstancingController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Singleton = this;
}

void AOpenLandInstancingController::EnsureSpawnController(AOpenLandMeshActor* Owner)
{
	TArray<AActor*> ExistingControllers;
	UGameplayStatics::GetAllActorsOfClass(Owner->GetWorld(), StaticClass(), ExistingControllers);
	if (ExistingControllers.Num() > 0)
	{
		return;
	}

	Owner->GetWorld()->SpawnActor(StaticClass());
}

void AOpenLandInstancingController::CreateInstances(AOpenLandMeshActor* Owner, TArray<FOpenLandInstancingRequestPoint> Points)
{
	EnsureSpawnController(Owner);
	
	if (RequestsRegistry.Find(Owner->GetObjectId()) == nullptr)
	{
		RequestsRegistry.Add(Owner->GetObjectId(), {});
	}

	FOpenLandInstancingRequest &Registration = RequestsRegistry[Owner->GetObjectId()];
	Registration.OwnerId = Owner->GetObjectId();
	Registration.NewPoints = Points;
	Registration.ComponentTransform = Owner->MeshComponent->GetComponentTransform();

	RequestsToUpdate.Push({
		Owner->GetObjectId(),
		OSC_SET_POINTS
	});
}

void AOpenLandInstancingController::UpdateTransforms(AOpenLandMeshActor* Owner)
{
	if (RequestsRegistry.Find(Owner->GetObjectId()) == nullptr)
	{
		FOpenLandInstancingRequest Registration;
		Registration.OwnerId = Owner->GetObjectId();
		Registration.ComponentTransform = Owner->MeshComponent->GetComponentTransform();
		RequestsRegistry.Add(Owner->GetObjectId(), Registration);
	}

	const FTransform NewTransform =  Owner->MeshComponent->GetComponentTransform();
	FOpenLandInstancingRequest &Registration = RequestsRegistry[Owner->GetObjectId()];
	if (Registration.ComponentTransform.Equals(NewTransform))
	{
		return;
	}

	Registration.ComponentTransform = NewTransform;

	RequestsToUpdate.Push({
		Owner->GetObjectId(),
		OSC_UPDATE_TRANSFORM
	});
}

void AOpenLandInstancingController::Unregister(AOpenLandMeshActor* Owner)
{
	RequestsToUpdate.Push({
		Owner->GetObjectId(),
		OSC_UNREGISTER
	});
}

AOpenLandInstancingTransformedInfo AOpenLandInstancingController::ApplyTransformation(
	FOpenLandInstancingRequestPoint Point, FTransform Transform)
{
	AOpenLandInstancingTransformedInfo TransformedInfo;

	// Add the random scale
	TransformedInfo.Scale = Point.RandomScale;

	// Add rotation
	const FQuat Random(FRotator::MakeFromEuler(Point.RandomRotation));
	const FQuat CorrectTangents(UKismetMathLibrary::MakeRotFromZX(Point.Normal, Point.TangentX));
	
	if (Point.bUseLocalRandomRotation)
	{
		TransformedInfo.Rotator = (Transform.Rotator().Quaternion() * CorrectTangents * Random).Rotator();
	} else
	{
		TransformedInfo.Rotator = (Transform.Rotator().Quaternion() * Random * CorrectTangents).Rotator();
	}

	// Apply transformed position
	TransformedInfo.Position = Transform.TransformPosition(Point.Position);
	// Apply random displacement
	if (Point.bUseLocalRandomDisplacement)
	{
		TransformedInfo.Position += TransformedInfo.Rotator.RotateVector(Point.RandomDisplacement);
	} else
	{
		TransformedInfo.Position += Point.RandomDisplacement;
	}

	return TransformedInfo;
}

void AOpenLandInstancingController::SetPoints(FOpenLandInstancingRequest &Registration)
{
	if (InstancedGroupsMap.Find(Registration.OwnerId) == nullptr)
	{
		FOpenLandInstancedActorGroup SpawnedRegistration;
		InstancedGroupsMap.Add(Registration.OwnerId, {});
	}

	FOpenLandInstancedActorGroup &SpawnedRegistration = InstancedGroupsMap[Registration.OwnerId];

	TArray<FOpenLandInstancedActorInfo> NewActors;
	TArray<FOpenLandInstancingRequestPoint> NewPoints = Registration.NewPoints;
	Registration.NewPoints = {};
	
	TMap<UClass*, TArray<FOpenLandInstancedActorInfo>> ExistingActorsByClass;
	TMap<UStaticMesh*, TArray<FOpenLandInstancedActorInfo>> ExistingActorsByStaticMesh;
	
	for(FOpenLandInstancedActorInfo ExistingActor: SpawnedRegistration.SpawnedActors)
	{
		UClass* ActorClass = ExistingActor.OriginalPoint.ActorClass;
		UStaticMesh* StaticMesh = ExistingActor.OriginalPoint.StaticMesh;

		if (ActorClass != nullptr)
		{
			if (ExistingActorsByClass.Find(ActorClass) == nullptr)
			{
				ExistingActorsByClass.Add(ActorClass, {});
			}

			ExistingActorsByClass[ActorClass].Push(ExistingActor);
			continue;
		}

		if (StaticMesh != nullptr)
		{
			if (ExistingActorsByStaticMesh.Find(StaticMesh) == nullptr)
			{
				ExistingActorsByStaticMesh.Add(StaticMesh, {});
			}

			ExistingActorsByStaticMesh[StaticMesh].Push(ExistingActor);
			continue;
		}
		
	}

	auto NewPointsCloned = NewPoints;
	while (NewPoints.Num() > 0)
	{
		const FOpenLandInstancingRequestPoint NewPoint = NewPoints.Pop();

		if (NewPoint.ActorClass == nullptr && NewPoint.StaticMesh == nullptr)
		{
			continue;
		}

		bool bHasExistingActor = ExistingActorsByClass.Find(NewPoint.ActorClass) != nullptr && ExistingActorsByClass[NewPoint.ActorClass].Num() > 0;
		bool bHasExistingStaticMesh = ExistingActorsByStaticMesh.Find(NewPoint.StaticMesh) != nullptr && ExistingActorsByStaticMesh[NewPoint.StaticMesh].Num() > 0;
		
		// Try translating an existing Actor or StaticMesh to the new position
		if (bHasExistingActor || bHasExistingStaticMesh)
		{
			FOpenLandInstancedActorInfo ExistingActor;
			if (NewPoint.ActorClass != nullptr)
			{
				ExistingActor = ExistingActorsByClass[NewPoint.ActorClass].Pop();
			} else
			{
				ExistingActor = ExistingActorsByStaticMesh[NewPoint.StaticMesh].Pop();
			}
			
			if (ExistingActor.Actor == nullptr)
			{
				NewPoints.Push(NewPoint);
				continue;
			}
			
			ExistingActor.OriginalPoint = NewPoint;
			
			const AOpenLandInstancingTransformedInfo TransformedInfo = ApplyTransformation(NewPoint, Registration.ComponentTransform);
			ExistingActor.Actor->SetActorLocationAndRotation(TransformedInfo.Position, TransformedInfo.Rotator);
			ExistingActor.Actor->SetActorRelativeScale3D(TransformedInfo.Scale);

			if (NewPoint.StaticMesh != nullptr)
			{
				AOpenLandStaticMeshActor* StaticMeshActor = Cast<AOpenLandStaticMeshActor>(ExistingActor.Actor);
				if (NewPoint.bEnableCollisions)
				{
					StaticMeshActor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
				} else
				{
					StaticMeshActor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}

			NewActors.Push(ExistingActor);
			continue;
		}

		// Create a new actor if there are no existing actors
		const AOpenLandInstancingTransformedInfo TransformedInfo = ApplyTransformation(NewPoint, Registration.ComponentTransform);
		
		FOpenLandInstancedActorInfo SpawnedActorInfo;
		SpawnedActorInfo.OriginalPoint = NewPoint;

		if (NewPoint.ActorClass != nullptr)
		{
			SpawnedActorInfo.Actor = GetWorld()->SpawnActor(NewPoint.ActorClass, &TransformedInfo.Position, &TransformedInfo.Rotator);
			SpawnedActorInfo.Actor->SetActorRelativeScale3D(TransformedInfo.Scale);
			SpawnedActorInfo.Actor->SetFolderPath("OpenLandMeshInstances");
		} else
		{
			SpawnedActorInfo.Actor = GetWorld()->SpawnActor(AOpenLandStaticMeshActor::StaticClass(), &TransformedInfo.Position, &TransformedInfo.Rotator);
			SpawnedActorInfo.Actor->SetActorRelativeScale3D(TransformedInfo.Scale);
			SpawnedActorInfo.Actor->SetFolderPath("OpenLandMeshInstances");
			AOpenLandStaticMeshActor* StaticMeshActor = Cast<AOpenLandStaticMeshActor>(SpawnedActorInfo.Actor);
			StaticMeshActor->StaticMesh->SetStaticMesh(NewPoint.StaticMesh);
			if (NewPoint.bEnableCollisions)
			{
				StaticMeshActor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			} else
			{
				StaticMeshActor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			}
		}
		
		NewActors.Push(SpawnedActorInfo);
	}

	// Delete remaining old instances
	for(const auto ExistingActors: ExistingActorsByClass)
	{
		for (const FOpenLandInstancedActorInfo ExistingActor: ExistingActors.Value)
		{
			if (ExistingActor.Actor != nullptr)
			{
				ExistingActor.Actor->Destroy(true);
			}
		}
	}

	for(const auto ExistingActors: ExistingActorsByStaticMesh)
	{
		for (const FOpenLandInstancedActorInfo ExistingActor: ExistingActors.Value)
		{
			if (ExistingActor.Actor != nullptr)
			{
				ExistingActor.Actor->Destroy(true);
			}
		}
	}
	
	SpawnedRegistration.SpawnedActors = NewActors;
}

void AOpenLandInstancingController::RemovePoints(FString OwnerId)
{
	if (InstancedGroupsMap.Find(OwnerId) == nullptr)
	{
		return;
	}

	FOpenLandInstancedActorGroup &SpawnedRegistration = InstancedGroupsMap[OwnerId];

	// Delete existing instances
	for (const FOpenLandInstancedActorInfo SpawnedActorInfo: SpawnedRegistration.SpawnedActors)
	{
		if (SpawnedActorInfo.Actor != nullptr)
		{
			SpawnedActorInfo.Actor->Destroy(true);
		}
	}

	InstancedGroupsMap.Remove(OwnerId);
	// TODO: This can be problematic, but easy to implement
	RequestsRegistry.Remove(OwnerId);
}

void AOpenLandInstancingController::UpdatePointTransform(FOpenLandInstancingRequest& Registration)
{
	if (InstancedGroupsMap.Find(Registration.OwnerId) == nullptr)
	{
		return;
	}

	FOpenLandInstancedActorGroup &SpawnedRegistration = InstancedGroupsMap[Registration.OwnerId];
	for (const FOpenLandInstancedActorInfo SpawnedActorInfo: SpawnedRegistration.SpawnedActors)
	{
		if (SpawnedActorInfo.Actor == nullptr)
		{
			continue;
		}
		
		const AOpenLandInstancingTransformedInfo TransformedInfo = ApplyTransformation(SpawnedActorInfo.OriginalPoint, Registration.ComponentTransform);
		SpawnedActorInfo.Actor->SetActorLocationAndRotation(TransformedInfo.Position, TransformedInfo.Rotator);
		SpawnedActorInfo.Actor->SetActorRelativeScale3D(TransformedInfo.Scale);
	}

}

// Called when the game starts or when spawned
void AOpenLandInstancingController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOpenLandInstancingController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RemainingTimeToCleanUp -= DeltaTime;
	if (GetWorld()->IsEditorWorld() && RemainingTimeToCleanUp <= 0)
	{
		CleanUnlinkedInstances();
		RemainingTimeToCleanUp = InstanceCleaningInterval;
	}

	auto RequestsToUpdateCloned = RequestsToUpdate;
	RequestsToUpdate = {};
	
	for (const FOpenLandInstancingRequestPayload UpdatePayload: RequestsToUpdateCloned)
	{
		FString OwnerId = UpdatePayload.OwnerId;
		if (RequestsRegistry.Find(OwnerId) == nullptr)
		{
			continue;
		}
		
		FOpenLandInstancingRequest &Registration = RequestsRegistry[OwnerId];

		switch (UpdatePayload.Action)
		{
			case OSC_SET_POINTS:
				SetPoints(Registration);
				break;
			case OSC_UNREGISTER:
				RemovePoints(UpdatePayload.OwnerId);
				break;
			case OSC_UPDATE_TRANSFORM:
				UpdatePointTransform(Registration);
				break;
			default:
				break;
		}
	}
}

bool AOpenLandInstancingController::ShouldTickIfViewportsOnly() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	return World->WorldType == EWorldType::Editor;
}

void AOpenLandInstancingController::CleanUnlinkedInstances()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOpenLandMeshActor::StaticClass(), FoundActors);

	TSet<FString> ObjectIds;
	for (const AActor* Actor: FoundActors)
	{
		const AOpenLandMeshActor* MeshActor = Cast<AOpenLandMeshActor>(Actor);
		ObjectIds.Add(MeshActor->GetObjectId());
	}

	TArray<FString> ItemsToRemove;
	for (const auto Pair: InstancedGroupsMap)
	{
		if (ObjectIds.Find(Pair.Key) == nullptr)
		{
			for(const auto ActorInfo: Pair.Value.SpawnedActors)
			{
				if (ActorInfo.Actor != nullptr)
				{
					ActorInfo.Actor->Destroy(true);
				}
			}

			ItemsToRemove.Push(Pair.Key);
		}
	}

	for (const FString Key: ItemsToRemove)
	{
		InstancedGroupsMap.Remove(Key);
	}
}

void AOpenLandInstancingController::RemoveAllInstances()
{
	for (const auto Pair: InstancedGroupsMap)
	{
		for(const auto ActorInfo: Pair.Value.SpawnedActors)
		{
			if (ActorInfo.Actor != nullptr)
			{
				ActorInfo.Actor->Destroy(true);
			}
		}
	}

	InstancedGroupsMap.Empty();
}

TArray<AActor*> AOpenLandInstancingController::GetInstancesForOwner(AOpenLandMeshActor* OwnerMesh)
{
	if (Singleton == nullptr)
	{
		return {};
	}

	if (Singleton->InstancedGroupsMap.Find(OwnerMesh->GetObjectId()) == nullptr)
	{
		return {};
	}

	TArray<AActor*> Instances;
	for (const FOpenLandInstancedActorInfo ActorInfo: Singleton->InstancedGroupsMap[OwnerMesh->GetObjectId()].SpawnedActors)
	{
		Instances.Push(ActorInfo.Actor);
	}

	return Instances;
}

