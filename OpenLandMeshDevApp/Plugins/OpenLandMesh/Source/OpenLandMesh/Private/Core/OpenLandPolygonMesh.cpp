// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Core/OpenLandPolygonMesh.h"
#include "Utils/TrackTime.h"
#include "Compute/OpenLandThreading.h"

bool FOpenLandPolygonMesh::bIsDeleteSchedulerRunning = false;
TArray<FOpenLandPolygonMesh*> FOpenLandPolygonMesh::PolygonMeshesToDelete = {};

void FOpenLandPolygonMesh::ApplyNormalSmoothing(FOpenLandMeshInfo* MeshInfo, float CuspAngle)
{
	TMap<FVector, TArray<int32>> PointsToVertices;

	// Build PointsToVertices
	// This contains a list of vertices for a given position.
	// That happens when multiple triangles shares the same position.
	for (size_t VertexIndex = 0; VertexIndex < MeshInfo->Vertices.Length(); VertexIndex++)
	{
		FOpenLandMeshVertex Vertex = MeshInfo->Vertices.Get(VertexIndex);
		if (!PointsToVertices.Contains(Vertex.Position))
			PointsToVertices.Add(Vertex.Position, {});

		PointsToVertices[Vertex.Position].Push(VertexIndex);
	}

	for (auto& PointElement : PointsToVertices)
	{
		TArray<int32> VertexIndices = PointElement.Value;

		TOpenLandArray<FVector> TangentZList = {};
		TOpenLandArray<FVector> TangentXList = {};
		TangentZList.SetLength(VertexIndices.Num());
		TangentXList.SetLength(VertexIndices.Num());
		for (int32 TangentsIndex = 0; TangentsIndex < VertexIndices.Num(); TangentsIndex ++)
		{
			TangentZList.Set(TangentsIndex, {0, 0, 0});
			TangentXList.Set(TangentsIndex, {0, 0, 0});
		}

		for (int32 IndicesIndex = 0; IndicesIndex < VertexIndices.Num(); IndicesIndex++)
		{
			int32 VertexIndex = VertexIndices[IndicesIndex];
			const FOpenLandMeshVertex Vertex = MeshInfo->Vertices.Get(VertexIndex);
			for (int32 TangentsIndex = 0; TangentsIndex < VertexIndices.Num(); TangentsIndex ++)
				if (IndicesIndex == TangentsIndex)
				{
					TangentZList.Set(TangentsIndex, TangentZList.Get(TangentsIndex) + Vertex.Normal);
					TangentXList.Set(TangentsIndex, TangentXList.Get(TangentsIndex) + Vertex.Tangent.TangentX);
				}
				else
				{
					const FOpenLandMeshVertex RelatedVertex = MeshInfo->Vertices.Get(VertexIndices[TangentsIndex]);
					float AngleBetween = FMath::RadiansToDegrees(
						FMath::Acos(FVector::DotProduct(Vertex.Normal, RelatedVertex.Normal)));
					AngleBetween = FMath::RoundToInt(AngleBetween * 100) / 100;
					if (AngleBetween <= CuspAngle)
					{
						TangentZList.Set(TangentsIndex, TangentZList.Get(TangentsIndex) + Vertex.Normal);
						TangentXList.Set(TangentsIndex, TangentXList.Get(TangentsIndex) + Vertex.Tangent.TangentX);
					}
				}
		}

		for (int32 IndicesIndex = 0; IndicesIndex < VertexIndices.Num(); IndicesIndex++)
		{
			const int32 VertexIndex = VertexIndices[IndicesIndex];
			MeshInfo->Vertices.GetRef(VertexIndex).Normal = TangentZList.Get(IndicesIndex).GetSafeNormal();
			MeshInfo->Vertices.GetRef(VertexIndex).Tangent.TangentX = TangentXList.Get(IndicesIndex).GetSafeNormal();
		}
	}
}

