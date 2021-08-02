// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Compute/Types/ComputeMaterial.h"
#include "UObject/Object.h"
#include "ComputeMaterialUtils.generated.h"

/**
 * 
 */
UCLASS()
class OPENLANDMESH_API UComputeMaterialUtils : public UObject
{
	GENERATED_BODY()

	public:

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	static void SetScalarParameter(FComputeMaterial& ComputeMaterial, FName Name, float Value);
};
