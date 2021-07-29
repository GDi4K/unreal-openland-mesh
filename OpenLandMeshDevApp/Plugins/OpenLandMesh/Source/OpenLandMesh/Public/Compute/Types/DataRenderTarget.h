#pragma once

class OPENLANDMESH_API FDataRenderTarget
{
	int32 TextureWidth = 0;
	bool bIsReadingData = false;

	UPROPERTY()
	;
	UTextureRenderTarget2D* RenderTarget;

public:
	~FDataRenderTarget();
	// Getters
	bool IsReadingData() const { return bIsReadingData; }

	FDataRenderTarget(UObject* WorldContext, int32 Width);
	bool DrawMaterial(UObject* WorldContext, UMaterialInterface* Material);
	bool ReadDataAsync(TArray<FColor>& ModifiedData, TFunction<void()> ReadCompleteCallback);
};
