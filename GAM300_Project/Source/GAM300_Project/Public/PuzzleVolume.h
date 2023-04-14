/*
 * File: PuzzleVolume.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PuzzleVolume.generated.h"

UCLASS()
class GAM300_PROJECT_API APuzzleVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzleVolume();
	void ResetPuzzle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int puzzleNumber = 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
