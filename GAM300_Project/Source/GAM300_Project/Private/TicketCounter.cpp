/*
 * File: TicketCounter.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "TicketCounter.h"
#include "DA_Potion.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterBase.h"
#include "Components/BillboardComponent.h"
#include "Components/Border.h"
#include "GameFramework/SpringArmComponent.h"

#define DEBUG_LOG 0

// Sets default values
ATicketCounter::ATicketCounter()
{
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	meshComp->SetupAttachment(RootComponent);

	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	widgetComp->SetupAttachment(RootComponent);

	particleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	particleComp->SetupAttachment(RootComponent);

	promptComp = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPrompt"));
	promptComp->SetupAttachment(RootComponent);

	textComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InteractionText"));
	textComp->SetText(FText::FromString("E"));
	textComp->SetupAttachment(promptComp);

	// Audio
	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;

	// load data tables
	FString ticketTablePath = "/Game/DataTables/DT_TicketList";
	static ConstructorHelpers::FObjectFinder<UDataTable> TicketTableObject(GetData(ticketTablePath));
	if (TicketTableObject.Succeeded())
	{
		ticketList = TicketTableObject.Object;
		totalTicketLeft = ticketList->GetRowMap().Num();
		totalTicketCount = ticketList->GetRowMap().Num();
	}

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

	currentStatus = E_TICKET_STATUS::CLOSED;
	isCoinParticleOn = false;
}

// Called when the game starts or when spawned
void ATicketCounter::BeginPlay()
{
	Super::BeginPlay();

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	textComp->SetWorldLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 230.f));
	widgetComp->SetWorldLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 300.f));

	FStringClassReference TicketUIClassRef(TEXT("/Game/UI/CraftingUI_Blueprints/TicketListUI.TicketListUI_C"));
	UClass* ticketUIClass = TicketUIClassRef.TryLoadClass<UUserWidget>();
	ticketListUI = CreateWidget<UUserWidget>(GetWorld(), ticketUIClass);
	ticketListUI->AddToViewport();

	// enables input
	EnableInput(GetWorld()->GetFirstPlayerController());

	// sets initial ticket number
	currentTicket = 0;

	// ticket UIs
	for (int i = 0; i < sizeMax; ++i)
	{
		if (i < maxTicketNumber)
		{
			ticketNumbers[i] = i;
			totalTicketLeft--;
			tickets[i] = Cast<UCanvasPanel>(ticketListUI->GetWidgetFromName((FString("Ticket") + FString::FromInt(i)).GetCharArray().GetData()));
			SetTicketUI(GetTicket(i), i);
		}
		else
		{
			ticketNumbers[i] = -1;
			tickets[i] = Cast<UCanvasPanel>(ticketListUI->GetWidgetFromName((FString("Ticket") + FString::FromInt(i)).GetCharArray().GetData()));
		}
	}

	SetTicketListVisibility(false);
	SetCurrentTicket();
}

void ATicketCounter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// show key prompt on a ticket counter
	if (player->GetHitActor() == this)
	{
		if (currentStatus == E_TICKET_STATUS::CLOSED || currentStatus == E_TICKET_STATUS::CAN_RETURN)
		{
			textComp->SetVisibility(true);
			FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
			textComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
		}
		else
		{
			textComp->SetVisibility(false);
		}
	}
	else
	{
		textComp->SetVisibility(false);
		SetTicketListVisibility(false);

		if (currentStatus == E_TICKET_STATUS::OPEN)
		{
			currentStatus = E_TICKET_STATUS::CLOSED;
		}
	}

	if (currentStatus == E_TICKET_STATUS::CLOSED)
	{
		// Exclamation icon UI
		widgetComp->SetVisibility(true);
		FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
		widgetComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
	}

	// updates coin rain particle location
	if (isCoinParticleOn)
	{
		particleComp->SetAllPhysicsPosition(player->GetActorLocation() + FVector(0, 0, 100));
	}

	// checks if the ticket player has can be returned
	FTicket* ticket = (player->ticketComp ? player->ticketComp->GetTicket() : nullptr);
	
	if (ticket == nullptr)
	{
		game_stats_->SetHasTicket(false);
		game_stats_->SetDoneTicket(false);
	}
	else
	{
		game_stats_->SetHasTicket(true);
	}
}

void ATicketCounter::BeginDestroy()
{
	Super::BeginDestroy();
}

void ATicketCounter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (ticketListUI)
	{
		ticketListUI->RemoveFromParent();
	}
	if (widgetComp->GetUserWidgetObject())
	{
		widgetComp->GetUserWidgetObject()->RemoveFromParent();
	}
}

// Get ticket data from the data table
FTicket* ATicketCounter::GetTicket(int ticketNumber)
{
	return ticketList->FindRow<FTicket>(FName(*FString::FromInt(ticketNumber)), "");
}

// Called when the coin rain particle timer ends
void ATicketCounter::TurnOffCoinParticle()
{
	particleComp->Deactivate();
	isCoinParticleOn = false;
}

void ATicketCounter::UpdateTickets()
{
	if (totalTicketLeft <= 0) return;
	
	for (int i = 0; i < maxTicketNumber; ++i)
	{
		int ticketNumber = totalTicketCount - totalTicketLeft;
		FTicket* ticket = GetTicket(ticketNumber);

		if (ticketNumbers[i] == -1 && ticket && ticket->puzzleNumber <= currentPuzzleNumber)
		{
			ticketNumbers[i] = ticketNumber;
			SetTicketUI(GetTicket(ticketNumbers[i]), i);
			totalTicketLeft -= 1;

			if (player->ticketComp->GetTicket() == nullptr) currentStatus = E_TICKET_STATUS::CLOSED;
		}
	}
}

// Scale ticket sizes (make currently selected one bigger than others)
void ATicketCounter::SetCurrentTicket()
{
	if (currentTicket < maxTicketNumber && currentTicket > -1)
	{
		for (int i = 0; i < maxTicketNumber; ++i)
		{
			if (i == currentTicket)
				tickets[i]->SetRenderScale(FVector2D(0.9f, 0.9f));
			else
				tickets[i]->SetRenderScale(FVector2D(0.5f, 0.5f));
		}
	}
}

void ATicketCounter::SetTicketUI(FTicket* ticket, int idx)
{
	if (tickets[idx] && ticket)
	{
		UTextBlock* ticketName = Cast<UTextBlock>(ticketListUI->GetWidgetFromName(FName(FString("Name") + FString::FromInt(idx))));
		if (ticketName)
		{
			ticketName->SetText(ticket->name);
		}

		UTextBlock* ticketDescription = Cast<UTextBlock>(ticketListUI->GetWidgetFromName(FName(FString("Description") + FString::FromInt(idx))));
		if (ticketDescription)
		{
			ticketDescription->SetText(ticket->description);
		}

		UTextBlock* potionName = Cast<UTextBlock>(ticketListUI->GetWidgetFromName((FString("PotionName_") + FString::FromInt(idx)).GetCharArray().GetData()));
		if (potionName)
		{
			potionName->SetText(FText::FromString(ticket->requirement->name_));
		}

		UTextBlock* rewardName = Cast<UTextBlock>(ticketListUI->GetWidgetFromName(FName(FString("RewardName") + FString::FromInt(idx) + "_" + FString::FromInt(0))));
		if (rewardName)
		{
			rewardName->SetText(FText::FromString(FString::FromInt(ticket->currency).GetCharArray().GetData()));
		}

		UTextBlock* ticketNumber = Cast<UTextBlock>(ticketListUI->GetWidgetFromName(FName(FString("TicketNumber_") + FString::FromInt(idx))));
		if (ticketNumber)
		{
			ticketNumber->SetText(FText::FromString(FString::FromInt(ticketNumbers[idx]+1) + FString("/") + FString::FromInt(totalTicketCount)));
		}

		UTextBlock* npcName = Cast<UTextBlock>(ticketListUI->GetWidgetFromName(FName(FString("NPCName_") + FString::FromInt(idx))));
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
			}

			npcName->SetText(FText::FromString(npc));
		}

		UImage* potionImage = Cast<UImage>(ticketListUI->GetWidgetFromName(FName(FString("Potion_Icon_") + FString::FromInt(idx))));
		if (potionImage)
		{
			potionImage->SetBrushFromTexture(ticket->requirement->texture_);
		}

		UImage* npcImage = Cast<UImage>(ticketListUI->GetWidgetFromName(FName(FString("NPCImage_") + FString::FromInt(idx))));
		if (npcImage)
		{
			npcImage->SetBrushFromTexture(ticket->npcImage);
		}
	}
}

void ATicketCounter::Interact()
{
	// picking up a ticket
	if (currentStatus == E_TICKET_STATUS::OPEN)
	{
		if (currentTicket != -1)
		{
			// send the current ticket data to the player
			//player->ticketComp->SetTicket(GetTicket(ticketNumbers[currentTicket]), ticketNumbers[currentTicket]+1, totalTicketCount);
			ticketNumbers[currentTicket] = -1;
			
			// update the currently selected ticket with new ticket data
			if (totalTicketLeft > 0)
				UpdateTickets();
			else
				currentTicket = -1;

			currentStatus = E_TICKET_STATUS::NOT_AVAILABLE;

			// UI visibility
			SetTicketListVisibility(false);
			textComp->SetVisibility(false);

			// unable player input
			SetPlayerInputAvailability(true);

			// set the player has earned at least one ticket
			player->canOpenRecipeBook = true;
			player->ticketComp->SetKeyIndicatorVisibility(true);
			
			PostEvent(akTicketCollect);
		}
	}
	// opening up a ticket
	else if (currentStatus == E_TICKET_STATUS::CLOSED)
	{
		for (int i = 0; i < maxTicketNumber; ++i)
		{
			if (ticketNumbers[i] != -1)
			{
				currentTicket = i;
				break;
			}
		}

		SetCurrentTicket();

		if (currentTicket != -1)
		{
			currentStatus = E_TICKET_STATUS::OPEN;

			// disable any player input
			SetPlayerInputAvailability(false);

			// UI visibility
			SetTicketListVisibility(true);
			widgetComp->SetVisibility(false);
		}

		PostEvent(akTicketView);
	}
}

// Called when player exits ticket counter
void ATicketCounter::Back()
{
	currentStatus = E_TICKET_STATUS::CLOSED;
	SetTicketListVisibility(false);

	// only available when there is at least one ticket left on the counter
	for (int i = 0; i < maxTicketNumber; ++i)
	{
		if (ticketNumbers[i] != -1)
		{
			widgetComp->SetVisibility(true);
			break;
		}
	}
}

void ATicketCounter::SetCurrentPuzzleNumber(int puzzleNum)
{
	if(currentPuzzleNumber < puzzleNum)
		currentPuzzleNumber = puzzleNum;

	UpdateTickets();
}

void ATicketCounter::ActivateExclamationIcon()
{
	currentStatus = E_TICKET_STATUS::NOT_AVAILABLE;
	widgetComp->SetVisibility(false);
	textComp->SetVisibility(false);

	// turn on exclamation point if there is at least one ticket left on the counter
	for (int i = 0; i < maxTicketNumber; ++i)
	{
		if (ticketNumbers[i] != -1)
		{
			currentStatus = E_TICKET_STATUS::CLOSED;
			widgetComp->SetVisibility(true);
			textComp->SetVisibility(true);
			break;
		}
	}
}

void ATicketCounter::ActivateCoinParticleEffect()
{
	// turn on coin rain particle effect
	isCoinParticleOn = true;
	particleComp->SetAllPhysicsPosition(player->GetActorLocation() + FVector(0, 0, 100));
	particleComp->Activate();
	GetWorld()->GetTimerManager().SetTimer(timer, this, &ATicketCounter::TurnOffCoinParticle, 3.f);
}

void ATicketCounter::SetStatusTicketReturnable()
{
	currentStatus = E_TICKET_STATUS::CAN_RETURN;
	game_stats_->SetDoneTicket(true);
}

// Check whether the player has all the requirements for the ticket
bool ATicketCounter::CheckForRequirements(FTicket* ticket)
{
	if (ticket)
	{
		tempPotion = ticket->requirement;
		
		// see if there is a matching potion in the inventory
		int size = player->inventoryComp->GetInventorySize();
		for (int i = 0; i < size; ++i)
		{
			const UDA_Potion* potionInv = Cast<UDA_Potion>(player->inventoryComp->items[i].itemType_);
			if (potionInv && potionInv == tempPotion)
			{
				return true;
			}
		}
	}

	return false;
}

void ATicketCounter::MoveTicketSelectLeft()
{
	if (currentTicket - 1 >= 0)
	{
		if (ticketNumbers[currentTicket - 1] != -1)
		{
			currentTicket -= 1;
			SetCurrentTicket();
		}
	}
}

void ATicketCounter::MoveTicketSelectRight()
{
	if (currentTicket + 1 < maxTicketNumber)
	{
		if (ticketNumbers[currentTicket + 1] != -1)
		{
			currentTicket += 1;
			SetCurrentTicket();
		}
	}
}

void ATicketCounter::SetTicketListVisibility(bool b)
{
	int count = 0;
	for (int i = 0; i < sizeMax; ++i)
	{
		if (tickets[i])
		{
			if (b == false || ticketNumbers[i] == -1)
			{
				tickets[i]->SetVisibility(ESlateVisibility::Hidden);
			}
			else
			{
				tickets[i]->SetVisibility(ESlateVisibility::Visible);
				count++;
			}
		}
	}

	if (ticketListUI)
	{
		UBorder* Q = Cast<UBorder>(ticketListUI->GetWidgetFromName(FString("Background_Q").GetCharArray().GetData()));
		UBorder* R = Cast<UBorder>(ticketListUI->GetWidgetFromName(FString("Background_R").GetCharArray().GetData()));
		
		if (Q == nullptr || R == nullptr)
			return;

		if (b == true && count >= 2)
		{
			Q->SetVisibility(ESlateVisibility::Visible);
			R->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Q->SetVisibility(ESlateVisibility::Hidden);
			R->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// Disable all other inputs except ones for interacting with the counter
void ATicketCounter::SetPlayerInputAvailability(bool b)
{
	if (b)
	{
		player->EnableInput(GetWorld()->GetFirstPlayerController());
		InputComponent->ClearActionBindings();
	}
	else
	{
		player->DisableInput(GetWorld()->GetFirstPlayerController());
		InputComponent->BindAction("InteractWithObject", EInputEvent::IE_Pressed, this, &ATicketCounter::Interact).bExecuteWhenPaused = false;
		InputComponent->BindAction("MoveHotbarSelectRight", EInputEvent::IE_Pressed, this, &ATicketCounter::MoveTicketSelectRight).bExecuteWhenPaused = false;
		InputComponent->BindAction("MoveHotbarSelectLeft", EInputEvent::IE_Pressed, this, &ATicketCounter::MoveTicketSelectLeft).bExecuteWhenPaused = false;
	}
}

void ATicketCounter::PostEvent(UAkAudioEvent* akEvent)
{
	// Audio-related
	FOnAkPostEventCallback nullCallback;
	akAudioComponent->PostAkEvent(akEvent, int32(0), nullCallback, akEvent->GetName());
}

// Returns reward if the ticket can be returned
bool ATicketCounter::ReturnTicket(FTicket* ticket)
{
	// if current player doesn't have a required potion in the inventory
	if (currentStatus != E_TICKET_STATUS::CAN_RETURN)
		return false;

	// remove required potions from the inventory
	int size = player->inventoryComp->GetInventorySize();
	for (int i = 0; i < size; ++i)
	{
		const UDA_Potion* potionInv = Cast<UDA_Potion>(player->inventoryComp->items[i].itemType_);
		if (potionInv && potionInv == tempPotion)
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
	player->ticketComp->ToggleCongratulationsHUD();
	// remove player ticket
	player->ticketComp->SetTicket(nullptr, 0);
	// give ticket reward to the player
	player->currency_ += ticket->currency;

	// activate coin particle effect on player's head
	ActivateCoinParticleEffect();

	// activate exclamation point icon if there is a ticket left that player can take
	ActivateExclamationIcon();

	// play sfx
	PostEvent(akQuestComplete);

	return true;
}

