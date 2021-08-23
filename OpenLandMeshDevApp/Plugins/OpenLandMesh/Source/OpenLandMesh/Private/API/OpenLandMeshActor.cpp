// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "API/OpenLandMeshActor.h"

#include "Utils/TrackTime.h"


// Sets default values
AOpenLandMeshActor::AOpenLandMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	MeshComponent = CreateDefaultSubobject<UOpenLandMeshComponent>(TEXT("MeshComponent"));

	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

AOpenLandMeshActor::~AOpenLandMeshActor()
{
	CurrentLOD = nullptr;
	LODList.Empty();
}

// Called when the game starts or when spawned
void AOpenLandMeshActor::BeginPlay()
{
	Super::BeginPlay();
	if (bUseAsyncBuildMeshOnGame)
	{
		SwitchLODs();
		bMeshGenerated = true;
	}
	else
	{
		BuildMesh();
		if (bDisableGPUVertexModifiersOnAnimate)
			PolygonMesh->RegisterGpuVertexModifier({});
	}
}

UOpenLandMeshPolygonMeshProxy* AOpenLandMeshActor::GetPolygonMesh_Implementation()
{
	return nullptr;
}

FVertexModifierResult AOpenLandMeshActor::OnModifyVertex_Implementation(FVertexModifierPayload Payload)
{
	return {Payload.Position};
}

void AOpenLandMeshActor::OnAfterAnimations_Implementation()
{
}

void AOpenLandMeshActor::RunAsyncModifyMeshProcess(float LastFrameTime)
{
	
	const FLODInfoPtr ModifyingLOD = AsyncBuildingLODIndex >= 0? LODList[AsyncBuildingLODIndex] : CurrentLOD;
	
	if (!ModifyStatus.bStarted)
	{
		FOpenLandPolygonMeshModifyOptions ModifyOptions = {};
		ModifyOptions.RealTimeSeconds = GetWorld()->RealTimeSeconds;
		ModifyOptions.CuspAngle = SmoothNormalAngle;
		ModifyOptions.LastFrameTime = LastFrameTime;
		ModifyOptions.DesiredFrameRate = DesiredFrameRateOnModify;
		ModifyStatus = PolygonMesh->StartModifyVertices(this, ModifyingLOD->MeshBuildResult, ModifyOptions);
		
		return;
	}
	
	ModifyStatus = PolygonMesh->CheckModifyVerticesStatus(LastFrameTime);
	
	if (ModifyStatus.bAborted)
	{
		UE_LOG(LogTemp, Warning, TEXT(" RunAsyncModifyMeshProcess Aborted"))
		FOpenLandPolygonMeshModifyOptions ModifyOptions = {};
		ModifyOptions.RealTimeSeconds = GetWorld()->RealTimeSeconds;
		ModifyOptions.CuspAngle = SmoothNormalAngle;
		ModifyOptions.LastFrameTime = LastFrameTime;
		ModifyOptions.DesiredFrameRate = DesiredFrameRateOnModify;
		ModifyStatus = PolygonMesh->StartModifyVertices(this, ModifyingLOD->MeshBuildResult, ModifyOptions);
		return;
	}

	if (ModifyStatus.bCompleted)
	{
		OnAfterAnimations();
		// When someone updated GPU parameters inside the above hook
		// We need to update them like this
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
		// This is important to notify that we process the current modify operation
		ModifyStatus = {};
		
		if (AsyncBuildingLODIndex >= 0)
		{
			ModifyingLOD->MeshSectionIndex = MeshComponent->NumMeshSections();
			MeshComponent->CreateMeshSection(ModifyingLOD->MeshSectionIndex, ModifyingLOD->MeshBuildResult->Target);
			MeshComponent->InvalidateRendering();
			
			if (ModifyingLOD->MeshBuildResult->Target->bEnableCollision)
			{
				MeshComponent->SetupCollisions(true);
			}
			AsyncBuildingLODIndex = -1;
			SetMaterial(Material);
			EnsureLODVisibility();
			UE_LOG(LogTemp, Warning, TEXT("LOD Building Completed: LODIndex: %d"), ModifyingLOD->LODIndex)
			return;
		}
		
		MeshComponent->UpdateMeshSection(CurrentLOD->MeshSectionIndex, {0, -1});
		if (bNeedLODVisibilityChange)
		{
			EnsureLODVisibility();
		}
		
		if (SwitchLODs())
		{
			bNeedLODVisibilityChange = true;
		}
	}
}

