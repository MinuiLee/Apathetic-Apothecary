/*
 * File: ChamomileGroveInstance.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameStats.h"
#include "ChamomileGroveInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API UChamomileGroveInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UChamomileGroveInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
		AGameStats* GameStats_;
	
};
