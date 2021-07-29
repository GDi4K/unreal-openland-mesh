#pragma once

/**
*	Struct used to specify a tangent vector for a vertex
*	The Y tangent is computed from the cross product of the vertex normal (Tangent Z) and the TangentX member.
*/
struct FOpenLandMeshTangent
{
public:

	/** Direction of X tangent for this vertex */
	FVector TangentX;

	/** Bool that indicates whether we should flip the Y tangent when we compute it using cross product */
	bool bFlipTangentY;

	FOpenLandMeshTangent()
        : TangentX(1.f, 0.f, 0.f)
          , bFlipTangentY(false)
	{
	}

	FOpenLandMeshTangent(float X, float Y, float Z)
        : TangentX(X, Y, Z)
          , bFlipTangentY(false)
	{
	}

	FOpenLandMeshTangent(FVector InTangentX, bool bInFlipTangentY)
        : TangentX(InTangentX)
          , bFlipTangentY(bInFlipTangentY)
	{
	}
};