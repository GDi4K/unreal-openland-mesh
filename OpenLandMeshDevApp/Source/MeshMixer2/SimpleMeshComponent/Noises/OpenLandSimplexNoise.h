// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeshMixer2/SimpleMeshComponent/ThirdParty/SimplexNoise/SimplexNoise.h"

#include "OpenLandSimplexNoise.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MESHMIXER2_API UOpenLandSimplexNoise : public UActorComponent
{
	GENERATED_BODY()

	SimplexNoise Noise;

public:
	// Sets default values for this component's properties
	UOpenLandSimplexNoise();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	float GetRandom(FVector Input) const;

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    float Get3DNoise(FVector Input, float Roughness=1, float Strength=1, FVector Center=FVector(0, 0, 0), bool bNormalizeRange=true) const;

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    float Get3DFractal(FVector Input, int32 Octaves=1, float Frequency=1, float Roughness=2, float Persistence=0.5, float Strength=1, FVector Center=FVector(0, 0, 0), bool bNormalizeRange=true) const;
};
