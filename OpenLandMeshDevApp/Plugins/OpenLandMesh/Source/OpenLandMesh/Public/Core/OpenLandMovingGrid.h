#pragma once
#include "OpenLandMeshComponent.h"
#include "Utils/OpenLandGrid.h"
#include "Utils/OpenLandGridRenderer.h"

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

	void InitializeMesh(UOpenLandMeshComponent* MeshComponent)
	{
		MeshInfo = GridRenderer->Initialize(Grid);
		MeshInfo->bEnableCollision = Index == 0;

		if (MeshSectionIndex < 0)
		{
			MeshSectionIndex = MeshComponent->NumMeshSections();
			MeshComponent->CreateMeshSection(MeshSectionIndex, MeshInfo);
		} else
		{
			MeshComponent->ReplaceMeshSection(MeshSectionIndex, MeshInfo);
		}
	}
};

class FOpenLandMovingGrid
{
	UOpenLandMeshComponent* MeshComponent = nullptr;
	FOpenLandMovingGridBuildOptions CurrentBuildOptions;
	
	TArray<FOpenLandMovingGridLOD> LODs = {};
	TArray<FOpenLandMovingGridUpdatingLOD> UpdatingLODs = {};

	int32 times = 0;

public:
	FOpenLandMovingGrid(UOpenLandMeshComponent* Component);
	void Build(FOpenLandMovingGridBuildOptions BuildOptions);
	void UpdatePositionAsync(FVector NewCenter);
};

typedef TSharedPtr<FOpenLandMovingGrid> FOpenLandMovingGridPtr;