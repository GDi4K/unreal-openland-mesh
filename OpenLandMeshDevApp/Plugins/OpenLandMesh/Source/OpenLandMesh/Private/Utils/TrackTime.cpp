// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Utils/TrackTime.h"

TrackTime::TrackTime(FString CaptionInput, bool bEnableIt)
{
	Caption = CaptionInput;
	StartTime = FDateTime::Now();
	bEnable = bEnableIt;
}

void TrackTime::Finish() const
{
	if (!bEnable)
	{
		return;
	}
	
	const float TimeDiff = (FDateTime::Now() - StartTime).GetTotalMilliseconds();
	UE_LOG(LogTemp, Log, TEXT("TrackTime: %s for: %fms"), *Caption, TimeDiff);
}
