/*
 * File: Crate.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "Crate.h"
#include "CharacterBase.h"
#include "Crop.h"
#include "Blueprint/UserWidget.h"

 // Sets default values
ACrate::ACrate()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	meshComp->SetupAttachment(RootComponent);

	promptComp = CreateDefaultSubobject<USceneComponent>(TEXT("Prompt"));
	promptComp->SetupAttachment(RootComponent);

	promptIconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	promptIconComp->SetupAttachment(promptComp);

	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;
}

void ACrate::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	FStringClassReference StorageUIClassRef(TEXT("/Game/UI/CraftingUI_Blueprints/StorageUI.StorageUI_C"));
	UClass* storageUIClass = StorageUIClassRef.TryLoadClass<UUserWidget>();
	storageUI = CreateWidget<UUserWidget>(GetWorld(), storageUIClass);
	storageUI->AddToViewport(0);
	storageUI->SetVisibility(ESlateVisibility::Hidden);

	for (int i = 0; i < storageSize; ++i)
	{
		slotImages[i] = Cast<UImage>(storageUI->GetWidgetFromName(FName(FString("Button_Image_") + FString::FromInt(i))));
		slotOutlines[i] = Cast<UImage>(storageUI->GetWidgetFromName(FName(FString("Outline_") + FString::FromInt(i))));
		slotAmounts[i] = Cast<UTextBlock>(storageUI->GetWidgetFromName(FName(FString("Amount_") + FString::FromInt(i))));
		slotNames[i] = Cast<UTextBlock>(storageUI->GetWidgetFromName(FName(FString("Name_") + FString::FromInt(i))));
		slotButtons[i] = Cast<UButton>(storageUI->GetWidgetFromName(FName(FString("Button_") + FString::FromInt(i))));

		if (slotButtons[i])
		{
			slotButtons[i]->OnClicked.AddDynamic(this, &ACrate::AddToInventory);
		}
	}

	Cast<UButton>(storageUI->GetWidgetFromName(TEXT("ExitButton")))->OnClicked.AddDynamic(this, &ACrate::CloseCrate);

	UpdateUI();
}

void ACrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// hide prompt when not being looked at
	promptIconComp->SetVisibility(player->GetHitActor() == this);

	// turn prompt to face camera
	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	promptComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));

	if (isCrateOpen)
	{
		if (player->b_gamepad == false)
		{
			selectedSlot = -1;

			for (int i = 0; i < storageSize; ++i)
			{
				if (slotButtons[i] && slotButtons[i]->IsHovered())
				{
					selectedSlot = i;
					break;
				}
			}
		}

		UpdateOutlines();
	}
}

void ACrate::OpenCrate()
{
	player->SetIsHUDOn(true);
	player->ticketComp->SetKeyIndicatorVisibility(false);
	player->crate = this;

	storageUI->SetVisibility(ESlateVisibility::Visible);
	isCrateOpen = true;

	// Play "Open Crate" Sound
	PostEvent(akOpenCrate);


	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller)
	{
		controller->SetInputMode(FInputModeGameAndUI());
	}

	controllerPosition = 0;
	selectedSlot = 0;
	player->inventoryComp->selectedIndex = -1;

	UpdateOutlines();
}

void ACrate::ResetCrate()
{
	if (isCrateOpen)
	{
		player->inventoryComp->selectedIndex = -1;
		player->inventoryComp->UpdateHUD();
		selectedSlot = 0;
		controllerPosition = 0;
		UpdateOutlines();
	}
}

void ACrate::MoveSlotSlectionLeft()
{
	if (controllerPosition == 0)
	{
		if (selectedSlot > 0)
		{
			selectedSlot--;
		}
	}
	else if(controllerPosition == 1)
	{
		player->inventoryComp->selectedIndex--;
		if (player->inventoryComp->selectedIndex < 0)
		{
			player->inventoryComp->selectedIndex = player->inventoryComp->inventorySize - 1;
		}
	}

	UpdateOutlines();
}

void ACrate::MoveSlotSlectionRight()
{
	if (controllerPosition == 0)
	{
		if (selectedSlot < storageSize - 1)
		{
			selectedSlot++;
		}
	}
	else if(controllerPosition == 1)
	{
		player->inventoryComp->selectedIndex++;
		player->inventoryComp->selectedIndex %= player->inventoryComp->inventorySize;
	}

	UpdateOutlines();
}

void ACrate::MoveSlotSlectionUp()
{
	if (controllerPosition == 0)
	{
		if (selectedSlot >= 3)
		{
			selectedSlot -= 3;
		}
	}
	else
	{
		controllerPosition = 0;
		selectedSlot = 7;
		player->inventoryComp->selectedIndex = -1;
	}

	UpdateOutlines();
}

void ACrate::MoveSlotSlectionDown()
{
	if (controllerPosition == 0)
	{
		if (selectedSlot < 6)
		{
			selectedSlot += 3;
		}
		else
		{
			controllerPosition = 1;
			selectedSlot = -1;
			player->inventoryComp->selectedIndex = 0;
		}
	}

	UpdateOutlines();
}

void ACrate::SelectButtonPressed()
{
	if (controllerPosition == 0)
	{
		if (selectedSlot != -1)
		{
			AddToInventory();
		}
	}
	else
	{
		if (player->inventoryComp->selectedIndex != -1)
		{
			player->MoveItemToCrate();
		}
	}
}

void ACrate::CloseCrate()
{
	player->SetIsHUDOn(false);
	player->DeactivateUIControls();
	player->inventoryComp->SetLock(false);
	player->inventoryComp->selectedIndex = 0;
	player->crate = nullptr;

	// Play "Close Crate" Sound
	PostEvent(akCloseCrate);

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller)
	{
		controller->SetInputMode(FInputModeGameOnly());
	}

	if (player->canOpenRecipeBook)
	{
		player->ticketComp->SetKeyIndicatorVisibility(true);
	}

	storageUI->SetVisibility(ESlateVisibility::Hidden);
	isCrateOpen = false;
}

bool ACrate::AddItem(FUS_Item const& newItem)
{
	for (int i = 0; i < storageSize; ++i)
	{
		if (items[i].itemType_ == newItem.itemType_)
		{
			items[i].amount_ += newItem.amount_;
			UpdateUI();
			return true;
		}
	}

	for (int i = 0; i < storageSize; ++i)
	{
		if (items[i].itemType_ == NULL || items[i].amount_ == 0)
		{
			items[i].itemType_ = newItem.itemType_;
			items[i].amount_ = newItem.amount_;
			UpdateUI();
			return true;
		}
	}

	return false;
}


void ACrate::UpdateUI()
{
	for (int i = 0; i < storageSize; ++i)
	{
		if (slotImages[i])
		{
			UTexture2D* texture = GetItemTexture(items[i]);
			if (texture)
			{
				slotImages[i]->SetBrushFromTexture(texture);
			}
			else
			{
				slotImages[i]->SetBrushFromTexture(emptySlotTexture);
			}
		}

		if (slotNames[i])
		{
			FString name = GetItemName(items[i]);
			slotNames[i]->SetText(FText::FromString(name));
		}

		if (slotAmounts[i])
		{
			if (items[i].itemType_)
			{
				slotAmounts[i]->SetText(FText::FromString(FString::FromInt(items[i].amount_)));
			}
			else
			{
				slotAmounts[i]->SetText(FText::FromString(""));
			}
		}
	}
}

void ACrate::UpdateOutlines()
{
	for (int i = 0; i < storageSize; ++i)
	{
		if (selectedSlot == i)
		{
			if (slotOutlines[i])
			{
				slotOutlines[i]->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			if (slotOutlines[i])
			{
				slotOutlines[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

UTexture2D* ACrate::GetItemTexture(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_);
	if (potion != nullptr)
	{
		return potion->texture_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_);
	if (seed != nullptr)
	{
		return seed->texture_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_);
	if (crop != nullptr)
	{
		return crop->texture_;
	}

	return nullptr;
}

FString ACrate::GetItemName(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_);
	if (potion != NULL)
	{
		return potion->name_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_);
	if (seed != NULL)
	{
		return seed->name_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_);
	if (crop != NULL)
	{
		return crop->name_;
	}

	return "";
}

void ACrate::AddToInventory()
{
	if (items[selectedSlot].itemType_ == nullptr)
		return;

	if (player->inventoryComp->AddItem(items[selectedSlot]))
	{
		items[selectedSlot].amount_ = 0;
		items[selectedSlot].itemType_ = nullptr;

		UpdateUI();

		PostEvent(akAddItem);
	}
	else
	{
		PostEvent(akAddFailed);
	}
}

void ACrate::ChangeInputSprite(bool is_controller)
{
	if (is_controller)
	{
		promptIconComp->SetSprite(controllerIcon);
	}
	else
	{
		promptIconComp->SetSprite(keyboardIcon);
	}
}

//--------------------- shop functions ---------------------//

FUS_Item& ACrate::GetItem()
{
	//need to fix
	return items[0];
}

void ACrate::SellCrops()
{
	/*const UDA_Crop* crop = Cast<UDA_Crop>(crops.itemType_);
	int money = crops.amount_ * crop->sell_value_;

	crops.amount_ = 0;
	crops.itemType_ = nullptr;
	cropOnTop->Destroy();
	cropOnTop = nullptr;
	keyText->SetVisibility(ESlateVisibility::Hidden);
	UpdateUI();

	player->currency_ += money;*/

}

int ACrate::SellValue()
{
	/*const UDA_Crop* crop = Cast<UDA_Crop>(crops.itemType_);
	return crops.amount_ * crop->sell_value_;*/
	return 0;
}

void ACrate::PostEvent(UAkAudioEvent* akEvent)
{
	// Audio-related
	FOnAkPostEventCallback nullCallback;
	akAudioComponent->PostAkEvent(akEvent, int32(0), nullCallback, akEvent->GetName());
}

