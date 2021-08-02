// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/ComputeMaterialUtils.h"

void UComputeMaterialUtils::SetScalarParameter(FComputeMaterial& ComputeMaterial, FName Name, float Value)
{
	for(FComputeMaterialParameter& Param: ComputeMaterial.Parameters)
	{
		if (Param.Name == Name)
		{
			UE_LOG(LogTemp, Warning, TEXT("Set Value: %s - %f"), *(Name.ToString()), Value)
			Param.ScalarValue = Value;
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Create Value: %s - %f"), *(Name.ToString()), Value)
	ComputeMaterial.Parameters.Push({
		Name,
		CMPT_SCALAR,
		Value
	});
}
