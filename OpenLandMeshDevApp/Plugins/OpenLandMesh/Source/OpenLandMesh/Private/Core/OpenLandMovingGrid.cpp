#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component)
{
	MeshComponent = Component;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;

	for (int32 LODIndex=0; LODIndex<3; LODIndex++)
	{
		FOpenLandMovingGridLOD CurrentLOD = FOpenLandMovingGridLOD::New();
		CurrentLOD.Index = LODIndex;
		FOpenLandGridBuildInfo BuildInfo;
		BuildInfo.RootCell = {0, 0};
		BuildInfo.Size = { 100, 100 };
		BuildInfo.CellWidth = CurrentBuildOptions.CellWidth * FMath::Pow(2, LODIndex);
		BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth * FMath::Pow(2, LODIndex + 1);

		FOpenLandMovingGridLOD* InnerLOD = LODIndex == 0? nullptr : &(LODs[LODIndex-1]);
		if (InnerLOD)
		{
			BuildInfo.HoleRootCell = InnerLOD->Grid->GetRootCell();
			BuildInfo.HoleSize = InnerLOD->Grid->GetSize() / 2;
		}

		CurrentLOD.Grid->Build(BuildInfo);
		CurrentLOD.InitializeMesh(MeshComponent);

		LODs.Push(CurrentLOD);
	}

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
