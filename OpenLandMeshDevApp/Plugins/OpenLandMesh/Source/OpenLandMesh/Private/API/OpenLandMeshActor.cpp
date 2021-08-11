// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "API/OpenLandMeshActor.h"


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
		BuildMeshAsync([this]()
		{
			if (bDisableGPUVertexModifiersOnAnimate)
				PolygonMesh->RegisterGpuVertexModifier({});
		});
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

// Called every frame
void AOpenLandMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetWorld()->WorldType == EWorldType::Editor)
		return;


	if (CurrentLOD == nullptr)
		return;

	if (CurrentLOD->MeshBuildResult.Target->IsLocked())
		return;

	if (!bAnimate)
		return;

	if (bUseAsyncAnimations)
	{
		const bool bCanUpdate = PolygonMesh->ModifyVerticesAsync(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle});
		if (bCanUpdate)
		{
			MeshComponent->UpdateMeshSection(0);
			OnAfterAnimations();
			// When someone updated GPU parameters inside the above hook
			// We need to update them like this
			PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
		}
	} else
	{
		PolygonMesh->ModifyVertices(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle});
		MeshComponent->UpdateMeshSection(0);
		OnAfterAnimations();
		// When someone updated GPU parameters inside the above hook
		// We need to update them like this
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	}
}

void AOpenLandMeshActor::BuildMesh()
{
	SetMaterial(Material);
	
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

	const FOpenLandPolygonMeshBuildOptions BuildMeshOptions = {
		SubDivisions,
        SmoothNormalAngle
    };

	const FOpenLandPolygonMeshBuildResult NewMeshBuildResult = PolygonMesh->BuildMesh(this, BuildMeshOptions);
	
	NewMeshBuildResult.Target->bEnableCollision = bEnableCollision;
	NewMeshBuildResult.Target->bUseAsyncCollisionCooking = bUseAsyncCollisionCooking;

	if (CurrentLOD == nullptr)
	{
		TSharedPtr<FLODInfo> LOD0 = MakeShared<FLODInfo>();
		LOD0->MeshBuildResult = NewMeshBuildResult;
		LOD0->MeshComponentIndex = 0;
		LOD0->LODIndex = 0;
		
		LODList.Push(LOD0);
		CurrentLOD = LOD0;
		
		MeshComponent->CreateMeshSection(LOD0->MeshComponentIndex, LOD0->MeshBuildResult.Target);
		MeshComponent->Invalidate();
	}
	else
	{
		CurrentLOD->MeshBuildResult = NewMeshBuildResult;
		MeshComponent->ReplaceMeshSection(CurrentLOD->MeshComponentIndex, CurrentLOD->MeshBuildResult.Target);
		MeshComponent->Invalidate();
	}

	bMeshGenerated = true;
}

void AOpenLandMeshActor::ModifyMesh()
{
	bModifyMeshIsInProgress = false;
	
	if (bRunGpuVertexModifiers)
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	else
		PolygonMesh->RegisterGpuVertexModifier({});

	PolygonMesh->ModifyVertices(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle});
	MeshComponent->UpdateMeshSection(0);
}

void AOpenLandMeshActor::ModifyMeshAsync()
{
	if (bModifyMeshIsInProgress)
	{
		bNeedToModifyMesh = true;
		return;
	}

	bModifyMeshIsInProgress = true;
	bNeedToModifyMesh = false;
	
	if (bRunGpuVertexModifiers)
	{
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	}
	else
	{
		PolygonMesh->RegisterGpuVertexModifier({});
	}

	UE_LOG(LogTemp, Warning, TEXT("Start Modifying"));
	auto AfterModifiedMesh = [this]()
	{

		UE_LOG(LogTemp, Warning, TEXT("Done!"));
		MeshComponent->UpdateMeshSection(0);
		bModifyMeshIsInProgress = false;

		if (bNeedToModifyMesh)
		{
			ModifyMeshAsync();
		}
	};
	PolygonMesh->ModifyVerticesAsync(this, CurrentLOD->MeshBuildResult, {GetWorld()->RealTimeSeconds, SmoothNormalAngle}, AfterModifiedMesh);
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

void AOpenLandMeshActor::BuildMeshAsync(TFunction<void()> Callback)
{
	SetMaterial(Material);
	
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
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	}
	else
	{
		PolygonMesh->RegisterGpuVertexModifier({});
	}

	const FOpenLandPolygonMeshBuildOptions BuildMeshOptions = {
		SubDivisions,
		SmoothNormalAngle
	};
	
	PolygonMesh->BuildMeshAsync(this, BuildMeshOptions, [this, Callback](FOpenLandPolygonMeshBuildResult Result)
	{
		Result.Target->bEnableCollision = bEnableCollision;
		Result.Target->bUseAsyncCollisionCooking = bUseAsyncCollisionCooking;

		if (CurrentLOD == nullptr)
		{
			TSharedPtr<FLODInfo> LOD0 = MakeShared<FLODInfo>();
            LOD0->MeshBuildResult = Result;
            LOD0->MeshComponentIndex = 0;
            LOD0->LODIndex = 0;
		
            LODList.Push(LOD0);
            CurrentLOD = LOD0;
		
            MeshComponent->CreateMeshSection(LOD0->MeshComponentIndex, LOD0->MeshBuildResult.Target);
            MeshComponent->Invalidate();
        }
        else
        {
            CurrentLOD->MeshBuildResult = Result;
            MeshComponent->ReplaceMeshSection(CurrentLOD->MeshComponentIndex, CurrentLOD->MeshBuildResult.Target);
            MeshComponent->Invalidate();
        }
		
		if (Callback != nullptr)
			Callback();
	});

	bMeshGenerated = true;
}

void AOpenLandMeshActor::SetMaterial(UMaterialInterface* InputMaterial)
{
	Material = InputMaterial;
	MeshComponent->SetMaterial(0, Material);
}

void AOpenLandMeshActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (!bMeshGenerated)
		BuildMesh();
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