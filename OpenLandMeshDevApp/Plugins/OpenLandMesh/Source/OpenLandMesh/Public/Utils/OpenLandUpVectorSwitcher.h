#pragma once

class OPENLANDMESH_API FOpenLandUpVectorSwitcher
{
	FVector Axis;
	float AngleInRadians;
	float AngleInDegrees;
public:
	FOpenLandUpVectorSwitcher(FVector Current, FVector Desired);
	FVector Switch(FVector Position) const;
};
