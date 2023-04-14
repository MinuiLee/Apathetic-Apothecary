/*
 * File: GameModePlay.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "GameModePlay.h"
#include "CalendarHUD.h"

AGameModePlay::AGameModePlay() : Super()
{
	//calendar_hud_ = ACalendarHUD::StaticClass();
}

void AGameModePlay::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentState(EGamePlayState::EPlaying);

	//p_day_timer_ = 
}

void AGameModePlay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (p_day_timer_ != nullptr)
	{

	}
}

EGamePlayState AGameModePlay::GetCurrentState() const
{
	return current_state_;
}

void AGameModePlay::SetCurrentState(EGamePlayState new_state)
{
	current_state_ = new_state;
	HandleNewState(current_state_);
}

void AGameModePlay::HandleNewState(EGamePlayState new_state)
{
	switch (new_state)
	{
	case EGamePlayState::EPlaying:
		break;
	default:
		break;
	}
}