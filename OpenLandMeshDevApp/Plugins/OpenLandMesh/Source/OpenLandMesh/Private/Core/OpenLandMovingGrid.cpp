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

	for (int32 LODIndex=LODs.Num() -1; LODIndex >= 0; LODIndex --)
	{
		FOpenLandMovingGridLOD CurrentLOD = LODs[LODIndex];
		const FOpenLandMovingGridLOD* InnerLOD = LODIndex == 0? nullptr : &(LODs[LODIndex - 1]);
		const FOpenLandMovingGridLOD* OuterLOD = LODIndex == LODs.Num() -1 ? nullptr : &(LODs[LODIndex + 1]);

		FOpenLandGridRendererChangedInfo ChangedInfo;

		// Apply Recenter Logic
		if (InnerLOD)
		{
			ChangedInfo = CurrentLOD.GridRenderer->ReCenter(NewCenter, InnerLOD->Grid->GetRootCell()/2);
		}
		else
		{
			ChangedInfo = CurrentLOD.GridRenderer->ReCenter(NewCenter);
		}

		const bool bMeshUpdated = ChangedInfo.ChangedTriangles.Num() > 0;
		if (bMeshUpdated)
		{
			MeshComponent->UpdateMeshSection(CurrentLOD.MeshSectionIndex, ChangedInfo.ChangedTriangles);
		}

		// Update Outer Grid Hole
		if (OuterLOD)
		{
			const FOpenLandGridRendererChangedInfo ChangedInfoHole = OuterLOD->GridRenderer->ChangeHoleRootCell(CurrentLOD.Grid->GetRootCell()/2);
			if (ChangedInfoHole.ChangedTriangles.Num() > 0)
			{
				MeshComponent->UpdateMeshSection(OuterLOD->MeshSectionIndex, ChangedInfoHole.ChangedTriangles);
			}
		}

		if (bMeshUpdated)
		{
			// We need to return the tick & let it render
			return;
		}
	}
}
