/*
 * File: CalendarHUD.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "CalendarHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

ACalendarHUD::ACalendarHUD()
{
	//static ConstructorHelpers::FClassFinder<UUserWidget> CalendarObj(TEXT("Game/UI/CalendarUI"));
	//HUDWidgetClass = CalendarObj.Class;
}

void ACalendarHUD::DrawHUD()
{
	Super::DrawHUD();
}

void ACalendarHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass != nullptr)
	{
		current_widget_ = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);

		if (current_widget_ != nullptr)
		{
			current_widget_->AddToViewport();
		}
	}
}