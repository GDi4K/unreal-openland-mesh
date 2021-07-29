#pragma once

/** One vertex for the procedural mesh, used for storing data internally */
#include "OpenLandMeshTangent.h"

size_t VertexCounter = 0;

struct FOpenLandMeshVertex
{
public:	
	/** Vertex position */
	FVector Position;

	/** Vertex normal */
	FVector Normal;

	/** Vertex tangent */
	FOpenLandMeshTangent Tangent;

	/** Vertex color */
	FColor Color;

	/** Vertex texture co-ordinate */
	FVector2D UV0;

	/** Vertex texture co-ordinate */
	FVector2D UV1;

	/** Vertex texture co-ordinate */
	FVector2D UV2;

	/** Vertex texture co-ordinate */
	FVector2D UV3;

	size_t ObjectId;

	size_t TriangleId;

	FOpenLandMeshVertex()
		: Position(0.f, 0.f, 0.f)
		  , Normal(0.f, 0.f, 1.f)
		  , Tangent(FVector(1.f, 0.f, 0.f), false)
		  , Color(255, 255, 255)
		  , UV0(0.f, 0.f)
		  , UV1(0.f, 0.f)
		  , UV2(0.f, 0.f)
		  , UV3(0.f, 0.f)
		  , ObjectId(++VertexCounter)
		  ,TriangleId(0)
	{
	}

	FOpenLandMeshVertex(const FVector P)
        : Position(P)
          , Normal(0.f, 0.f, 1.f)
          , Tangent(FVector(1.f, 0.f, 0.f), false)
          , Color(255, 255, 255)
          , UV0(0.f, 0.f)
          , UV1(0.f, 0.f)
          , UV2(0.f, 0.f)
          , UV3(0.f, 0.f)
          , ObjectId(++VertexCounter)
	      ,TriangleId(0)
	{
	}

	FOpenLandMeshVertex(const FVector P, const FVector2D UV)
	{
		Position = P;
		UV0 = UV;
		ObjectId = ++VertexCounter;
	}


	FOpenLandMeshVertex Interpolate(FOpenLandMeshVertex Other, float Range) const
	{
		FOpenLandMeshVertex NewVertex;

		NewVertex.Position = (Position * Range) + (Other.Position * (1-Range));
		NewVertex.Normal = (Normal * Range) + (Other.Normal * (1-Range));
		NewVertex.Tangent.TangentX = (Tangent.TangentX * Range) + (Other.Tangent.TangentX * (1-Range));
		NewVertex.UV0 = (UV0 * Range) + (Other.UV0 * (1-Range));
		NewVertex.UV1 = (UV1 * Range) + (Other.UV1 * (1-Range));
		NewVertex.UV2 = (UV2 * Range) + (Other.UV2 * (1-Range));
		NewVertex.UV3 = (UV3 * Range) + (Other.UV3 * (1-Range));

		return NewVertex;
	}

};