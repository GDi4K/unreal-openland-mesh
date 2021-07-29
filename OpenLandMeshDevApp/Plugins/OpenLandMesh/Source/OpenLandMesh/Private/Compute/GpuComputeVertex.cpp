#include "Compute/GpuComputeVertex.h"

#include "Kismet/KismetMaterialLibrary.h"

void FGpuComputeVertex::Init(UObject* WorldContext, TArray<FGpuComputeVertexInput>& SourceData,
                             int32 Width)
{
	// Make the Source Data Textures
	DataTexture0 = MakeShared<FDataTexture>(Width);
	DataTexture1 = MakeShared<FDataTexture>(Width);
	DataTexture2 = MakeShared<FDataTexture>(Width);
	UpdateSourceData(SourceData);

	// Create the Render Targets
	DataRenderTarget0 = MakeShared<FDataRenderTarget>(WorldContext, Width);
	DataRenderTarget1 = MakeShared<FDataRenderTarget>(WorldContext, Width);
	DataRenderTarget2 = MakeShared<FDataRenderTarget>(WorldContext, Width);
	DataRenderTarget3 = MakeShared<FDataRenderTarget>(WorldContext, Width);
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

void FGpuComputeVertex::UpdateSourceData(TArray<FGpuComputeVertexInput>& SourceData)
{
	const int32 PixelCount = FMath::Min(SourceData.Num(),
	                                    DataTexture0->GetTextureWidth() * DataTexture0->GetTextureWidth());
	for (int32 Index = 0; Index < PixelCount; Index++)
	{
		float* ValuePointer0 = &SourceData[Index].Position.X;
		uint8* ValueBytes0 = reinterpret_cast<uint8*>(ValuePointer0);
		DataTexture0->SetPixelValue(Index, ValueBytes0[0], ValueBytes0[1], ValueBytes0[2], ValueBytes0[3]);

		float* ValuePointer1 = &SourceData[Index].Position.Y;
		uint8* ValueBytes1 = reinterpret_cast<uint8*>(ValuePointer1);
		DataTexture1->SetPixelValue(Index, ValueBytes1[0], ValueBytes1[1], ValueBytes1[2], ValueBytes1[3]);

		float* ValuePointer2 = &SourceData[Index].Position.Z;
		uint8* ValueBytes2 = reinterpret_cast<uint8*>(ValuePointer2);
		DataTexture2->SetPixelValue(Index, ValueBytes2[0], ValueBytes2[1], ValueBytes2[2], ValueBytes2[3]);
	}

	DataTexture0->UpdateTexture();
	DataTexture1->UpdateTexture();
	DataTexture2->UpdateTexture();
}

void FGpuComputeVertex::Compute(UObject* WorldContext, TArray<FGpuComputeVertexOutput>& ModifiedData,
                                FComputeMaterial ComputeMaterial)
{
	checkf(ComputeMaterial.Material != nullptr, TEXT("Compute Material Needs a Proper Material"));

	// Create the Dynamic Materials`
	DynamicMaterialInstance0 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance0->SetTextureParameterValue("InputFloat0", DataTexture0->GetTexture());
	DynamicMaterialInstance0->SetTextureParameterValue("InputFloat1", DataTexture1->GetTexture());
	DynamicMaterialInstance0->SetTextureParameterValue("InputFloat2", DataTexture2->GetTexture());
	DynamicMaterialInstance0->SetScalarParameterValue("OutputFloat", 0);

	DynamicMaterialInstance1 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance1->SetTextureParameterValue("InputFloat0", DataTexture0->GetTexture());
	DynamicMaterialInstance1->SetTextureParameterValue("InputFloat1", DataTexture1->GetTexture());
	DynamicMaterialInstance1->SetTextureParameterValue("InputFloat2", DataTexture2->GetTexture());
	DynamicMaterialInstance1->SetScalarParameterValue("OutputFloat", 1);

	DynamicMaterialInstance2 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance2->SetTextureParameterValue("InputFloat0", DataTexture0->GetTexture());
	DynamicMaterialInstance2->SetTextureParameterValue("InputFloat1", DataTexture1->GetTexture());
	DynamicMaterialInstance2->SetTextureParameterValue("InputFloat2", DataTexture2->GetTexture());
	DynamicMaterialInstance2->SetScalarParameterValue("OutputFloat", 2);

	DynamicMaterialInstance3 = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
		WorldContext, ComputeMaterial.Material);
	DynamicMaterialInstance3->SetTextureParameterValue("InputFloat0", DataTexture0->GetTexture());
	DynamicMaterialInstance3->SetTextureParameterValue("InputFloat1", DataTexture1->GetTexture());
	DynamicMaterialInstance3->SetTextureParameterValue("InputFloat2", DataTexture2->GetTexture());
	DynamicMaterialInstance3->SetScalarParameterValue("OutputVertexColor", 1); // This indicate vertex colors

	ApplyParameterValues(DynamicMaterialInstance0, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance1, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance2, ComputeMaterial.Parameters);
	ApplyParameterValues(DynamicMaterialInstance3, ComputeMaterial.Parameters);

	// Draw the RenderTarget
	DataRenderTarget0->DrawMaterial(WorldContext, DynamicMaterialInstance0);
	DataRenderTarget1->DrawMaterial(WorldContext, DynamicMaterialInstance1);
	DataRenderTarget2->DrawMaterial(WorldContext, DynamicMaterialInstance2);
	DataRenderTarget3->DrawMaterial(WorldContext, DynamicMaterialInstance3);

	// Read from the Render Target
	const int32 NumPixelsToRead = DataTexture0->GetTextureWidth() * DataTexture0->GetTextureWidth();
	TArray<FColor> ReadBuffer0;
	TArray<FColor> ReadBuffer1;
	TArray<FColor> ReadBuffer2;
	TArray<FColor> ReadBuffer3;
	ReadBuffer0.SetNumUninitialized(NumPixelsToRead);
	ReadBuffer1.SetNumUninitialized(NumPixelsToRead);
	ReadBuffer2.SetNumUninitialized(NumPixelsToRead);
	ReadBuffer3.SetNumUninitialized(NumPixelsToRead);

	DataRenderTarget0->ReadDataAsync(ReadBuffer0, nullptr);
	DataRenderTarget1->ReadDataAsync(ReadBuffer1, nullptr);
	DataRenderTarget2->ReadDataAsync(ReadBuffer2, nullptr);
	DataRenderTarget3->ReadDataAsync(ReadBuffer3, nullptr);
	// This will block the game thread, until it finishes all the rendering commands
	// That's why we don't need to use the callback above
	FlushRenderingCommands();

	// Write them into the Modified Buffer
	const int WritePixelCount = FMath::Min(NumPixelsToRead, ModifiedData.Num());
	for (int32 Index = 0; Index < WritePixelCount; Index++)
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

void FGpuComputeVertex::ApplyParameterValues(UMaterialInstanceDynamic* Material,
                                             TArray<FComputeMaterialParameter> MaterialParameters)
{
	for (auto ParamInfo : MaterialParameters)
		if (ParamInfo.Type == CMPT_SCALAR)
			Material->SetScalarParameterValue(ParamInfo.Name, ParamInfo.ScalarValue);
		else if (ParamInfo.Type == CMPT_VECTOR)
			Material->SetVectorParameterValue(ParamInfo.Name, ParamInfo.VectorValue);
}

FGpuComputeVertex::~FGpuComputeVertex()
{
	UE_LOG(LogTemp, Warning, TEXT("Deleting... FGpuComputeVector"))
}
