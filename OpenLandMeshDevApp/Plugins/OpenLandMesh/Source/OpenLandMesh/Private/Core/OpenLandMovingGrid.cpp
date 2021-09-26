#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;

	RootGrid = MakeShared<FOpenLandGrid>();
	FOpenLandGridBuildInfo BuildInfo;
	BuildInfo.RootCell = {0, 0};
	BuildInfo.Size = { 200, 200 };
	BuildInfo.CellWidth = CurrentBuildOptions.CellWidth;
	BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth*2;

	BuildInfo.HoleRootCell = {50, 50};
	BuildInfo.HoleSize = {40, 40};
	RootGrid->Build(BuildInfo);

	GridRenderer = MakeShared<FOpenLandGridRenderer>();
	MeshInfo = GridRenderer->Initialize(RootGrid);
	//GridRenderer->ReCenter({0, 0, 0});

	if (MeshSectionIndex < 0)
	{
		MeshSectionIndex = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(MeshSectionIndex, MeshInfo);
	} else
	{
		MeshComponent->ReplaceMeshSection(MeshSectionIndex, MeshInfo);
	}

	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter) const
{
	if (MeshSectionIndex < 0)
	{
		return;
	}
	
	if (MeshInfo->IsLocked())
	{
		return;
	}

	// TODO: Get a list of triangles which has changed instead. So, we can update only those
	const FOpenLandGridRendererChangedInfo ChangedInfo = GridRenderer->ReCenter(NewCenter);
	if (ChangedInfo.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(MeshSectionIndex, ChangedInfo.ChangedTriangles);
	}
}
