// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Misc/SecureHash.h"
#include "OpenLandMeshHash.generated.h"

/**
 * 
 */
UCLASS()
class OPENLANDMESH_API UOpenLandMeshHash : public UObject
{
	GENERATED_BODY()

	private:
	FSHA1 Hash;
	bool bIsNull = true;
	bool bCompleted = false;

	public:

	UOpenLandMeshHash();

	bool IsNull() const;
	
	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	UOpenLandMeshHash* AddString(FString Value);

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	UOpenLandMeshHash* AddInteger(int32 Value);

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	UOpenLandMeshHash* AddFloat(float Value);

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	UOpenLandMeshHash* AddVector(FVector Value);

	UFUNCTION(BlueprintCallable, Category="OpenLandMesh")
	FString Generate();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	static UOpenLandMeshHash* MakeHash();
};
