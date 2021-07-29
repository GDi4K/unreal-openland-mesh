#pragma once

class TrackTime
{
	FString Caption;
	FDateTime StartTime;
public:
	TrackTime(FString Caption);
	void Finish();
};
