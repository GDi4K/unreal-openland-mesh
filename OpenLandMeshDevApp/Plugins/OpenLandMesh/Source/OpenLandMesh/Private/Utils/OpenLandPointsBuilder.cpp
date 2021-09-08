// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Utils/OpenLandPointsBuilder.h"
#include "Utils/OpenLandMeshGrid.h"
#include "Utils/OpenLandPointLine.h"
#include "Utils/OpenLandPointTriangle.h"
#include "Utils/OpenLandUpVectorSwitcher.h"

TArray<FOpenLandMeshPoint> FOpenLandPointsBuilder::BuildPointsModifiedPoisson2D(FSimpleMeshInfoPtr MeshInfo, float Density, float MinRadius)
{
	TArray<FOpenLandMeshPoint> InstancingPoints;

	// Finding Points
	const FVector ZVector = {0, 0, 1};
	
	for (size_t TriangleIndex=0; TriangleIndex<MeshInfo->Triangles.Length(); TriangleIndex++)
	{
		const FOpenLandMeshTriangle MeshTriangle = MeshInfo->Triangles.Get(TriangleIndex);
		const FVector P0 = MeshInfo->Vertices.Get(MeshTriangle.T0).Position;
		const FVector P1 = MeshInfo->Vertices.Get(MeshTriangle.T1).Position;
		const FVector P2 = MeshInfo->Vertices.Get(MeshTriangle.T2).Position;
		
		const float Area = FOpenLandPointTriangle::FindArea(P0, P1, P2);
		const float PointCountFromDensity = Area/10000 * Density;
		int32 PointCount;
		if (PointCountFromDensity >= 1)
		{
			PointCount = FMath::RoundToInt(PointCountFromDensity);
		} else
		{
			PointCount = FMath::RandRange(0.0f, 1.0f) <= PointCountFromDensity? 1 : 0;
		}
		
		if (PointCount <= 0)
		{
			continue;
		}
		
		TArray<FOpenLandMeshPoint> LocalPoints = BuildPointsOnTriangle(MeshInfo, TriangleIndex, PointCount, MinRadius);
		
		// Add these points to the default list;
		for(FOpenLandMeshPoint Point: LocalPoints)
		{
			InstancingPoints.Push(Point);
		}
	}

	return InstancingPoints;
}

TArray<FOpenLandMeshPoint> FOpenLandPointsBuilder::BuildPointsUseOrigin(FSimpleMeshInfoPtr MeshInfo)
{
	FOpenLandMeshPoint Origin;
	Origin.Normal = {0.0f, 0.0f, 1.0f};
	Origin.Position = {0.0f, 0.0f, 0.0f};
	Origin.TangentX = {1.0f, 0.0f, 0.0f};

	return {Origin};
}

TArray<FOpenLandMeshPoint> FOpenLandPointsBuilder::BuildPointsPickVertices(FSimpleMeshInfoPtr MeshInfo)
{
	TArray<FOpenLandMeshPoint> PointList;
	TMap<FVector, bool> PointsMap;
	for (size_t Index=0; Index<MeshInfo->Vertices.Length(); Index++)
	{
		const FOpenLandMeshVertex Vertex = MeshInfo->Vertices.Get(Index);
		if (PointsMap.Find(Vertex.Position) != nullptr)
		{
			continue;
		}

		FOpenLandMeshPoint Point;
		Point.Position = Vertex.Position;
		Point.Normal = Vertex.Normal;
		Point.TangentX = Vertex.Tangent.TangentX;
		
		PointsMap.Add(Vertex.Position, true);
		PointList.Push(Point);
	}

	return PointList;
}

TArray<FOpenLandMeshPoint> FOpenLandPointsBuilder::BuildPointsPickCentroids(FSimpleMeshInfoPtr MeshInfo)
{
	TArray<FOpenLandMeshPoint> PointList;
	for (size_t TriangleIndex=0; TriangleIndex<MeshInfo->Triangles.Length(); TriangleIndex++)
	{
		const FOpenLandMeshTriangle Triangle = MeshInfo->Triangles.Get(TriangleIndex);
		const FOpenLandMeshVertex T0 = MeshInfo->Vertices.Get(Triangle.T0);
		
		FVector A = T0.Position;
		FVector B = MeshInfo->Vertices.Get(Triangle.T1).Position;
		FVector C = MeshInfo->Vertices.Get(Triangle.T2).Position;
		const FVector Centroid = (A + B + C) / 3.0f;
		
		FOpenLandMeshPoint Point;
		Point.Position = Centroid;
		Point.Normal = T0.Normal;
		Point.TangentX = T0.Tangent.TangentX;

		PointList.Push(Point);
	}

	return PointList;
}

