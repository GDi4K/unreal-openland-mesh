#pragma once

class OPENLANDMESH_API FOpenLandThreading
{
public:
	static FGraphEventRef RunOnGameThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyBackgroundThread(TFunction<void()> InFunction);
};
