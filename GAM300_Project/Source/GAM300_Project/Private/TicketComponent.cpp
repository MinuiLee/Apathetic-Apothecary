/*
 * File: TicketComponent.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "TicketComponent.h"
#include "Cauldron.h"
#include "GAM300_Project/GAM300_ProjectGameModeBase.h"
#include "Kismet/GameplayStatics.h"

 // Sets default values for this component's properties
UTicketComponent::UTicketComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	FString attributeTablePath = "/Game/DataTables/DT_Attributes";
	static ConstructorHelpers::FObjectFinder<UDataTable> attributeTableObject(GetData(attributeTablePath));
	if (attributeTableObject.Succeeded())
	{
		attributesTable = attributeTableObject.Object;
	}

	FString craftingTablePath = "/Game/DataTables/DT_CraftingTable";
	static ConstructorHelpers::FObjectFinder<UDataTable> craftingTableObject(GetData(craftingTablePath));
	if (craftingTableObject.Succeeded())
	{
		craftingTable = craftingTableObject.Object;
	}
}

// Called when the game starts
void UTicketComponent::BeginPlay()
{
	Super::BeginPlay();

	player = player = Cast<ACharacterBase>(GetOwner());

	// create ticket related HUD
	FStringClassReference TicketHUDClassRef(TEXT("/Game/UI/CraftingUI_Blueprints/TicketHUD.TicketHUD_C"));
	UClass* ticketHUDClass = TicketHUDClassRef.TryLoadClass<UUserWidget>();
	ticketHUD = CreateWidget<UUserWidget>(GetWorld(), ticketHUDClass);
	ticketHUD->AddToViewport();

	// create recipe book UI
	FStringClassReference RecipeBookUIClassRef(TEXT("/Game/UI/CraftingUI_Blueprints/RecipeBookUI.RecipeBookUI_C"));
	UClass* ticketUIClass = RecipeBookUIClassRef.TryLoadClass<UUserWidget>();
	recipeBookUI = CreateWidget<UUserWidget>(GetWorld(), ticketUIClass);
	recipeBookUI->AddToViewport();

	// get pointers to components of the ticket HUD
	congratPanel = Cast<UPanelWidget>(ticketHUD->GetWidgetFromName(TEXT("CongratesPanel")));
	if (congratPanel) congratPanel->SetVisibility(ESlateVisibility::Hidden);
	keyIndicator = Cast<UPanelWidget>(ticketHUD->GetWidgetFromName(TEXT("TicketIndicator")));

	// get pointers to components of the ticket UI
	ticketName = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("Name")));
	ticketDescription = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("Description")));
	rewardName = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("RewardAmount")));
	npcName = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("NPCName"))); npcImage = Cast<UImage>(recipeBookUI->GetWidgetFromName(TEXT("NPCImage")));
	ticketPotionImage = Cast<UImage>(recipeBookUI->GetWidgetFromName(TEXT("PotionImageT")));
	ticketPanel = Cast<UPanelWidget>(recipeBookUI->GetWidgetFromName(TEXT("Ticket")));

	// get pointers to components of the recipe book
	potionImage = Cast<UImage>(recipeBookUI->GetWidgetFromName(TEXT("PotionImage")));
	potionName = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("PotionName")));
	potionDescription = Cast<UTextBlock>(recipeBookUI->GetWidgetFromName(TEXT("PotionDescription")));
	potionImageOutline = Cast<UImage>(recipeBookUI->GetWidgetFromName(TEXT("PotionImageOutline")));
	recipePanel = Cast<UPanelWidget>(recipeBookUI->GetWidgetFromName(TEXT("Recipe")));
	for (int i = 0; i < ingredientSize; ++i)
	{
		potionIngredients[i] = Cast<UImage>(recipeBookUI->GetWidgetFromName(FName(FString("IngredientImage_") + FString::FromInt(i))));
		potionIngredientOutlines[i] = Cast<UImage>(recipeBookUI->GetWidgetFromName(FName(FString("IngredientOutline_") + FString::FromInt(i))));
		ingredientUI[i] = Cast<UUserWidget>(recipeBookUI->GetWidgetFromName(FName(FString("IngredientDetailUI_") + FString::FromInt(i))));
		ingredientUI[i]->SetVisibility(ESlateVisibility::Hidden);
	}
	for (int i = 0; i < recipeSize; ++i)
	{
		recipeImages[i] = Cast<UImage>(recipeBookUI->GetWidgetFromName(FName(FString("RecipeImage_") + FString::FromInt(i))));
		recipeButtons[i] = Cast<UButton>(recipeBookUI->GetWidgetFromName(FName(FString("RecipeButton_") + FString::FromInt(i))));
		recipeOutlines[i] = Cast<UImage>(recipeBookUI->GetWidgetFromName(FName(FString("Outline_") + FString::FromInt(i))));
		recipeSelectedOutlines[i] = Cast<UImage>(recipeBookUI->GetWidgetFromName(FName(FString("SelectedOutline_") + FString::FromInt(i))));
	}

	isFadeOut = false;

	//add dynamics to the buttons
	for (int i = 0; i < recipeSize; ++i)
	{
		recipeButtons[i]->OnClicked.AddDynamic(this, &UTicketComponent::MoveToRecipeDetail);
	}

	//hide unused buttons
	/*for (int i = recipeSize; i < 20; ++i)
	{
		Cast<UPanelWidget>(recipeBookUI->GetWidgetFromName(FName(FString("Recipe_") + FString::FromInt(i))))->SetVisibility(ESlateVisibility::Hidden);
	}*/

	//add dynamic to the exit button
	Cast<UButton>(recipeBookUI->GetWidgetFromName(TEXT("Exit")))->OnClicked.AddDynamic(this, &UTicketComponent::CloseRecipeBook);

	SetKeyIndicatorVisibility(false);
	//SetTicket(ticketData, 0, 0);
	//game_stats_->SetHasTicket(false);
	UpdateRecipies();
}


