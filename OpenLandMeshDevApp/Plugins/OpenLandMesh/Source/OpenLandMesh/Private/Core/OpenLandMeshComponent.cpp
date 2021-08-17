// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Core/OpenLandMeshComponent.h"
#include "Core/OpenLandMeshSceneProxy.h"
#include "PhysicsEngine/PhysicsSettings.h"

void UOpenLandMeshComponent::AddCollisionConvexMesh(TArray<FVector> ConvexVerts)
{
	if (ConvexVerts.Num() >= 4)
	{
		// New element
		FKConvexElem NewConvexElem;
		// Copy in vertex info
		NewConvexElem.VertexData = ConvexVerts;
		// Update bounding box
		NewConvexElem.ElemBox = FBox(NewConvexElem.VertexData);
		// Add to array of convex elements
		CollisionConvexElems.Add(NewConvexElem);
		// Refresh collision
		SetupCollisions(false);
	}
}

void UOpenLandMeshComponent::ClearCollisionConvexMeshes()
{
	// Empty simple collision info
	CollisionConvexElems.Empty();
	// Refresh collision
	SetupCollisions(false);
}

void UOpenLandMeshComponent::SetCollisionConvexMeshes(const TArray<TArray<FVector>>& ConvexMeshes)
{
	CollisionConvexElems.Reset();

	// Create element for each convex mesh
	for (int32 ConvexIndex = 0; ConvexIndex < ConvexMeshes.Num(); ConvexIndex++)
	{
		FKConvexElem NewConvexElem;
		NewConvexElem.VertexData = ConvexMeshes[ConvexIndex];
		NewConvexElem.ElemBox = FBox(NewConvexElem.VertexData);

		CollisionConvexElems.Add(NewConvexElem);
	}

	SetupCollisions(false);
}

bool UOpenLandMeshComponent::GetPhysicsTriMeshData(FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	int32 VertexBase = 0; // Base vertex index for current section

	// See if we should copy UVs
	bool bCopyUVs = UPhysicsSettings::Get()->bSupportUVFromHitResults;
	if (bCopyUVs)
		CollisionData->UVs.AddZeroed(1); // only one UV channel

	// For each section..
	for (int32 SectionIdx = 0; SectionIdx < MeshSections.Num(); SectionIdx++)
	{
		const FSimpleMeshInfoPtr Section = MeshSections[SectionIdx];
		// Do we have collision enabled?
		if (Section->bEnableCollision)
		{
			// Copy vert data
			for (size_t VertIdx = 0; VertIdx < Section->Vertices.Length(); VertIdx++)
			{
				CollisionData->Vertices.Add(Section->Vertices.Get(VertIdx).Position);

				// Copy UV if desired
				if (bCopyUVs)
					CollisionData->UVs[0].Add(Section->Vertices.Get(VertIdx).UV0);
			}

			// Copy triangle data
			const int32 NumTriangles = Section->Triangles.Length();
			for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
			{
				// Need to add base offset for indices
				const FOpenLandMeshTriangle MeshTriangle = Section->Triangles.Get(TriIdx);
				FTriIndices Triangle;
				Triangle.v0 = MeshTriangle.T0 + VertexBase;
				Triangle.v1 = MeshTriangle.T1 + VertexBase;
				Triangle.v2 = MeshTriangle.T2 + VertexBase;
				CollisionData->Indices.Add(Triangle);

				// Also store material info
				CollisionData->MaterialIndices.Add(SectionIdx);
			}

			// Remember the base index that new verts will be added from in next section
			VertexBase = CollisionData->Vertices.Num();
		}
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;

	return true;
}

bool UOpenLandMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	for (int32 Index = 0; Index < MeshSections.Num(); Index++)
	{
		const FSimpleMeshInfoPtr Section = MeshSections[Index];
		if (Section->Triangles.Length() >= 0 && Section->bEnableCollision)
			return true;
	}

	return false;
}

FPrimitiveSceneProxy* UOpenLandMeshComponent::CreateSceneProxy()
{
	//return nullptr;
	return new FOpenLandMeshSceneProxy(this);
}

UBodySetup* UOpenLandMeshComponent::GetBodySetup()
{
	CreateSimpleMeshBodySetup();
	return SimpleMeshBodySetup;
}

void UOpenLandMeshComponent::InvalidateRendering()
{
	MarkRenderStateDirty();
}

