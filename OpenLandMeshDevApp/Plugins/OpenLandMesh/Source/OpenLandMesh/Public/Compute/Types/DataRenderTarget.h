// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "Engine/TextureRenderTarget2D.h"

class OPENLANDMESH_API FDataRenderTarget
{
	int32 TextureWidth = 0;
	bool bIsReadingData = false;

	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

public:
	~FDataRenderTarget();
	// Getters
	bool IsReadingData() const { return bIsReadingData; }

	FDataRenderTarget(UObject* WorldContext, int32 Width);
	bool DrawMaterial(UObject* WorldContext, UMaterialInterface* Material);
	bool ReadDataAsync(int32 RowStart, int32 RowEnd, TArray<FColor>& ModifiedData, TFunction<void()> ReadCompleteCallback);
};
