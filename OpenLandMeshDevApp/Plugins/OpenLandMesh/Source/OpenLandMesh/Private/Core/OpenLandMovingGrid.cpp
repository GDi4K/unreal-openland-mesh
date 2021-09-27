#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;

	// Inner Cell Creation
	FOpenLandGridBuildInfo BuildInfo;
	BuildInfo.RootCell = {0, 0};
	BuildInfo.Size = { 100, 100 };
	BuildInfo.CellWidth = CurrentBuildOptions.CellWidth;
	BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth*2;
	
	LOD0.Grid->Build(BuildInfo);

	LOD0.MeshInfo = LOD0.GridRenderer->Initialize(LOD0.Grid);

	if (LOD0.MeshSectionIndex < 0)
	{
		LOD0.MeshSectionIndex = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(LOD0.MeshSectionIndex, LOD0.MeshInfo);
	} else
	{
		MeshComponent->ReplaceMeshSection(LOD0.MeshSectionIndex, LOD0.MeshInfo);
	}
	
	// Outer Cell Creation
	FOpenLandGridBuildInfo BuildInfoOuter;
	BuildInfoOuter.RootCell = {0, 0};
	BuildInfoOuter.Size = { 100, 100 };
	BuildInfoOuter.CellWidth = CurrentBuildOptions.CellWidth *2;
	BuildInfoOuter.UpperCellWidth = CurrentBuildOptions.CellWidth*4;

	BuildInfoOuter.HoleRootCell = LOD0.Grid->GetRootCell();
	BuildInfoOuter.HoleSize = LOD0.Grid->GetSize() / 2;
	LOD1.Grid->Build(BuildInfoOuter);

	LOD1.MeshInfo = LOD1.GridRenderer->Initialize(LOD1.Grid);

	if (LOD1.MeshSectionIndex < 0)
	{
		LOD1.MeshSectionIndex = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(LOD1.MeshSectionIndex, LOD1.MeshInfo);
	} else
	{
		MeshComponent->ReplaceMeshSection(LOD1.MeshSectionIndex, LOD1.MeshInfo);
	}

	// Mesh Updates
	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();

	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter) const
{
	if (LOD0.MeshSectionIndex < 0)
	{
		return;
	}
	
	if (LOD0.MeshInfo->IsLocked())
	{
		return;
	}

	if (LOD1.MeshInfo->IsLocked())
	{
		return;
	}

	// Update Outer
	const FOpenLandGridRendererChangedInfo ChangedInfoOuter = LOD1.GridRenderer->ReCenter(NewCenter, LOD0.Grid->GetRootCell()/2);
	if (ChangedInfoOuter.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LOD1.MeshSectionIndex, ChangedInfoOuter.ChangedTriangles);
		return;
	}
	
	// Update Inner
	const FOpenLandGridRendererChangedInfo ChangedInfoInner = LOD0.GridRenderer->ReCenter(NewCenter);
	if (ChangedInfoInner.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LOD0.MeshSectionIndex, ChangedInfoInner.ChangedTriangles);
	}

	// Update Outer Hole
	const FOpenLandGridRendererChangedInfo ChangedInfoOuterHole = LOD1.GridRenderer->ChangeHoleRootCell(LOD0.Grid->GetRootCell()/2);
	if (ChangedInfoOuterHole.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LOD1.MeshSectionIndex, ChangedInfoOuterHole.ChangedTriangles);
	}
}
