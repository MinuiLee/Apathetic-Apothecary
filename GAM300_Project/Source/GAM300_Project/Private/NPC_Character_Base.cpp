/*
 * File: NPC_Character_Base.cpp
 * Course: GAM300
 * Author(s): Macie White, Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "NPC_Character_Base.h"
#include "CharacterBase.h"
#include "Components/BillboardComponent.h"
#include "GAM300_Project/GAM300_ProjectGameModeBase.h"
#include "Kismet/GameplayStatics.h"

#define DEBUG_LOG 0

// Sets default values
ANPC_Character_Base::ANPC_Character_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	iconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	iconComp->bHiddenInGame = false;
	iconComp->SetupAttachment(RootComponent);

	exclamationWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	exclamationWidget->SetupAttachment(RootComponent);

	// load ticket list data table
	FString ticketTablePath = "/Game/DataTables/DT_TicketList";
	static ConstructorHelpers::FObjectFinder<UDataTable> TicketTableObject(GetData(ticketTablePath));
	if (TicketTableObject.Succeeded())
	{
		ticketListDataTable = TicketTableObject.Object;
		totalTicketCount = ticketListDataTable->GetRowMap().Num();
	}
}

// Called when the game starts or when spawned
void ANPC_Character_Base::BeginPlay()
{
	Super::BeginPlay();
	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	iconComp->SetVisibility(true);

	// create ticket UI
	FStringClassReference TicketUIClassRef(TEXT("/Game/UI/CraftingUI_Blueprints/TicketListUI.TicketListUI_C"));
	UClass* ticketUIClass = TicketUIClassRef.TryLoadClass<UUserWidget>();
	ticketUI = CreateWidget<UUserWidget>(GetWorld(), ticketUIClass);
	if (ticketUI)
	{
		ticketUI->AddToViewport();
		ticketUI->SetVisibility(ESlateVisibility::Hidden);
	}

	for (auto it : ticketListDataTable->GetRowMap())
	{
		FTicket* ticket = (FTicket*)it.Value;
		if (ticket->npc == npcKind && ticket->npcLocation == npcLocation)
		{
			ticketList.Add(it.Key);
		}
	}
}

// Called every frame
void ANPC_Character_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	iconComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
	exclamationWidget->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
}

// Called to bind functionality to input
void ANPC_Character_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void ANPC_Character_Base::OnTrace_Implementation(ACharacter* Caller)
{
	if (DEBUG_LOG)
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message! but cooler"));
}

//----------------------------------------------------------------------------------------------------
// Ticket Section
//----------------------------------------------------------------------------------------------------

// Return true if the npc can give ticket to the player
bool ANPC_Character_Base::CanGiveTicket()
{
	// check if player has a ticket already
	if (player->ticketComp->GetTicket())
		return false;

	// check if NPC doesn't have any tickets to give
	if (ticketList.Num() == 0)
		return false;

	// check if NPC has a ticket that player can take
	int puzzleNum = player->ticketComp->currentPuzzleNumber;
	for (int i = 0; i < ticketList.Num(); ++i)
	{
		if (ticketListDataTable->FindRow<FTicket>(ticketList[i], "")->puzzleNumber <= puzzleNum)
		{
			currentTicket = i;
			return true;
		}
	}

	return false;
}

// Give ticket to player and remove it from NPC
void ANPC_Character_Base::GiveTicketToPlayer()
{
	// give ticket to the player
	player->ticketComp->SetTicket(ticketListDataTable->FindRow<FTicket>(ticketList[currentTicket], ""), totalTicketCount);

	// make ticket & recipe book available to open
	player->canOpenRecipeBook = true;
	player->ticketComp->SetKeyIndicatorVisibility(true);

	// remove ticket from the ticket list NPC has
	ticketList.RemoveAt(currentTicket);
}

// return if ticket UI is visible
bool ANPC_Character_Base::OpenTicketUI()
{
	if (ticketUI->GetVisibility() == ESlateVisibility::Visible)
	{
		// hide ticket UI
		ticketUI->SetVisibility(ESlateVisibility::Hidden);

		// reset input options and HUD visibility back to normal
		AGAM300_ProjectGameModeBase* gameMode = Cast<AGAM300_ProjectGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

		if (gameMode)
		{
			gameMode->UnhideUI();
		}

		if (controller)
		{
			controller->SetInputMode(FInputModeGameOnly());
		}

		player->canOpenRecipeBook = true;
		player->DeactivateUIControls();
		player->DisableMovementInput(false);
		player->inventoryComp->SetLock(false);
		player->inventoryComp->AddInventoryHUD();
		player->ticketComp->SetKeyIndicatorVisibility(true);

		return true;
	}
	else
	{
		// make ticket UI visible on the screen
		SetTicketUI(ticketListDataTable->FindRow<FTicket>(ticketList[currentTicket], ""), 0);
		ticketUI->SetVisibility(ESlateVisibility::Visible);

		// block all other inputs & remove other HUD
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

		player->canOpenRecipeBook = false;
		player->DisableMovementInput(true);
		player->inventoryComp->SetLock(true);
		player->inventoryComp->RemoveInventoryHUD();
		player->ticketComp->SetKeyIndicatorVisibility(false);

		player->PlayTicketCollectSound();

		return false;
	}
}

// Update ticket UI to the ticket NPC is currently holding
void ANPC_Character_Base::SetTicketUI(FTicket* ticket, int idx)
{
	if (ticket)
	{
		UTextBlock* ticketName = Cast<UTextBlock>(ticketUI->GetWidgetFromName(FName(FString("Name") + FString::FromInt(idx))));
		if (ticketName)
		{
			ticketName->SetText(ticket->name);
		}

		UTextBlock* ticketDescription = Cast<UTextBlock>(ticketUI->GetWidgetFromName(FName(FString("Description") + FString::FromInt(idx))));
		if (ticketDescription)
		{
			ticketDescription->SetText(ticket->description);
		}

		UTextBlock* rewardName = Cast<UTextBlock>(ticketUI->GetWidgetFromName(FName(FString("RewardName") + FString::FromInt(idx) + "_" + FString::FromInt(0))));
		if (rewardName)
		{
			rewardName->SetText(FText::FromString(FString::FromInt(ticket->currency).GetCharArray().GetData()));
		}

		UTextBlock* npcName = Cast<UTextBlock>(ticketUI->GetWidgetFromName(FName(FString("NPCName_") + FString::FromInt(idx))));
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

		UImage* potionImage = Cast<UImage>(ticketUI->GetWidgetFromName(FName(FString("Potion_Icon_") + FString::FromInt(idx))));
		if (potionImage)
		{
			potionImage->SetBrushFromTexture(ticket->requirement->texture_);
		}

		UImage* npcImage = Cast<UImage>(ticketUI->GetWidgetFromName(FName(FString("NPCImage_") + FString::FromInt(idx))));
		if (npcImage)
		{
			npcImage->SetBrushFromTexture(ticket->npcImage);
		}
	}
}
