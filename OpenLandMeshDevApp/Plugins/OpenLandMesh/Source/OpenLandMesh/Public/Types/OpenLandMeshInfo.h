// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "OpenLandMesh/Public/Types/OpenLandMeshVertex.h"
#include "OpenLandMesh/Public/Types/OpenLandMeshTriangle.h"
#include "Types/OpenLandArray.h"

class FOpenLandMeshInfo;
typedef TSharedPtr<FOpenLandMeshInfo, ESPMode::ThreadSafe> FSimpleMeshInfoPtr;

class OPENLANDMESH_API FOpenLandMeshInfo
{
	bool bLocked = false;

public:
	TOpenLandArray<FOpenLandMeshVertex> Vertices;
	TOpenLandArray<FOpenLandMeshTriangle> Triangles;
	FBox BoundingBox;
	bool bEnableCollision = true;
	bool bSectionVisible = true;

	FOpenLandMeshInfo();

	~FOpenLandMeshInfo();

	void Release();

	void Freeze();

	bool IsLocked() const;

	void Lock();

	void UnLock();

	FSimpleMeshInfoPtr Clone();

	static FSimpleMeshInfoPtr New();
};
