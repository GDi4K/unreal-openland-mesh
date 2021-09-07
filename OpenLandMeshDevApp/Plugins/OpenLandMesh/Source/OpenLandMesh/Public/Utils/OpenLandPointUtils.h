#pragma once
#include "API/OpenLandInstancingController.h"

class FOpenLandPointUtils
{
	
public:
	static void ApplyPointRandomization(FOpenLandInstancingRequestPoint& RequestPoint, FOpenLandInstancingRules Rules);
	static void CalculateTangentX(FOpenLandInstancingRequestPoint& RequestPoint, FOpenLandInstancingRules Rules);
};
