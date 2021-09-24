// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Core/OpenLandMeshSceneProxy.h"
#include "Math/Color.h"
#include "Engine.h"

static void ConvertProcMeshToDynMeshVertex(FDynamicMeshVertex& Vert, const FOpenLandMeshVertex& ProcVert)
{
	Vert.Position = ProcVert.Position;
	Vert.Color = ProcVert.Color;
	Vert.TextureCoordinate[0] = ProcVert.UV0;
	Vert.TextureCoordinate[1] = ProcVert.UV1;
	Vert.TextureCoordinate[2] = ProcVert.UV2;
	Vert.TextureCoordinate[3] = ProcVert.UV3;
	Vert.TangentX = ProcVert.Tangent.TangentX;
	Vert.TangentZ = ProcVert.Normal;
	Vert.TangentZ.Vector.W = ProcVert.Tangent.bFlipTangentY ? -127 : 127;
}

FOpenLandMeshSceneProxy::FOpenLandMeshSceneProxy(UOpenLandMeshComponent* Component)
	: FPrimitiveSceneProxy(Component)
	  , BodySetup(Component->GetBodySetup())
	  , MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	if (ProxySections.Num() < Component->NumMeshSections())
		ProxySections.SetNum(Component->NumMeshSections());

	// Move Game Thread MeshSections into the Render Thread
	for (int SectionId = 0; SectionId < Component->NumMeshSections(); SectionId++)
	{
		FOpenLandMeshInfoPtr SrcSection = Component->MeshSections[SectionId];
		if (SrcSection->Triangles.Length() > 0 && SrcSection->Vertices.Length() > 0)
		{
			FOpenLandMeshProxySection* NewSection = new FOpenLandMeshProxySection(GetScene().GetFeatureLevel());

			// Copy data from vertex buffer
			const int32 NumVerts = SrcSection->Vertices.Length();

			// Allocate verts

			TArray<FDynamicMeshVertex> Vertices;
			Vertices.SetNumUninitialized(NumVerts);
			// Copy verts
			for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
			{
				const FOpenLandMeshVertex& ProcVert = SrcSection->Vertices.Get(VertIdx);
				FDynamicMeshVertex& Vert = Vertices[VertIdx];
				ConvertProcMeshToDynMeshVertex(Vert, ProcVert);
			}

			// Copy index buffer
			const int32 NumIndices = SrcSection->Triangles.Length() * 3;
			NewSection->IndexBuffer.Indices.SetNum(NumIndices);
			for (size_t Index = 0; Index < SrcSection->Triangles.Length(); Index++)
			{
				const FOpenLandMeshTriangle Triangle = SrcSection->Triangles.Get(Index);
				NewSection->IndexBuffer.Indices[Index * 3 + 0] = Triangle.T0;
				NewSection->IndexBuffer.Indices[Index * 3 + 1] = Triangle.T1;
				NewSection->IndexBuffer.Indices[Index * 3 + 2] = Triangle.T2;
			}

			NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, Vertices, 4);

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->VertexFactory);

			NewSection->Material = Component->GetMaterial(SectionId);
			if (NewSection->Material == nullptr)
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);

			// Copy visibility info
			NewSection->bSectionVisible = SrcSection->bSectionVisible;

			// Save ref to new section
			ProxySections[SectionId] = NewSection;
		}
	}
}

FOpenLandMeshSceneProxy::~FOpenLandMeshSceneProxy()
{
	for (auto ProxySection : ProxySections)
		if (ProxySection != nullptr)
		{
			ProxySection->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			ProxySection->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			ProxySection->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			ProxySection->IndexBuffer.ReleaseResource();
			ProxySection->VertexFactory.ReleaseResource();

			delete ProxySection;
		}
}

void FOpenLandMeshSceneProxy::SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility)
{
	check(IsInRenderingThread());
	if (SectionIndex < ProxySections.Num())
		ProxySections[SectionIndex]->bSectionVisible = bNewVisibility;
}

