// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

class OPENLANDMESH_API FOpenLandPointTriangle
{
	public:;
	
	FVector A;
	FVector B;
	FVector C;

	FOpenLandPointTriangle(FVector T0, FVector T1, FVector T2);
	FVector GetCentroid() const;
	FVector FindRandomPoint(float EdgeDistance = 0.0f) const;
	float FindArea() const;
	static float FindArea(FVector T0, FVector T1, FVector T2);
	bool IsPointInside(FVector Point) const;
};
