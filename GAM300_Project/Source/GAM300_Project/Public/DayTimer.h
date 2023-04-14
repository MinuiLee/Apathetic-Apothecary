/*
 * File: DayTimer.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayTimer.generated.h"

UCLASS()
class GAM300_PROJECT_API ADayTimer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADayTimer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int day_time_left_;

	int day_num_;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimerVariables")
		FString ui_time_left_str_;

	UPROPERTY(EditAnywhere)
		int SECONDS_IN_A_DAY_;

	UFUNCTION(BlueprintPure, Category = "TimerVariables")
		int GetTimeLeft();

	UFUNCTION(BlueprintPure, Category = "TimerVariables")
		FText GetTimeLeftInText();

	UFUNCTION(BlueprintPure, Category = "TimerVariables")
		FText GetDayNumInText();
};