TArray<FOpenLandMeshPoint> FOpenLandPointsBuilder::BuildPointsOnTriangle(FSimpleMeshInfoPtr MeshInfo, int32 TriangleIndex, int32 Count, float MinRadius)
{
	const FVector ZVector = {0, 0, 1};
	const FOpenLandMeshTriangle MeshTriangle = MeshInfo->Triangles.Get(TriangleIndex);
	
	FVector P0 = MeshInfo->Vertices.Get(MeshTriangle.T0).Position;
	FVector P1 = MeshInfo->Vertices.Get(MeshTriangle.T1).Position;
	FVector P2 = MeshInfo->Vertices.Get(MeshTriangle.T2).Position;
	
	const FVector FaceNormal = MeshInfo->Vertices.Get(MeshTriangle.T0).Normal;
	const FVector TangentX = MeshInfo->Vertices.Get(MeshTriangle.T0).Tangent.TangentX;
	const FVector Centroid = (P0 + P1 + P2) / 3;

	// Bring the Triangle the Center & Default Plane
	const FOpenLandUpVectorSwitcher SwitchToDefaultPlane(FaceNormal, ZVector);
	const FOpenLandPointTriangle PointTriangle = {
		SwitchToDefaultPlane.Switch(P0-Centroid),
		SwitchToDefaultPlane.Switch(P1-Centroid),
		SwitchToDefaultPlane.Switch(P2-Centroid)
	};

	// ----------------------------------------------------------------- //

	TArray<FOpenLandMeshPoint> LocalPoints;
	const float CellWidth = MinRadius / FMath::Sqrt(2);

	// Calculate Points
	FOpenLandMeshGrid Grid(CellWidth);
	Grid.AddPoint(PointTriangle.A);
	Grid.AddPoint(PointTriangle.B);
	Grid.AddPoint(PointTriangle.C);
	Grid.MakeBounds();

	TMap<int32, int32> PointsInCell;

	// We need iterations more than "Count" to get those number of points
	// That's because we can continue to the loop for various reasons
	// This "Count*4" is a safe maximum amount
	for (int32 Index=0; Index<Count*4; Index++)
	{
		if (LocalPoints.Num() >= Count)
		{
			break;
		}
		
		// Find a random point on the triangle
		FVector NewPoint = PointTriangle.FindRandomPoint(MinRadius/2.0f);
		
		// Find the cell of that point
		int32 NewCellId = Grid.FindCellId(NewPoint);

		// If there's a point on the cell continue
		if (PointsInCell.Find(NewCellId) != nullptr)
		{
			continue;
		}
		
		// Get cells around the new point
		TArray<int32> CellsAround = Grid.FindCellsAround(NewCellId, 2);
		
		// Check all the points on those cells for minradius conflicts
		bool bFoundConflicts = false;
		for(const int32 CellIndex: CellsAround)
		{
			if (PointsInCell.Find(CellIndex) == nullptr)
			{
				continue;
			}

			FVector PointInCell = LocalPoints[PointsInCell[CellIndex]].Position;
			float Distance = FVector::Distance(PointInCell, NewPoint);
			if (Distance <= MinRadius)
			{
				bFoundConflicts = true;
				break;
			}
		}

		// Continue, if there are conflicts
		if (bFoundConflicts)
		{
			continue;
		}
		
		// If not add that point
		PointsInCell.Add(Grid.FindCellId(NewPoint), LocalPoints.Num());
		LocalPoints.Push({NewPoint, ZVector});
	}
	
	// ----------------------------------------------------------------- //

	// Bring Points to the Desired Place
	const FOpenLandUpVectorSwitcher SwitchToOriginalPlane(ZVector, FaceNormal);
	for(FOpenLandMeshPoint &Point: LocalPoints)
	{
		Point.Position = SwitchToOriginalPlane.Switch(Point.Position);
		Point.Position = Point.Position + Centroid;
		Point.Normal = FaceNormal;
		Point.TangentX = TangentX;
	}

	return LocalPoints;
}
