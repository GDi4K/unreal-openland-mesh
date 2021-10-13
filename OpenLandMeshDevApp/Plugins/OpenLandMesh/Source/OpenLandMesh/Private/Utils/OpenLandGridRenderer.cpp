#include "Utils/OpenLandGridRenderer.h"

#include "Compute/OpenLandThreading.h"
#include "Core/OpenLandPolygonMesh.h"

FOpenLandGridRenderer::FOpenLandGridRenderer()
{
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildCell(FOpenLandGridCell Cell) const
{
	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	
	FVector CellPos = FVector(Cell.X, Cell.Y, 0) * Grid->GetCellWidth();
		
	FVector A = CellPos + FVector(0, 0, 0) * Grid->GetCellWidth();
	FVector B = CellPos + FVector(0, 1, 0) * Grid->GetCellWidth();
	FVector C = CellPos + FVector(1, 1, 0) * Grid->GetCellWidth();
	FVector D = CellPos + FVector(1, 0, 0) * Grid->GetCellWidth();

	FVector2D UVRoot = Cell.ToVector2D() * UVCellWidth;
			
	FOpenLandMeshVertex MA = {ApplyCpuVertexModifier(A), UVRoot + FVector2D(0, 0) * UVCellWidth};
	FOpenLandMeshVertex MB = {ApplyCpuVertexModifier(B), UVRoot + FVector2D(0, 1) * UVCellWidth};
	FOpenLandMeshVertex MC = {ApplyCpuVertexModifier(C), UVRoot + FVector2D(1, 1) * UVCellWidth};
	FOpenLandMeshVertex MD = {ApplyCpuVertexModifier(D), UVRoot + FVector2D(1, 0) * UVCellWidth};

	FOpenLandMeshVertex T0_1 = MA;
	FOpenLandMeshVertex T0_2 = MB;
	FOpenLandMeshVertex T0_3 = MC;
	FOpenLandPolygonMesh::BuildFaceTangents(T0_1, T0_2, T0_3);

	FOpenLandMeshVertex T1_1 = MA;
	FOpenLandMeshVertex T1_2 = MC;
	FOpenLandMeshVertex T1_3 = MD;
	FOpenLandPolygonMesh::BuildFaceTangents(T1_1, T1_2, T1_3);

	return {
		T0_1, T0_2, T0_3,
		T1_1, T1_2, T1_3
	};
}

TOpenLandArray<FOpenLandMeshVertex> FOpenLandGridRenderer::BuildEdgeCell(FOpenLandGridCell Cell) const
{
	// TODO: Get UVWidth from outside
	const float UVCellWidth = Grid->GetCellWidth() / 100.0;
	const FVector2D VectorFromHoleMid = Cell.ToVector2D() - (Grid->GetHoleRootCell().ToVector2D() + Grid->GetHoleSize().ToVector2D() / 2.0f);
	// TODO: Try to get rid of Atan and simply use the tangent value.
	const float Angle = FMath::Atan2(VectorFromHoleMid.Y, VectorFromHoleMid.X) + PI;

	TOpenLandArray<FOpenLandMeshVertex> Vertices;

	const auto AddTri = [&Vertices](FOpenLandMeshVertex A, FOpenLandMeshVertex B, FOpenLandMeshVertex C)
	{
		FOpenLandPolygonMesh::BuildFaceTangents(A, B, C);
		Vertices.Push(A);
		Vertices.Push(B);
		Vertices.Push(C);
	};
	
	FVector CellPos = FVector(Cell.X, Cell.Y, 0) * Grid->GetCellWidth();
		
	FVector A = CellPos + FVector(0, 0, 0) * Grid->GetCellWidth();
	FVector B = CellPos + FVector(0, 1, 0) * Grid->GetCellWidth();
	FVector C = CellPos + FVector(1, 1, 0) * Grid->GetCellWidth();
	FVector D = CellPos + FVector(1, 0, 0) * Grid->GetCellWidth();
	

	FVector2D UVRoot = Cell.ToVector2D() * UVCellWidth;
			
	FOpenLandMeshVertex MA = {ApplyCpuVertexModifier(A), UVRoot + FVector2D(0, 0) * UVCellWidth};
	FOpenLandMeshVertex MB = {ApplyCpuVertexModifier(B), UVRoot + FVector2D(0, 1) * UVCellWidth};
	FOpenLandMeshVertex MC = {ApplyCpuVertexModifier(C), UVRoot + FVector2D(1, 1) * UVCellWidth};
	FOpenLandMeshVertex MD = {ApplyCpuVertexModifier(D), UVRoot + FVector2D(1, 0) * UVCellWidth};

	const FOpenLandGridCell EdgeRoot = Grid->GetHoleRootCell() - FOpenLandGridCell(1, 1);
	const FOpenLandGridCell EdgeRoot2 = EdgeRoot + FOpenLandGridCell(0, Grid->GetHoleSize().Y + 1);
	const FOpenLandGridCell EdgeRoot3 = EdgeRoot + FOpenLandGridCell(Grid->GetHoleSize().X + 1, Grid->GetHoleSize().Y + 1);
	const FOpenLandGridCell EdgeRoot4 = EdgeRoot + FOpenLandGridCell(Grid->GetHoleSize().X + 1, 0);

	if (Cell == EdgeRoot || Cell == EdgeRoot2 || Cell == EdgeRoot3 || Cell == EdgeRoot4)
	{
		AddTri(MA, MB, MC);
		AddTri(MA, MC, MD);
		// This is just to make the computing easier and all edge cell has 3 triangles.
		AddTri(MA, MC, MB);
		return Vertices;
	}

	if (Angle > PI + PI/4 && Angle <= 2*PI - PI/4)
	{
		FVector E = CellPos + FVector(0.5, 0, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyCpuVertexModifier(E), UVRoot + FVector2D(0.5, 0) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MC, MD);
	}
	else if(Angle > 2*PI - PI/4 || Angle <= + PI/4)
	{
		FVector E = CellPos + FVector(1, 0.5, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyCpuVertexModifier(E), UVRoot + FVector2D(1, 0.5) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MB, MC);
		AddTri(ME, MD, MA);
	} else if (Angle > PI/4 && Angle < PI - PI/4)
	{
		FVector E = CellPos + FVector(0.5, 1, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyCpuVertexModifier(E), UVRoot + FVector2D(0.5, 1) * UVCellWidth};

		AddTri(MA, MB, ME);
		AddTri(ME, MD, MA);
		AddTri(ME, MC, MD);
	} else
	{
		FVector E = CellPos + FVector(0, 0.5, 0) * Grid->GetCellWidth();
		FOpenLandMeshVertex ME = {ApplyCpuVertexModifier(E), UVRoot + FVector2D(0, 0.5) * UVCellWidth};

		AddTri(MA, ME, MD);
		AddTri(ME, MB, MC);
		AddTri(ME, MC, MD);
	}
	
	return Vertices;
}

void FOpenLandGridRenderer::ApplyCellChangesAsync(FOpenLandGridChangedCells ChangedCells)
{
	CurrentGridChangedCells = ChangedCells.Clone();
	CellGenInfo = FOpenLandGridRendererGeneratedCells::New(
		ChangedCells.CellsToAdd.Num(),
		ChangedCells.EdgeCellsToAdd.Num(),
		ChangedCells.ModifiedEdgeCells.Num()
	);

	ApplyVertexModifiersAsync(ChangedCells);
	
	MeshInfo->BoundingBox = Grid->GetBoundingBox();
}

void FOpenLandGridRenderer::ApplyVertexModifiersAsync(FOpenLandGridChangedCells ChangedCells)
{
	// Build Cells
	//	CPU Vertex Modifiers will run in this stage
	for (int32 CellIndex=0; CellIndex<ChangedCells.CellsToAdd.Num(); CellIndex++)
	{
		const FOpenLandGridRendererCellBuildResult Cell = {
			BuildCell(CurrentGridChangedCells->CellsToAdd[CellIndex])
		};
		CellGenInfo->GeneratedCells.Set(CellIndex, Cell);
	}

	for (int32 CellIndex=0; CellIndex<ChangedCells.EdgeCellsToAdd.Num(); CellIndex++)
	{
		const FOpenLandGridRendererCellBuildResult Cell = {
			BuildEdgeCell(CurrentGridChangedCells->EdgeCellsToAdd[CellIndex])
		};
		CellGenInfo->GeneratedEdgeCells.Set(CellIndex, Cell);
	}

	for (int32 CellIndex=0; CellIndex<ChangedCells.ModifiedEdgeCells.Num(); CellIndex++)
	{
		const FOpenLandGridRendererCellBuildResult Cell = {
			BuildEdgeCell(CurrentGridChangedCells->ModifiedEdgeCells[CellIndex])
		};
		CellGenInfo->ReGeneratedEdgeCells.Set(CellIndex, Cell);
	}

	// Apply GPU Vertex Modifier if needed
	if (GpuVertexModifier.Material)
	{
		const int32 TotalPoints = CellGenInfo->GeneratedCells.Length() * 6 + CellGenInfo->GeneratedEdgeCells.Length() * 9 + CellGenInfo->ReGeneratedEdgeCells.Length() * 9;
		const int32 TextureWidth = FMath::CeilToInt(FMath::Sqrt(TotalPoints));
		const int32 RowsToRead = FMath::CeilToInt(TotalPoints / 300.0);
		UE_LOG(LogTemp, Warning, TEXT("Using GpuVertexModifier. TextureWidth: %d, RowsToRead: %d"), TextureWidth, RowsToRead)

		// TODO: We need to resize this texture as needed.
		check(TextureWidth < 300);
		
		int32 Index = 0;
		for (size_t CellIndex=0; CellIndex<CellGenInfo->GeneratedCells.Length(); CellIndex++)
		{
			
			TOpenLandArray<FOpenLandMeshVertex>& Vertices = CellGenInfo->GeneratedCells.GetRef(CellIndex).Vertices;
			for (size_t VertexIndex=0; VertexIndex < Vertices.Length(); VertexIndex++)
			{
				FVector Position = Vertices.Get(VertexIndex).Position;
				DataTextureX->SetFloatValue(Index, Position.X);
				DataTextureY->SetFloatValue(Index, Position.Y);
				DataTextureZ->SetFloatValue(Index, Position.Z);
		
				Index += 1;
			}
		}
		
		for (size_t CellIndex=0; CellIndex<CellGenInfo->GeneratedEdgeCells.Length(); CellIndex++)
		{
			
			TOpenLandArray<FOpenLandMeshVertex>& Vertices = CellGenInfo->GeneratedEdgeCells.GetRef(CellIndex).Vertices;
			for (size_t VertexIndex=0; VertexIndex < Vertices.Length(); VertexIndex++)
			{
				FVector Position = Vertices.Get(VertexIndex).Position;
				DataTextureX->SetFloatValue(Index, Position.X);
				DataTextureY->SetFloatValue(Index, Position.Y);
				DataTextureZ->SetFloatValue(Index, Position.Z);
		
				Index += 1;
			}
		}
		
		for (size_t CellIndex=0; CellIndex<CellGenInfo->ReGeneratedEdgeCells.Length(); CellIndex++)
		{
			
			TOpenLandArray<FOpenLandMeshVertex>& Vertices = CellGenInfo->ReGeneratedEdgeCells.GetRef(CellIndex).Vertices;
			for (size_t VertexIndex=0; VertexIndex < Vertices.Length(); VertexIndex++)
			{
				FVector Position = Vertices.Get(VertexIndex).Position;
				DataTextureX->SetFloatValue(Index, Position.X);
				DataTextureY->SetFloatValue(Index, Position.Y);
				DataTextureZ->SetFloatValue(Index, Position.Z);
		
				Index += 1;
			}
		}
		
		DataTextureX->UpdateTexture();
		DataTextureY->UpdateTexture();
		DataTextureZ->UpdateTexture();
		
		TArray<FGpuComputeVertexDataTextureItem> DataTextures = {
			{"Position_X", DataTextureX},
			{"Position_Y", DataTextureY},
			{"Position_Z", DataTextureZ},
		};

		GpuComputeEngine->Compute(WorldContext, DataTextures, GpuVertexModifier);

		TArray<FGpuComputeVertexOutput> ModifiedData;
		ModifiedData.SetNumUninitialized(RowsToRead * 300);
		GpuComputeEngine->ReadData(ModifiedData, 0, RowsToRead);
	}
	
	for (int32 ThreadIndex=0; ThreadIndex < 5; ThreadIndex ++)
	{
		FOpenLandThreading::RunOnAnyBackgroundThread([this]()
		{
			while (true)
			{
				const bool hasMoreWork = GenerateCellTangentsAsync();
				if (!hasMoreWork)
				{
					break;
				}
			}
		});
	}
}

void FOpenLandGridRenderer::FinishCellGeneration()
{
	for (int32 Index = 0; Index < CurrentGridChangedCells->CellsToRemove.Num(); Index++)
	{
		SwapCell(Index);
	}

	// Apply Edge Cell Changes
	for (int32 Index = 0; Index < CurrentGridChangedCells->EdgeCellsToRemove.Num(); Index++)
	{
		SwapEdgeCell(Index);
	}
		
	// Go through existing edge cells & regenerate cells
	for (int32 Index = 0; Index < CurrentGridChangedCells->ModifiedEdgeCells.Num(); Index++)
	{
		RegenerateEdgeCell(Index);
	}
}

bool FOpenLandGridRenderer::GenerateCellTangentsAsync() const
{
	const int32 CellIndex = CellGenInfo->GetNextGeneratedCellIndex();
	if (CellIndex >= 0)
	{
		BuildTangents(CellGenInfo->GeneratedCells.GetRef(CellIndex).Vertices);
		return true;
	}

	const int32 EdgeCellIndex = CellGenInfo->GetNextGeneratedEdgeCellIndex();
	if (EdgeCellIndex >= 0)
	{
		BuildTangents(CellGenInfo->GeneratedEdgeCells.GetRef(EdgeCellIndex).Vertices);
		return true;
	}

	const int32 RegeneratedEdgeCellIndex = CellGenInfo->GetNextReGeneratedEdgeCellIndex();
	if (RegeneratedEdgeCellIndex >= 0)
	{
		BuildTangents(CellGenInfo->ReGeneratedEdgeCells.GetRef(RegeneratedEdgeCellIndex).Vertices);
		return true;
	}

	if (CellGenInfo->CanFinish())
	{
		CurrentOperation->bCompleted = true;
	}

	return false;
}

void FOpenLandGridRenderer::SwapCell(int32 Index)
{
	const FOpenLandGridCell RemovingCellPos = CurrentGridChangedCells->CellsToRemove[Index];
	const FOpenLandGridCell AddingCellPos = CurrentGridChangedCells->CellsToAdd[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = CellGenInfo->GeneratedCells.Get(Index).Vertices;
	// UE_LOG(LogTemp, Warning, TEXT("CellVertices Length: %d of Total: %d"), CellVertices.Length(), CurrentGridChangedCells->CellsToAdd.Num())

	const FOpenLandGridRendererCell RenderCell = Cells[GetTypeHash(RemovingCellPos)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));

	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));

	Cells.Remove(GetTypeHash(RemovingCellPos));
	Cells.Add(GetTypeHash(AddingCellPos), {
		AddingCellPos,
		RenderCell.IndexT0,
		RenderCell.IndexT1
	});

	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

