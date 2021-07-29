#pragma once

class OPENLANDMESH_API TrackTime
{

	FString Caption = "";
	FDateTime StartTime;

public:
	TrackTime(FString C);
	void Finish();
};

