// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Utils/OpenLandUpVectorSwitcher.h"

FOpenLandUpVectorSwitcher::FOpenLandUpVectorSwitcher(FVector Current, FVector Desired)
{
	const float RotateAngleRadians = FMath::Acos(FVector::DotProduct(Current, Desired));
	AngleInDegrees = FMath::RadiansToDegrees(RotateAngleRadians);
	Axis = FVector::CrossProduct(Current, Desired).GetSafeNormal();
}

FVector FOpenLandUpVectorSwitcher::Switch(FVector Position) const
{
	return Position.RotateAngleAxis(AngleInDegrees, Axis);
}
