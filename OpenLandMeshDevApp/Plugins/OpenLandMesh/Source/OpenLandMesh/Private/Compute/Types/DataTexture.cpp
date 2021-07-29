#include "Compute/Types/DataTexture.h"

FDataTexture::FDataTexture(int Width)
{
	TextureWidth = Width;

	// Assign the Texture
	Texture = UTexture2D::CreateTransient(TextureWidth, TextureWidth);
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	Texture->SRGB = 0;
	Texture->AddToRoot();
	Texture->Filter = TextureFilter::TF_Nearest;
	Texture->UpdateResource();

	// Set the Region
	WholeTextureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureWidth);

	// Allocate Data
	const int32 BufferSize = TextureWidth * TextureWidth * 4;
	SourceData = new uint8[BufferSize];

	Reset();
}

FDataTexture::~FDataTexture()
{
	if (SourceData != nullptr)
	{
		delete[] SourceData;
		SourceData = nullptr;
	}

	if (Texture != nullptr)
	{
		if(Texture->IsValidLowLevel())
		{
			Texture->ReleaseResource();
		}
		Texture = nullptr;
	}
}

void FDataTexture::SetPixelValue(int32 Index, uint8 R, uint8 G, uint8 B, uint8 A)
{
	uint8* pointer = SourceData + (Index * 4);
	*pointer = B; //b
	*(pointer + 1) = G; //g
	*(pointer + 2) = R; //r
	*(pointer + 3) = A; //a
}

void FDataTexture::Reset()
{
	for (int32 Index=0; Index<TextureWidth * TextureWidth; Index++)
	{
		SetPixelValue(Index, 0, 0, 0, 0);
	}
	
	UpdateTexture();
}

void FDataTexture::UpdateTexture()
{
	const int BytesPerPixel = 4;
	const int32 BytesPerRow = TextureWidth * BytesPerPixel;
	Texture->UpdateTextureRegions((int32)0, (uint32)1, &WholeTextureRegion, (uint32)BytesPerRow, (uint32)BytesPerPixel, SourceData);
}
