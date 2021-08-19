// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

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
	FVector2D UV0;
};

struct FGpuComputeVertexOutput
{
	FVector Position;
	FColor VertexColor;
};

struct FGpuComputeVertexDataTextureItem
{
	FString Name;
	TSharedPtr<FDataTexture> DataTexture;
};

struct FGpuComputeVertexFetchOptions
{
	int32 RowStart = 0;
	int32 RowEnd = -1;
};

class OPENLANDMESH_API FGpuComputeVertex
{
	UPROPERTY(Transient);
	UMaterialInstanceDynamic* DynamicMaterialInstance0 = nullptr; // to Write X to Render Target
	
	UPROPERTY(Transient);
	UMaterialInstanceDynamic* DynamicMaterialInstance1 = nullptr; // to Write Y to Render Target
	
	UPROPERTY(Transient);
	UMaterialInstanceDynamic* DynamicMaterialInstance2 = nullptr; // to Write Z to Render Target
	
	UPROPERTY(Transient);
	UMaterialInstanceDynamic* DynamicMaterialInstance3 = nullptr; // to Write VertexColor to Render Target

	int32 TextureWidth=0;

	TSharedPtr<FDataRenderTarget> DataRenderTarget0 = nullptr; // for X float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget1 = nullptr; // for Y float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget2 = nullptr; // for Z float of Vector
	TSharedPtr<FDataRenderTarget> DataRenderTarget3 = nullptr; // for Vertex Color

	void ApplyParameterValues(UMaterialInstanceDynamic* Material, TArray<FGpuComputeVertexDataTextureItem> DataTextures,
	                                 TArray<FComputeMaterialParameter> MaterialParameters);

public:
	~FGpuComputeVertex();
	void Init(UObject* WorldContext, int32 Width);
	static FGpuComputeMaterialStatus IsValidMaterial(UMaterialInterface* Material);
	void Compute(UObject* WorldContext, TArray<FGpuComputeVertexDataTextureItem> DataTextures, TArray<FGpuComputeVertexOutput>& ModifiedData,
	             FComputeMaterial ComputeMaterial, FGpuComputeVertexFetchOptions FetchOptions);
};
