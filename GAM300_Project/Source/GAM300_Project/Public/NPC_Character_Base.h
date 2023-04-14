/*
 * File: NPC_Character_Base.h
 * Course: GAM300
 * Author(s): Macie White
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Dialogue_Interface.h"
#include "Engine/DataTable.h"
#include "TicketComponent.h"
#include "Components/CanvasPanel.h"
#include "BehaviorTree/BehaviorTree.h"
#include "NPC_Character_Base.generated.h"

class ACharacterBase;

enum E_NPC_STATUS
{
	CAN_TALK,	 // NPC has something to say
	CANNOT_TALK // NPC has already been spoken with/nothing new
};

UCLASS()
class GAM300_PROJECT_API ANPC_Character_Base : public ACharacter, public IDialogue_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPC_Character_Base();
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool is_displaying = false;
	virtual void OnTrace_Implementation(ACharacter* Caller) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UBillboardComponent* iconComp; // Interaction icon

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UWidgetComponent* exclamationWidget; // Exclamation icon

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
		TArray<UBehaviorTree*> gettingScripts;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialogue")
		TArray<UBehaviorTree*> returningScripts;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	ACharacterBase* player;

//---------------------- Ticket Stuff -----------------------//
public:
	UFUNCTION(BlueprintCallable)
		bool CanGiveTicket();
	UFUNCTION(BlueprintCallable)
		void GiveTicketToPlayer();
	UFUNCTION(BlueprintCallable)
		bool OpenTicketUI();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TEnumAsByte<E_NPC_KIND> npcKind;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TEnumAsByte<E_NPC_LOCATION_KIND> npcLocation;

private:
	void SetTicketUI(FTicket* ticket, int idx);

	int totalTicketCount;
	int currentTicket;
	TArray<FName> ticketList;

	UDataTable* ticketListDataTable;
	UUserWidget* ticketUI;
};
