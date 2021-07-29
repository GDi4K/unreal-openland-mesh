// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<FComputeMaterialParameterType> Type = CMPT_SCALAR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScalarValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInterface* Material = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FComputeMaterialParameter> Parameters;
};
