// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "Engine/TextureRenderTarget2D.h"

class OPENLANDMESH_API FDataRenderTarget
{
	int32 TextureWidth = 0;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

public:
	~FDataRenderTarget();

	FDataRenderTarget(UObject* WorldContext, int32 Width);
	bool DrawMaterial(UObject* WorldContext, UMaterialInterface* Material);
	bool ReadDataAsync(int32 RowStart, int32 RowEnd, TArray<FColor>& ModifiedData, TFunction<void()> ReadCompleteCallback);
	bool IsActive() const;
};
