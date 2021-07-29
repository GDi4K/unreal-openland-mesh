#pragma once
#include "Compute/Types/ComputeMaterial.h"
#include "Compute/Types/DataRenderTarget.h"
#include "Compute/Types/DataTexture.h"

struct FGpuComputeMaterialStatus
{
	bool bIsValid = false;
	FString Message = "";
};

struct FGpuComputeVertexInput
{
	FVector Position;
};

struct FGpuComputeVertexOutput
{
	FVector Position;
	FColor VertexColor;
};

class OPENLANDMESH_API FGpuComputeVertex
{
	UPROPERTY()
	;
	UMaterialInstanceDynamic* DynamicMaterialInstance0 = nullptr; // to Write X to Render Target
	UPROPERTY()
	;
	UMaterialInstanceDynamic* DynamicMaterialInstance1 = nullptr; // to Write Y to Render Target
	UPROPERTY()
	;
	UMaterialInstanceDynamic* DynamicMaterialInstance2 = nullptr; // to Write Z to Render Target
	UPROPERTY()
	;
	UMaterialInstanceDynamic* DynamicMaterialInstance3 = nullptr; // to Write VertexColor to Render Target

	TSharedPtr<FDataTexture> DataTexture0 = nullptr; // Input float X of Vector
	TSharedPtr<FDataTexture> DataTexture1 = nullptr; // Input float X of Vector
	TSharedPtr<FDataTexture> DataTexture2 = nullptr; // Input float X of Vector

	TSharedPtr<FDataRenderTarget> DataRenderTarget0 = nullptr; // for X float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget1 = nullptr; // for Y float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget2 = nullptr; // for Z float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget3 = nullptr; // for Vertex Color

	static void ApplyParameterValues(UMaterialInstanceDynamic* Material,
	                                 TArray<FComputeMaterialParameter> MaterialParameters);

public:
	~FGpuComputeVertex();
	void Init(UObject* WorldContext, TArray<FGpuComputeVertexInput>& SourceData, int32 Width);
	static FGpuComputeMaterialStatus IsValidMaterial(UMaterialInterface* Material);
	void UpdateSourceData(TArray<FGpuComputeVertexInput>& SourceData);
	void Compute(UObject* WorldContext, TArray<FGpuComputeVertexOutput>& ModifiedData,
	             FComputeMaterial ComputeMaterial);
};