void FOpenLandGridRenderer::SwapEdgeCell(int32 Index)
{
	const FOpenLandGridCell RemovingCellPos = CurrentGridChangedCells->EdgeCellsToRemove[Index];
	const FOpenLandGridCell AddingCellPos = CurrentGridChangedCells->EdgeCellsToAdd[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = CellGenInfo->GeneratedEdgeCells.Get(Index).Vertices;
	
	const FOpenLandGridRendererEdgeCell RenderCell = EdgeCells[GetTypeHash(RemovingCellPos)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));
	
	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));
	
	const FOpenLandMeshTriangle T2 = MeshInfo->Triangles.Get(RenderCell.IndexT2);
	MeshInfo->Vertices.Set(T2.T0, CellVertices.Get(6));
	MeshInfo->Vertices.Set(T2.T1, CellVertices.Get(7));
	MeshInfo->Vertices.Set(T2.T2, CellVertices.Get(8));
	
	EdgeCells.Remove(GetTypeHash(RemovingCellPos));
	EdgeCells.Add(GetTypeHash(AddingCellPos), {
		AddingCellPos,
		RenderCell.IndexT0,
		RenderCell.IndexT1,
		RenderCell.IndexT2
	}); 
	
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT2);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

void FOpenLandGridRenderer::RegenerateEdgeCell(int32 Index)
{
	const FOpenLandGridCell Cell = CurrentGridChangedCells->ModifiedEdgeCells[Index];
	TOpenLandArray<FOpenLandMeshVertex> CellVertices = CellGenInfo->ReGeneratedEdgeCells.Get(Index).Vertices;
	
	const FOpenLandGridRendererEdgeCell RenderCell = EdgeCells[GetTypeHash(Cell)];
		
	const FOpenLandMeshTriangle T0 = MeshInfo->Triangles.Get(RenderCell.IndexT0);
	MeshInfo->Vertices.Set(T0.T0, CellVertices.Get(0));
	MeshInfo->Vertices.Set(T0.T1, CellVertices.Get(1));
	MeshInfo->Vertices.Set(T0.T2, CellVertices.Get(2));
	
	const FOpenLandMeshTriangle T1 = MeshInfo->Triangles.Get(RenderCell.IndexT1);
	MeshInfo->Vertices.Set(T1.T0, CellVertices.Get(3));
	MeshInfo->Vertices.Set(T1.T1, CellVertices.Get(4));
	MeshInfo->Vertices.Set(T1.T2, CellVertices.Get(5));
	
	const FOpenLandMeshTriangle T2 = MeshInfo->Triangles.Get(RenderCell.IndexT2);
	MeshInfo->Vertices.Set(T2.T0, CellVertices.Get(6));
	MeshInfo->Vertices.Set(T2.T1, CellVertices.Get(7));
	MeshInfo->Vertices.Set(T2.T2, CellVertices.Get(8));
		
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT2);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT1);
	CurrentOperation->ChangedInfo.ChangedTriangles.Push(RenderCell.IndexT0);
}