// Called every frame
void UTicketComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (isFadeOut && congratPanel)
	{
		float opacity = congratPanel->GetRenderOpacity();
		if (opacity <= 0.f)
		{
			isFadeOut = false;
			congratPanel->SetVisibility(ESlateVisibility::Hidden);
		}
		else
			congratPanel->SetRenderOpacity(opacity - 0.0075f);
	}

	if (isRecipeBookOpen)
	{
		if (player->b_gamepad == false)
		{
			UpadteHoveredEventLeft();
			UpadteHoveredEventRight();
		}

		UpdateOutlines();
	}
}

void UTicketComponent::BeginDestroy()
{
	Super::BeginDestroy();
}

void UTicketComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (recipeBookUI)
	{
		SetKeyIndicatorVisibility(false);
		recipeBookUI->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UTicketComponent::ToggleCongratulationsHUD(FText text)
{
	if (congratPanel)
	{
		if (text.ToString().Equals(""))
		{
			text = FText::FromString(FString::FromInt(ticketData->currency) + FString(" coins"));
		}
		Cast<UTextBlock>(ticketHUD->GetWidgetFromName((FString("text3")).GetCharArray().GetData()))->SetText(text);
		congratPanel->SetVisibility(ESlateVisibility::Visible);
		congratPanel->SetRenderOpacity(1.f);
		GetWorld()->GetTimerManager().SetTimer(timer, this, &UTicketComponent::StartFadeOut, 2.f);
	}
}

void UTicketComponent::StartFadeOut()
{
	isFadeOut = true;
}

void UTicketComponent::SetKeyIndicatorVisibility(bool b)
{
	if (b && player->canOpenRecipeBook)
		keyIndicator->SetVisibility(ESlateVisibility::Visible);
	else
		keyIndicator->SetVisibility(ESlateVisibility::Hidden);
}

void UTicketComponent::RecipeButtonOnClicked()
{
	onClickedRecipe = onHoveredRecipe;
}

void UTicketComponent::ResetRecipeBook()
{
	if (isRecipeBookOpen == false)
	{
		return;
	}

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller)
	{
		controller->SetInputMode(FInputModeGameOnly());
		controller->SetInputMode(FInputModeGameAndUI());

		if (player->b_gamepad == false)
		{
			controller->bShowMouseCursor = true;
			controller->bEnableClickEvents = true;
			controller->bEnableMouseOverEvents = true;
		}
		else
		{
			isOnLeft = true;
			onHoveredRecipe = onClickedRecipe;
			onClickedRecipe = -1;
			onHoveredIngredient = -1;
			onClickedIngredient = -1;
			UpdateIngredientDetail(true);
			UpdateOutlines();

			potionImageOutline->SetColorAndOpacity(FLinearColor(1, 1, 1));
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Recipe Book Functions
//----------------------------------------------------------------------------------------------------

void UTicketComponent::UpdateOutlines()
{
	for (int i = 0; i < recipeSize; ++i)
	{
		if (i == onHoveredRecipe || i == onClickedRecipe)
		{
			if (recipeOutlines[i])
			{
				recipeOutlines[i]->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			if (recipeOutlines[i])
			{
				recipeOutlines[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (i == onClickedRecipe)
		{
			if (recipeSelectedOutlines[i])
			{
				recipeSelectedOutlines[i]->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			if (recipeSelectedOutlines[i])
			{
				recipeSelectedOutlines[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UTicketComponent::UpdateRecipies()
{
	int recipeNum = 0;
	for (auto it : craftingTable->GetRowMap())
	{
		if (recipeNum >= recipeSize) return;

		FCraftingRecipe* recipe = (FCraftingRecipe*)it.Value;

		if (recipeVisibility[recipeNum] == true && recipeImages[recipeNum])
			recipeImages[recipeNum]->SetBrushFromTexture(recipe->potion->texture_);

		++recipeNum;
	}
}

void UTicketComponent::UpdateRecipeDetail()
{
	int i = 0;
	int recipeNum = onHoveredRecipe;
	bool isHoveredOne = true;

	if (onHoveredRecipe == -1)
	{
		if (onClickedRecipe != -1)
		{
			isHoveredOne = false;
			recipeNum = onClickedRecipe;
		}
		else
		{
			recipePanel->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
	}


	for (auto it : craftingTable->GetRowMap())
	{
		if (i != recipeNum)
		{
			++i;
			continue;
		}

		FCraftingRecipe* recipe = (FCraftingRecipe*)it.Value;
		if (recipeVisibility[recipeNum] == true)
		{
			UDA_Potion* potion = recipe->potion;

			recipePanel->SetVisibility(ESlateVisibility::Visible);

			//potion details
			potionImage->SetBrushFromTexture(potion->texture_);
			potionName->SetText(FText::FromString(potion->name_));
			potionDescription->SetText(FText::FromString(potion->description_));

			for (int j = 0; j < ingredientSize; ++j)
			{
				potionIngredients[j]->SetBrushFromTexture(recipeIngredients[recipeNum][j]->attributeTextureOutline_);

				Cast<UTextBlock>(ingredientUI[j]->GetWidgetFromName(TEXT("CropName")))->
					SetText(FText::FromString(recipeIngredients[recipeNum][j]->name_));

				Cast<UImage>(ingredientUI[j]->GetWidgetFromName(TEXT("AttributeImage")))->
					SetBrushFromTexture(recipeIngredients[recipeNum][j]->texture_);
			}

			if (onClickedRecipe == i)
			{
				potionImageOutline->SetColorAndOpacity(FLinearColor(0.1, 0.1, 1));
			}
			else
			{
				potionImageOutline->SetColorAndOpacity(FLinearColor(1, 1, 1));
			}
		}
		else
		{
			recipePanel->SetVisibility(ESlateVisibility::Hidden);
		}

		return;
	}

	recipePanel->SetVisibility(ESlateVisibility::Hidden);
}

void UTicketComponent::UpadteHoveredEventRight()
{
	onHoveredIngredient = -1;

	if (recipeVisibility[onClickedRecipe])
	{
		for (int i = 0; i < ingredientSize; ++i)
		{
			if (potionIngredients[i]->IsHovered() || ingredientUI[i]->IsHovered() || potionIngredientOutlines[i]->IsHovered())
			{
				potionIngredientOutlines[i]->SetVisibility(ESlateVisibility::Visible);
				ingredientUI[i]->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				potionIngredientOutlines[i]->SetVisibility(ESlateVisibility::Hidden);
				ingredientUI[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UTicketComponent::UpadteHoveredEventLeft()
{
	onHoveredRecipe = -1;
	onHoveredRecipe = -1;

	for (int i = 0; i < recipeSize; ++i)
	{
		if (recipeButtons[i]->IsHovered())
		{
			onHoveredRecipe = i;
		}
	}

	UpdateRecipeDetail();
}

void UTicketComponent::UpdateIngredientDetail(bool reset)
{
	if (recipeVisibility[onClickedRecipe])
	{
		for (int i = 0; i < ingredientSize; ++i)
		{
			if ((onHoveredIngredient == i || onClickedIngredient == i) && reset == false)
			{
				potionIngredientOutlines[i]->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				potionIngredientOutlines[i]->SetVisibility(ESlateVisibility::Hidden);
			}

			if (onClickedIngredient == i && reset == false)
			{
				ingredientUI[i]->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				ingredientUI[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UTicketComponent::SetRecipeVisible(int recipeNumber)
{
	recipeVisibility[recipeNumber] = true;
	recipeImages[recipeNumber]->SetVisibility(ESlateVisibility::Visible);
	UpdateRecipies();
}

void UTicketComponent::SetRecipeIngredients(int recipeNumber, TArray<const UDA_Crop*> ingredients)
{
	recipeIngredients[recipeNumber].Emplace(ingredients[0]);
	recipeIngredients[recipeNumber].Emplace(ingredients[1]);
}

void UTicketComponent::MoveToRecipeDetail()
{
	int recipeNum = 0;
	for (auto it : craftingTable->GetRowMap())
	{
		if (recipeNum != onHoveredRecipe)
		{
			++recipeNum;
			continue;
		}

		if (recipeVisibility[recipeNum] == true)
		{
			onClickedRecipe = onHoveredRecipe;
			isOnLeft = false;
		}
		else
		{
			onClickedRecipe = -1;
			isOnLeft = true;
		}

		return;
	}
}

bool UTicketComponent::IsRecipeBookOpen()
{
	return isRecipeBookOpen;
}

//----------------------------------------------------------------------------------------------------
// Input Binding Functions
//----------------------------------------------------------------------------------------------------

void UTicketComponent::MoveRecipeSelectLeft()
{
	if (isRecipeBookOpen == false) return;

	if (isOnLeft)
	{
		if (onHoveredRecipe > 0)
			onHoveredRecipe--;
		else
			onHoveredRecipe = recipeSize - 1;

		UpdateRecipeDetail();
	}
	else
	{
		if (onClickedIngredient == -1)
		{
			onHoveredIngredient = 0;
			UpdateIngredientDetail(false);
		}
	}
}

void UTicketComponent::MoveRecipeSelectRight()
{
	if (isRecipeBookOpen == false) return;

	if (isOnLeft)
	{
		onHoveredRecipe++;
		onHoveredRecipe %= recipeSize;

		UpdateRecipeDetail();
	}
	else
	{
		if (onClickedIngredient == -1)
		{
			onHoveredIngredient = 1;
			UpdateIngredientDetail(false);
		}
	}
}

void UTicketComponent::MoveRecipeSelectUp()
{
	if (isRecipeBookOpen == false || isOnLeft == false) return;

	if (onHoveredRecipe >= 2)
		onHoveredRecipe -= 2;

	UpdateRecipeDetail();
}

void UTicketComponent::MoveRecipeSelectDown()
{
	if (isRecipeBookOpen == false || isOnLeft == false) return;

	if (onHoveredRecipe < recipeSize - 2)
		onHoveredRecipe += 2;

	UpdateRecipeDetail();
}

void UTicketComponent::BackButtonPressed()
{
	if (isOnLeft)
	{
		CloseRecipeBook();
	}
	else
	{
		if (onClickedIngredient != -1)
		{
			onClickedIngredient = -1;
			UpdateIngredientDetail(false);
		}
		else
		{
			isOnLeft = true;
			onClickedRecipe = -1;
			onHoveredIngredient = -1;
			onClickedIngredient = -1;
			UpdateIngredientDetail(true);
			UpdateOutlines();

			potionImageOutline->SetColorAndOpacity(FLinearColor(1, 1, 1));
		}
	}
}

void UTicketComponent::SelectButtonPressed()
{
	if (isOnLeft) 
	{
		MoveToRecipeDetail();

		// if the recipe is available to show
		if (isOnLeft == false)
		{
			onHoveredIngredient = 0;
			onClickedIngredient = -1;
			UpdateIngredientDetail(false);

			potionImageOutline->SetColorAndOpacity(FLinearColor(0.1, 0.1, 1));
		}
	}
	else
	{
		if (onClickedIngredient == -1)
		{
			onClickedIngredient = onHoveredIngredient;
		}
		else
		{
			onClickedIngredient = -1;
		}

		UpdateIngredientDetail(false);
	}
}

void UTicketComponent::OpenRecipeBook()
{
	AGAM300_ProjectGameModeBase* gameMode = Cast<AGAM300_ProjectGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (gameMode)
	{
		gameMode->HideUI();
	}

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller)
	{
		controller->SetInputMode(FInputModeGameAndUI());
	}

	player->SetIsHUDOn(true);
	player->DisableMovementInput(true);
	recipeBookUI->SetVisibility(ESlateVisibility::Visible);
	congratPanel->SetVisibility(ESlateVisibility::Hidden);

	isRecipeBookOpen = true;
	isOnLeft = true;
}

void UTicketComponent::CloseRecipeBook()
{
	AGAM300_ProjectGameModeBase *gameMode = Cast<AGAM300_ProjectGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (gameMode)
	{
		gameMode->UnhideUI();
	}

	if (controller)
	{
		controller->SetInputMode(FInputModeGameOnly());
	}
	
	recipeBookUI->SetVisibility(ESlateVisibility::Hidden);

	player->SetIsHUDOn(false);
	player->DeactivateUIControls();
	player->inventoryComp->SetLock(false);
	player->DisableMovementInput(false);

	isRecipeBookOpen = false;
}

//----------------------------------------------------------------------------------------------------
// Ticket Section
//----------------------------------------------------------------------------------------------------

void UTicketComponent::SetTicket(FTicket* ticket, int totalNumber)
{
	ticketData = ticket;

	if (ticket != nullptr)
	{
		if (ticketName)
		{
			ticketName->SetText(ticket->name);
		}

		if (ticketDescription)
		{
			ticketDescription->SetText(ticket->description);
		}

		if (ticketPotionImage)
		{
			ticketPotionImage->SetBrushFromTexture(ticket->requirement->texture_);
		}

		if (rewardName)
		{
			rewardName->SetText(FText::FromString(FString::FromInt(ticket->currency)));
		}

		if (npcName)
		{
			FString npc;
			switch (ticket->npc)
			{
			case E_NPC_KIND::Aoife: npc += "Aoife"; break;
			case E_NPC_KIND::Fiz:  npc += "Fiz"; break;
			case E_NPC_KIND::Bran:  npc += "Bran"; break;
			default: break;
			}

			switch (ticket->npcLocation)
			{
			case E_NPC_LOCATION_KIND::LevelHub: npc += " (Level Hub)"; break;
			case E_NPC_LOCATION_KIND::MainHub: npc += " (Main Hub)"; break;
			case E_NPC_LOCATION_KIND::Puzzle1: npc += " (Puzzle 1)"; break;
			case E_NPC_LOCATION_KIND::Puzzle2: npc += " (Puzzle 2)"; break;
			case E_NPC_LOCATION_KIND::Puzzle3: npc += " (Puzzle 3)"; break;
			default: break;
			}

			npcName->SetText(FText::FromString(npc));
		}

		if (npcImage)
		{
			npcImage->SetBrushFromTexture(ticket->npcImage);
		}

		ticketPanel->SetVisibility(ESlateVisibility::Visible);
		congratPanel->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		ticketPanel->SetVisibility(ESlateVisibility::Hidden);
	}
}

FTicket* UTicketComponent::GetTicket()
{
	return ticketData;
}

bool UTicketComponent::CanReturnTicket(E_NPC_KIND npc, E_NPC_LOCATION_KIND npcLocation)
{
	// if player doesn't have a ticket 
	if (ticketData == nullptr)
		return false;

	// check if the player can return a ticket
	if (ticketData->npc == npc && ticketData->npcLocation == npcLocation)
	{
		// see if there is a matching potion in the inventory
		int size = player->inventoryComp->GetInventorySize();
		for (int i = 0; i < size; ++i)
		{
			const UDA_Potion* potionInv = Cast<UDA_Potion>(player->inventoryComp->items[i].itemType_);
			if (potionInv && potionInv == ticketData->requirement)
			{
				return true;
			}
		}
	}

	return false;
}

void UTicketComponent::ReturnTicket()
{
	//game_stats_->SetDoneTicket(true);
	//game_stats_->SetHasTicket(false);

	// remove required potion from the inventory
	int size = player->inventoryComp->GetInventorySize();
	for (int i = 0; i < size; ++i)
	{
		const UDA_Potion* potionInv = Cast<UDA_Potion>(player->inventoryComp->items[i].itemType_);
		if (potionInv && potionInv == ticketData->requirement)
		{
			player->inventoryComp->items[i].amount_ -= 1;
			if (player->inventoryComp->items[i].amount_ <= 0)
			{
				player->inventoryComp->items[i].itemType_ = nullptr;
				player->inventoryComp->items[i].amount_ = 0;
			}
		}
	}

	// turn on congratulations HUD
	ToggleCongratulationsHUD();

	player->PlayTicketCompleteSound();
	// give ticket reward to the player
	player->currency_ += ticketData->currency;
	// remove player ticket
	SetTicket(nullptr, 0);

	// activate coin particle effect on player's head
	isCoinParticleOn = true;
	//particleComp->SetAllPhysicsPosition(player->GetActorLocation() + FVector(0, 0, 100));
	//particleComp->Activate();
	//GetWorld()->GetTimerManager().SetTimer(timer, this, &ATicketCounter::TurnOffCoinParticle, 3.f);

	// play sfx
	//PostEvent(akQuestComplete);
}

void UTicketComponent::SetCurrentPuzzleNumber(int puzzleNum)
{
	if (currentPuzzleNumber < puzzleNum)
		currentPuzzleNumber = puzzleNum;
}

bool UTicketComponent::HasTicket()
{
	return ticketData != nullptr;
}

