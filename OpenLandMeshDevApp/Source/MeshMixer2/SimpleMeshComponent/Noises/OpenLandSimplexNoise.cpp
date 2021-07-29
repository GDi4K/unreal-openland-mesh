// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenLandSimplexNoise.h"


// Sets default values for this component's properties
UOpenLandSimplexNoise::UOpenLandSimplexNoise()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOpenLandSimplexNoise::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UOpenLandSimplexNoise::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UOpenLandSimplexNoise::GetRandom(FVector Input) const
{
	const float SinInput = FVector::DotProduct(Input, FVector(13.4, 43.5, 33.5));
	const float MultiplyFactor = 48889.34;
	return FMath::Frac(FMath::Sin(SinInput * MultiplyFactor));
}

float UOpenLandSimplexNoise::Get3DNoise(FVector Input, float Roughness, float Strength, FVector Center,
                                        bool bNormalizeRange) const
{
	const FVector ModifiedInput = (Input * Roughness) + Center;
	const float NoiseValue = SimplexNoise::noise(ModifiedInput.X, ModifiedInput.Y, ModifiedInput.Z);
	if (!bNormalizeRange)
		return NoiseValue * Strength;

	const float NormalizedNoise = (NoiseValue * 0.5) + 0.5;
	return NormalizedNoise * Strength;
}

float UOpenLandSimplexNoise::Get3DFractal(FVector Input, int32 Octaves, float Frequency, float Roughness,
                                          float Persistence,
                                          float Strength, FVector Center, bool bNormalizeRange) const
{
	float CurrentFrequency = Frequency;
	float CurrentAmplitude = 1;
	float Fractal = 0;
	float TotalAmplitude = 0;

	for (int32 Index = 0; Index < Octaves; Index++)
	{
		const FVector ModifiedInput = (Input * CurrentFrequency) + Center;
		Fractal += CurrentAmplitude * (SimplexNoise::noise(ModifiedInput.X, ModifiedInput.Y, ModifiedInput.Z) * 0.5 +
			0.5);

		TotalAmplitude += CurrentAmplitude;
		CurrentFrequency *= Roughness;
		CurrentAmplitude *= Persistence;
	}

	Fractal = (Fractal / TotalAmplitude) * Strength;

	return Fractal;
}
