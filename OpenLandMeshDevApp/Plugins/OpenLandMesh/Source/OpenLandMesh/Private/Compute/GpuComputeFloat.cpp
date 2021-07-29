// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Compute/GpuComputeFloat.h"

#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"

void FGpuComputeFloat::Init(UObject* WorldContext, TArray<float>& SourceData, UMaterialInterface* Material, int32 Width)
{
	checkf(Material != nullptr, TEXT("Compute Material should be a valid one"))

	// Make the Source Data Texture
	DataTexture = MakeShared<FDataTexture>(Width);
	UpdateSourceData(SourceData);

	// Create the Render Target
	DataRenderTarget = MakeShared<FDataRenderTarget>(WorldContext, Width);

	// Create the Dynamic Material
	DynamicMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(WorldContext, Material);
	DynamicMaterialInstance->SetTextureParameterValue("InputFloat", DataTexture->GetTexture());
}

void FGpuComputeFloat::UpdateSourceData(TArray<float>& SourceData)
{
	const int32 PixelCount = FMath::Min(SourceData.Num(),
	                                    DataTexture->GetTextureWidth() * DataTexture->GetTextureWidth());
	for (int32 Index = 0; Index < PixelCount; Index++)
	{
		float* ValuePointer = &SourceData[Index];
		uint8* ValueBytes = reinterpret_cast<uint8*>(ValuePointer);
		DataTexture->SetPixelValue(Index, ValueBytes[0], ValueBytes[1], ValueBytes[2], ValueBytes[3]);
	}

	DataTexture->UpdateTexture();
}

void FGpuComputeFloat::Compute(UObject* WorldContext, TArray<float>& ModifiedData)
{
	// Draw the RenderTarget
	DataRenderTarget->DrawMaterial(WorldContext, DynamicMaterialInstance);

	// Read from the Render Target
	const int32 NumPixelsToRead = DataTexture->GetTextureWidth() * DataTexture->GetTextureWidth();
	TArray<FColor> ReadBuffer;
	ReadBuffer.SetNumUninitialized(NumPixelsToRead);

	DataRenderTarget->ReadDataAsync(ReadBuffer, nullptr);
	// This will block the game thread, until it finishes all the rendering commands
	// That's why we don't need to use the callback above
	FlushRenderingCommands();

	// Write them into the Modified Buffer
	const int WritePixelCount = FMath::Min(NumPixelsToRead, ModifiedData.Num());
	for (int32 Index = 0; Index < WritePixelCount; Index++)
	{
		const FColor Color = ReadBuffer[Index];
		float& Result = ModifiedData[Index];
		float* ResultData = &Result;
		uint8* ResultBytes = reinterpret_cast<uint8*>(ResultData);

		ResultBytes[0] = Color.R;
		ResultBytes[1] = Color.G;
		ResultBytes[2] = Color.B;
		ResultBytes[3] = Color.A;
	}
}