FOpenLandMeshInfo FOpenLandPolygonMesh::SubDivide(FOpenLandMeshInfo SourceMeshInfo, int Depth)
{
	FOpenLandMeshInfo MeshInfo = SourceMeshInfo;

	for (int DepthIndex = 0; DepthIndex < Depth; DepthIndex++)
	{
		FOpenLandMeshInfo NewMeshInfo;
		int NumTris = MeshInfo.Triangles.Length();

		for (int TriIndex = 0; TriIndex < NumTris; TriIndex ++)
		{
			FOpenLandMeshTriangle Triangle = MeshInfo.Triangles.Get(TriIndex);

			FOpenLandMeshVertex T0 = MeshInfo.Vertices.Get(Triangle.T0);
			FOpenLandMeshVertex T1 = MeshInfo.Vertices.Get(Triangle.T1);
			FOpenLandMeshVertex T2 = MeshInfo.Vertices.Get(Triangle.T2);

			FOpenLandMeshVertex T01 = T0.Interpolate(T1, 0.5);
			FOpenLandMeshVertex T12 = T1.Interpolate(T2, 0.5);
			FOpenLandMeshVertex T02 = T0.Interpolate(T2, 0.5);

			TOpenLandArray<FOpenLandMeshVertex> NewVertices = {
				T0, T01, T02,
				T01, T1, T12,
				T02, T12, T2,
				T02, T01, T12
			};

			AddFace(&NewMeshInfo, NewVertices);
		}

		MeshInfo = NewMeshInfo;
	}

	return MeshInfo;
}

FSimpleMeshInfoPtr FOpenLandPolygonMesh::BuildMesh(UObject* WorldContext, int SubDivisions, float CuspAngle)
{
	// Apply Source transformation
	FOpenLandMeshInfo TransformedMeshInfo = SourceMeshInfo;
	for (size_t Index = 0; Index < TransformedMeshInfo.Vertices.Length(); Index++)
	{
		FOpenLandMeshVertex& Vertex = TransformedMeshInfo.Vertices.GetRef(Index);
		Vertex.Position = SourceTransformer.TransformVector(Vertex.Position);
	}

	FOpenLandMeshInfo _Original = SubDivide(TransformedMeshInfo, SubDivisions);
	FOpenLandMeshInfo* Original = &_Original;
	// Here we subdivide & clone this meshInfo
	// That because, we need to send this memory out of this block
	// And after that, we no longer manage that memory
	FSimpleMeshInfoPtr Target = _Original.Clone();

	// Setup & Gpu Vertex Modifiers if needed
	EnsureGpuComputeEngine(WorldContext, Original);
	if (GpuVertexModifier.Material != nullptr)
	{
		auto TrackGpuVertexModifiers = TrackTime("GpuVertexModifiers");
		ApplyGpuVertexModifers(WorldContext, Original, Target.Get(), MakeParameters(0, true));
		TrackGpuVertexModifiers.Finish();
	}

	// Build Faces

	auto TrackCpuVertexModifiers = TrackTime("CpuVertexModifiers");
	ApplyVertexModifiers(Target.Get(), Target.Get(), 0, Original->Triangles.Length(), 0, true);
	TrackCpuVertexModifiers.Finish();

	if (CuspAngle > 0.0)
	{
		auto TrackNormalSmoothing = TrackTime("NormalSmoothing");
		ApplyNormalSmoothing(Target.Get(), CuspAngle);
		TrackNormalSmoothing.Finish();
	}

	return Target;
}

void FOpenLandPolygonMesh::BuildMeshAsync(UObject* WorldContext, int SubDivisions, float CuspAngle,
                                          std::function<void(FSimpleMeshInfoPtr)> Callback)
{
	std::function<void(FSimpleMeshInfoPtr)> HandleCallback = [this, WorldContext, Callback, CuspAngle
		](FSimpleMeshInfoPtr Target)
	{
		ModifyVertices(WorldContext, Target, Target, 0, CuspAngle, true);
		Callback(Target);
	};

	// Apply Source transformation
	FOpenLandThreading::RunOnAnyBackgroundThread([this, SubDivisions, CuspAngle, HandleCallback]()
	{
		FOpenLandMeshInfo TransformedMeshInfo = SourceMeshInfo;
		for (size_t Index = 0; Index < TransformedMeshInfo.Vertices.Length(); Index++)
		{
			FOpenLandMeshVertex& Vertex = TransformedMeshInfo.Vertices.GetRef(Index);
			Vertex.Position = SourceTransformer.TransformVector(Vertex.Position);
		}

		FOpenLandMeshInfo _Original = SubDivide(TransformedMeshInfo, SubDivisions);
		FOpenLandMeshInfo* Original = &_Original;
		// Here we subdivide & clone this meshInfo
		// That because, we need to send this memory out of this block
		// And after that, we no longer manage that memory
		FSimpleMeshInfoPtr Target = _Original.Clone();

		FOpenLandThreading::RunOnGameThread([HandleCallback, Target]()
		{
			HandleCallback(Target);
		});
	});
}

