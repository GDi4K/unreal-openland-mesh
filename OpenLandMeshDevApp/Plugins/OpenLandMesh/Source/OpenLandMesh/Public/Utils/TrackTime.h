// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

class OPENLANDMESH_API TrackTime
{
	FString Caption = "";
	FDateTime StartTime;

public:
	TrackTime(FString C);
	void Finish();
};
