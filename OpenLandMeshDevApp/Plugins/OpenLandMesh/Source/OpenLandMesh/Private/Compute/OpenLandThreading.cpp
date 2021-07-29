#pragma once

#include "Compute/OpenLandThreading.h"

FGraphEventRef FOpenLandThreading::RunOnGameThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

FGraphEventRef FOpenLandThreading::RunOnAnyThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::AnyThread);
}

FGraphEventRef FOpenLandThreading::RunOnAnyBackgroundThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr,
	                                                      ENamedThreads::AnyBackgroundThreadNormalTask);
}
