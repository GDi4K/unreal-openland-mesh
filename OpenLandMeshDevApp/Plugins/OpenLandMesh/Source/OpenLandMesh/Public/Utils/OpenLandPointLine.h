// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

class OPENLANDMESH_API FOpenLandPointLine
{
	public:
	
	FVector A;
	FVector B;
	FVector C;

	FOpenLandPointLine(FVector P0, FVector P1);
	FVector Interpolate(float Amount) const;
};
