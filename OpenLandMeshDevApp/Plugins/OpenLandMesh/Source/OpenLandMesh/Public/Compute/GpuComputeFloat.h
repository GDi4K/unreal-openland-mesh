#pragma once
#include <functional>

#include "Compute/Types/DataRenderTarget.h"
#include "Compute/Types/DataTexture.h"

class OPENLANDMESH_API FGpuComputeFloat
{
	UPROPERTY()
	;
	UMaterialInstanceDynamic* DynamicMaterialInstance = nullptr;

	TSharedPtr<FDataTexture> DataTexture = nullptr;
	TSharedPtr<FDataRenderTarget> DataRenderTarget = nullptr;

public:
	void Init(UObject* WorldContext, TArray<float>& SourceData, UMaterialInterface* Material, int32 Width);
	void UpdateSourceData(TArray<float>& SourceData);
	void Compute(UObject* WorldContext, TArray<float>& ModifiedData);
};
