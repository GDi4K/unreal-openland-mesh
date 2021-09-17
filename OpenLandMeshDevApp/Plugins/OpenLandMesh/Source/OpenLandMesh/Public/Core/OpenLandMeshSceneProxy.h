// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once
#include "OpenLandMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "StaticMeshResources.h"
#include "DynamicMeshBuilder.h"
#include "LocalVertexFactory.h"
#include "PrimitiveSceneProxy.h"
#include "RenderResource.h"
#include "Materials/MaterialRelevance.h"
#include "PhysicsEngine/BodySetup.h"

class OPENLANDMESH_API FOpenLandMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FDynamicMeshIndexBuffer32 IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible;

#if RHI_RAYTRACING
	FRayTracingGeometry RayTracingGeometry;
#endif

	FOpenLandMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(nullptr)
		  , VertexFactory(InFeatureLevel, "FOpenLandMeshProxySection")
		  , bSectionVisible(true)
	{
	}
};

class FOpenLandMeshSceneProxy final : public FPrimitiveSceneProxy
{
private:
	TArray<FOpenLandMeshProxySection*> ProxySections;

	UBodySetup* BodySetup;
	FMaterialRelevance MaterialRelevance;

public:
	FOpenLandMeshSceneProxy(UOpenLandMeshComponent* Component);
	virtual ~FOpenLandMeshSceneProxy();

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	void SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility);
	void UpdateSection_RenderThread(int32 SectionIndex, FOpenLandMeshInfoPtr const SectionData, FOpenLandMeshComponentUpdateRange UpdateRange);

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
	                                    uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual bool CanBeOccluded() const override;
	virtual uint32 GetMemoryFootprint(void) const override;
	uint32 GetAllocatedSize(void) const;
};
