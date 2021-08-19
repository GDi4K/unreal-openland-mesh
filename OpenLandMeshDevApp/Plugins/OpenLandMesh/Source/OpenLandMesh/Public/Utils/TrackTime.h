// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

class OPENLANDMESH_API TrackTime
{
	FString Caption = "";
	FDateTime StartTime;
	bool bEnable = false;

public:
	TrackTime(FString C, bool bEnableIt = false);
	void Finish() const;
};
