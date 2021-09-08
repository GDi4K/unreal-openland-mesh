// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once
#include "API/OpenLandInstancingController.h"

class FOpenLandPointUtils
{
	
public:
	static void ApplyPointRandomization(FOpenLandInstancingRequestPoint& RequestPoint, FOpenLandInstancingRules Rules);
	static void CalculateTangentX(FOpenLandInstancingRequestPoint& RequestPoint, FOpenLandInstancingRules Rules);
};
