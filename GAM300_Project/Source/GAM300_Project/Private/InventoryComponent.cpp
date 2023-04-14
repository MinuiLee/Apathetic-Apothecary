/*
 * File: InventoryComponent.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "InventoryComponent.h"
#include "DA_Potion.h"
#include "DA_Seed.h"
#include "DA_WateringCan.h"
#include "DA_Hand.h"
#include "Crop.h"
#include "CharacterBase.h"
#include "PuzzleObject.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "FarmingPlot.h"

#define DEBUG_LOG 0

 // Sets default values for this component's properties
UInventoryComponent::UInventoryComponent() : player(NULL), lock_(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACharacterBase>(GetOwner()); // Get player actor

	// Create inventory HUD
	FStringClassReference inventoryHUDClassRef(TEXT("/Game/UI/InventoryHUD.InventoryHUD_C"));
	UClass* inventoryHUDClass = inventoryHUDClassRef.TryLoadClass<UUserWidget>();
	inventoryHUD = CreateWidget<UUserWidget>(GetWorld(), inventoryHUDClass);
	inventoryHUD->AddToViewport();

	// Get pointers to every component of the inventory HUD
	for (int i = 0; i < inventorySize; ++i)
	{
		slots[i] = inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i)).GetCharArray().GetData());
		slotImages[i] = Cast<UImage>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("Icon")).GetCharArray().GetData()));
		slotAmounts[i] = Cast<UTextBlock>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("Amount")).GetCharArray().GetData()));
		slotNames[i] = Cast<UTextBlock>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("Name")).GetCharArray().GetData()));
		slotSpacePrompts[i] = Cast<UTextBlock>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("SpacePrompt")).GetCharArray().GetData()));
		slotOutlines[i] = Cast<UImage>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("Outline")).GetCharArray().GetData()));
		slotButtons[i] = Cast<UButton>(inventoryHUD->GetWidgetFromName((FString("Slot") + FString::FromInt(i) + FString("Button")).GetCharArray().GetData()));
		
		if (slotButtons[i] != nullptr)
		{
			slotButtons[i]->OnClicked.AddDynamic(player, &ACharacterBase::MoveItemToCrate);
		}
	}
}

void UInventoryComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (inventoryHUD)
	{
		inventoryHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateHUD(); // Update the inventory HUD

	// if player is interacting with a crate then update the selected index using button onHovered event
	if (player->crate)
	{
		UpdateButtonOnHovered();
	}
}

// Attempts to put an item stack in the first available slot. Returns true if the item was successfully inserted, false otherwise.
// new_item: The item stack to add to the inventory.
bool UInventoryComponent::AddItem(FUS_Item const& new_item)
{
	// Look for a slot that already contains the new item
	for (int i = 1; i < inventorySize; ++i)
	{
		// If the current slot contains the same type of item as the new item, add the new item to the slot
		if (items[i].itemType_ == new_item.itemType_)
		{
			items[i].amount_ += new_item.amount_;    // Add the new item's amount to the item slot's amount
			UpdateHUD();
			return true;                             // The item was added, so return true to indicate success
		}
	}

	// If no instance of the new item was found, put it in an empty slot instead
	for (int i = 1; i < inventorySize; ++i)
	{
		// If the current slot is empty, add the new item to the slot
		if (items[i].itemType_ == NULL || items[i].amount_ == 0)
		{
			items[i].itemType_ = new_item.itemType_; // Set the slot's item type to the new item's type
			items[i].amount_ = new_item.amount_;     // Add the new item's amount to the item slot's amount
			UpdateHUD();
			return true;                             // The item was added, so return true to indicate success
		}
	}

	return false; // If no slot for the item was found, return false to indicate failure
}

// Attempts to put an item stack in the first available slot. Returns true if the item was successfully inserted, false otherwise.
// new_item: The type of item to add.
// amount:   The number of items to add.
bool UInventoryComponent::AddItem(UDataAsset const* new_item, int amount)
{
	// Construct an item stack with the given type and amount
	FUS_Item item;
	item.itemType_ = new_item;
	item.amount_ = amount;

	return AddItem(item); // Attempt to add the item stack to the inventory and return whether it was successfully inserted
}

// Uses the selected item on the player.
void UInventoryComponent::UseSelectedItemOnPlayer()
{
	// Drinking potions is disabled for now.
	// TODO: Remove this line or remove this function
	return;

	// If the selected item's type is null, the slot is empty
	if (items[selectedIndex].itemType_ == NULL)
	{
		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Selected slot is empty.");
		}
		return;
	}

	UDA_Potion const* potion = Cast<UDA_Potion const>(items[selectedIndex].itemType_); // Attempt to cast item to potion

	// If the item is a potion, trigger its effect
	if (potion != NULL)
	{
		player->EventUsePotionSelf();
		return;
	}

	if (DEBUG_LOG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Item cannot be used on player.");
	}
}

// Uses the selected item on the object the player is looking at.
void UInventoryComponent::UseSelectedItemOnObject()
{
	// If the selected item's type is null, the slot is empty
	if (items[selectedIndex].itemType_ == NULL)
	{
		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Selected slot is empty.");
		}
		return;
	}

	UDA_Potion const* potion = Cast<UDA_Potion const>(items[selectedIndex].itemType_); // Attempt to cast item to potion

	// If the item is a potion, trigger its effect on the object the player is looking at
	if (potion != NULL)
	{
		APuzzleObject* puzzleObject = Cast<APuzzleObject>(player->GetHitActor()); // Attempt to cast object to puzzle object

		// If the player is looking at a puzzle object, attempt to use the potion on it
		if (puzzleObject != NULL)
		{
			if (puzzleObject->IsValidPotionEffect(potion->potion_effect_))
			{
				player->AnimUsePotionObject(player->GetHitActor());
			}
			else
			{
				player->PostEvent(player->akPuzzleInvalid);
			}
		}
		else if (player->GetHitActor()->GetClass()->GetName().Contains("MovingPlatform"))
		{
			if (potion->potion_effect_ == E_POTION_EFFECT::EFX_TIMESTOP)
			{
				player->AnimUsePotionObject(player->GetHitActor());
			}
		}
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: No puzzle object found.");
			}
		}

		return;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(items[selectedIndex].itemType_); // Attempt to cast item to seed

	// If the item is a seed, plant it
	if (seed != NULL)
	{
		// If the seed was successfully planted, remove it from the inventory
		if (player->CanPlantSeed(seed))
		{
			player->PlantEvent(player->GetCurrentHitActor());
		}
		// Otherwise, the seed could not be planted, so nothing happens
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No plot nearby to plant in!");
			}
		}

		return;
	}

	UDA_WateringCan const* wateringcan = Cast<UDA_WateringCan const>(items[selectedIndex].itemType_); // Attempt to cast item to watering can

	// If the item is the watering can, water a plot
	if (wateringcan != NULL)
	{
		if (player->GetHitActor()->IsA(AFarmingPlot::StaticClass()))
		{
			AFarmingPlot *plot = Cast<AFarmingPlot>(player->GetHitActor());

			if (plot->IsWaterable())
			{
				player->AnimWaterPlot(player->GetCurrentHitActor());
			}
			else
			{
				player->PostEvent(player->akPuzzleInvalid);
			}
		}
		else
		{
			player->PostEvent(player->akPuzzleInvalid);
		}

		return;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(items[selectedIndex].itemType_); // Attempt to cast item to crop

	if (crop != NULL)
	{
		if (player->GetHitActor()->IsA(ACauldron::StaticClass()) && player->AddCropToCauldron())
		{
			DecreaseStack(selectedIndex); // Cosume the crop
		}
		else
		{
			player->PostEvent(player->akPuzzleInvalid);
		}
		
		return;
	}

	// If this point is reached, the item is of a type that cannot be used on an object
	if (DEBUG_LOG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Item cannot be used on object, or no object was found.");
	}
}

// Adds the specified value to the currently-selected item index.
// 1 moves the selection 1 slot to the right, -1 moves it 1 slot to the left, etc.
void UInventoryComponent::MoveSelection(int delta)
{
	if (lock_)
	{
		return;
	}

	selectedIndex += delta; // Move selection by given movement value

	// Wrap around if the left end of the inventory was passed
	if (selectedIndex < 0)
	{
		selectedIndex += inventorySize;
	}

	// Wrap around if the right end of the inventory was passed
	selectedIndex %= inventorySize;

	if (DEBUG_LOG)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Slot " + FString::FromInt(selectedIndex));
	}
}

// Increases selection index by 1.
void UInventoryComponent::MoveSelectionRight()
{
	if (lock_ == false)
	{
		MoveSelection(1);
	}
}

// Decreases selection index by 1.
void UInventoryComponent::MoveSelectionLeft()
{
	if (lock_ == false)
	{
		MoveSelection(-1);
	}
}

// Decreases the amount of an item stack by 1. If this reduces the amount to 0, the item's type is set to null.
void UInventoryComponent::DecreaseStack(int index)
{
	// If there are any items in the specified slot, reduce the stack size by 1
	if (items[index].amount_ > 0)
	{
		--items[index].amount_; // Reduce stack size by 1

		// If stack is now empty, set its type to null
		if (items[index].amount_ == 0)
		{
			items[index].itemType_ = NULL;
		}

		UpdateHUD(); // Update the inventory HUD
	}
}

// Decreases the amount of an item stack by amount if possible. If this reduces the amount to 0, the item's type is set to null.
void UInventoryComponent::SubtractStack(int index, int amount)
{
	if (items[index].amount_ >= amount)
	{
		items[index].amount_ -= amount;

		if (items[index].amount_ == 0)
		{
			items[index].itemType_ = NULL;
		}

		UpdateHUD();
	}
}

// Update all HUD item slot images and numbers
void UInventoryComponent::UpdateHUD()
{
	// Go through each slot and update its elements
	for (int i = 0; i < inventorySize; ++i)
	{
		// If there is a slot at this index, update its scale
		if (slots[i] != NULL)
		{
			// If the slot is selected, make it a bit bigger
			if (i == selectedIndex)
			{
				slots[i]->SetRenderScale(FVector2D(1.2f, 1.2f));
			}
			// Otherwise, set it to its normal size
			else
			{
				slots[i]->SetRenderScale(FVector2D(1.0f, 1.0f));
			}
		}

		// If there is an item icon at this index, update its texture
		if (slotImages[i] != NULL)
		{
			// If the slot has an item in it, set the texture to the item's icon
			if (GetItemTexture(items[i]) != NULL)
			{
				slotImages[i]->SetBrushFromTexture(GetItemTexture(items[i]));
			}
			// Otherwise, set the texture to the empty slot texture
			else
			{
				slotImages[i]->SetBrushFromTexture(emptySlotTexture);
			}
		}

		// If there is a stack size counter at this index, update its text
		if (slotAmounts[i] != NULL)
		{
			// If the slot has any items in it, set the text to the number of items
			if (items[i].amount_ > 0)
			{
				slotAmounts[i]->SetText(FText::FromString(FString::FromInt(items[i].amount_)));
			}
			// Otherwise, don't display any text
			else
			{
				slotAmounts[i]->SetText(FText::FromString(""));
			}
		}

		// If there is an item name indicator at this index, update its text
		if (slotNames[i] != NULL)
		{
			// If the slot has an item in it, set the text to the item's name
			if (items[i].itemType_ != NULL)
			{
				slotNames[i]->SetText(FText::FromString(GetItemName(items[i])));
			}
			// Otherwise, don't display any text
			else
			{
				slotNames[i]->SetText(FText::FromString(""));
			}
		}

		// If there is a spacebar prompt at this index, update its visibility
		if (slotSpacePrompts[i] != NULL)
		{
			// If the slot is not selected OR its item can't be used on the player, hide the prompt
			if (i != selectedIndex || !CanUseOnPlayer(selectedIndex))
			{
				slotSpacePrompts[i]->SetVisibility(ESlateVisibility::Hidden);
			}
			// Otherwise, show the prompt
			else
			{
				slotSpacePrompts[i]->SetVisibility(ESlateVisibility::Visible);
			}
		}

		// If there is an outline at this index, update its visiblity
		if (slotOutlines[i] != NULL)
		{
			// Show the outline if the slot is selected. Otherwise, hide it.
			slotOutlines[i]->SetVisibility(i == selectedIndex ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
	}
}

//////////////////////////////
// Private Functions
//////////////////////////////

// Get the icon texture associated with an item stack
UTexture2D* UInventoryComponent::GetItemTexture(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_); // Attempt to cast item to potion

	// If item is a potion, return its potion's texture
	if (potion != NULL)
	{
		return potion->texture_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_); // Attempt to cast item to seed

	// If item is a seed, return its seed's texture
	if (seed != NULL)
	{
		return seed->texture_;
	}

	UDA_WateringCan const* can = Cast<UDA_WateringCan const>(item.itemType_); // Attempt to cast item to watering can

	// If item is a watering can, return its watering can's texture
	if (can != NULL)
	{
		return can->texture_;
	}

	UDA_Hand const* hand = Cast<UDA_Hand const>(item.itemType_); // Attempt to cast item to hand

	// If item is a hand, return its hand's texture
	if (hand != NULL)
	{
		return hand->texture_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_); // Attempt to cast item to crop

	// If item is a crop, return its crop's texture
	if (crop != NULL)
	{
		return crop->texture_;
	}
	return NULL; // If this point is reached, the item does not have a texture, so return null
}

// Get the name of an item stack
FString UInventoryComponent::GetItemName(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_); // Attempt to cast item to potion

	// If item is a potion, return its potion's name
	if (potion != NULL)
	{
		return potion->name_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_); // Attempt to cast item to seed

	// If item is a seed, return its seed's name
	if (seed != NULL)
	{
		return seed->name_;
	}

	UDA_WateringCan const* can = Cast<UDA_WateringCan const>(item.itemType_); // Attempt to cast item to watering can

	// If item is a watering can, return its watering can's name
	if (can != NULL)
	{
		return can->name_;
	}

	UDA_Hand const* hand = Cast<UDA_Hand const>(item.itemType_); // Attempt to cast item to hand

	// If item is a hand, return its hand's name
	if (hand != NULL)
	{
		return hand->name_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_); // Attempt to cast item to crop

	// If item is a crop, return its crop's name
	if (crop != NULL)
	{
		return crop->name_;
	}

	return ""; // If this point is reached, the item does not have a name, so return an empty string
}

// Returns whether the item in a given inventory slot can be used on the player.
bool UInventoryComponent::CanUseOnPlayer(int index)
{
	// Drinking potions is disabled for now.
	// TODO: Remove this line or remove this function.
	return false;

	// If the currently selected slot contains an item and the item is a potion, check whether that potion can be used
	if (items[index].itemType_ != nullptr && items[index].itemType_->IsA(UDA_Potion::StaticClass()))
	{
		UDA_Potion const* potion = Cast<UDA_Potion const>(items[index].itemType_); // Cast item to potion

		// Check potion's effect's usability
		switch (potion->potion_effect_)
		{
			// Shrink effect
		case EFX_SHRINK:
			return !player->IsSmall(); // Return true if the player is not already small
		// Grow effect
		case EFX_GROW:
			return player->IsSmall(); // Return true if the player is not already big
		// All other effects do nothing to the player (fall through to the return statement at the end of the function)
		default:
			break;
		}
	}

	return false;
}

// Makes the inventory HUD visible if it exists.
void UInventoryComponent::AddInventoryHUD()
{
	if (inventoryHUD)
	{
		inventoryHUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

// Hides the inventory HUD if it exists.
void UInventoryComponent::RemoveInventoryHUD()
{
	if (inventoryHUD)
	{
		inventoryHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

// Returns the item stack in a specified slot.
FUS_Item UInventoryComponent::GetItem(int index) const
{
	return items[index];
}

// Returns the item stack in a the current slot.
FUS_Item UInventoryComponent::GetCurrentItem() const
{
	return items[selectedIndex];
}

// Returns the number of slots in the inventory.
int UInventoryComponent::GetInventorySize() const
{
	return inventorySize;
}

// Returns true if the watering can is selected, false otherwise.
bool UInventoryComponent::OnWateringCan()
{
	// If the selected slot is empty, the watering can is not selected
	if (items[selectedIndex].itemType_ == NULL)
	{
		return false;
	}

	UDA_WateringCan const* wateringCan = Cast<UDA_WateringCan const>(items[selectedIndex].itemType_); // Attempt to cast selected item to watering can

	// If the selected item is a watering can, return true. Otherwise, return false.
	return wateringCan != NULL;
}

// Returns true if a seed is selected, false otherwise.
bool UInventoryComponent::OnSeed()
{
	// If the selected slot is empty, a seed is not selected
	if (items[selectedIndex].itemType_ == NULL)
	{
		return false;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(items[selectedIndex].itemType_); // Attempt to cast selected item to seed

	// If the selected item is a seed, return true. Otherwise, return false.
	return seed != NULL;
}

// Returns true if the hand is selected, false otherwise.
bool UInventoryComponent::OnHand()
{
	// If the selected slot is empty, act as though the hand is selected
	if (items[selectedIndex].itemType_ == NULL)
	{
		return true;
	}

	UDA_Hand const* hand = Cast<UDA_Hand const>(items[selectedIndex].itemType_); // Attempt to cast selected item to hand

	// If the selected item is the hand, return true. Otherwise, return false.
	return hand != NULL;
}

// Returns true if a potion is selected, false otherwise.
bool UInventoryComponent::OnPotion()
{
	// If the selected slot is empty, a potion is not selected
	if (items[selectedIndex].itemType_ == NULL)
	{
		return false;
	}

	UDA_Potion const* potion = Cast<UDA_Potion const>(items[selectedIndex].itemType_); // Attempt to cast selected item to potion

	// If the selected item is a potion, return true. Otherwise, return false.
	return potion != NULL;
}

// Returns true if a crop is selected, false otherwise.
bool UInventoryComponent::OnCrop()
{
	// If the selected slot is empty, a crop is not selected
	if (items[selectedIndex].itemType_ == NULL)
	{
		return false;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(items[selectedIndex].itemType_); // Attempt to cast selected item to crop

	// If the selected item is a crop, return true. Otherwise, return false.
	return crop != NULL;
}

void UInventoryComponent::SetLock(bool lock)
{
	lock_ = lock;
}

bool UInventoryComponent::GetLock() const
{
	return lock_;
}

// Update inventory slot outline if a cursor is hovered over a slot
void UInventoryComponent::UpdateButtonOnHovered()
{
	if (!player->b_gamepad)
	{
		selectedIndex = -1;

		for (int i = 0; i < inventorySize; ++i)
		{
			if (slotButtons[i] && slotButtons[i]->IsHovered())
			{
				selectedIndex = i;
				UpdateHUD();
				return;
			}
		}
	}
}

void UInventoryComponent::SetHitTestOn()
{
	inventoryHUD->GetWidgetFromName("Hotbar")->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UInventoryComponent::SetHitTestOff()
{
	inventoryHUD->GetWidgetFromName("Hotbar")->SetVisibility(ESlateVisibility::HitTestInvisible);
}