UMaterialInterface* UOpenLandMeshComponent::GetMaterialFromCollisionFaceIndex(
	int32 FaceIndex, int32& SectionIndex) const
{
	UMaterialInterface* Result = nullptr;
	SectionIndex = 0;

	if (FaceIndex >= 0)
	{
		// Look for element that corresponds to the supplied face
		int32 TotalFaceCount = 0;
		for (int32 SectionIdx = 0; SectionIdx < MeshSections.Num(); SectionIdx++)
		{
			const FSimpleMeshInfoPtr Section = MeshSections[SectionIdx];
			const int32 NumFaces = Section->Triangles.Length();
			TotalFaceCount += NumFaces;

			if (FaceIndex < TotalFaceCount)
			{
				// Grab the material
				Result = GetMaterial(SectionIdx);
				SectionIndex = SectionIdx;
				break;
			}
		}
	}

	return Result;
}

int32 UOpenLandMeshComponent::GetNumMaterials() const
{
	return MeshSections.Num();
}

void UOpenLandMeshComponent::PostLoad()
{
	Super::PostLoad();

	if (SimpleMeshBodySetup && IsTemplate())
		SimpleMeshBodySetup->SetFlags(RF_Public | RF_ArchetypeObject);
}

void UOpenLandMeshComponent::CreateMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo)
{
	if (SectionIndex < MeshSections.Num())
	{
		checkf(false, TEXT("It's not possible to create an already created mesh section with index: %d"), SectionIndex);
	}

	MeshSections.SetNum(SectionIndex + 1);
	MeshSections[SectionIndex] = MeshInfo;

	// Here we are Freezing the mesh info
	// Only the values of vertices can be changed
	MeshInfo->Freeze();

	UpdateLocalBounds(); // Update overall bounds
}

void UOpenLandMeshComponent::ReplaceMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo)
{
	if (SectionIndex >= MeshSections.Num())
	{
		checkf(false, TEXT("There is no existing mesh section with the index: %d"), SectionIndex);
	}

	MeshSections[SectionIndex] = MeshInfo;

	// Here we are Freezing the mesh info
	// Only the values of vertices can be changed
	MeshInfo->Freeze();

	UpdateLocalBounds(); // Update overall bounds
}

void UOpenLandMeshComponent::UpdateCollisionMesh()
{
	TArray<FVector> CollisionPositions;

	// We have one collision mesh for all sections, so need to build array of _all_ positions
	for (int32 Index = 0; Index < MeshSections.Num(); Index++)
	{
		const FSimpleMeshInfoPtr CollisionSection = MeshSections[Index];
		// If section has collision, copy it
		if (CollisionSection->bEnableCollision )
		{
			for (size_t VertIdx = 0; VertIdx < CollisionSection->Vertices.Length(); VertIdx++)
			{
				if (CollisionSection->bSectionVisible)
				{
					CollisionPositions.Add(CollisionSection->Vertices.Get(VertIdx).Position);
				} else
				{
					CollisionPositions.Add(FVector(0, 0, -9999999));
				}
			}
		}
	}

	// Pass new positions to trimesh
	BodyInstance.UpdateTriMeshVertices(CollisionPositions);
}

void UOpenLandMeshComponent::UpdateMeshSection(int32 SectionIndex)
{
	// We cannot update not existing mesh section.
	// TODO: May be we need to throw
	if (SectionIndex >= MeshSections.Num())
		return;

	//MeshSections[SectionIndex] = MeshInfo;
	FSimpleMeshInfoPtr MeshSection = MeshSections[SectionIndex];

	// If we have collision enabled on this section, update that too
	if (MeshSection->bEnableCollision)
	{
		UpdateCollisionMesh();
	}

	// If we have a valid proxy and it is not pending recreation
	if (SceneProxy && !IsRenderStateDirty())
	{
		// Enqueue command to send to render thread
		FOpenLandMeshSceneProxy* ProcMeshSceneProxy = static_cast<FOpenLandMeshSceneProxy*>(SceneProxy);
		MeshSection->Lock();
		ENQUEUE_RENDER_COMMAND(FProcMeshSectionUpdate)
		([ProcMeshSceneProxy, SectionIndex, MeshSection](FRHICommandListImmediate& RHICmdList)
		{
			ProcMeshSceneProxy->UpdateSection_RenderThread(SectionIndex, MeshSection);
			MeshSection->UnLock();
		});
	}

	UpdateLocalBounds(); // Update overall bounds
	MarkRenderTransformDirty(); // Need to send new bounds to render thread
}

int32 UOpenLandMeshComponent::NumMeshSections()
{
	return MeshSections.Num();
}

