/*
 * File: WindCurrent.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindCurrent.generated.h"

class UNiagaraComponent;
class UBoxComponent;

UCLASS()
class GAM300_PROJECT_API AWindCurrent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWindCurrent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
		UNiagaraComponent* niagaraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Collision Box")
		UBoxComponent* boxCollision;

	UPROPERTY(EditAnywhere)
		float playerWindForceMultiplier = 1.0f / 25.0f; // The value to multiply the wind force by when applying it to the player

	UPROPERTY(EditAnywhere)
		float objectOcclusionHalfWidth = 20.f;

	UPROPERTY(EditAnywhere)
		float objectSpeedMax = 100.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "100000000.0", UIMin = "0.0", UIMax = "100000000.0"))
		float wind_strength_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float wind_speed_delta_;
};