void FOpenLandPolygonMesh::ApplyVertexModifiers(FOpenLandMeshInfo* Original, FOpenLandMeshInfo* Target, int RangeStart,
                                                int RangeEnd, float RealTimeSeconds, bool bOnBuilding) const
{
	for (int TriIndex = RangeStart; TriIndex < RangeEnd; TriIndex++)
	{
		const FOpenLandMeshTriangle OTriangle = Original->Triangles.Get(TriIndex);
		FOpenLandMeshVertex O0 = Original->Vertices.Get(OTriangle.T0);
		FOpenLandMeshVertex O1 = Original->Vertices.Get(OTriangle.T1);
		FOpenLandMeshVertex O2 = Original->Vertices.Get(OTriangle.T2);

		const FOpenLandMeshTriangle TTriangle = Target->Triangles.Get(TriIndex);
		FOpenLandMeshVertex& T0 = Target->Vertices.GetRef(TTriangle.T0);
		FOpenLandMeshVertex& T1 = Target->Vertices.GetRef(TTriangle.T1);
		FOpenLandMeshVertex& T2 = Target->Vertices.GetRef(TTriangle.T2);

		// Run the Vertex Modifier If exists
		// Here we input Original vertex to the modifier & update the target
		// So, we don't change anything inside the original
		if (VertexModifier != nullptr)
		{
			// When we are building the mesh, we need to calculate normals just to send them to the vertex modifier
			// But, we don't need to do that when animating.
			if (!bOnBuilding)
				BuildFaceTangents(O0, O1, O2);

			T0.Position = VertexModifier({O0.Position, O0.Normal, O0.UV0, RealTimeSeconds, bOnBuilding}).Position;
			T1.Position = VertexModifier({O1.Position, O1.Normal, O1.UV0, RealTimeSeconds, bOnBuilding}).Position;
			T2.Position = VertexModifier({O2.Position, O2.Normal, O2.UV0, RealTimeSeconds, bOnBuilding}).Position;
		}

		BuildFaceTangents(T0, T1, T2);


		// Build Bounding Box
		Target->BoundingBox += T0.Position;
		Target->BoundingBox += T1.Position;
		Target->BoundingBox += T2.Position;
	}
}

void FOpenLandPolygonMesh::EnsureGpuComputeEngine(UObject* WorldContext, FOpenLandMeshInfo* MeshInfo)
{
	// TODO: Try to disconnect ComputeEngine from the VertexCount. It should automatically expand or shrink
	// based on the demand.

	if (GpuVertexModifier.Material == nullptr)
	{
		GpuComputeEngine = nullptr;
		return;
	}

	const int32 VertexCount = MeshInfo->Vertices.Length();
	const int32 TextureWidth = FMath::CeilToInt(FMath::Sqrt(VertexCount));
	UE_LOG(LogTemp, Warning, TEXT("TextureWidth is: %d for Vertices: %d"), TextureWidth, VertexCount)

	GpuComputeEngine = MakeShared<FGpuComputeVertex>();
	TArray<FGpuComputeVertexInput> SourceData;
	GpuComputeEngine->Init(WorldContext, SourceData, TextureWidth);
}