void AOpenLandMeshActor::RunSyncModifyMeshProcess()
{
	if (SwitchLODs())
	{
		EnsureLODVisibility();
	}
	PolygonMesh->ModifyVertices(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle});
	MeshComponent->UpdateMeshSection(CurrentLODIndex, {0, -1});
	OnAfterAnimations();
	// When someone updated GPU parameters inside the above hook
	// We need to update them like this
	PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
}

// Called every frame
void AOpenLandMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	const bool bIsEditor = GetWorld()->WorldType == EWorldType::Editor;

	if (bIsEditor || !bAnimate)
	{
		if(SwitchLODs())
		{
			EnsureLODVisibility();
		}
	}

	if (CurrentLOD == nullptr)
	{
		return;
	}

	if (CurrentLOD->MeshBuildResult->Target->IsLocked())
	{
		return;
	}
	
	if (!ModifyStatus.bStarted && bNeedToAsyncModifyMesh)
	{
		bNeedToAsyncModifyMesh = false;
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
		RunAsyncModifyMeshProcess(DeltaTime);
		return;
	}

	if (ModifyStatus.bStarted)
	{
		RunAsyncModifyMeshProcess(DeltaTime);
		return;
	}
	
	if (bIsEditor)
	{
		return;
	}

	if (!bAnimate)
	{
		return;
	}

	if (bUseAsyncAnimations)
	{
		RunAsyncModifyMeshProcess(DeltaTime);
	} else
	{
		RunSyncModifyMeshProcess();
	}
}

void AOpenLandMeshActor::BuildMesh()
{
	// TODO: Remove this once we introduced a pool for RenderTargets & Textures
	if (GetWorld()->WorldType == EWorldType::Editor)
		// Inside Editor, it's possible to call this function multiple times.
		// Then it'll create multiple PolygonMesh obejects.
		// So, it won't garbage collect old instances, that'll lead to huge memory leak
		// (It will once the Actor is deleted)
		// So, to prevent the memory leak, we need to call `RegisterGpuVertexModifier({})`
		// That will delete the underline GPUComputeEngine & release all textures & render targets
		if (PolygonMesh)
		{
			PolygonMesh->RegisterGpuVertexModifier({});
			PolygonMesh->RegisterVertexModifier(nullptr);
		}

	PolygonMesh = GetPolygonMesh();
	if (!PolygonMesh)
		PolygonMesh = NewObject<UOpenLandMeshPolygonMeshProxy>();

	if (bRunCpuVertexModifiers)
		PolygonMesh->RegisterVertexModifier([this](const FVertexModifierPayload Payload) -> FVertexModifierResult
		{
			return OnModifyVertex(Payload);
		});
	else
		PolygonMesh->RegisterVertexModifier(nullptr);

	if (bRunGpuVertexModifiers)
	{
		// TODO: Check whether we really need this.
		// This is something to make sure, we are starting with a compiled version
		// And we are not stucked forever until when loading the actor
#if WITH_EDITOR
		if (GpuVertexModifier.Material != nullptr)
		{
			GpuVertexModifier.Material->ForceRecompileForRendering();
		}
#endif
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	}
	else
	{
		PolygonMesh->RegisterGpuVertexModifier({});
	}


	TArray<FLODInfoPtr> NewLODList;
	int32 ForcedTextureWidth = 0;

	TrackTime TotalLODTime = TrackTime("Total LOD Gen", true);
	for (int32 LODIndex=0; LODIndex<MaximumLODCount; LODIndex++)
	{
		auto LODGenTime = TrackTime(" LOD Gen: " + FString::FromInt(LODIndex), true);
		FLODInfoPtr LOD = MakeShared<FLODInfo>();

		const FOpenLandPolygonMeshBuildOptions BuildMeshOptions = {
			FMath::Max(SubDivisions - LODIndex, 0),
	        SmoothNormalAngle,
			ForcedTextureWidth
	    };
		const FOpenLandPolygonMeshBuildResultPtr NewMeshBuildResult = PolygonMesh->BuildMesh(this, BuildMeshOptions);

		NewMeshBuildResult->Target->bSectionVisible = false;
		NewMeshBuildResult->Target->bEnableCollision = bEnableCollision;

		// With this setting, we use the given LOD as the collision mesh
		// Otherwise, we will use all of these sections
		if (LODIndexForCollisions >= 0 && bEnableCollision)
		{
			NewMeshBuildResult->Target->bEnableCollision = LODIndex == LODIndexForCollisions;
		}

		// We will set the first LOD's texture width for all other data textures
		// For now, we use a single RenderTarget for each instance
		// So, it's size will be the LOD0's size.
		// That's why we do this.
		if (LODIndex == 0)
		{
			ForcedTextureWidth = NewMeshBuildResult->TextureWidth;
		}
		
		LOD->MeshBuildResult = NewMeshBuildResult;
		LOD->MeshSectionIndex = LODIndex;
		LOD->LODIndex = LODIndex;
		
		NewLODList.Push(LOD);
		LODGenTime.Finish();
	}
	TotalLODTime.Finish();

	TrackTime TotalLRenderingRegTime = TrackTime("Total Render Registration", true);
	for (const FLODInfoPtr LOD: NewLODList)
	{
		LOD->MeshBuildResult->Target->bSectionVisible = LOD->LODIndex == CurrentLODIndex;
		const bool bHasSection = MeshComponent->NumMeshSections() > LOD->MeshSectionIndex;
		if (bHasSection)
		{
			MeshComponent->ReplaceMeshSection(LOD->MeshSectionIndex, LOD->MeshBuildResult->Target);
		} else
		{
			MeshComponent->CreateMeshSection(LOD->MeshSectionIndex, LOD->MeshBuildResult->Target);
		}
	}
	TotalLRenderingRegTime.Finish();

	TrackTime UpdateCollisionTime = TrackTime("Setup Collisions", true);
	MeshComponent->SetupCollisions(bUseAsyncCollisionCooking);
	UpdateCollisionTime.Finish();

	MeshComponent->InvalidateRendering();

	LODList.Empty();
	LODList = NewLODList;

	if (CurrentLODIndex >= LODList.Num())
	{
		CurrentLODIndex = 0;
	}
	CurrentLOD = LODList[CurrentLODIndex];
	
	SetMaterial(Material);
	bMeshGenerated = true;
}

