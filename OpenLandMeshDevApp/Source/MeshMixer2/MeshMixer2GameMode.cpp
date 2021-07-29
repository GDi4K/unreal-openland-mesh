// Copyright Epic Games, Inc. All Rights Reserved.

#include "MeshMixer2GameMode.h"
#include "MeshMixer2Character.h"
#include "UObject/ConstructorHelpers.h"

AMeshMixer2GameMode::AMeshMixer2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
		DefaultPawnClass = PlayerPawnBPClass.Class;
}
