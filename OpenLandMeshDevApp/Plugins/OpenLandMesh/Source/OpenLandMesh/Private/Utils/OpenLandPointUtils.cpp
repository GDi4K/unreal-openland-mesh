// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Utils/OpenLandPointUtils.h"

void FOpenLandPointUtils::ApplyPointRandomization(FOpenLandInstancingRequestPoint& RequestPoint,
                                                  FOpenLandInstancingRules Rules)
{
	RequestPoint.RandomScale = {
		FMath::RandRange(Rules.RandomScale3D.Min.X, Rules.RandomScale3D.Max.X),
		FMath::RandRange(Rules.RandomScale3D.Min.Y, Rules.RandomScale3D.Max.Y),
		FMath::RandRange(Rules.RandomScale3D.Min.Z, Rules.RandomScale3D.Max.Z)
	};

	RequestPoint.RandomScale = RequestPoint.RandomScale * FMath::RandRange(Rules.RandomScaleUniform.Min, Rules.RandomScaleUniform.Max);
	
	RequestPoint.RandomRotation = {
    	FMath::RandRange(Rules.RandomRotation.Min.X, Rules.RandomRotation.Max.X),
    	FMath::RandRange(Rules.RandomRotation.Min.Y, Rules.RandomRotation.Max.Y),
    	FMath::RandRange(Rules.RandomRotation.Min.Z, Rules.RandomRotation.Max.Z)
    };
	RequestPoint.bUseLocalRandomRotation = Rules.RandomRotation.bUseLocalRotation;

	RequestPoint.RandomDisplacement = {
		FMath::RandRange(Rules.RandomDisplacement.Min.X, Rules.RandomDisplacement.Max.X),
		FMath::RandRange(Rules.RandomDisplacement.Min.Y, Rules.RandomDisplacement.Max.Y),
		FMath::RandRange(Rules.RandomDisplacement.Min.Z, Rules.RandomDisplacement.Max.Z)
	};
	RequestPoint.bUseLocalRandomDisplacement = Rules.RandomDisplacement.bUseLocalDisplacement;
}

void FOpenLandPointUtils::CalculateTangentX(FOpenLandInstancingRequestPoint& RequestPoint,
												FOpenLandInstancingRules Rules)
{
	if (Rules.CalculateTangentX == ITG_FROM_UVS)
	{
		return;
	}

	if (Rules.CalculateTangentX == ITG_TO_PIVOT)
	{
		RequestPoint.TangentX = RequestPoint.Position.GetSafeNormal();
		return;
	}

	if (Rules.CalculateTangentX == ITG_TO_CENTER)
	{
		const FVector Center = {0, 0, RequestPoint.Position.Z};
		RequestPoint.TangentX = (Center - RequestPoint.Position).GetSafeNormal();
		RequestPoint.Normal = {0.0, 0.0, 1.0};
		return;
	}
}