FVector FOpenLandGridRenderer::ApplyCpuVertexModifier(FVector Source)
{
	const float Distance = FVector::Distance(Source, FVector(0, 0, 0));
	constexpr float Divider = 2000.0f;
	constexpr float Height = 300.0f;
	
	float NewHeight = FMath::Sin(Distance/Divider) * Height;
	return Source + FVector(0, 0, NewHeight);
}

FOpenLandMeshInfoPtr FOpenLandGridRenderer::Initialize(FOpenLandGridPtr SourceGrid)
{
	check(bInitialized == false);
	bInitialized = true;
	Grid = SourceGrid;
	MeshInfo = FOpenLandMeshInfo::New();
	Cells = {};

	FOpenLandGridChangedCells InitialCells = Grid->GetAllCells();
	
	for(const FOpenLandGridCell Cell: InitialCells.CellsToAdd)
	{
		TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildCell(Cell);
		BuildTangents(CellVertices);
		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), CellVertices);
		
		const int32 TotalTriangles = MeshInfo->Triangles.Length();
		Cells.Add(GetTypeHash(Cell), {
			Cell,
			TotalTriangles - 2,
			TotalTriangles - 1,
		});
	}

	for(const FOpenLandGridCell Cell: InitialCells.EdgeCellsToAdd)
	{
		TOpenLandArray<FOpenLandMeshVertex> CellVertices = BuildEdgeCell(Cell);
		BuildTangents(CellVertices);
		FOpenLandPolygonMesh::AddFace(MeshInfo.Get(), CellVertices);
		
		const int32 TotalTriangles = MeshInfo->Triangles.Length();
		EdgeCells.Add(GetTypeHash(Cell), {
			Cell,
			TotalTriangles - 3,
			TotalTriangles - 2,
			TotalTriangles - 1,
		});
	}

	MeshInfo->BoundingBox = Grid->GetBoundingBox();
	
	return  MeshInfo;
}