void FOpenLandPolygonMesh::ApplyGpuVertexModifers(UObject* WorldContext, FOpenLandMeshInfo* Original,
                                                  FOpenLandMeshInfo* Target,
                                                  TArray<FComputeMaterialParameter> AdditionalMaterialParameters)
{
	// Set the Original Data
	TArray<FGpuComputeVertexInput> OriginalPositions;
	OriginalPositions.SetNumUninitialized(Original->Vertices.Length());
	for (size_t Index = 0; Index < Original->Vertices.Length(); Index++)
		OriginalPositions[Index].Position = Original->Vertices.Get(Index).Position;

	GpuComputeEngine->UpdateSourceData(OriginalPositions);

	// Apply Modifiers
	TArray<FGpuComputeVertexOutput> ModifiedPositions;
	ModifiedPositions.SetNumUninitialized(Original->Vertices.Length());

	for (auto Param : AdditionalMaterialParameters)
		GpuVertexModifier.Parameters.Push(Param);
	GpuComputeEngine->Compute(WorldContext, ModifiedPositions, GpuVertexModifier);

	for (size_t Index = 0; Index < Original->Vertices.Length(); Index++)
	{
		FOpenLandMeshVertex& Vertex = Target->Vertices.GetRef(Index);
		Vertex.Position = ModifiedPositions[Index].Position;
		Vertex.Color = ModifiedPositions[Index].VertexColor;
	}
}


void FOpenLandPolygonMesh::ModifyVertices(UObject* WorldContext, FSimpleMeshInfoPtr Original, FSimpleMeshInfoPtr Target,
                                          float RealTimeSeconds,
                                          float CuspAngle, bool bOnBuilding)
{
	// TODO: check for sizes of both original & target
	// Setup & Gpu Vertex Modifiers if needed
	auto TrackEnsureGpuComputeEngine = TrackTime("EnsureGpuComputeEngine");
	EnsureGpuComputeEngine(WorldContext, Original.Get());
	TrackEnsureGpuComputeEngine.Finish();

	FSimpleMeshInfoPtr Intermediate = Original;
	if (GpuVertexModifier.Material != nullptr)
	{
		auto TrackGpuVertexModifiers = TrackTime("GpuVertexModifiers");
		ApplyGpuVertexModifers(WorldContext, Original.Get(), Target.Get(),
		                       MakeParameters(RealTimeSeconds, bOnBuilding));
		Intermediate = Target;
		TrackGpuVertexModifiers.Finish();
	}

	// Build Faces
	Target->BoundingBox.Init();

	auto TrackCpuVertexModifiers = TrackTime("CpuVertexModifiers");
	ApplyVertexModifiers(Intermediate.Get(), Target.Get(), 0, Original->Triangles.Length(), RealTimeSeconds,
	                     bOnBuilding);
	TrackCpuVertexModifiers.Finish();

	if (CuspAngle > 0.0)
	{
		auto TrackNormalSmoothing = TrackTime("NormalSmoothing");
		ApplyNormalSmoothing(Target.Get(), CuspAngle);
		TrackNormalSmoothing.Finish();
	}
}

