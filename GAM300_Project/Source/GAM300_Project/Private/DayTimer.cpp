/*
 * File: DayTimer.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "DayTimer.h"

// Sets default values
ADayTimer::ADayTimer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADayTimer::BeginPlay()
{
	Super::BeginPlay();
	
	day_time_left_ = (SECONDS_IN_A_DAY_ * 60);
	day_num_ = 1;
}

// Called every frame
void ADayTimer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (day_time_left_ <= 0)
	{
		day_time_left_ = (SECONDS_IN_A_DAY_ * 60);
		++day_num_;
	}

	day_time_left_ -= DeltaTime;
}

int ADayTimer::GetTimeLeft()
{
	return day_time_left_;
}

FText ADayTimer::GetTimeLeftInText()
{
	FString min_str = FString::FromInt(day_time_left_ / 3600);
	FString sec_str = FString::FromInt((day_time_left_ / 60) % 60);
	FString timer_str = min_str + FString(TEXT(":")) + (((day_time_left_ / 60) % 60 < 10)? TEXT("0") : TEXT("")) + sec_str;
	FText timer_text = FText::FromString(timer_str);
	return timer_text;
}

FText ADayTimer::GetDayNumInText()
{
	return FText::FromString(FString::FromInt(day_num_));
}
