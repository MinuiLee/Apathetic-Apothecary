/*
 * File: InventoryComponent.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "S_Item.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "InventoryComponent.generated.h"

class ACharacterBase;

// The Inventory component allows the player to store, select, and use items like potions and seeds.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAM300_PROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UFUNCTION(BlueprintCallable)
	void AddInventoryHUD();
	UFUNCTION(BlueprintCallable)
	void RemoveInventoryHUD();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	bool AddItem(UDataAsset const* new_item, int amount);
	bool AddItem(FUS_Item const& new_item);
	void UseSelectedItemOnPlayer();
	void UseSelectedItemOnObject();
	void MoveSelection(int distance);
	void MoveSelectionRight();
	void MoveSelectionLeft();
	UFUNCTION(BlueprintCallable)
	void DecreaseStack(int index);
	UFUNCTION(BlueprintCallable)
	void SubtractStack(int index, int amount);
	void UpdateHUD();
	bool CanUseOnPlayer(int index);
	
	UFUNCTION(BlueprintCallable)
	FUS_Item GetItem(int index) const;
	UFUNCTION(BlueprintCallable)
	FUS_Item GetCurrentItem() const;

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetItemTexture(FUS_Item const& item);
	UFUNCTION(BlueprintCallable)
	FString GetItemName(FUS_Item const& item);

	UFUNCTION(BlueprintCallable)
	int GetInventorySize() const;

	UPROPERTY(EditAnywhere, Category = "Textures")
	UTexture2D *emptySlotTexture; // Texture to apply to the icons of empty inventory slots

	static const int inventorySize = 10; // Number of slots in the inventory
	UPROPERTY(EditAnywhere, Category = "Items")
	FUS_Item items[inventorySize];       // Item stacks in the inventory
	int selectedIndex = 0;               // The index of the currently selected inventory slot

	// return true if watering can is selected
	UFUNCTION(BlueprintCallable)
	bool OnWateringCan();

	// return true if seed is selected
	UFUNCTION(BlueprintCallable)
	bool OnSeed();

	// returns true if inventory item is character hand
	UFUNCTION(BlueprintCallable)
	bool OnHand();

	// returns true if potion is selected
	UFUNCTION(BlueprintCallable)
	bool OnPotion();

	// returns true if crop is selected
	UFUNCTION(BlueprintCallable)
	bool OnCrop();

	// Locks movement in inventory
	UFUNCTION(BlueprintCallable)
	void SetLock(bool lock);
	UFUNCTION(BlueprintCallable)
	bool GetLock() const;

	UFUNCTION(BlueprintCallable)
		void SetHitTestOn();
	UFUNCTION(BlueprintCallable)
		void SetHitTestOff();

private:
	void UpdateButtonOnHovered();

private:

	ACharacterBase *player;                      // Pointer to the player character
	UUserWidget *inventoryHUD;                   // Pointer to the inventory HUD
	UWidget *slots[inventorySize];               // Array of pointers to slot widgets
	UImage *slotImages[inventorySize];           // Array of pointers to slot icons
	UTextBlock *slotAmounts[inventorySize];      // Array of pointers to slot stack size counters
	UTextBlock *slotNames[inventorySize];        // Array of pointers to slot item name indicators
	UTextBlock *slotSpacePrompts[inventorySize]; // Array of pointers to slot spacebar prompts
	UImage* slotOutlines[inventorySize];         // Array of pointers to slot outlines
	UButton* slotButtons[inventorySize];         // Array of pointers to slot buttons
	bool lock_;																	 // Decides if movement in hotbar can be made
};
