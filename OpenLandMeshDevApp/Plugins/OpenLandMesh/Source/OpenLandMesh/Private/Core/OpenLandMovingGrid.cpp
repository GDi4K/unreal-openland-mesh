#include "Core/OpenLandMovingGrid.h"

#include "Core/OpenLandPolygonMesh.h"

FOpenLandMovingGrid::FOpenLandMovingGrid(UOpenLandMeshComponent* Component, UObject* InputWorldContext)
{
	MeshComponent = Component;
	WorldContext = InputWorldContext;
}

void FOpenLandMovingGrid::SetVertexModifier(FComputeMaterial Material)
{
	VertexModifier = Material;
}

void FOpenLandMovingGrid::Build(FOpenLandMovingGridBuildOptions BuildOptions)
{
	CurrentBuildOptions = BuildOptions;
	TArray<FOpenLandMovingGridLOD> NewLODs = {};
	
	for (int32 LODIndex=0; LODIndex<20; LODIndex++)
	{
		FOpenLandMovingGridLOD CurrentLOD = FOpenLandMovingGridLOD::New();
		CurrentLOD.Index = LODIndex;
		FOpenLandGridBuildInfo BuildInfo;
		BuildInfo.RootCell = {-50, -50};
		BuildInfo.Size = { 100, 100 };
		BuildInfo.CellWidth = CurrentBuildOptions.CellWidth * FMath::Pow(2, LODIndex);
		BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth * FMath::Pow(2, LODIndex + 1);

		FOpenLandMovingGridLOD* InnerLOD = LODIndex == 0? nullptr : &(NewLODs[LODIndex-1]);
		if (InnerLOD)
		{
			BuildInfo.HoleRootCell = InnerLOD->Grid->GetRootCell() / 2;
			BuildInfo.HoleSize = InnerLOD->Grid->GetSize() / 2;
		}

		CurrentLOD.Grid->Build(BuildInfo);
		CurrentLOD.GridRenderer->SetGpuVertexModifier(VertexModifier, WorldContext);
		CurrentLOD.InitializeMesh(MeshComponent, LODIndex);

		NewLODs.Push(CurrentLOD);
	}

	// Mesh Updates
	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();

	LODs = NewLODs;
}

void FOpenLandMovingGrid::UpdatePositionAsync(FVector NewCenter)
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

	if (UpdatingLODs.Num() > 0)
	{
		bool bAllCompleted = true;
		
		for (FOpenLandMovingGridUpdatingLOD& UpdatingLOD: UpdatingLODs)
		{
			if (UpdatingLOD.ChangedInfo != nullptr)
			{
				continue;
			}
			
			const FOpenLandMovingGridLOD LOD = LODs[UpdatingLOD.LODIndex];
			UpdatingLOD.ChangedInfo = LOD.GridRenderer->CheckStatus();
			bAllCompleted = bAllCompleted && UpdatingLOD.ChangedInfo != nullptr;
		}

		if (!bAllCompleted)
		{
			return;
		}
		
		for (const FOpenLandMovingGridUpdatingLOD UpdatingLOD: UpdatingLODs)
        {
        	const FOpenLandMovingGridLOD LOD = LODs[UpdatingLOD.LODIndex];
        	MeshComponent->UpdateMeshSection(LOD.MeshSectionIndex, UpdatingLOD.ChangedInfo->ChangedTriangles);
        }

		UpdatingLODs = {};
		times ++;
		return;
	}


	if (times > 0)
	{
		for (int32 LODIndex=LODs.Num() -1; LODIndex >= 0; LODIndex --)
		{
			const FOpenLandMovingGridLOD CurrentLOD = LODs[LODIndex];
			const FOpenLandMovingGridLOD* InnerLOD = LODIndex == 0? nullptr : &(LODs[LODIndex - 1]);
			const FOpenLandMovingGridLOD* OuterLOD = LODIndex == LODs.Num() -1 ? nullptr : &(LODs[LODIndex + 1]);
			
			// Apply Recenter Logic
			bool bHasChanges = false;
			if (InnerLOD)
			{
				bHasChanges = CurrentLOD.GridRenderer->StartReCenter(NewCenter, InnerLOD->Grid->GetRootCell().ToVector2D()/2);
			}
			else
			{
				bHasChanges = CurrentLOD.GridRenderer->StartReCenter(NewCenter);
			}
			
			
			if (!bHasChanges)
			{
				continue;
			}
			
			UpdatingLODs.Push({
				LODIndex,
				nullptr
			});
			
			// Update Outer Grid Hole
			if (OuterLOD)
			{
				const bool bHasOuterChanges = OuterLOD->GridRenderer->StartChangeHoleRootCell(CurrentLOD.Grid->GetRootCell().ToVector2D()/2);
				if (bHasOuterChanges)
				{
					UpdatingLODs.Push({
						OuterLOD->Index,
						nullptr
					});
				}
			}
			
			return;
		}

		return;
	}
	
	for (int32 LODIndex=LODs.Num() -1; LODIndex >= 0; LODIndex --)
	{
		const FOpenLandMovingGridLOD CurrentLOD = LODs[LODIndex];
		const FOpenLandMovingGridLOD* InnerLOD = LODIndex == 0? nullptr : &(LODs[LODIndex - 1]);
		const FOpenLandMovingGridLOD* OuterLOD = LODIndex == LODs.Num() -1 ? nullptr : &(LODs[LODIndex + 1]);

		// Apply Recenter Logic
		bool bHasChanges = false;
		if (InnerLOD)
		{
			bHasChanges = CurrentLOD.GridRenderer->StartReCenter(NewCenter, InnerLOD->Grid->GetRootCell().ToVector2D()/2);
		}
		else
		{
			bHasChanges = CurrentLOD.GridRenderer->StartReCenter(NewCenter);
		}


		if (!bHasChanges)
		{
			continue;
		}
		
		UpdatingLODs.Push({
			LODIndex,
			nullptr
		});

		// Update Outer Grid Hole
		if (OuterLOD)
		{
			const bool bHasOuterChanges = OuterLOD->GridRenderer->StartChangeHoleRootCell(CurrentLOD.Grid->GetRootCell().ToVector2D()/2);
			if (bHasOuterChanges)
			{
				UpdatingLODs.Push({
					OuterLOD->Index,
					nullptr
				});
			}
		}

		return;
	}
}
