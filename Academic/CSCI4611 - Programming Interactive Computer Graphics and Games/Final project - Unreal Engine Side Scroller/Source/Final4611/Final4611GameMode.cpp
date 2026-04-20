// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Final4611GameMode.h"
#include "Final4611Character.h"
#include "UObject/ConstructorHelpers.h"

AFinal4611GameMode::AFinal4611GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
