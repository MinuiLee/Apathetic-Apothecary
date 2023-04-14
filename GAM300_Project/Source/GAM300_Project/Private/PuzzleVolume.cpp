/*
 * File: PuzzleVolume.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "PuzzleVolume.h"
#include "PuzzleObject.h"

// Sets default values
APuzzleVolume::APuzzleVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APuzzleVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APuzzleVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APuzzleVolume::ResetPuzzle()
{
	TArray<AActor *> overlappingActors;

	GetOverlappingActors(overlappingActors, TSubclassOf<APuzzleObject>());

	for (int i = 0; i < overlappingActors.Num(); ++i)
	{
		APuzzleObject *puzzleObject = Cast<APuzzleObject>(overlappingActors[i]);

		if (puzzleObject != nullptr)
		{
			puzzleObject->Reset();
			puzzleObject->SetActorLocation(puzzleObject->startPos);

			if (puzzleObject->startedSmall)
			{
				if (!puzzleObject->is_shrinked)
				{
					puzzleObject->canGrow = true;
					puzzleObject->canShrink = false;
				}

				puzzleObject->is_shrinked = true;
			}
			else
			{
				if (puzzleObject->canGrow)
				{
					puzzleObject->canGrow = false;
					puzzleObject->canShrink = true;
					puzzleObject->canPickup = false;
				}

				puzzleObject->is_shrinked = false;
			}

			puzzleObject->SetActorScale3D(puzzleObject->original_scale_);
			puzzleObject->SetActorRotation(puzzleObject->original_angle_, ETeleportType::ResetPhysics);
			puzzleObject->SetVelocity(FVector::ZeroVector);
			puzzleObject->Gravitate();
		}
	}
}