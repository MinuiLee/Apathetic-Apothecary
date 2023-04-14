/*
 * File: NPC_Base.h
 * Course: GAM300
 * Author(s): Macie White
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dialogue_Interface.h"
#include "NPC_Base.generated.h"

UCLASS() // Inherit from public ACharacter instead of actor
class GAM300_PROJECT_API ANPC_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPC_Base();
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool is_displaying = false;
	//virtual void OnTrace_Implementation(AActor* Caller) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