void UOpenLandMeshComponent::UpdateMeshSectionVisibility(int32 SectionIndex)
{
	if (SectionIndex < NumMeshSections())
	{
		
		// Set game thread state
		bool bVisibility = MeshSections[SectionIndex]->bSectionVisible;

		// update the render thread
		if (SceneProxy)
		{
			// Enqueue command to modify render thread info
			FOpenLandMeshSceneProxy* ProcMeshSceneProxy = static_cast<FOpenLandMeshSceneProxy*>(SceneProxy);
			ENQUEUE_RENDER_COMMAND(FProcMeshSectionVisibilityUpdate)(
				[ProcMeshSceneProxy, SectionIndex, bVisibility](FRHICommandListImmediate& RHICmdList)
				{
					ProcMeshSceneProxy->SetSectionVisibility_RenderThread(SectionIndex, bVisibility);
				});
		}
	}
}

void UOpenLandMeshComponent::UpdateLocalBounds()
{
	FBox LocalBox(ForceInit);
	for (int32 Index = 0; Index < MeshSections.Num(); Index++)
		LocalBox += MeshSections[Index]->BoundingBox;

	LocalBounds = LocalBox.IsValid
		              ? FBoxSphereBounds(LocalBox)
		              : FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0); // fallback to reset box sphere bounds

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

FBoxSphereBounds UOpenLandMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}

UBodySetup* UOpenLandMeshComponent::CreateBodySetupHelper()
{
	// The body setup in a template needs to be public since the property is Instanced and thus is the archetype of the instance meaning there is a direct reference
	UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None,
	                                                 (IsTemplate() ? RF_Public | RF_ArchetypeObject : RF_NoFlags));
	NewBodySetup->BodySetupGuid = FGuid::NewGuid();

	NewBodySetup->bGenerateMirroredCollision = false;
	NewBodySetup->bDoubleSidedGeometry = true;
	NewBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	return NewBodySetup;
}

void UOpenLandMeshComponent::CreateSimpleMeshBodySetup()
{
	if (SimpleMeshBodySetup == nullptr)
		SimpleMeshBodySetup = CreateBodySetupHelper();
}

void UOpenLandMeshComponent::SetupCollisions(bool bUseAsyncCollisionCooking)
{
	UWorld* World = GetWorld();

	if (bUseAsyncCollisionCooking)
	{
		// Abort all previous ones still standing
		for (UBodySetup* OldBody : AsyncBodySetupQueue)
			OldBody->AbortPhysicsMeshAsyncCreation();

		AsyncBodySetupQueue.Add(CreateBodySetupHelper());
	}
	else
	{
		AsyncBodySetupQueue.Empty();
		CreateSimpleMeshBodySetup();
	}

	UBodySetup* UseBodySetup = bUseAsyncCollisionCooking ? AsyncBodySetupQueue.Last() : SimpleMeshBodySetup;

	// Fill in simple collision convex elements
	UseBodySetup->AggGeom.ConvexElems = CollisionConvexElems;

	// Set trace flag
	UseBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	if (bUseAsyncCollisionCooking)
		UseBodySetup->CreatePhysicsMeshesAsync(
			FOnAsyncPhysicsCookFinished::CreateUObject(this, &UOpenLandMeshComponent::FinishPhysicsAsyncCook,
			                                           UseBodySetup));
	else
	{
		// New GUID as collision has changed
		UseBodySetup->BodySetupGuid = FGuid::NewGuid();
		// Also we want cooked data for this
		UseBodySetup->bHasCookedCollisionData = true;
		UseBodySetup->InvalidatePhysicsData();
		UseBodySetup->CreatePhysicsMeshes();
		RecreatePhysicsState();
	}
}

void UOpenLandMeshComponent::FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup)
{
	TArray<UBodySetup*> NewQueue;
	NewQueue.Reserve(AsyncBodySetupQueue.Num());

	int32 FoundIdx;
	if (AsyncBodySetupQueue.Find(FinishedBodySetup, FoundIdx))
	{
		if (bSuccess)
		{
			//The new body was found in the array meaning it's newer so use it
			SimpleMeshBodySetup = FinishedBodySetup;
			RecreatePhysicsState();

			//remove any async body setups that were requested before this one
			for (int32 AsyncIdx = FoundIdx + 1; AsyncIdx < AsyncBodySetupQueue.Num(); ++AsyncIdx)
				NewQueue.Add(AsyncBodySetupQueue[AsyncIdx]);

			AsyncBodySetupQueue = NewQueue;
		}
		else
			AsyncBodySetupQueue.RemoveAt(FoundIdx);
	}
}
