// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Compute/GpuComputeVertex.h"
#include "Kismet/KismetMaterialLibrary.h"

void FGpuComputeVertex::Init(UObject* WorldContext, int32 Width)
{
	TextureWidth = Width;

	// Create the Render Targets
	DataRenderTarget0 = MakeShared<FDataRenderTarget>(WorldContext, TextureWidth);
	DataRenderTarget1 = MakeShared<FDataRenderTarget>(WorldContext, TextureWidth);
	DataRenderTarget2 = MakeShared<FDataRenderTarget>(WorldContext, TextureWidth);
	DataRenderTarget3 = MakeShared<FDataRenderTarget>(WorldContext, TextureWidth);
}

FGpuComputeMaterialStatus FGpuComputeVertex::IsValidMaterial(UMaterialInterface* Material)
{
	if (Material == nullptr)
		return {
			false,
			"Material does not exists."
		};

	if (Material->GetBlendMode() != BLEND_AlphaComposite)
		return {
			false,
			"Material should use the AlphaCompute Blend Mode"
		};

	// Check whether this uses correct nodes by inspecting some parameter info.

	return {
		true,
		"Material is valid!"
	};
}

void FGpuComputeVertex::Compute(UObject* WorldContext, TArray<FGpuComputeVertexDataTextureItem> DataTextures, FComputeMaterial ComputeMaterial)
{
	checkf(ComputeMaterial.Material != nullptr, TEXT("Compute Material Needs a Proper Material"));

	// Create the Dynamic Materials`
	DynamicMaterialInstance0 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance0->SetScalarParameterValue("OutputFloat", 0);

	DynamicMaterialInstance1 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance1->SetScalarParameterValue("OutputFloat", 1);

	DynamicMaterialInstance2 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance2->SetScalarParameterValue("OutputFloat", 2);

	DynamicMaterialInstance3 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance3->SetScalarParameterValue("OutputVertexColor", 1); // This indicate vertex colors

	ApplyParameterValues(DynamicMaterialInstance0, DataTextures, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance1, DataTextures, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance2, DataTextures, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance3, DataTextures, ComputeMaterial.Parameters);

	// Draw the RenderTarget
	DataRenderTarget0->DrawMaterial(WorldContext, DynamicMaterialInstance0);
	DataRenderTarget1->DrawMaterial(WorldContext, DynamicMaterialInstance1);
	DataRenderTarget2->DrawMaterial(WorldContext, DynamicMaterialInstance2);
	DataRenderTarget3->DrawMaterial(WorldContext, DynamicMaterialInstance3);
}

void FGpuComputeVertex::ReadData(TArray<FGpuComputeVertexOutput>& ModifiedData, int32 RowStart, int32 RowEnd) const
{
	// Read from the Render Target
	const int32 NumPixelsToRead = FMath::Min(TextureWidth * (RowEnd - RowStart), ModifiedData.Num());
	TArray<FColor> ReadBuffer0;
	TArray<FColor> ReadBuffer1;
	TArray<FColor> ReadBuffer2;
	TArray<FColor> ReadBuffer3;

	DataRenderTarget0->ReadDataAsync(RowStart, RowEnd, ReadBuffer0, nullptr);
	DataRenderTarget1->ReadDataAsync(RowStart, RowEnd, ReadBuffer1, nullptr);
	DataRenderTarget2->ReadDataAsync(RowStart, RowEnd, ReadBuffer2, nullptr);
	DataRenderTarget3->ReadDataAsync(RowStart, RowEnd, ReadBuffer3, nullptr);
	// This will block the game thread, until it finishes all the rendering commands
	// That's why we don't need to use the callback above
	FlushRenderingCommands();

	// Write them into the Modified Buffer
	for (int32 Index = 0; Index < NumPixelsToRead; Index++)
	{
		const FColor Color0 = ReadBuffer0[Index];
		float& Result0 = ModifiedData[Index].Position.X;
		float* ResultData0 = &Result0;
		uint8* ResultBytes0 = reinterpret_cast<uint8*>(ResultData0);
		ResultBytes0[0] = Color0.R;
		ResultBytes0[1] = Color0.G;
		ResultBytes0[2] = Color0.B;
		ResultBytes0[3] = Color0.A;

		const FColor Color1 = ReadBuffer1[Index];
		float& Result1 = ModifiedData[Index].Position.Y;
		float* ResultData1 = &Result1;
		uint8* ResultBytes1 = reinterpret_cast<uint8*>(ResultData1);

		ResultBytes1[0] = Color1.R;
		ResultBytes1[1] = Color1.G;
		ResultBytes1[2] = Color1.B;
		ResultBytes1[3] = Color1.A;

		const FColor Color2 = ReadBuffer2[Index];
		float& Result2 = ModifiedData[Index].Position.Z;
		float* ResultData2 = &Result2;
		uint8* ResultBytes2 = reinterpret_cast<uint8*>(ResultData2);

		ResultBytes2[0] = Color2.R;
		ResultBytes2[1] = Color2.G;
		ResultBytes2[2] = Color2.B;
		ResultBytes2[3] = Color2.A;

		// Apply Vertex Colors
		ModifiedData[Index].VertexColor = ReadBuffer3[Index];
	}
}

bool FGpuComputeVertex::IsActive() const
{
	return DataRenderTarget0->IsActive() && DataRenderTarget1->IsActive() && DataRenderTarget2->IsActive() && DataRenderTarget3->IsActive();
}


void FGpuComputeVertex::ApplyParameterValues(UMaterialInstanceDynamic* Material, TArray<FGpuComputeVertexDataTextureItem> DataTextures,
                                             TArray<FComputeMaterialParameter> MaterialParameters)
{
	for (FGpuComputeVertexDataTextureItem DataTextureItem: DataTextures)
	{
		Material->SetTextureParameterValue(FName(DataTextureItem.Name), DataTextureItem.DataTexture->GetTexture());
	}
	
	for (auto ParamInfo : MaterialParameters)
    {
    	switch (ParamInfo.Type)
    	{
    		case CMPT_SCALAR:
    			Material->SetScalarParameterValue(ParamInfo.Name, ParamInfo.ScalarValue);
    			break;
    		case CMPT_VECTOR:
    			Material->SetVectorParameterValue(ParamInfo.Name, ParamInfo.VectorValue);
    			break;
    		case CMPT_TEXTURE:
    			Material->SetTextureParameterValue(ParamInfo.Name, ParamInfo.TextureValue);
    			break;
    		default:
    			checkf(false, TEXT("Unknown Compute Material parameter type"));
    		
    	}
    }
}

FGpuComputeVertex::~FGpuComputeVertex()
{
	
}
