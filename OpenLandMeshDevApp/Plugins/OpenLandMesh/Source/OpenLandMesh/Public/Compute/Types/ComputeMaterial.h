// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"
#include "ComputeMaterial.generated.h"

struct FComputeMaterialValidationStatus
{
	bool bIsValid = false;
	FString Message = "";
};

UENUM(BlueprintType)
enum FComputeMaterialParameterType
{
	CMPT_SCALAR UMETA(DisplayName="Scalar"),
	CMPT_VECTOR UMETA(DisplayName="Vector")
};

USTRUCT(BlueprintType)
struct FComputeMaterialParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh")
	FName Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh")
	TEnumAsByte<FComputeMaterialParameterType> Type = CMPT_SCALAR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh")
	float ScalarValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh")
	FVector VectorValue = {};
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FComputeMaterial
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="OpenLandMesh")
	UMaterialInterface* Material = NULL;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="OpenLandMesh")
	TArray<FComputeMaterialParameter> Parameters;
};
