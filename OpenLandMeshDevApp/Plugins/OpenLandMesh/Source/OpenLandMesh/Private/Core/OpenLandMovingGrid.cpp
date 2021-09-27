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
	GridInner = MakeShared<FOpenLandGrid>();
	FOpenLandGridBuildInfo BuildInfo;
	BuildInfo.RootCell = {0, 0};
	BuildInfo.Size = { 100, 100 };
	BuildInfo.CellWidth = CurrentBuildOptions.CellWidth;
	BuildInfo.UpperCellWidth = CurrentBuildOptions.CellWidth*2;
	
	GridInner->Build(BuildInfo);

	GridRendererInner = MakeShared<FOpenLandGridRenderer>();
	MeshInfoInner = GridRendererInner->Initialize(GridInner);

	if (MeshSectionIndexInner < 0)
	{
		MeshSectionIndexInner = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(MeshSectionIndexInner, MeshInfoInner);
	} else
	{
		MeshComponent->ReplaceMeshSection(MeshSectionIndexInner, MeshInfoInner);
	}
	
	// Outer Cell Creation
	GridOuter = MakeShared<FOpenLandGrid>();
	FOpenLandGridBuildInfo BuildInfoOuter;
	BuildInfoOuter.RootCell = {0, 0};
	BuildInfoOuter.Size = { 100, 100 };
	BuildInfoOuter.CellWidth = CurrentBuildOptions.CellWidth *2;
	BuildInfoOuter.UpperCellWidth = CurrentBuildOptions.CellWidth*4;

	BuildInfoOuter.HoleRootCell = GridInner->GetRootCell();
	BuildInfoOuter.HoleSize = GridInner->GetSize() / 2;
	GridOuter->Build(BuildInfoOuter);

	GridRendererOuter = MakeShared<FOpenLandGridRenderer>();
	MeshInfoOuter = GridRendererOuter->Initialize(GridOuter);

	if (MeshSectionIndexOuter < 0)
	{
		MeshSectionIndexOuter = MeshComponent->NumMeshSections();
		MeshComponent->CreateMeshSection(MeshSectionIndexOuter, MeshInfoOuter);
	} else
	{
		MeshComponent->ReplaceMeshSection(MeshSectionIndexOuter, MeshInfoOuter);
	}

	// Mesh Updates
	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();

	MeshComponent->SetupCollisions(false);
	MeshComponent->InvalidateRendering();
}

void FOpenLandMovingGrid::UpdatePosition(FVector NewCenter) const
{
	if (MeshSectionIndexInner < 0)
	{
		return;
	}
	
	if (MeshInfoInner->IsLocked())
	{
		return;
	}

	if (MeshInfoOuter->IsLocked())
	{
		return;
	}

	// Update Outer
	const FOpenLandGridRendererChangedInfo ChangedInfoOuter = GridRendererOuter->ReCenter(NewCenter);
	if (ChangedInfoOuter.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(MeshSectionIndexOuter, ChangedInfoOuter.ChangedTriangles);
		return;
	}
	
	// Update Inner
	const FOpenLandGridRendererChangedInfo ChangedInfoInner = GridRendererInner->ReCenter(NewCenter);
	if (ChangedInfoInner.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(MeshSectionIndexInner, ChangedInfoInner.ChangedTriangles);
	}

	// Update Outer Hole
	const FOpenLandGridRendererChangedInfo ChangedInfoOuterHole = GridRendererOuter->ChangeHoleRootCell(GridInner->GetRootCell()/2);
	if (ChangedInfoOuterHole.ChangedTriangles.Num() > 0)
	{
		MeshComponent->UpdateMeshSection(MeshSectionIndexOuter, ChangedInfoOuterHole.ChangedTriangles);
	}
	
}