void FOpenLandMeshSceneProxy::UpdateGpuVertex(FOpenLandMeshProxySection* Section, int32 Index, FOpenLandMeshVertex NewVertex)
{
	FDynamicMeshVertex Vertex;
	ConvertProcMeshToDynMeshVertex(Vertex, NewVertex);
			
	Section->VertexBuffers.PositionVertexBuffer.VertexPosition(Index) = Vertex.Position;
	Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(Index, Vertex.TangentX.ToFVector(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector());
	Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(Index, 0, Vertex.TextureCoordinate[0]);
	Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(Index, 1, Vertex.TextureCoordinate[1]);
	Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(Index, 2, Vertex.TextureCoordinate[2]);
	Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(Index, 3, Vertex.TextureCoordinate[3]);
	Section->VertexBuffers.ColorVertexBuffer.VertexColor(Index) = Vertex.Color;
}

void FOpenLandMeshSceneProxy::UpdateGpuBuffers(FOpenLandMeshProxySection* Section) const
{
	{
		auto& VertexBuffer = Section->VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0,
		                                             VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(),
		                                             RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(),
		                VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}
	
	{
		auto& VertexBuffer = Section->VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0,
		                                             VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(),
		                                             RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(),
		                VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}
	
	{
		auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0,
		                                             VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHIUnlockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
	}
	
	{
		auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0,
		                                             VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHIUnlockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
	}
}

void FOpenLandMeshSceneProxy::UpdateSection_RenderThread(int32 SectionIndex, FOpenLandMeshInfoPtr const CpuSection)
{
	check(IsInRenderingThread());
	check(CpuSection != nullptr);
	check(SectionIndex < ProxySections.Num());
	check(ProxySections[SectionIndex] != nullptr)

	// Check we have data 
	if (CpuSection == nullptr)
	{
		return;
	}
	
	FOpenLandMeshProxySection* GpuSection = ProxySections[SectionIndex];
	const int32 NumVerts = CpuSection->Vertices.Length();

	// Iterate through vertex data, copying in new info
	for (int32 i = 0; i < NumVerts; i++)
	{
		UpdateGpuVertex(GpuSection, i, CpuSection->Vertices.Get(i));
	}

	UpdateGpuBuffers(GpuSection);
}

void FOpenLandMeshSceneProxy::UpdateSection_RenderThread(int32 SectionIndex, FOpenLandMeshInfoPtr const CpuSection,
	TArray<int32> UpdatedTriangles)
{
	check(IsInRenderingThread());
	check(CpuSection != nullptr);
	check(SectionIndex < ProxySections.Num());
	check(ProxySections[SectionIndex] != nullptr)

	// Check we have data 
	if (CpuSection == nullptr)
	{
		return;
	}
	
	FOpenLandMeshProxySection* GpuSection = ProxySections[SectionIndex];

	for (const int32 TriangleIndex: UpdatedTriangles)
	{
		const FOpenLandMeshTriangle CpuTriangle = CpuSection->Triangles.Get(TriangleIndex);

		UpdateGpuVertex(GpuSection, CpuTriangle.T0, CpuSection->Vertices.Get(CpuTriangle.T0));
		UpdateGpuVertex(GpuSection, CpuTriangle.T1, CpuSection->Vertices.Get(CpuTriangle.T1));
		UpdateGpuVertex(GpuSection, CpuTriangle.T2, CpuSection->Vertices.Get(CpuTriangle.T2));
	}

	UpdateGpuBuffers(GpuSection);
}

void FOpenLandMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                                     const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
                                                     FMeshElementCollector& Collector) const
{
	// Set up wireframe material (if needed)
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
	if (bWireframe)
	{
		WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
	}

	// Iterate over sections
	for (auto ProxySection : ProxySections)
		if (ProxySection != nullptr && ProxySection->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = bWireframe
				                                      ? WireframeMaterialInstance
				                                      : ProxySection->Material->GetRenderProxy();

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &ProxySection->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &ProxySection->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;

					bool bHasPrecomputedVolumetricLightmap;
					FMatrix PreviousLocalToWorld;
					int32 SingleCaptureIndex;
					bool bOutputVelocity;
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
						GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld,
						SingleCaptureIndex, bOutputVelocity);

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<
						FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(),
					                                  GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap,
					                                  DrawsVelocity(), bOutputVelocity);
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = ProxySection->IndexBuffer.Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = ProxySection->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);
				}
		}


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		if (VisibilityMap & (1 << ViewIndex))
			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
#endif
}

FPrimitiveViewRelevance FOpenLandMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;

	// This adds the support for all the shading & blending modes in materials
	// Including transparent, etc.
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	
	return Result;
}

bool FOpenLandMeshSceneProxy::CanBeOccluded() const
{
	return true;
}

uint32 FOpenLandMeshSceneProxy::GetMemoryFootprint(void) const
{
	return (sizeof(*this) + GetAllocatedSize());
}

uint32 FOpenLandMeshSceneProxy::GetAllocatedSize(void) const
{
	return (FPrimitiveSceneProxy::GetAllocatedSize());
}