bool FOpenLandPolygonMesh::ModifyVerticesAsync(UObject* WorldContext, FSimpleMeshInfoPtr Original,
                                               FSimpleMeshInfoPtr Target, float RealTimeSeconds, float CuspAngle)
{
	// There's a currently running job.
	// So, we need to check the state of that
	if (AsyncCompletions.Num() != 0)
	{
		bool bAsyncTaskCompleted = true;

		for (int32 Index = 0; Index < AsyncCompletions.Num(); Index++)
			//UE_LOG(LogTemp, Warning, TEXT("Value inside the Task Queue: %d=%s"), Index, AsyncCompletions[Index] == true? TEXT("T") : TEXT("F"))
			bAsyncTaskCompleted = bAsyncTaskCompleted && AsyncCompletions[Index];

		if (bAsyncTaskCompleted == true)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Finished & Queue for Rendering: %d"), AsyncCompletions.Num())
			AsyncCompletions = {};
			return true;
		}

		//UE_LOG(LogTemp, Warning, TEXT("Not Finished Yet!: %d"), AsyncCompletions.Num())
		return false;
	}

	auto TrackEnsureGpuComputeEngine = TrackTime("EnsureGpuComputeEngine");
	EnsureGpuComputeEngine(WorldContext, Original.Get());
	TrackEnsureGpuComputeEngine.Finish();

	FSimpleMeshInfoPtr Intermediate = Original;
	if (GpuVertexModifier.Material != nullptr)
	{
		auto TrackGpuVertexModifiers = TrackTime("GpuVertexModifiers");
		ApplyGpuVertexModifers(WorldContext, Original.Get(), Target.Get(), MakeParameters(RealTimeSeconds, false));
		Intermediate = Target;
		TrackGpuVertexModifiers.Finish();
	}

	// Build Faces
	Target->BoundingBox.Init();

	const int NumTris = Original->Triangles.Length();
	// TODO: Find out number of workers in the core & detect the workers accordingly
	// TODO: May be we need to get worker information from the user may be.
	const int NumWorkers = 10; // find this automatically based on number of cores in the system
	const int TasksPerWorker = (NumTris / NumWorkers);

	// Submit jobs
	for (int WorkerId = 0; WorkerId < NumWorkers; WorkerId++)
	{
		AsyncCompletions.Push(false);
		FOpenLandThreading::RunOnAnyBackgroundThread(
			[this, Intermediate, Target, RealTimeSeconds, NumWorkers, TasksPerWorker, WorkerId]
			{
				const int NumTris = Intermediate->Triangles.Length();
				const int StartIndex = TasksPerWorker * WorkerId;
				const int EndIndex = (WorkerId == NumWorkers - 1) ? NumTris : StartIndex + TasksPerWorker;
				ApplyVertexModifiers(Intermediate.Get(), Target.Get(), StartIndex, EndIndex, RealTimeSeconds, false);

				// Mark the work as completed.
				// We need to do this on the game thread since AsyncCompletions is not thread safe.
				FOpenLandThreading::RunOnGameThread([this, WorkerId]
				{
					//UE_LOG(LogTemp, Warning, TEXT("Completing Worker: %d"), WorkerId)
					AsyncCompletions[WorkerId] = true;
				});
			});
	}

	// Add a task to do the normal smoothing
	AsyncCompletions.Push(false);
	FOpenLandThreading::RunOnAnyBackgroundThread([this, Target, CuspAngle]
	{
		while (true)
		{
			bool bAsyncTaskCompleted = true;

			for (int32 Index = 0; Index < AsyncCompletions.Num() - 1; Index++)
				//UE_LOG(LogTemp, Warning, TEXT("Value inside the Task Queue: %d=%s"), Index, AsyncCompletions[Index] == true? TEXT("T") : TEXT("F"))
				bAsyncTaskCompleted = bAsyncTaskCompleted && AsyncCompletions[Index];
			if (bAsyncTaskCompleted == false)
			{
				FPlatformProcess::Sleep(0.001);
				continue;
			}

			ApplyNormalSmoothing(Target.Get(), CuspAngle);
			break;
		}

		FOpenLandThreading::RunOnGameThread([this]
		{
			//UE_LOG(LogTemp, Warning, TEXT("Completing Worker: %d"), AsyncCompletions.Num() -1)
			AsyncCompletions[AsyncCompletions.Num() - 1] = true;
		});
		//UE_LOG(LogTemp, Warning, TEXT("Completing Worker: %d"), WorkerId)    
	});

	//UE_LOG(LogTemp, Warning, TEXT("Just Submit Some Work"))
	return false;
}

void FOpenLandPolygonMesh::AddTriFace(const FVector A, const FVector B, const FVector C)
{
	const TOpenLandArray<FOpenLandMeshVertex> InputVertices = {
		FOpenLandMeshVertex(A, FVector2D(0, 1)),
		FOpenLandMeshVertex(B, FVector2D(1, 1)),
		FOpenLandMeshVertex(C, FVector2D(0.5, 0))
	};

	AddFace(&SourceMeshInfo, InputVertices);
}

void FOpenLandPolygonMesh::AddQuadFace(const FVector A, const FVector B, const FVector C, const FVector D)
{
	const TOpenLandArray<FOpenLandMeshVertex> InputVertices = {
		FOpenLandMeshVertex(A, FVector2D(0, 1)),
		FOpenLandMeshVertex(B, FVector2D(1, 1)),
		FOpenLandMeshVertex(C, FVector2D(1, 0)),

		FOpenLandMeshVertex(A, FVector2D(0, 1)),
		FOpenLandMeshVertex(C, FVector2D(1, 0)),
		FOpenLandMeshVertex(D, FVector2D(0, 0))
	};

	AddFace(&SourceMeshInfo, InputVertices);
}

