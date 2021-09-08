// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Utils/OpenLandPointTriangle.h"
#include "Utils/OpenLandPointLine.h"

FOpenLandPointTriangle::FOpenLandPointTriangle(FVector T0, FVector T1, FVector T2)
{	
	A = T0;
	B = T1;
	C = T2;
}

FVector FOpenLandPointTriangle::FindRandomPoint(float EdgeDistance) const
{
	TArray<FVector> Points = {A, B, C};

	if (EdgeDistance != 0.0f)
	{
	    // Move the points into the triangle with specified EdgeDistance if possible
		const FVector Centroid = GetCentroid();
		for (int32 PointIndex=0; PointIndex<3; PointIndex++)
		{
			FVector ToCentroid = Centroid - Points[PointIndex];
			// There is no point on moving points if the triangle is small
			if (ToCentroid.Size() <= EdgeDistance)
			{
				continue;
			}
			
			FVector Direction = ToCentroid.GetSafeNormal();
			Points[PointIndex] += Direction * EdgeDistance;
		}
	}
	
	TArray<FOpenLandPointLine> Lines = {
		{Points[0], Points[1]},
		{Points[0], Points[2]},
		{Points[1], Points[2]}
	};

	const int32 FirstLineIndex = FMath::RandRange(0, 2);
	const FOpenLandPointLine FirstLine = Lines[FirstLineIndex];
	Lines.RemoveAt(FirstLineIndex);

	const int32 SecondLineIndex = FMath::RandRange(0, 1);
	const FOpenLandPointLine SecondLine = Lines[SecondLineIndex];
		
	const FVector X = FirstLine.Interpolate(FMath::RandRange(0.0f, 1.0f));
	const FVector Y = SecondLine.Interpolate(FMath::RandRange(0.0f, 1.0f));
	
	return FOpenLandPointLine(X, Y).Interpolate(FMath::RandRange(0.0f, 1.0f));
}

float FOpenLandPointTriangle::FindArea() const
{
	return FindArea(A, B, C);
}

float FOpenLandPointTriangle::FindArea(FVector T0, FVector T1, FVector T2)
{
	return FVector::CrossProduct(T0-T2, T0-T1).Size()/2;
}

float GetSign(FVector P1, FVector P2, FVector P3)
{
	return (P1.X - P3.X) * (P2.Y - P3.Y) - (P2.X - P3.X) * (P1.Y - P3.Y);
}

bool FOpenLandPointTriangle::IsPointInside(FVector Point) const
{
	const float D1 = GetSign(Point, A, B);
	const float D2 = GetSign(Point, B, C);
	const float D3 = GetSign(Point, C, A);

	const bool HasNeg = (D1 < 0) || (D2 < 0) || (D3 < 0);
	const bool HasPos = (D1 > 0) || (D2 > 0) || (D3 > 0);

	return !(HasNeg && HasPos);
}

FVector FOpenLandPointTriangle::GetCentroid() const
{
	return (A + B + C) / 3.0f;
}
