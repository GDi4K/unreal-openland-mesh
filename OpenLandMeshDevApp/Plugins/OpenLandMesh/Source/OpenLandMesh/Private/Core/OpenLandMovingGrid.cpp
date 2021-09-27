#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;

	LODs.Push(FOpenLandMovingGridLOD::New());
	LODs.Push(FOpenLandMovingGridLOD::New());

	// Inner Cell Creation
	FOpenLandGridBuildInfo BuildInfo;
	BuildInfo.RootCell = {0, 0};
	BuildInfo.Size = { 100, 100 };
	BuildInfo.CellWidth = CurrentBuildOptions.CellWidth;
	BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth*2;
	
	LODs[0].Grid->Build(BuildInfo);
	LODs[0].InitializeMesh(MeshComponent);
	
	// Outer Cell Creation
	FOpenLandGridBuildInfo BuildInfoOuter;
	BuildInfoOuter.RootCell = {0, 0};
	BuildInfoOuter.Size = { 100, 100 };
	BuildInfoOuter.CellWidth = CurrentBuildOptions.CellWidth *2;
	BuildInfoOuter.UpperCellWidth = CurrentBuildOptions.CellWidth*4;

	BuildInfoOuter.HoleRootCell = LODs[0].Grid->GetRootCell();
	BuildInfoOuter.HoleSize = LODs[0].Grid->GetSize() / 2;
	LODs[1].Grid->Build(BuildInfoOuter);
	LODs[1].InitializeMesh(MeshComponent);

	// Mesh Updates
	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();

	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter) const
{
	for (const FOpenLandMovingGridLOD LOD: LODs)
	{
		if (LOD.MeshSectionIndex < 0)
		{
			return;
		}
		
		if (LOD.MeshInfo->IsLocked())
		{
			return;
		}
	}
	
	// Update Outer
	const FOpenLandGridRendererChangedInfo ChangedInfoOuter = LODs[1].GridRenderer->ReCenter(NewCenter, LODs[0].Grid->GetRootCell()/2);
	if (ChangedInfoOuter.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LODs[1].MeshSectionIndex, ChangedInfoOuter.ChangedTriangles);
		return;
	}
	
	// Update Inner
	const FOpenLandGridRendererChangedInfo ChangedInfoInner = LODs[0].GridRenderer->ReCenter(NewCenter);
	if (ChangedInfoInner.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LODs[0].MeshSectionIndex, ChangedInfoInner.ChangedTriangles);
	}

	// Update Outer Hole
	const FOpenLandGridRendererChangedInfo ChangedInfoOuterHole = LODs[1].GridRenderer->ChangeHoleRootCell(LODs[0].Grid->GetRootCell()/2);
	if (ChangedInfoOuterHole.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(LODs[1].MeshSectionIndex, ChangedInfoOuterHole.ChangedTriangles);
	}
}
