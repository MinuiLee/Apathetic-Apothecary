/*
 * File: GameModePlay.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DayTimer.h"
#include "GameModePlay.generated.h"

UENUM()
enum class EGamePlayState
{
	EPlaying,
	EGameOver,
	EPause,
	EUnknown
};

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API AGameModePlay : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGameModePlay();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	ADayTimer* p_day_timer_;

	UFUNCTION(BlueprintPure, Category = "TimeVariables")
		EGamePlayState GetCurrentState() const;

	void SetCurrentState(EGamePlayState new_state);

private:
	EGamePlayState current_state_;

	void HandleNewState(EGamePlayState new_state);
};