bool FOpenLandGridRenderer::StartReCenter(FVector NewCenter)
{
	check(CurrentOperation == nullptr);
	
	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter);
	//UE_LOG(LogTemp, Warning, TEXT("CellsToAdd: %d, CellsToRemove: %d, EdgeCellsToAdd: %d, EdgeCellsToRemove: %d"), ChangedCells.CellsToAdd.Num(), ChangedCells.CellsToRemove.Num(), ChangedCells.EdgeCellsToAdd.Num(), ChangedCells.EdgeCellsToRemove.Num())
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());
	check(ChangedCells.EdgeCellsToAdd.Num() == ChangedCells.EdgeCellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}

	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

bool FOpenLandGridRenderer::StartReCenter(FVector NewCenter, FOpenLandGridCell NewHoleRootCell)
{
	check(CurrentOperation == nullptr);

	const FOpenLandGridChangedCells ChangedCells = Grid->ReCenter(NewCenter, NewHoleRootCell);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());
	check(ChangedCells.EdgeCellsToAdd.Num() == ChangedCells.EdgeCellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}
	
	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

bool FOpenLandGridRenderer::StartChangeHoleRootCell(FOpenLandGridCell NewHoleRootCell)
{
	check(CurrentOperation == nullptr);

	const FOpenLandGridChangedCells ChangedCells = Grid->ChangeHoleRootCell(NewHoleRootCell);
	check(ChangedCells.CellsToAdd.Num() == ChangedCells.CellsToRemove.Num());

	if (ChangedCells.CellsToAdd.Num() == 0 && ChangedCells.EdgeCellsToAdd.Num() == 0)
	{
		return false;
	}

	CurrentOperation = MakeShared<FOpenLandGridRendererChangedInfoStatus>();
	ApplyCellChangesAsync(ChangedCells);

	return true;
}