void AOpenLandMeshActor::ModifyMesh()
{
	if (ModifyStatus.bStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot ModifyMesh while there's an async ModifyMesh task in process"))
		bNeedToAsyncModifyMesh = true;
		return;
	}
	
	if (bRunGpuVertexModifiers)
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	else
		PolygonMesh->RegisterGpuVertexModifier({});

	PolygonMesh->ModifyVertices(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle});
	MeshComponent->UpdateMeshSection(CurrentLODIndex, {0, -1});
}

void AOpenLandMeshActor::ModifyMeshAsync()
{
	bNeedToAsyncModifyMesh = true;
}

void AOpenLandMeshActor::SetGPUScalarParameter(FName Name, float Value)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			Param.ScalarValue = Value;
			return;
		}
	}

	GpuVertexModifier.Parameters.Push({
        Name,
        CMPT_SCALAR,
        Value
    });
}

float AOpenLandMeshActor::GetGPUScalarParameter(FName Name)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			return Param.ScalarValue;
		}
	}

	return 0;
}

void AOpenLandMeshActor::SetGPUVectorParameter(FName Name, FVector Value)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			Param.VectorValue = Value;
			return;
		}
	}

	GpuVertexModifier.Parameters.Push({
        Name,
        CMPT_VECTOR,
		0,
        Value
    });
}

FVector AOpenLandMeshActor::GetGPUVectorParameter(FName Name)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			return Param.VectorValue;
		}
	}

	return FVector::ZeroVector;
}

void AOpenLandMeshActor::SetGPUTextureParameter(FName Name, UTexture2D* Value)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			Param.TextureValue = Value;
			return;
		}
	}

	GpuVertexModifier.Parameters.Push({
        Name,
        CMPT_VECTOR,
        0,
        FVector::ZeroVector,
		Value
    });
}

UTexture2D* AOpenLandMeshActor::GetGPUTextureParameter(FName Name)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			return Param.TextureValue;
		}
	}

	return nullptr;
}

void AOpenLandMeshActor::BuildMeshAsync(int32 LODIndex)
{
	PolygonMesh = GetPolygonMesh();
	if (!PolygonMesh)
	{
		PolygonMesh = NewObject<UOpenLandMeshPolygonMeshProxy>();
	}

	if (bRunCpuVertexModifiers)
	{
		PolygonMesh->RegisterVertexModifier([this](const FVertexModifierPayload Payload) -> FVertexModifierResult
		{
			return OnModifyVertex(Payload);
		});
	}
	else
	{
		PolygonMesh->RegisterVertexModifier(nullptr);
	}

	if (bRunGpuVertexModifiers)
	{
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	}
	else
	{
		PolygonMesh->RegisterGpuVertexModifier({});
	}

	FOpenLandPolygonMeshBuildOptions BuildMeshOptions = {};
	BuildMeshOptions.SubDivisions = FMath::Max(0, SubDivisions - LODIndex);
	BuildMeshOptions.CuspAngle = SmoothNormalAngle;

	PolygonMesh->BuildMeshAsync(this, BuildMeshOptions, [this, LODIndex](FOpenLandPolygonMeshBuildResultPtr Result)
	{
		Result->Target->bSectionVisible = true;
		Result->Target->bEnableCollision = bEnableCollision;
		
		// With this setting, we use the given LOD as the collision mesh
		// Otherwise, we will use all of these sections
		if (LODIndexForCollisions >= 0 && bEnableCollision)
		{
			Result->Target->bEnableCollision = LODIndex == LODIndexForCollisions;
		}

		const FLODInfoPtr LOD = MakeShared<FLODInfo>();
		LOD->MeshBuildResult = Result;
		LOD->MeshSectionIndex = -1;
		LOD->LODIndex = LODIndex;

		LODList[LODIndex] = LOD;
		CurrentLOD = LOD;
		
		ModifyMeshAsync();
	});
}

