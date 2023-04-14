/*
 * File: GameStats.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameStats.generated.h"

UCLASS()
class GAM300_PROJECT_API AGameStats : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameStats();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// return "INCOMPLETE" if false, otherwise "COMPLETE"
	FString completionText(bool is_complete);

	void UpdateTextCropNature();
	void UpdateTextCropWater();

	FString text_crop_nature_;
	FString text_crop_water_;

	//FString farmingtasks = FString::Printf(TEXT("Farming:\r\n\t1. Harvest crops!%s\r\n\t2. Fill up crates!\r\n\t3. Sell your crops!"), TEXT(" --- COMPLETED"));
	FString farmingtasks;

	int crops_harvested_water_ = 0;
	int crops_harvested_nature_ = 0;

	bool b_done_crops_nature_ = false;
	bool b_done_crops_water_ = false;

	bool b_has_ticket_ = false;
	bool b_done_ticket_ = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// increment crops harvested
	void IncrementCropCount(bool is_earth_crop);

	// true if player has a ticket
	void SetHasTicket(bool);

	// true if player can fulfill ticket
	void SetDoneTicket(bool);

	UFUNCTION(BlueprintPure, Category = "Tasks")
		FText GetTasksString();
};
