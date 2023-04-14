/*
 * File: GameStats.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "GameStats.h"

// Sets default values
AGameStats::AGameStats()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGameStats::BeginPlay()
{
	Super::BeginPlay();
}

// increment crops harvested
void AGameStats::IncrementCropCount(bool is_earth_crop)
{
	if (is_earth_crop)
	{
		++crops_harvested_nature_;
	}
	else
	{
		++crops_harvested_water_;
	}
}

// true if player has a ticket
void AGameStats::SetHasTicket(bool hasTicket)
{
	b_has_ticket_ = hasTicket;
}

// true if player can fulfill ticket
void AGameStats::SetDoneTicket(bool doneTicket)
{
	b_done_ticket_ = doneTicket;
}

// return "INCOMPLETE" if false, otherwise "COMPLETE"
FString AGameStats::completionText(bool is_complete)
{
	if (is_complete)
	{
		return FString(TEXT("COMPLETE"));
	}
	else
	{
		return FString(TEXT("INCOMPLETE"));
	}
}

// Called every frame
void AGameStats::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!b_done_crops_nature_)
	{
		UpdateTextCropNature();
	}
	if (!b_done_crops_water_)
	{
		UpdateTextCropWater();
	}
}

void AGameStats::UpdateTextCropNature()
{
	if (crops_harvested_nature_ >= 3)
	{
		b_done_crops_nature_ = true;
		text_crop_nature_ = FString(TEXT(" --- COMPLETED"));
	}
	else
	{
		text_crop_nature_ = FString::Printf(TEXT(" --- (%d/3)"),crops_harvested_nature_);
	}
}
void AGameStats::UpdateTextCropWater()
{
	if (crops_harvested_water_ >= 3)
	{
		b_done_crops_water_ = true;
		text_crop_water_ = FString(TEXT(" --- COMPLETED"));
	}
	else
	{
		text_crop_water_ = FString::Printf(TEXT(" --- (%d/3)"), crops_harvested_water_);
	}
}

// Get the current list of tasks
FText AGameStats::GetTasksString()
{
	// farming
	FString s1 = FString::Printf(TEXT("FARMING:\r\n\t1. Harvest crops!\r\n\t> Aqua Crops"));
	FString s2 = FString::Printf(TEXT("\r\n\t> Terra Crops"));
	farmingtasks = s1 + text_crop_water_ + s2 + text_crop_nature_;

	// tickets
	FString tickettasks = FString::Printf(TEXT("\r\n"));
	if (b_has_ticket_)
	{
		FString ticketheader = FString::Printf(TEXT("TICKET: Press 'T' for details.\r\n"));
		FString doticket = FString::Printf(TEXT("\t1. Fullfill Ticket --- ")) + completionText(b_done_ticket_);
		FString turnin = FString::Printf(TEXT("\r\n\t2. Turn in Ticket --- ")) + completionText(false);
		tickettasks += ticketheader + doticket + turnin;
	}

	// total
	FString tasks = farmingtasks + tickettasks;

	return FText::FromString(tasks);
}

