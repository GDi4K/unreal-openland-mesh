#pragma once
#include "OpenLandMeshComponent.h"
#include "Utils/OpenLandGrid.h"
#include "Utils/OpenLandGridRenderer.h"
#include "Materials/MaterialInterface.h"

struct FOpenLandMovingGridUpdatingLOD
{
	int32 LODIndex = 0;
	TSharedPtr<FOpenLandGridRendererChangedInfo> ChangedInfo = nullptr;
};

struct FOpenLandMovingGridBuildOptions
{
	float CuspAngle = 60.0f;
	float CellWidth = 10.0f;
	int32 CellCount = 10;
	float UnitUVLenght = 100.0f;
	int32 MaxUVs = 10;
};

struct FOpenLandMovingGridLOD
{
	int32 Index = 0;
	FOpenLandMeshInfoPtr MeshInfo = nullptr;
	int32 MeshSectionIndex = -1;
	FOpenLandGridPtr Grid = nullptr;
	FOpenLandGridRendererPtr GridRenderer = nullptr;

	static FOpenLandMovingGridLOD New()
	{
		FOpenLandMovingGridLOD LOD;
		LOD.Grid = MakeShared<FOpenLandGrid>();
		LOD.GridRenderer = MakeShared<FOpenLandGridRenderer>();
		return LOD;
	}

	void InitializeMesh(UOpenLandMeshComponent* MeshComponent, int32 InputMeshSectionIndex)
	{
		MeshInfo = GridRenderer->Initialize(Grid);
		MeshInfo->bEnableCollision = Index == 0;
		MeshSectionIndex = InputMeshSectionIndex;

		const bool bHasMeshSection = MeshComponent->NumMeshSections() > MeshSectionIndex;
		if (bHasMeshSection)
		{
			MeshComponent->ReplaceMeshSection(MeshSectionIndex, MeshInfo);
		} else
		{
			MeshComponent->CreateMeshSection(MeshSectionIndex, MeshInfo);
		}
	}
};

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;
	
	TArray<FOpenLandMovingGridLOD> LODs = {};
	TArray<FOpenLandMovingGridUpdatingLOD> UpdatingLODs = {};
	FComputeMaterial VertexModifier = {};

	UObject* WorldContext = nullptr;

	int32 times = 0;

public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component, UObject* InputWorldContext);
	void SetVertexModifier(FComputeMaterial Material);
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePositionAsync(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;