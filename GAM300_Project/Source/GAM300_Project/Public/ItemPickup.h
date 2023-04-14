/*
 * File: ItemPickup.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S_Item.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "CharacterBase.h"

#include "ItemPickup.generated.h"

class ADoor;

UCLASS()
class GAM300_PROJECT_API AItemPickup : public AActor
{
	GENERATED_BODY()
	
	public:	
		// Sets default values for this actor's properties
		AItemPickup();

		UPROPERTY(EditAnywhere, Category = "Item")
			FUS_Item item;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Icon")
			UBillboardComponent *iconComp;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
			UBillboardComponent *promptIconComp;

		UPROPERTY(EditAnywhere, Category = "Interaction")
			UTexture2D* controllerIcon;

		UPROPERTY(EditAnywhere, Category = "Interaction")
			UTexture2D* keyboardIcon;

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			ADoor *unlockedDoor = nullptr; // The door unlocked by this pickup, if any.

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			int puzzleNumber = 0; // The puzzle number unlocked by this pickup, if any. Used to determine ticket availability. 

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			bool isLastPuzzle = false; // Used to know if its the last puzzle. 

		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			FText unlockText; // The text that flashes on the screen when this item is picked up (if and only if it unlocks a door)

	protected:
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;

	public:	
		// Called every frame
		virtual void Tick(float DeltaTime) override;

		UFUNCTION(BlueprintCallable)
		void ChangeInputSprite(bool is_controller);

		UFUNCTION(BlueprintImplementableEvent)
			void GameEnd();

	private:
		FVector startPos;
		ACharacterBase *player;
};
