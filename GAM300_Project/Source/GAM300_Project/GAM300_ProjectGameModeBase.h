/*
 * File: GAM300_ProjectGameModeBase.h
 * Course: GAM300
 * Author(s): N/A
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AkComponent.h"
#include "AkAmbientSound.h"
#include "GAM300_ProjectGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API AGAM300_ProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGAM300_ProjectGameModeBase();

	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkComponent* akAudioComponent;
	// local IDs for current song and ambience
	int32 audioAmbID;
	int32 audioMusID;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UnhideUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void HideUI();

	UFUNCTION(BlueprintCallable)
		bool IsGamePadConnected();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Controller")
		bool is_controller_used = false;

private:
	
};