void FOpenLandPolygonMesh::Transform(FTransform Transformer)
{
	SourceTransformer = Transformer;
}

bool FOpenLandPolygonMesh::IsThereAnyAsyncTask() const
{
	if (AsyncCompletions.Num() == 0)
		return false;

	// There are some tasks. Now we need to check whether they completed or not.
	for (int32 Index = 0; Index < AsyncCompletions.Num(); Index++)
		if (AsyncCompletions[Index] == false)
			// This is this task is running
			return true;

	return false;
}

void FOpenLandPolygonMesh::RegisterVertexModifier(std::function<FVertexModifierResult(FVertexModifierPayload)> Callback)
{
	VertexModifier = Callback;
}

FGpuComputeMaterialStatus FOpenLandPolygonMesh::RegisterGpuVertexModifier(FComputeMaterial ComputeMaterial)
{
	if (ComputeMaterial.Material == nullptr)
	{
		GpuComputeEngine = nullptr;
		GpuVertexModifier = {};
		return {true};
	}

	FGpuComputeMaterialStatus ValidityStatus = FGpuComputeVertex::IsValidMaterial(ComputeMaterial.Material);
	if (!ValidityStatus.bIsValid)
		return ValidityStatus;

	GpuVertexModifier = ComputeMaterial;
	return ValidityStatus;
}

void FOpenLandPolygonMesh::AddFace(FOpenLandMeshInfo* MeshInfo, TOpenLandArray<FOpenLandMeshVertex> InputVertices)
{
	const int NumTris = InputVertices.Length() / 3;
	for (int TriIndex = 0; TriIndex < NumTris; TriIndex++)
	{
		FOpenLandMeshVertex T0 = InputVertices.Get(TriIndex * 3 + 0);
		FOpenLandMeshVertex T1 = InputVertices.Get(TriIndex * 3 + 1);
		FOpenLandMeshVertex T2 = InputVertices.Get(TriIndex * 3 + 2);

		int32 TriangleIndex = MeshInfo->Triangles.Push({});
		FOpenLandMeshTriangle& Triangle = MeshInfo->Triangles.GetRef(TriangleIndex);

		// Add the Triangle
		const int32 T0Index = MeshInfo->Vertices.Length();
		T0.TriangleId = TriangleIndex;
		MeshInfo->Vertices.Push(T0);
		Triangle.T0 = T0Index;

		const int32 T1Index = MeshInfo->Vertices.Length();
		T1.TriangleId = TriangleIndex;
		MeshInfo->Vertices.Push(T1);
		Triangle.T1 = T1Index;

		const int32 T2Index = MeshInfo->Vertices.Length();
		T2.TriangleId = TriangleIndex;
		MeshInfo->Vertices.Push(T2);
		Triangle.T2 = T2Index;
	}
}

