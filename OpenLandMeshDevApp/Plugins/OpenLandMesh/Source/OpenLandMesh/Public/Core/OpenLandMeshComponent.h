// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "Types/OpenLandMeshInfo.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "PhysicsEngine/ConvexElem.h"

#include "OpenLandMeshComponent.generated.h"

UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class OPENLANDMESH_API UOpenLandMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()

private:
	// properties
	FBoxSphereBounds LocalBounds;
	void UpdateLocalBounds();
	static uint32 VertexCounter;
	UPROPERTY()
	TArray<UBodySetup*> AsyncBodySetupQueue;

	// methods
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	UBodySetup* CreateBodySetupHelper();
	void CreateSimpleMeshBodySetup();
	void UpdateCollision(bool bUseAsyncCollisionCooking);
	void FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup);

public:

	// properties
	TArray<FSimpleMeshInfoPtr> MeshSections;

	// --- START INTERNAL METHODS & PROPERTIES ---

	/** Add simple collision convex to this component */
	UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh")
	void AddCollisionConvexMesh(TArray<FVector> ConvexVerts);

	/** Remove collision meshes from this component */
	UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh")
	void ClearCollisionConvexMeshes();

	/** Function to replace _all_ simple collision in one go */
	void SetCollisionConvexMeshes(const TArray<TArray<FVector>>& ConvexMeshes);

	//~ Begin Interface_CollisionDataProvider Interface
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override { return false; }
	//~ End Interface_CollisionDataProvider Interface

	/** 
	 *	Controls whether the complex (Per poly) geometry should be treated as 'simple' collision. 
	 *	Should be set to false if this component is going to be given simple collision and simulated.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Procedural Mesh")
	bool bUseComplexAsSimpleCollision = true;

	/** Collision data */
	UPROPERTY(Instanced)
	class UBodySetup* SimpleMeshBodySetup;

	/** Convex shapes used for simple collision */
	UPROPERTY()
	TArray<FKConvexElem> CollisionConvexElems;

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual class UBodySetup* GetBodySetup() override;
	virtual UMaterialInterface* GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface.

	// --- END INTERNAL METHODS & PROPERTIES ---

	// methods
	void CreateMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo);
	void ReplaceMeshSection(int32 SectionIndex, FSimpleMeshInfoPtr MeshInfo);
	void UpdateMeshSection(int32 SectionIndex);

	int32 NumMeshSections();
	void UpdateMeshSectionVisibility(int32 SectionIndex);

	void Invalidate();
};
