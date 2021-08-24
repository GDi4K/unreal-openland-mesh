// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "Compute/Types/DataRenderTarget.h"
#include "Compute/OpenLandThreading.h"
#include "Kismet/KismetRenderingLibrary.h"

FDataRenderTarget::~FDataRenderTarget()
{
	if (RenderTarget != nullptr)
	{
		if (RenderTarget->IsValidLowLevel())
			RenderTarget->ReleaseResource();
		RenderTarget = nullptr;
	}
}

FDataRenderTarget::FDataRenderTarget(UObject* WorldContext, int32 Width)
{
	TextureWidth = Width;
	RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(WorldContext, TextureWidth, TextureWidth, RTF_RGBA8,
	                                                             FLinearColor(0, 0, 0, 1), false);
	RenderTarget->SRGB = 0;
}

bool FDataRenderTarget::DrawMaterial(UObject* WorldContext, UMaterialInterface* Material)
{
	checkf(Material != nullptr, TEXT("Material should exists to render"))
	checkf(Material->GetBlendMode() == EBlendMode::BLEND_AlphaComposite,
	       TEXT("Material should use the AlphaComposite blending mode"))
	
	if (!RenderTarget->Resource)
	{
		// TODO: Try to get render targets from a pool. So, we can avoid this issue.
		UE_LOG(LogTemp, Warning, TEXT("Creating RenderTargets Again"))
		RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(WorldContext, TextureWidth, TextureWidth, RTF_RGBA8,
																FLinearColor(0, 0, 0, 1), false);
		RenderTarget->SRGB = 0;
	}

	// Here we use AlphaComposite blending mode to get the alpha channel
	// In order to use that correct, we need to clear the color to following before rendering
	RenderTarget->ClearColor = FColor(0, 0, 0, 1);
	RenderTarget->UpdateResourceImmediate();
	
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(WorldContext, RenderTarget, Material);
	return true;
}

bool FDataRenderTarget::ReadDataAsync(int32 RowStart, int32 RowEnd, TArray<FColor>& ModifiedData, TFunction<void()> ReadCompleteCallback)
{
	check(RowStart >= 0);
	check(RowEnd <= TextureWidth);

	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	const FIntRect SampleRect = {0, RowStart, TextureWidth, RowEnd};
	
	const FReadSurfaceDataFlags ReadSurfaceDataFlags;

	// Read the render target surface data back.	
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FColor>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
		TFunction<void()> Callback;
	};

	ModifiedData.Reset();

	FReadSurfaceContext Context =
	{
		RenderTargetResource,
		&ModifiedData,
		SampleRect,
		ReadSurfaceDataFlags,
	};

	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
		[Context, ReadCompleteCallback, this](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceData(
				Context.SrcRenderTarget->GetRenderTargetTexture(),
				Context.Rect,
				*Context.OutData,
				Context.Flags
			);

			FOpenLandThreading::RunOnGameThread([this, ReadCompleteCallback]()
			{
				if (ReadCompleteCallback != nullptr)
					ReadCompleteCallback();
			});
		});

	return true;
}

bool FDataRenderTarget::IsActive() const
{
	return RenderTarget->Resource != nullptr;
}
