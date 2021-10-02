#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class FOpenLandGridCell
{
public:
	int32 X = 0;
	int32 Y = 0;

	FOpenLandGridCell(): FOpenLandGridCell(0, 0) {}
	FOpenLandGridCell(int32 X, int32 Y): X(X), Y(Y) {}
	FOpenLandGridCell(const FOpenLandGridCell& Other): X(Other.X), Y(Other.Y) {}
	FOpenLandGridCell(const FVector2D Vector): FOpenLandGridCell(Vector.IntPoint().X, Vector.IntPoint().Y) {}

	FOpenLandGridCell operator+(const FOpenLandGridCell& Other) const
	{
		return {X + Other.X, Y + Other.Y};
	}

	FOpenLandGridCell operator-(const FOpenLandGridCell& Other) const
	{
		return {X - Other.X, Y - Other.Y};
	}

	FOpenLandGridCell operator*(const int32 Value) const
	{
		return {X * Value, Y * Value};
	}

	FOpenLandGridCell operator/(const int32 Value) const
	{
		return {X / Value, Y / Value};
	}
	
	bool operator==(const FOpenLandGridCell& Other) const
	{
		return Equals(Other);
	}

	bool Equals(const FOpenLandGridCell& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	FString ToString() const
	{
		return "(" + FString::FromInt(X) + ", " + FString::FromInt(Y) + " )";
	}

	FVector2D ToVector2D() const
	{
		return {static_cast<float>(X), static_cast<float>(Y)};
	}
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FOpenLandGridCell& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FOpenLandGridCell& Thing)
{
	const FVector2D Pos = Thing.ToVector2D();
	return FCrc::MemCrc32(&Pos, sizeof(FVector2D));
}
#endif