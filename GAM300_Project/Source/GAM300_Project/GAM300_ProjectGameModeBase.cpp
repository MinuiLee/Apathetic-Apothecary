/*
 * File: GAM300_ProjectGameModeBase.cpp
 * Course: GAM300
 * Author(s): N/A
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
// Copyright Epic Games, Inc. All Rights Reserved.


#include "GAM300_ProjectGameModeBase.h"
#include "Blueprint/UserWidget.h"
AGAM300_ProjectGameModeBase::AGAM300_ProjectGameModeBase()
    : Super()
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(TEXT("/Game/Blueprints/BP_CharacterBase"));
    if (PlayerPawnClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnClass.Class;
    }

    // tick to check if the controller ios attatched
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;
}

bool AGAM300_ProjectGameModeBase::IsGamePadConnected()
{
  auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
  return (genericApplication.Get() != nullptr && genericApplication->IsGamepadAttached());
}