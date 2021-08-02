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

	if (!bAnimate)
		return;

	if (OriginalMeshInfo == nullptr)
		return;

	if (RenderingMeshInfo->IsLocked())
		return;

	if (bUseAsyncAnimations)
	{
		const bool bCanUpdate = PolygonMesh->ModifyVerticesAsync(this, OriginalMeshInfo, RenderingMeshInfo,
		                                                         GetWorld()->RealTimeSeconds, SmoothNormalAngle);
		if (bCanUpdate)
		{
			MeshComponent->UpdateMeshSection(0);
			OnAfterAnimations();
		}
	} else
	{
		PolygonMesh->ModifyVertices(this, OriginalMeshInfo, RenderingMeshInfo,
	                                                             GetWorld()->RealTimeSeconds, SmoothNormalAngle);
		MeshComponent->UpdateMeshSection(0);
		OnAfterAnimations();
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
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	else
		PolygonMesh->RegisterGpuVertexModifier({});

	FSimpleMeshInfoPtr NewMeshInfo = PolygonMesh->BuildMesh(this, SubDivisions, SmoothNormalAngle);
	//NewMeshInfo->bEnableCollision = false;
	const FSimpleMeshInfoPtr NewRenderingMeshInfo = NewMeshInfo->Clone();
	NewRenderingMeshInfo->bEnableCollision = bEnableCollision;
	NewRenderingMeshInfo->bUseAsyncCollisionCooking = bUseAsyncCollisionCooking;

	if (OriginalMeshInfo == nullptr)
	{
		MeshComponent->CreateMeshSection(0, NewRenderingMeshInfo);
		MeshComponent->Invalidate();
	}
	else
	{
		MeshComponent->ReplaceMeshSection(0, NewRenderingMeshInfo);
		MeshComponent->Invalidate();
	}

	OriginalMeshInfo = NewMeshInfo;
	RenderingMeshInfo = NewRenderingMeshInfo;
	bMeshGenerated = true;
}

void AOpenLandMeshActor::ModifyMesh()
{
	if (bRunGpuVertexModifiers)
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	else
		PolygonMesh->RegisterGpuVertexModifier({});

	PolygonMesh->ModifyVertices(this, OriginalMeshInfo, RenderingMeshInfo, GetWorld()->RealTimeSeconds,
	                            SmoothNormalAngle);
	MeshComponent->UpdateMeshSection(0);
}

void AOpenLandMeshActor::SetGPUScalarParameter(FName Name, float Value)
{
	for(FComputeMaterialParameter& Param: GpuVertexModifier.Parameters)
	{
		if (Param.Name == Name)
		{
			UE_LOG(LogTemp, Warning, TEXT("Set Value: %s - %f"), *(Name.ToString()), Value)
			Param.ScalarValue = Value;
			PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Create Value: %s - %f"), *(Name.ToString()), Value)
	GpuVertexModifier.Parameters.Push({
        Name,
        CMPT_SCALAR,
        Value
    });

	PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
}

void AOpenLandMeshActor::BuildMeshAsync(TFunction<void()> Callback)
{
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
		PolygonMesh->RegisterGpuVertexModifier(GpuVertexModifier);
	else
		PolygonMesh->RegisterGpuVertexModifier({});

	PolygonMesh->BuildMeshAsync(this, SubDivisions, SmoothNormalAngle, [this, Callback](FSimpleMeshInfoPtr NewMeshInfo)
	{
		const FSimpleMeshInfoPtr NewRenderingMeshInfo = NewMeshInfo->Clone();
		NewRenderingMeshInfo->bEnableCollision = bEnableCollision;
		NewRenderingMeshInfo->bUseAsyncCollisionCooking = bUseAsyncCollisionCooking;

		if (OriginalMeshInfo == nullptr)
		{
			MeshComponent->CreateMeshSection(0, NewRenderingMeshInfo);
			MeshComponent->Invalidate();
		}
		else
		{
			MeshComponent->ReplaceMeshSection(0, NewRenderingMeshInfo);
			MeshComponent->Invalidate();
		}

		OriginalMeshInfo = NewMeshInfo;
		RenderingMeshInfo = NewRenderingMeshInfo;

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