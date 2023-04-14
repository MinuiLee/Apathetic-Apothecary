/*
 * File: Door.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/TextRenderComponent.h"
#include "Components/BillboardComponent.h"
#include "CharacterBase.h"

#include "Door.generated.h"

UCLASS()
class GAM300_PROJECT_API ADoor : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination")
		UBillboardComponent *destinationComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
		USceneComponent *promptComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
		UBillboardComponent *promptIconComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
		UBillboardComponent *xIconComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
		UStaticMeshComponent *vineMeshComp; // Mesh for vines that block the door

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* controllerIcon;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* keyboardIcon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool locked = false; // Whether the door is locked and requires a key to be unlocked

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		ADoor *destinationDoor = nullptr; // The door actor that should be used to determine the player's rotation after going through this door

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void ChangeInputSprite(bool is_controller);

private:
	ACharacterBase *player;
};
