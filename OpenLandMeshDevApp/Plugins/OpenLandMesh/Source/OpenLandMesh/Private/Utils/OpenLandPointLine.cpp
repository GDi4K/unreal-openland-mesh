#include "Utils/OpenLandPointLine.h"

FOpenLandPointLine::FOpenLandPointLine(FVector P0, FVector P1)
{
	A = P0;
	B = P1;
}

FVector FOpenLandPointLine::Interpolate(float Amount) const
{
	return (A * Amount) + (B * (1-Amount));
}