TSharedPtr<FOpenLandGridRendererChangedInfo> FOpenLandGridRenderer::CheckStatus()
{
	if (!CurrentOperation)
	{
		return nullptr;
	}

	if (!CurrentOperation->bCompleted)
	{
		return nullptr;
	}

	FinishCellGeneration();
	const TSharedPtr<FOpenLandGridRendererChangedInfo> ChangedInfo = MakeShared<FOpenLandGridRendererChangedInfo>();
	ChangedInfo->ChangedTriangles = CurrentOperation->ChangedInfo.ChangedTriangles;
	
	CurrentOperation = nullptr;

	return ChangedInfo;
}

void FOpenLandGridRenderer::SetGpuVertexModifier(FComputeMaterial ComputeMaterial, UObject* InputWorldContext)
{
	GpuVertexModifier = ComputeMaterial;
	WorldContext = InputWorldContext;
	DataTextureX= MakeShared<FDataTexture>(300);
	DataTextureY= MakeShared<FDataTexture>(300);
	DataTextureZ= MakeShared<FDataTexture>(300);
	GpuComputeEngine = MakeShared<FGpuComputeVertex>();
	GpuComputeEngine->Init(InputWorldContext, 300);
}

void FOpenLandGridRenderer::BuildTangents(TOpenLandArray<FOpenLandMeshVertex>& Vertices)
{
	for(int32 TriIndex=0; TriIndex < Vertices.Length() / 3; TriIndex ++)
	{
		FOpenLandMeshVertex& T0 = Vertices.GetRef(TriIndex + 0);
		FOpenLandMeshVertex& T1 = Vertices.GetRef(TriIndex + 1);
		FOpenLandMeshVertex& T2 = Vertices.GetRef(TriIndex + 2);

		FOpenLandPolygonMesh::BuildFaceTangents(T0, T1, T2);
	}
}
