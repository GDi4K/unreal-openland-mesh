// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

class OPENLANDMESH_API FOpenLandThreading
{
public:
	static FGraphEventRef RunOnGameThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyBackgroundThread(TFunction<void()> InFunction);
};
