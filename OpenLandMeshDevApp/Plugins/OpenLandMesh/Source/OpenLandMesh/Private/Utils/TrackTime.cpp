#include "Utils/TrackTime.h"

TrackTime::TrackTime(FString CaptionInput)
{
	Caption = CaptionInput;
	StartTime = FDateTime::Now();
}

void TrackTime::Finish()
{
	return;
	const float TimeDiff = (FDateTime::Now() - StartTime).GetTotalMilliseconds();
	UE_LOG(LogTemp, Log, TEXT("TrackTime: %s for: %fms"), *Caption, TimeDiff);
}
