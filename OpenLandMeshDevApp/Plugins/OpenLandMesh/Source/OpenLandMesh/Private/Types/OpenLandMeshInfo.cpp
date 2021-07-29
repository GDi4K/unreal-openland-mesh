#pragma once

#include "Types/OpenLandMeshInfo.h"

FOpenLandMeshInfo::FOpenLandMeshInfo()
{
	BoundingBox = {};
	BoundingBox.Init();
}

FOpenLandMeshInfo::~FOpenLandMeshInfo()
{
	Release();
}

void FOpenLandMeshInfo::Release()
{
	Vertices.Clear();
	Triangles.Clear();
	BoundingBox = {};
}

void FOpenLandMeshInfo::Freeze()
{
	Vertices.Freeze();
	Triangles.Freeze();
	Triangles.LockForever();
}

bool FOpenLandMeshInfo::IsLocked() const
{
	return bLocked;
}

void FOpenLandMeshInfo::Lock()
{
	bLocked = true;
	Vertices.Lock();
}

void FOpenLandMeshInfo::UnLock()
{
	bLocked = false;
	Vertices.UnLock();
}

FSimpleMeshInfoPtr FOpenLandMeshInfo::Clone()
{
	FSimpleMeshInfoPtr NewMeshInfo = MakeShared<FOpenLandMeshInfo, ESPMode::ThreadSafe>();
	NewMeshInfo->BoundingBox = BoundingBox;
	NewMeshInfo->bEnableCollision = bEnableCollision;
	NewMeshInfo->bSectionVisible = bSectionVisible;

	for (size_t Index = 0; Index < Vertices.Length(); Index++)
		NewMeshInfo->Vertices.Push(Vertices.Get(Index));

	for (size_t Index = 0; Index < Triangles.Length(); Index++)
		NewMeshInfo->Triangles.Push(Triangles.Get(Index));

	return NewMeshInfo;
}

FSimpleMeshInfoPtr FOpenLandMeshInfo::New()
{
	return MakeShared<FOpenLandMeshInfo, ESPMode::ThreadSafe>();
}
