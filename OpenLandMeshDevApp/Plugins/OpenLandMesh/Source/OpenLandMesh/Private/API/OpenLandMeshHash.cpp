// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#include "API/OpenLandMeshHash.h"

UOpenLandMeshHash::UOpenLandMeshHash()
{
}

bool UOpenLandMeshHash::IsNull() const
{
	return bIsNull;
}

UOpenLandMeshHash* UOpenLandMeshHash::AddString(FString Value)
{
	bIsNull = false;
	Hash.UpdateWithString(ToCStr(Value), Value.Len());
	return this;
}

UOpenLandMeshHash* UOpenLandMeshHash::AddInteger(int32 Value)
{
	AddString(FString::FromInt(Value));
	return this;
}

UOpenLandMeshHash* UOpenLandMeshHash::AddFloat(float Value)
{
	AddString(FString::SanitizeFloat(Value));
	return this;
}

UOpenLandMeshHash* UOpenLandMeshHash::AddVector(FVector Value)
{
	AddString(Value.ToString());
	return this;
}

FString UOpenLandMeshHash::Generate()
{
	if (IsNull())
	{
		return "";
	}

	check(!bCompleted);
	bCompleted = true;
	
	Hash.Final();
	
	uint8 HashResult[FSHA1::DigestSize];
	Hash.GetHash(HashResult);
	return BytesToHex(HashResult, FSHA1::DigestSize);
}

UOpenLandMeshHash* UOpenLandMeshHash::MakeHash()
{
	return NewObject<UOpenLandMeshHash>();
}