void AOpenLandMeshActor::SetMaterial(UMaterialInterface* InputMaterial)
{
	Material = InputMaterial;
	for(int32 MeshSectionIndex = 0; MeshSectionIndex< MeshComponent->NumMeshSections(); MeshSectionIndex++)
	{
		MeshComponent->SetMaterial(MeshSectionIndex, Material);	
	}
}

void AOpenLandMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!bMeshGenerated)
		BuildMesh();
}

void AOpenLandMeshActor::EnsureLODVisibility()
{
	for(const FLODInfoPtr LOD: LODList)
	{
		if (LOD == nullptr)
		{
			continue;
		}

		LOD->MeshBuildResult->Target->bSectionVisible = LOD->LODIndex == CurrentLODIndex;
		MeshComponent->UpdateMeshSectionVisibility(LOD->MeshSectionIndex);
	}
}

bool AOpenLandMeshActor::SwitchLODs()
{
	if (AsyncBuildingLODIndex >=0)
	{
		return false;
	}
	
	const UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return false;
	}
 
	const TArray<FVector> ViewLocations = World->ViewLocationsRenderedLastFrame;
	if(ViewLocations.Num() == 0)
	{
		return false;
	}

	const FVector CameraLocation = ViewLocations[0];
	const float Distance = FVector::Distance(CameraLocation, GetActorLocation());

	int32 DesiredLOD = 0;
	
	float RemainingDistance = Distance;
	for(int32 LODIndex=0; LODIndex<MaximumLODCount; LODIndex++)
	{
		DesiredLOD = LODIndex;
		RemainingDistance -= LODStepUnits * FMath::Pow(LODStepPower, LODIndex);
		if (RemainingDistance <= 0)
		{
			break;
		}
	}

	const int32 CorrectedLODIndexForCollisions = FMath::Min(LODIndexForCollisions, FMath::Max(MaximumLODCount - 1, 0));
	if (CorrectedLODIndexForCollisions >= 0)
	{
		const bool bHasCollisionLOD = LODList.Num() > CorrectedLODIndexForCollisions && LODList[CorrectedLODIndexForCollisions] != nullptr;
		if (DesiredLOD != CorrectedLODIndexForCollisions && !bHasCollisionLOD)
		{
			DesiredLOD = CorrectedLODIndexForCollisions;
		}
	}

	const bool bHasLOD = LODList.Num() > DesiredLOD && LODList[DesiredLOD] != nullptr;
	if (!bHasLOD)
	{
		if (GetWorld()->WorldType == EWorldType::Editor)
		{
			return false;
		}
		else
		{
			AsyncBuildingLODIndex = DesiredLOD;
			if (LODList.Num() <= DesiredLOD)
			{
				LODList.SetNumZeroed(DesiredLOD + 1, false);
			}
			BuildMeshAsync(DesiredLOD);
			return false;
		}
	}

	if (DesiredLOD == CurrentLODIndex) {
		return false;
	}

	CurrentLODIndex = DesiredLOD;
	CurrentLOD = LODList[CurrentLODIndex];
	return true;
}

bool AOpenLandMeshActor::ShouldTickIfViewportsOnly() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	return World->WorldType == EWorldType::Editor;
}

#if WITH_EDITOR
void AOpenLandMeshActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property == nullptr)
		return;

	// We only need to do this for the Material propertry only.
	// Since we use OnConstruction to run BuildMesh, it will override this.
	// We don't invoke SetMaterial inside that.
	// If we do that, users cannot add Materials by drag-n-drop
	if (PropertyChangedEvent.Property->GetFName() == "Material")
		SetMaterial(Material);

	// Update GPU Vertex Modifiers & It's Params
	if (PolygonMesh)
	{
		if (bRunGpuVertexModifiers)
			PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
		else
			PolygonMesh->RegisterGpuVertexModifier({});
	}
}
#endif