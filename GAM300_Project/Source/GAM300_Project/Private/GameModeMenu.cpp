/*
 * File: GameModeMenu.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "GameModeMenu.h"

AGameModeMenu::AGameModeMenu() : Super()
{
  DefaultPawnClass = NULL;
  /*UBlueprint *cgpc = Cast<UBlueprint>(StaticLoadObject(UObject::StaticClass(), nullptr, TEXT("/Game/Blueprints/ChamomileGrovePlayerController")));
  TSubclassOf<APlayerController> cgpcBlueprint = (UClass *)cgpc->GeneratedClass;*/
}
