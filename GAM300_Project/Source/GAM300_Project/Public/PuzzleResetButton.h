/*
 * File: PuzzleResetButton.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "CharacterBase.h"

#include "PuzzleResetButton.generated.h"

UCLASS()
class GAM300_PROJECT_API APuzzleResetButton : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzleResetButton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	ACharacterBase *player;
	UWidgetComponent *textComp;
};