void FOpenLandPolygonMesh::BuildFaceTangents(FOpenLandMeshVertex& T0, FOpenLandMeshVertex& T1, FOpenLandMeshVertex& T2)
{
	// Calculate Normal & Tangents
	const FVector Edge21 = T1.Position - T2.Position;
	const FVector Edge20 = T0.Position - T2.Position;
	const FVector TNormal = (Edge21 ^ Edge20).GetSafeNormal();

	// Since we have UVs always, we calculate tangents like this.
	// Otherwise, we need to do this:
	//	 FVector TangentX = Edge20.GetSafeNormal();
	//   FVector TangentY = (TangentX ^ TNormal).GetSafeNormal();
	// (This is based on the original, KismetProceduralMeshLibrary.cpp)
	const FMatrix ParameterToLocal(
		FPlane(T1.Position.X - T0.Position.X, T1.Position.Y - T0.Position.Y, T1.Position.Z - T0.Position.Z, 0),
		FPlane(T2.Position.X - T0.Position.X, T2.Position.Y - T0.Position.Y, T2.Position.Z - T0.Position.Z, 0),
		FPlane(T0.Position.X, T0.Position.Y, T0.Position.Z, 0),
		FPlane(0, 0, 0, 1)
	);

	const FMatrix ParameterToTexture(
		FPlane(T1.UV0.X - T0.UV0.X, T1.UV0.Y - T0.UV0.Y, 0, 0),
		FPlane(T2.UV0.X - T0.UV0.X, T2.UV0.Y - T0.UV0.Y, 0, 0),
		FPlane(T0.UV0.X, T0.UV0.Y, 1, 0),
		FPlane(0, 0, 0, 1)
	);

	// Use InverseSlow to catch singular matrices.  Inverse can miss this sometimes.
	const FMatrix TextureToLocal = ParameterToTexture.Inverse() * ParameterToLocal;

	FVector TangentX = TextureToLocal.TransformVector(FVector(1, 0, 0)).GetSafeNormal();
	const FVector TangentY = TextureToLocal.TransformVector(FVector(0, 1, 0)).GetSafeNormal();

	// Use Gram-Schmidt orthogonalization to make sure X is orth with Z
	TangentX -= TNormal * (TNormal | TangentX);
	TangentX.Normalize();

	// See if we need to flip TangentY when generating from cross product
	const bool bFlipBitangent = ((TNormal ^ TangentX) | TangentY) < 0.f;
	const FOpenLandMeshTangent MeshTangent = FOpenLandMeshTangent(TangentX, bFlipBitangent);

	T0.Normal = TNormal;
	T1.Normal = TNormal;
	T2.Normal = TNormal;

	T0.Tangent = MeshTangent;
	T1.Tangent = MeshTangent;
	T2.Tangent = MeshTangent;
}

TArray<FComputeMaterialParameter> FOpenLandPolygonMesh::MakeParameters(float Time, bool bOnBuilding)
{
	TArray<FComputeMaterialParameter> Params;

	Params.Push({
		"Time",
		CMPT_SCALAR,
		Time,
		{}
	});

	Params.Push({
		"OnBuilding",
		CMPT_SCALAR,
		bOnBuilding ? 1.0f : 0.0f,
		{}
	});

	return Params;
}

FOpenLandPolygonMesh::~FOpenLandPolygonMesh()
{
	VertexModifier = nullptr;
}

void FOpenLandPolygonMesh::RunDeleteScheduler()
{
	if (bIsDeleteSchedulerRunning)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Delete Schedular] Running"))
		return;
	}

	// Start the delete loop
	UE_LOG(LogTemp, Warning, TEXT("[Delete Schedular] Trying to delete: %d"), PolygonMeshesToDelete.Num())
	bIsDeleteSchedulerRunning = true;

	TArray<FOpenLandPolygonMesh*> StillRunningPolygonMeshes;
	for (int32 Index = 0; Index < PolygonMeshesToDelete.Num(); Index++)
	{
		FOpenLandPolygonMesh* PolygonMesh = PolygonMeshesToDelete[Index];
		if (PolygonMesh == nullptr)
			continue;

		if (PolygonMesh->IsThereAnyAsyncTask())
			StillRunningPolygonMeshes.Push(PolygonMeshesToDelete[Index]);
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Delete Schedular] Deleting polygon mesh"))
			delete PolygonMesh;
		}
	}

	PolygonMeshesToDelete = StillRunningPolygonMeshes;
	bIsDeleteSchedulerRunning = false;
	UE_LOG(LogTemp, Warning, TEXT("[Delete Schedular] Still need to delete: %d"), PolygonMeshesToDelete.Num())

	// Nothing to delete, let's stop this
	if (PolygonMeshesToDelete.Num() == 0)
		return;

	UE_LOG(LogTemp, Warning, TEXT("[Delete Schedular] Adding a Timeout for 1 sec"))
	// If there are more to delete, Schedule again with a timeout
	FOpenLandThreading::RunOnAnyBackgroundThread([]()
	{
		FPlatformProcess::Sleep(1);
		FOpenLandThreading::RunOnGameThread([]()
		{
			RunDeleteScheduler();
		});
	});
}

void FOpenLandPolygonMesh::DeletePolygonMesh(FOpenLandPolygonMesh* PolygonMesh)
{
	PolygonMeshesToDelete.Push(PolygonMesh);
	RunDeleteScheduler();
}
