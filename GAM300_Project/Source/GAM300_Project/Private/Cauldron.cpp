/*
 * File: Cauldron.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#include "Cauldron.h"

#include "Crop.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraComponent.h"
#include "CharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

 // Sets default values
ACauldron::ACauldron()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	meshComp->SetupAttachment(RootComponent);

	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	widgetComp->SetupAttachment(RootComponent);

	particleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	particleComp->SetupAttachment(RootComponent);

	niagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	niagaraComp->SetupAttachment(RootComponent);

	timelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	// Audio
	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;

	// Load data tables
	FString craftingTablePath = "/Game/DataTables/DT_CraftingTable";
	FString attributeTablePath = "/Game/DataTables/DT_Attributes";

	static ConstructorHelpers::FObjectFinder<UDataTable> craftingTableObject(GetData(craftingTablePath));
	if (craftingTableObject.Succeeded())
	{
		craftingTable = craftingTableObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> attributeTableObject(GetData(attributeTablePath));
	if (attributeTableObject.Succeeded())
	{
		attributesTable = attributeTableObject.Object;
	}
}

void ACauldron::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	UClass* cauldronUIClass = widgetComp->GetWidgetClass();
	cauldronUI = widgetComp->GetUserWidgetObject();
	widgetComp->AddLocalRotation(FRotator(0, 180, 0));
	widgetComp->SetAllPhysicsPosition(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 230.f));

	for (int i = 0; i < sizeLimit; ++i)
	{
		cropImages[i] = Cast<UImage>(cauldronUI->GetWidgetFromName((FString("Crop") + FString::FromInt(i) + FString("_Image")).GetCharArray().GetData()));
		cropHighlightImages[i] = Cast<UImage>(cauldronUI->GetWidgetFromName((FString("Crop") + FString::FromInt(i) + FString("_highlight")).GetCharArray().GetData()));
		cropNames[i] = Cast<UTextBlock>(cauldronUI->GetWidgetFromName((FString("Crop") + FString::FromInt(i) + FString("_Name")).GetCharArray().GetData()));
		cropCounts[i] = Cast<UTextBlock>(cauldronUI->GetWidgetFromName((FString("Crop") + FString::FromInt(i) + FString("_Number")).GetCharArray().GetData()));
	}

	potionImage = Cast<UImage>(cauldronUI->GetWidgetFromName("Potion_Image"));
	potionName = Cast<UTextBlock>(cauldronUI->GetWidgetFromName("Potion_Name"));
	potionCount = Cast<UTextBlock>(cauldronUI->GetWidgetFromName("Potion_Number"));
	newPotion = Cast<UTextBlock>(cauldronUI->GetWidgetFromName("NewPotion"));

	horizontalBoxes[0] = Cast<UHorizontalBox>(cauldronUI->GetWidgetFromName("CropList"));
	horizontalBoxes[1] = Cast<UHorizontalBox>(cauldronUI->GetWidgetFromName("PotionList"));

	backgroundImages[0] = Cast<UImage>(cauldronUI->GetWidgetFromName("CropList_Background"));
	backgroundImages[1] = Cast<UImage>(cauldronUI->GetWidgetFromName("Potion_Background"));

	key = Cast<UCanvasPanel>(cauldronUI->GetWidgetFromName("Key"));

	particleComp->SetAllPhysicsPosition(GetActorLocation() + FVector(0.f, 0.f, 100.f));
	particleComp->SetVisibility(false);

	niagaraComp = Cast<UNiagaraComponent>(GetComponentByClass(UNiagaraComponent::StaticClass()));
	niagaraComp->SetAllPhysicsPosition(GetActorLocation() + FVector(0.f, 0.f, 50.f));

	// spawn potion actor
	if (potionActorBP)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		potionActor = GetWorld()->SpawnActor<AActor>(potionActorBP, GetActorLocation(), GetActorRotation(), SpawnParams);
		potionActor->SetActorRelativeScale3D(FVector(50.f));
		potionActor->SetActorHiddenInGame(true);
	}

	// bind potion functions
	potionLerp.BindUFunction(this, FName("PotionPopup"));
	potionFinished.BindUFunction(this, FName("PotionPopupEnd"));
	timelineComp->AddInterpFloat(potionCurve, potionLerp, "yPosition");
	timelineComp->SetTimelineFinishedFunc(potionFinished);
	timelineComp->SetLooping(false);

	SetCurrentUIKind(CROP);
	UpdateUI();

	//akAudioComponent->PostAkEventByName("ItemCauldronBubble");
}

void ACauldron::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (player->GetHitActor() == this || UIKind == E_CAULDRON_UI_KIND::POTION)
	{
		cauldronUI->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		cauldronUI->SetVisibility(ESlateVisibility::Hidden);
	}

	// rotate UI to face camera
	if (widgetComp->IsVisible())
	{
		FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
		widgetComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));;
	}

	if (UIKind == E_CAULDRON_UI_KIND::POTION)
	{
		FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
		potionActor->SetActorRotation(FRotator(0.0f, cameraRot.Yaw + 180.f, 0.0f));
	}
}

void ACauldron::BeginDestroy()
{
	Super::BeginDestroy();
}

void ACauldron::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (cauldronUI)
	{
		cauldronUI->SetVisibility(ESlateVisibility::Hidden);
	}
}

// Called when the player tries to put an item in a cauldon
bool ACauldron::AddItemToCauldron(UDA_Crop const* crop)
{
	if (crops.Num() < sizeLimit)
	{
		crops.Emplace(crop);
		cropNumbers.Emplace(1);
	}
	else
	{
		FString crop1 = crops[0]->name_;
		FString crop2 = crops[1]->name_;

		if (crop1 != crop->name_ && crop2 != crop->name_)
		{
			return false;
		}
		else
		{
			if (crop1 != crop2)
			{
				if (crop1 == crop->name_)
					cropNumbers[0] += 1;
				else
					cropNumbers[1] += 1;
			}
			else
			{
				if (cropNumbers[0] <= cropNumbers[1])
					cropNumbers[0] += 1;
				else
					cropNumbers[1] += 1;
			}
		}

	}
	
	SetCurrentUIKind(CROP);

	UpdateUI();

	niagaraComp->SetAsset(niagaraSystem);
	niagaraComp->Activate(true);

	return true;
}

// Returns whether the player is close enough to interact with a cauldron
bool ACauldron::CanInteract()
{
	return FVector::Dist(player->GetActorLocation(), GetActorLocation()) < 200.f;
}

bool ACauldron::CanAddCrop(UDA_Crop const* crop)
{
	if (potion != nullptr)
	{
		return false;
	}
	
	if (crops.Num() < sizeLimit)
	{
		return true;
	}

	FString crop1 = crops[0]->name_;
	FString crop2 = crops[1]->name_;

	if (crop1 != crop->name_ && crop2 != crop->name_)
	{
		return false;
	}

	if (crop1 == crop->name_ && cropNumbers[0] >= maxCraftNumber)
	{
		return false;
	}
	else if(crop2 == crop->name_ && cropNumbers[1] >= maxCraftNumber)
	{
		return false;
	}

	return true;
}

// Called when the timer ends
void ACauldron::TurnOffBubbling()
{
	if (potionActor)
	{
		potionActor->SetActorHiddenInGame(false);
	}
	particleComp->SetVisibility(false);
	niagaraComp->Deactivate();
	timelineComp->PlayFromStart();
}

// Either return a potion or try to make a potion
int ACauldron::Interact()
{
	// nothing in the cauldron
	if (UIKind == E_CAULDRON_UI_KIND::NONE)
	{
		return 0;
	}
	// returns a potion if there is
	else if (UIKind == E_CAULDRON_UI_KIND::POTION && potion)
	{
		// add potion to player's inventory
		if (player->inventoryComp->AddItem(FUS_Item(potion, defaultPotionNumber * potionNumber)) == false)
			return 0;

		potion = nullptr;

		SetCurrentUIKind(CROP);
		UpdateUI();

		return 2;
	}
	// start making a potion if there is right amount of ingredients in it
	else if (MakePotion())
	{
		//particleComp->SetVisibility(true);
		niagaraComp->SetAsset(niagaraSystemCraft);
		niagaraComp->Activate(true);
		GetWorld()->GetTimerManager().SetTimer(timer, this, &ACauldron::TurnOffBubbling, bubblingTime);

		SetCurrentUIKind(E_CAULDRON_UI_KIND::NONE);

		UpdateUI();

		if (potion)
		{
			craftedPotionName = potion->name_;
			isPotionCrafted = true;
		}

		akAudioComponent->PostAkEventByName("ItemCauldronBubble");
		return 1;
	}
	else
		SetCurrentUIKind(CROP);

	UpdateUI();

	return 0;
}

void ACauldron::EmptyCauldron()
{
	cropNumbers.Empty();
	crops.Empty();
	UpdateUI();
}

void ACauldron::UpdateUI()
{
	if (UIKind == E_CAULDRON_UI_KIND::POTION || UIKind == E_CAULDRON_UI_KIND::NONE)
	{
		if (potionImage != nullptr)
		{
			if (potion)
			{
				potionImage->SetBrushFromTexture(potion->texture_);
				potionName->SetText(FText::FromString(potion->name_));
				potionCount->SetText(FText::FromString(FString::FromInt(defaultPotionNumber * potionNumber)));
			}
			else
			{
				potionImage->SetBrushFromTexture(emptySlotTexture);
				potionName->SetText(FText::FromString(""));
				potionCount->SetText(FText::FromString(""));
			}
		}
	}
	else if (UIKind == CROP)
	{
		for (int i = 0; i < sizeLimit; ++i)
		{
			if (cropImages[i] != nullptr)
			{
				if (i < crops.Num() && crops[i] && GetItemTexture(crops[i]) != NULL)
				{
					cropImages[i]->SetVisibility(ESlateVisibility::Visible);
					cropImages[i]->SetBrushFromTexture(GetItemTexture(crops[i]));
				}
				else
				{
					cropImages[i]->SetVisibility(ESlateVisibility::Hidden);
				}
			}

			if (cropNames[i] != nullptr)
			{
				if (i < crops.Num() && crops[i])
				{
					cropNames[i]->SetText(FText::FromString(GetItemName(crops[i])));
				}
				else
				{
					cropNames[i]->SetText(FText::FromString(""));
				}
			}

			if (cropCounts[i] != nullptr)
			{
				if (i < crops.Num() && crops[i] && cropNumbers[i] > 0)
				{
					cropCounts[i]->SetText(FText::FromString(FString::FromInt(cropNumbers[i])));
				}
				else
				{
					cropCounts[i]->SetText(FText::FromString(""));
				}
			}
		}

		if (crops.Num() == 1)
		{
			if (cropHighlightImages[0] != nullptr)
			{
				cropHighlightImages[0]->SetBrushFromTexture(highlightedSlotTexture);
			}
		}
		else if (crops.Num() == 2)
		{
			if (cropHighlightImages[1] != nullptr)
			{
				cropHighlightImages[1]->SetBrushFromTexture(highlightedSlotTexture);
			}

			if (backgroundImages[CROP] != nullptr)
			{
				backgroundImages[CROP]->SetBrushFromTexture(highlightedPanelTexture);
			}
		}
		else
		{
			if (backgroundImages[CROP] != nullptr)
			{
				backgroundImages[CROP]->SetBrushFromTexture(emptyPanelTexture);
			}
		}
	}
}

void ACauldron::SetCurrentUIKind(E_CAULDRON_UI_KIND kind)
{
	UIKind = kind;

	if (kind == CROP)
	{
		key->SetVisibility(ESlateVisibility::Visible);
		horizontalBoxes[CROP]->SetVisibility(ESlateVisibility::Visible);
		horizontalBoxes[POTION]->SetVisibility(ESlateVisibility::Hidden);
		backgroundImages[CROP]->SetVisibility(ESlateVisibility::Visible);
		backgroundImages[POTION]->SetVisibility(ESlateVisibility::Hidden);
		newPotion->SetVisibility(ESlateVisibility::Hidden);
	}
	else if (kind == POTION)
	{
		key->SetVisibility(ESlateVisibility::Visible);
		horizontalBoxes[POTION]->SetVisibility(ESlateVisibility::Visible);
		horizontalBoxes[CROP]->SetVisibility(ESlateVisibility::Hidden);
		backgroundImages[POTION]->SetVisibility(ESlateVisibility::Visible);
		backgroundImages[CROP]->SetVisibility(ESlateVisibility::Hidden);

		cropHighlightImages[0]->SetBrushFromTexture(emptySlotTexture);
		cropHighlightImages[1]->SetBrushFromTexture(emptySlotTexture);

		newPotion->SetVisibility(ESlateVisibility::Visible);
	}
	else if (kind == E_CAULDRON_UI_KIND::NONE)
	{
		key->SetVisibility(ESlateVisibility::Hidden);
		horizontalBoxes[POTION]->SetVisibility(ESlateVisibility::Hidden);
		horizontalBoxes[CROP]->SetVisibility(ESlateVisibility::Hidden);
		backgroundImages[POTION]->SetVisibility(ESlateVisibility::Hidden);
		backgroundImages[CROP]->SetVisibility(ESlateVisibility::Hidden);
		newPotion->SetVisibility(ESlateVisibility::Hidden);
	}
}

UTexture2D* ACauldron::GetItemTexture(UDA_Crop const* crop)
{
	if (crop != nullptr)
	{
		return crop->texture_;
	}

	return nullptr;
}

FString ACauldron::GetItemName(UDA_Crop const* crop)
{
	if (crop != NULL)
	{
		return crop->attribute_;
	}

	return FString();
}

// Returns number of potions made
int ACauldron::MakePotion()
{
	int size = crops.Num();

	// failed to make a potion -> need more ingredients
	if (size <= 1)
	{
		return 0;
	}

	// adds all crops' attribute elements
	int fire = 0, water = 0, wind = 0, nature = 0, spirit = 0;

	for (int i = 0; i < crops.Num(); ++i)
	{
		FAttribute* attribute = attributesTable->FindRow<FAttribute>(FName(crops[i]->attribute_), "");
		if (attribute)
		{
			fire += attribute->ignisElement;
			water += attribute->aquaElement;
			wind += attribute->ventusElement;
			nature += attribute->terraElement;
			spirit += attribute->spiritusElement;
		}
	}

	// gets a potion from the crafting table
	int i = 0;
	for (auto it : craftingTable->GetRowMap())
	{
		FCraftingRecipe* recipe = (FCraftingRecipe*)it.Value;

		// find if there is a matching recipe with ingredients
		if (recipe->aquaElement == water && recipe->terraElement == nature &&
			recipe->ignisElement == fire && recipe->ventusElement == wind &&
			recipe->spiritusElement == spirit)
		{
			potion = recipe->potion;

			int crop1 = cropNumbers[0];
			int crop2 = cropNumbers[1];

			if (crop1 <= crop2)
			{
				potionNumber = crop1;
				if(crop2 - crop1 != 0)
					player->inventoryComp->AddItem(FUS_Item(crops[1], crop2 - crop1));
			}
			else
			{
				potionNumber = crop2;
				if (crop1 - crop2 != 0)
					player->inventoryComp->AddItem(FUS_Item(crops[0], crop1 - crop2));
			}

			player->ticketComp->SetRecipeVisible(i);
			player->ticketComp->SetRecipeIngredients(i, crops);
			player->canOpenRecipeBook = true;
			player->ticketComp->SetKeyIndicatorVisibility(true);
			EmptyCauldron();

			return potionNumber;
		}
		
		++i;
	}

	potion = failedPotion;
	potionNumber = 1;
	EmptyCauldron();
	return 1;
}

// Called when a cauldron starts making a potion
void ACauldron::PotionPopup(float value)
{
	if (potionActor)
	{
		FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
		potionActor->SetActorRotation(FRotator(0.0f, cameraRot.Yaw + 180.f, 0.0f));
		potionActor->SetActorLocation(FVector(potionActor->GetActorLocation().X, potionActor->GetActorLocation().Y, value * 120.f));
	}
}

void ACauldron::PotionPopupEnd()
{
	SetCurrentUIKind(POTION);
	if (potionActor)
	{
		potionActor->SetActorHiddenInGame(true);
	}
	niagaraComp->Deactivate();

}