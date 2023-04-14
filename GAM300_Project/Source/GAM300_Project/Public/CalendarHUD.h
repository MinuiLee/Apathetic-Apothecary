/*
 * File: CalendarHUD.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CalendarHUD.generated.h"

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API ACalendarHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ACalendarHUD();

	// primary draw call
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere, Category = "TimerVariables")
		TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "TimerVariables")
		class UUserWidget* current_widget_;
};
