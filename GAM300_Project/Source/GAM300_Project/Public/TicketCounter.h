/*
 * File: TicketCounter.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "DA_Potion.h"
#include "Components/TextRenderComponent.h"
#include "Components/CanvasPanel.h"
#include "TicketComponent.h"
#include "GameFramework/Actor.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "GameStats.h"
#include "TicketCounter.generated.h"


class ACharacterBase;
class UAkComponent;

UENUM(BlueprintType)
enum E_TICKET_STATUS
{
	OPEN, //ticket is open
	CLOSED, //ticket is closed
	NOT_AVAILABLE, //there isn't any ticket available
	CAN_RETURN //player can return a ticket
};

UCLASS(BlueprintType)
class GAM300_PROJECT_API ATicketCounter : public AActor
{
	GENERATED_BODY()

public:
	ATicketCounter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UWidgetComponent* widgetComp; // Exclamation icon

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UParticleSystemComponent* particleComp; // Coin particle

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		USceneComponent* promptComp; // Interaction prompt

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UTextRenderComponent* textComp; // Interaction text

	UPROPERTY(EditAnywhere, Category = "TicketCounter")
		int maxTicketNumber;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wwise")
		UAkComponent* akAudioComponent;

	/* Audio Stuff */

	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akQuestComplete;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akTicketCollect;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akTicketView;

	/* End of audio Stuff */


	UPROPERTY(BlueprintReadWrite, Category = "GameStatsVar")
		AGameStats* game_stats_;

	void Interact();
	void Back();
	UFUNCTION(BlueprintCallable)
		void SetCurrentPuzzleNumber(int puzzleNum);
	void ActivateExclamationIcon();
	void ActivateCoinParticleEffect();
	void SetStatusTicketReturnable();
	bool ReturnTicket(FTicket* ticket);
	bool CheckForRequirements(FTicket* ticket);

public:
	E_TICKET_STATUS currentStatus;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FTicket* GetTicket(int ticketNumber);
	void TurnOffCoinParticle();
	void UpdateTickets();
	void SetCurrentTicket();
	void SetTicketUI(FTicket* ticket, int idx);
	void MoveTicketSelectLeft();
	void MoveTicketSelectRight();
	void SetTicketListVisibility(bool b);
	void SetPlayerInputAvailability(bool b);

	UFUNCTION()
		void PostEvent(UAkAudioEvent* akEvent);

	static const int sizeMax = 2; //maximum number of tickets can be displayed at once
								  //cannot be dynamically changed so fixed at 2

	int currentTicket;
	int ticketNumbers[sizeMax];
	int totalTicketLeft;
	int totalTicketCount;
	int currentPuzzleNumber = 0;
	bool isCoinParticleOn;

	UDataTable* ticketList;
	UDataTable* attributesTable;
	UDataTable* craftingTable;
	
	ACharacterBase* player;

	FTimerHandle timer;

	UCanvasPanel* tickets[sizeMax];
	UUserWidget* ticketListUI;

	UDA_Potion* tempPotion;
};
