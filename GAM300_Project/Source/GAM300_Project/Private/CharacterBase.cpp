/*
 * File: CharacterBase.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "CharacterBase.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DA_Potion.h"
#include "Crop.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "AssetRegistryModule.h"
#include "PuzzleObject.h"
#include "FarmingPlot.h"
#include "TestPlant.h"
#include "Dialogue_Interface.h"
#include "NPC_Character_Base.h"
#include "Shopkeeper.h"
#include "Door.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ItemPickup.h"
#include "PuzzleResetButton.h"

#include "GAM300_Project/GAM300_ProjectGameModeBase.h"


#define DEBUG_LOG 0

 // Sets default values
ACharacterBase::ACharacterBase() : pauseMenu(NULL), heldItem(NULL)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	meshComp->SetupAttachment(RootComponent);

	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	//springArmComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	springArmComp->TargetArmLength = 700.f;

	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	cameraComp->SetupAttachment(springArmComp, USpringArmComponent::SocketName);

	inventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
	this->AddInstanceComponent(inventoryComp);
	if (inventoryComp == nullptr)
	{
		if (DEBUG_LOG)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Inventory Component is NULL!");
	}

	ticketComp = CreateDefaultSubobject<UTicketComponent>(TEXT("TicketComp"));
	this->AddInstanceComponent(ticketComp);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	movementSpeed = 0.7f;
	sphereTraceRadius = 57.f;
	currFacingDir = E_FACING_DIRECTION::FRONT;

	// Potion
	niagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Potion"));
	niagaraComp->SetupAttachment(RootComponent);

	// Audio
	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;

	// Timeline --
	timelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	is_shrinked = false;
	BindPotionFunctions();

	InitItemLoading();

	currency_ = 0;
	overlapping_shop_ = false;

	b_puzzleSolved = false;
	b_canInteract = true;
}

void ACharacterBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(APuzzleVolume::StaticClass()))
	{
		inPuzzle = true;
		
		springArmComp->SetWorldRotation(FRotator(-45.f, GetActorRotation().Yaw, 0.f).Clamp(), false, nullptr, ETeleportType::ResetPhysics);
		puzzleCameraAngle = springArmComp->GetComponentRotation();
		puzzleCameraAngle.Yaw = 0;
		puzzleVolume = Cast<APuzzleVolume>(OtherActor);
		
		// Play Puzzle music
		EnterPuzzleRoomAudio();
		
	}
}

void ACharacterBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(APuzzleVolume::StaticClass()))
	{
		inPuzzle = false;
		
		if (heldItem != nullptr && heldItem->IsA(APuzzleObject::StaticClass()))
		{
			APuzzleObject *puzzleObject = Cast<APuzzleObject>(heldItem);
			puzzleObject->SetHeld(false);

			puzzleObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			puzzleObject->SetActorLocation(puzzleObject->startPos);
			puzzleObject->SetActorRotation(FRotator(0, 0, 0));
			puzzleObject->SetActorEnableCollision(true);

			capsuleComp->IgnoreActorWhenMoving(heldItem, false);
			heldItem = nullptr;
			AnimDrop();
		}

		LeavePuzzleRoomAudio();
	}
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController != NULL)
	{
		playerController->SetInputMode(FInputModeGameOnly());
	}

	FStringClassReference pauseMenuClassRef(TEXT("/Game/UI/PauseMenu.PauseMenu_C"));
	UClass* pauseMenuClass = pauseMenuClassRef.TryLoadClass<UUserWidget>();
	pauseMenu = CreateWidget<UUserWidget>(GetWorld(), pauseMenuClass);
	pauseMenu->AddToViewport();
	pauseMenu->SetVisibility(ESlateVisibility::Hidden);

	capsuleComp = Cast<UCapsuleComponent>(GetComponentByClass(UCapsuleComponent::StaticClass()));
	inventoryComp = Cast<UInventoryComponent>(GetComponentByClass(UInventoryComponent::StaticClass()));

	/*inventoryComp->AddItem(FUS_Item(GetPotion("ShrinkPotion"), 3));
	inventoryComp->AddItem(FUS_Item(GetPotion("GrowthPotion"), 4));
	inventoryComp->AddItem(FUS_Item(GetSeed("HerbSeed"), 7));*/

	original_scale_ = GetActorScale3D();

	// Finding cauldron
	TArray<AActor*> FoundCauldron;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACauldron::StaticClass(), FoundCauldron);
	if (FoundCauldron.Num())
	{
		for (AActor* actor : FoundCauldron)
		{
			cauldron = Cast<ACauldron>(actor);
			break;
		}
	}

	// Finding Shopkeeper
	TArray<AActor*> FoundShop;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShopkeeper::StaticClass(), FoundShop);
	if (FoundShop.Num())
	{
		for (AActor* actor : FoundShop)
		{
			shopkeeper_ = Cast<AShopkeeper>(actor);
			break;
		}
	}

	game_mode_ = Cast<AGAM300_ProjectGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	startPos = GetActorLocation();

	//PostEvent(akEnvironmentSounds);
	//PostEvent(akSetFarmMusic);
}

void ACharacterBase::BeginDestroy()
{
	Super::BeginDestroy();

	// Destructor

}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACharacterBase::MoveForward(float value)
{
	if (!mov_inputs_disabled && (value != 0.f && !IsFrozen()))
	{
		const FRotator camera_rotation = springArmComp->GetComponentRotation();

		// Forward - point in same direction as camera
		if (value > 0)
		{
			// Set Player direction
			const FRotator camera_yaw(0, camera_rotation.Yaw, 0); // pitch, yaw, roll
			SetActorRotation(camera_yaw - GetActorRotation());

			// Move in that direction
			AddMovementInput(GetActorForwardVector(), value * movementSpeed, false);
		}
		// Backward - look towards camera
		else
		{
			// Set Player direction
			const FRotator camera_yaw(0, camera_rotation.Yaw + 180, 0); // pitch, yaw, roll
			SetActorRotation(camera_yaw - GetActorRotation());

			// Move in that direction
			AddMovementInput(GetActorForwardVector(), value * movementSpeed * -1, false);
		}
	}
}

void ACharacterBase::MoveRight(float value)
{
	if (!mov_inputs_disabled && (value != 0.f && !IsFrozen()))
	{
		const FRotator camera_rotation = springArmComp->GetComponentRotation();

		// Right
		if (value > 0)
		{
			// Set Player direction
			const FRotator camera_yaw(0, camera_rotation.Yaw + 90, 0); // pitch, yaw, roll
			SetActorRotation(camera_yaw - GetActorRotation());

			// Move in that direction
			AddMovementInput(GetActorForwardVector(), value * movementSpeed, false);
		}
		// Left
		else
		{
			// Set Player direction
			const FRotator camera_yaw(0, camera_rotation.Yaw - 90, 0); // pitch, yaw, roll
			SetActorRotation(camera_yaw - GetActorRotation());

			// Move in that direction
			AddMovementInput(GetActorForwardVector(), value * movementSpeed * -1, false);
		}

	}
}

void ACharacterBase::PitchCamera(float AxisValue)
{
	if (!isHUDOn)
	{
		FRotator NewRotation = springArmComp->GetComponentRotation();
		NewRotation.Pitch += AxisValue;

		if (inPuzzle)
		{
			if (NewRotation.Pitch < puzzleCameraAngle.Pitch - puzzleCameraPitchRange)
			{
				NewRotation.Pitch = puzzleCameraAngle.Pitch - puzzleCameraPitchRange;
			}

			if (NewRotation.Pitch > puzzleCameraAngle.Pitch + puzzleCameraPitchRange)
			{
				NewRotation.Pitch = puzzleCameraAngle.Pitch + puzzleCameraPitchRange;
			}
		}

		// Avoid camera pointing directly up or down
		if (NewRotation.Pitch >= 65.0f)
			NewRotation.Pitch = 65.0f;
		else if (NewRotation.Pitch <= -65.0f)
			NewRotation.Pitch = -65.0f;

		springArmComp->SetWorldRotation(NewRotation);
	}
}

void ACharacterBase::YawCamera(float AxisValue)
{
	if (!isHUDOn)
	{
		FRotator NewRotation = FRotator(springArmComp->GetComponentRotation());

		NewRotation.Yaw += AxisValue;

		if (inPuzzle)
		{
			if (NewRotation.Yaw < puzzleCameraAngle.Yaw + GetActorRotation().Yaw - puzzleCameraYawRange)
			{
				NewRotation.Yaw = puzzleCameraAngle.Yaw + GetActorRotation().Yaw - puzzleCameraYawRange;
			}

			if (NewRotation.Yaw > puzzleCameraAngle.Yaw + GetActorRotation().Yaw + puzzleCameraYawRange)
			{
				NewRotation.Yaw = puzzleCameraAngle.Yaw + GetActorRotation().Yaw + puzzleCameraYawRange;
			}
		}

		springArmComp->SetWorldRotation(NewRotation);
	}
}

void ACharacterBase::TogglePause()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->CloseRecipeBook();
		return;
	}

	if (isHUDOn)
	{
		return;
	}

	if (pauseMenu)
	{
		bool was_paused = UGameplayStatics::IsGamePaused(GetWorld());
		UGameplayStatics::SetGamePaused(GetWorld(), !was_paused);

		if (playerController != NULL)
		{
			// Pause the game
			if (!was_paused)
			{
				inventoryComp->SetHitTestOff();
				pauseMenu->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				playerController->bShowMouseCursor = true;
				playerController->SetInputMode(FInputModeGameAndUI());
				if (game_mode_->is_controller_used)
				{
					Cast<UUserWidget>(pauseMenu->GetWidgetFromName("ResumeButton"))->GetWidgetFromName("ButtonBase")->SetFocus();
				}
				PostEvent(akOpenMenu);
				PostEvent(akPauseGame);
			}
			// Unpause the game
			else
			{
				inventoryComp->SetHitTestOn();
				playerController->bShowMouseCursor = false;
				playerController->SetInputMode(FInputModeGameOnly());
				pauseMenu->GetWidgetFromName("DestructiveActionMainMenu")->SetVisibility(ESlateVisibility::Hidden);
				pauseMenu->GetWidgetFromName("DestructiveActionExit")->SetVisibility(ESlateVisibility::Hidden);
				pauseMenu->GetWidgetFromName("UniformGridPanel_294")->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				pauseMenu->SetVisibility(ESlateVisibility::Hidden);
				PostEvent(akMenuBack);
				PostEvent(akUnpauseGame);
			}
		}
	}
}

bool ACharacterBase::isPaused()
{
	return UGameplayStatics::IsGamePaused(GetWorld());
}

// Checks for interactable actors in front of the player and sets currentHitActor to the most appropriate one, if any
void ACharacterBase::CheckForInteractables()
{
	// If the player is under the effects of a Time Stop potion, they cannot interact with anything
	if (IsFrozen())
	{
		currentHitActor = nullptr;
		return;
	}

	// Position of the center of the sphere to check for interactables inside (the sphere is directly in front of the player)
	FVector spherePos = meshComp->GetComponentLocation() + (meshComp->GetForwardVector() * (30.f + sphereTraceRadius) + meshComp->GetUpVector() * (40.f - sphereTraceRadius));

	TArray<FHitResult> hitResults; // Array of detected interactables

	// Construct parameter struct to check for interactables (which are on trace channel 2)
	FCollisionObjectQueryParams COQP;
	COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2);

	float scaled_radius = sphereTraceRadius; // Radius of sphere to check

	// Construct shape struct to define radius of collision sphere
	FCollisionShape shape;
	shape.SetSphere(scaled_radius);

	// Check for interactables inside sphere
	GetWorld()->SweepMultiByObjectType(hitResults, spherePos, spherePos, FQuat(), COQP, shape);
	//DrawDebugSphere(GetWorld(), spherePos, scaled_radius, 16, FColor::Yellow, false);

	FHitResult hitResult; // Hit result for the closest interactable to the player

	// Go through all detected interactables and find the closest one
	for (int i = 0; i < hitResults.Num(); ++i)
	{
		// Note from Ryan: I changed this loop from a sequence of else ifs to a sequence of ifs with continue statements so that
		//                 the special case for puzzle objects could be less messy while still allowing the proximity check
		//                 to be considered if the special case doesn't pan out

		// If no valid interactables have been found yet, just grab the first one so it can be compared to the others
		if (hitResult.Actor == nullptr)
		{
			hitResult = hitResults[i];
			continue;
		}

		// Held items cannot be interacted with, so ignore them
		if (hitResult.Actor == heldItem)
		{
			continue;
		}

		// SPECIAL CASE: If the current interactable and the closest interactable are both puzzle objects, check whether one would be
		//               better to interact with than the other one
		if (hitResult.Actor->IsA(APuzzleObject::StaticClass()) && hitResults[i].Actor->IsA(APuzzleObject::StaticClass()))
		{
			APuzzleObject *closestPuzzleObject = Cast<APuzzleObject>(hitResult.Actor);
			APuzzleObject *currentPuzzleObject = Cast<APuzzleObject>(hitResults[i].Actor);

			if (inventoryComp->OnPotion())
			{
				UDA_Potion const *potion = Cast<UDA_Potion>(inventoryComp->GetCurrentItem().itemType_);

				// If the player is holding a potion that can be used on the current object but not on the closest object, override the closest object
				if (!closestPuzzleObject->IsValidPotionEffect(potion->potion_effect_) && currentPuzzleObject->IsValidPotionEffect(potion->potion_effect_))
				{
					hitResult = hitResults[i];
					continue;
				}
				// If the player is holding a potion that can be used on the closest object but not on the current object, disregard the current object even if it's closer
				else if (closestPuzzleObject->IsValidPotionEffect(potion->potion_effect_) && !currentPuzzleObject->IsValidPotionEffect(potion->potion_effect_))
				{
					continue;
				}
			}
			// If the player is NOT holding a potion and the current object can be picked up but the closest object cannot, override the closest object
			else if (!closestPuzzleObject->canPickup && currentPuzzleObject->canPickup)
			{
				hitResult = hitResults[i];
				continue;
			}
			// If the player is NOT holding a potion and the closest object can be picked up but the current object cannot, disregard the current object even if it's closer
			else if (closestPuzzleObject->canPickup && !currentPuzzleObject->canPickup)
			{
				continue;
			}
		}

		// Otherwise, compare the old closest interactable's distance to the player to the current interactable's distance to the player
		float oldDist = (hitResult.Actor.Get()->GetActorLocation() - GetActorLocation()).Size();
		float newDist = (hitResults[i].Actor.Get()->GetActorLocation() - GetActorLocation()).Size();

		// If the current interactable is the closest interactable found so far, update the closest interactable
		if (newDist < oldDist)
		{
			hitResult = hitResults[i];
		}
	}

	if (isHUDOn)
	{
		hitResult.bBlockingHit = false;
		hitResult.Actor = nullptr;
	}

	// If an interactable was found, highlight it and update the pointer indicating which interactable is being looked at
	if (hitResult.bBlockingHit)
	{
		// Reset custom depth of all previous actors
		if (currentHitActor != hitResult.GetActor() && currentHitActor != nullptr)
		{
			TArray<UStaticMeshComponent*> components;
			currentHitActor->GetComponents<UStaticMeshComponent>(components);

			for (int i = 0; i < components.Num(); ++i)
			{
				components[i]->SetRenderCustomDepth(false);
				components[i]->SetCustomDepthStencilValue(0);
			}
		}

		currentHitActor = hitResult.GetActor(); // Update looked-at interactable pointer

		// Set custom depth of highlighted interactable
		TArray<UStaticMeshComponent*> components;
		currentHitActor->GetComponents<UStaticMeshComponent>(components);

		for (int i = 0; i < components.Num(); ++i)
		{
			components[i]->SetRenderCustomDepth(true);
			components[i]->SetCustomDepthStencilValue(50);
		}

		// Show cone above farming plot
		if (currentHitActor->IsA(AFarmingPlot::StaticClass()))
			Cast<AFarmingPlot>(currentHitActor)->InRangeOfPlayer();

		// Run whatever logic NPCs have for being highlighted
		if (currentHitActor && currentHitActor->IsA(ANPC_Character_Base::StaticClass()))
		{
			if (Cast<ANPC_Character_Base>(currentHitActor)->is_displaying == false)
			{
				Cast<ANPC_Character_Base>(currentHitActor)->is_displaying = true;
				Cast<ANPC_Character_Base>(currentHitActor)->Execute_OnTrace(Cast<ANPC_Character_Base>(currentHitActor), this);
			}
		}

		// Set shop for interaction
		if (overlapping_shop_)
		{
			shopkeeper_->ActivateInteractionSignifier(false);
		}
	}
	// If no interactable was found, un-highlight whatever was previously highlighted, if anything
	else
	{
		// Run whatever logic NPCs have for being un-highlighted
		if (currentHitActor && currentHitActor->IsA(ANPC_Character_Base::StaticClass()))
		{
			if (Cast<ANPC_Character_Base>(currentHitActor)->is_displaying == true)
			{
				Cast<ANPC_Character_Base>(currentHitActor)->is_displaying = false;
				Cast<ANPC_Character_Base>(currentHitActor)->Execute_OnTrace(Cast<ANPC_Character_Base>(currentHitActor), this);
			}
		}

		// Set shop for interaction
		if (overlapping_shop_)
		{
			shopkeeper_->ActivateInteractionSignifier(true);
		}

		// Reset custom depth of all previous actors
		if (currentHitActor != hitResult.GetActor() && currentHitActor != nullptr)
		{
			TArray<UStaticMeshComponent*> components;
			currentHitActor->GetComponents<UStaticMeshComponent>(components);

			for (int i = 0; i < components.Num(); ++i)
			{
				components[i]->SetRenderCustomDepth(false);
				components[i]->SetCustomDepthStencilValue(0);
			}
		}

		currentHitActor = nullptr; // Set currently looked-at interactable pointer to null (no interactable is being looked at)
	}
}

void ACharacterBase::PlayTicketCollectSound()
{
	PostEvent(akTicketCollect);
}

void ACharacterBase::PlayTicketCompleteSound()
{
	PostEvent(akTicketComplete);
}

// Get current hit actor
AActor* ACharacterBase::GetHitActor()
{
		return currentHitActor;
}

// Get actor used for current animation
AActor* ACharacterBase::GetAnimationFocusActor()
{
	return animationFocus;
}

// Get actor used for current door fade
AActor *ACharacterBase::GetDoorFocusActor()
{
	return doorFocus;
}

void ACharacterBase::InteractWithObject()
{
	if (!b_canInteract) return;

	// The player cannot interact with objects while frozen
	if (IsFrozen())
	{
		return;
	}

	// interact with recipe book buttons
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->SelectButtonPressed();
		return;
	}

	// interact with storage buttons
	if (crate != nullptr)
	{
		crate->SelectButtonPressed();
		return;
	}

	// Put a puzzle object down
	if (heldItem != nullptr)
	{
		if (heldItem->IsA(APuzzleObject::StaticClass()))
		{
			// run throw
			if (GetInputAxisValue(FName("MoveForward")) != 0.f || GetInputAxisValue(FName("MoveRight")) != 0.f)
			{
				AnimThrow();
			}
			else //drop
			{
				AnimDrop();
			}
			return;
		}
	}

	if (inventoryComp == nullptr)
	{
		if (DEBUG_LOG)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Inventory Component is NULL!");
		return;
	}

	// Put item in a cauldron based on the distance between the player and a cauldron
	if (currentHitActor && currentHitActor->IsA(ACauldron::StaticClass()))
	{
		ACauldron* cauldron_ = Cast<ACauldron>(currentHitActor);

		// get potion from cauldron or making one
		int result = cauldron_->Interact();
		switch (result)
		{
			case 0:
				akAudioComponent->PostAkEventByName("ItemCauldronFull");
				break;
			case 1:
				break;
			case 2:
			{
				PostEvent(akCollect);
				return;
			}
		}

		// try add a crop to the cauldron
		const UDA_Crop* crop = Cast<UDA_Crop>(inventoryComp->GetCurrentItem().itemType_);
		if (crop)
		{
			if (cauldron_->CanAddCrop(crop))
			{
				inventoryComp->UseSelectedItemOnObject();
			}
			else
			{
				
			}
		}
		return;
	}

	if (currentHitActor == nullptr || currentHitActor == this)
	{
		// SHOPKEEPER
		if (overlapping_shop_)
		{
			// Shop code
			OpenShopWidget();
		}
		return;
	}

	// HARVESTING TAKES PRIORTY OVER ANY ITEM
	if (currentHitActor->IsA(AFarmingPlot::StaticClass()))
	{
		AFarmingPlot* plot = Cast<AFarmingPlot>(currentHitActor);

		if (plot && plot->OnInteract())
		{
			if (inventoryComp->AddItem(FUS_Item(GetCrop(plot->GetPlantCropType()->name_), 1)))
			{
				plot->HarvestPlant();
				PostEvent(akHarvest);
			}

			return;
		}
	}

	/*niagaraComp->SetAsset(testNS);
	niagaraComp->Activate(true);*/

	// Talk to an NPC
	if (currentHitActor->IsA(ANPC_Character_Base::StaticClass()))
	{
		ANPC_Character_Base* NPC = Cast<ANPC_Character_Base>(currentHitActor);
		NPC->Execute_OnInteract(NPC, this);
		return;
	}
	// Grab a ticket or get reward by returning it
	else if (currentHitActor->IsA(ATicketCounter::StaticClass()))
	{
		ATicketCounter* ticketCounter_ = Cast<ATicketCounter>(currentHitActor);
		ticketCounter_->Interact();
		return;
	}
	// Go through a door
	else if (currentHitActor->IsA(ADoor::StaticClass()) && !Cast<ADoor>(currentHitActor)->locked)
	{
		Grow(*potionMap.Find(FString("Growth Potion")));
		isHUDOn = true;
		doorFocus = currentHitActor;
		DoorFade();
		akAudioComponent->PostAkEventByName("PlayerPortal");
		return;
	}
	// Activate a puzzle reset button
	else if (currentHitActor->IsA(APuzzleResetButton::StaticClass()))
	{
		if (heldItem != nullptr && heldItem->IsA(APuzzleObject::StaticClass()))
		{
			Cast<APuzzleObject>(heldItem)->SetHeld(false);
			capsuleComp->IgnoreActorWhenMoving(heldItem, false);
			heldItem = nullptr;
		}
		
		puzzleVolume->ResetPuzzle();
		SetActorLocation(lastDoorPos);
		return;
	}

	// Pick up an inventory item
	if (currentHitActor->IsA(AItemPickup::StaticClass()))
	{
		AItemPickup* itemPickup = Cast<AItemPickup>(currentHitActor);

		if (itemPickup->unlockedDoor != nullptr)
		{
			itemPickup->unlockedDoor->locked = false;
			
			UUserWidget *levelUnlockHUD = CreateWidget<UUserWidget>(GetWorld(), levelUnlockHUDClass);
			levelUnlockHUD->AddToViewport();
			UTextBlock *levelUnlockText = Cast<UTextBlock>(levelUnlockHUD->GetWidgetFromName((FString("UnlockText")).GetCharArray().GetData()));

			if (levelUnlockText != nullptr)
			{
				levelUnlockText->SetText(itemPickup->unlockText);
			}

			//if the player picks up a key, update ticket list
			if (itemPickup->puzzleNumber > 0)
			{
				ticketComp->SetCurrentPuzzleNumber(itemPickup->puzzleNumber);
				shopkeeper_->UpdateShopLevel(itemPickup->puzzleNumber);
				akAudioComponent->PostAkEventByName("PlayerPuzzleComplete");
				itemPickup->GameEnd();
			}

			itemPickup->Destroy();
		}
		else if (inventoryComp->AddItem(itemPickup->item))
		{
			itemPickup->Destroy();
		}
		
		PostEvent(akCollect);

	}
	// Pick up a crop
	else if (currentHitActor->IsA(ACrop::StaticClass()))
	{
		PostEvent(akPickup);

	}
	// Add a crop to a crate
	else if (currentHitActor->IsA(ACrate::StaticClass()))
	{
		Cast<ACrate>(currentHitActor)->OpenCrate();
		
		isHUDOn = true;
		inventoryComp->SetLock(true);

		APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		UCharacterMovementComponent* c_movement = GetCharacterMovement();
		c_movement->StopMovementImmediately();
		c_movement->DisableMovement();
		controller->StopMovement();
		controller->bShowMouseCursor = true;
		controller->bEnableClickEvents = true;
		controller->bEnableMouseOverEvents = true;

		return;
	}

	if (currentHitActor->GetClass()->GetName().Equals("MovingPlatform"))
	{
		inventoryComp->UseSelectedItemOnObject();
		return;
	}

	// Pickup a puzzle object
	if (currentHitActor->IsA(APuzzleObject::StaticClass()) && !IsSmall())
	{
		APuzzleObject* puzzleObject = Cast<APuzzleObject>(currentHitActor);
		if (inventoryComp->OnPotion() && puzzleObject->IsValidPotionEffect(Cast<UDA_Potion>(inventoryComp->GetCurrentItem().itemType_)->potion_effect_))
		{
			inventoryComp->UseSelectedItemOnObject();
			return;
		}
		else if (puzzleObject->canPickup)
		{
			AnimLift(currentHitActor);
			return;
		}
		else
		{
			PostEvent(akPuzzleInvalid);
		}
	}

	/***** * * * * * * * USE HOTBAR * * * * * * *   *****/

		//if (inventoryComp->items[inventoryComp->selectedIndex].itemType_ != nullptr)
	if (!inventoryComp->OnHand())
	{
		inventoryComp->UseSelectedItemOnObject();
		return;
	}

	/***** * * * * * * * END OF HOTBAR USE * * * * * * *   *****/
}

void ACharacterBase::BackKeyTrigger()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->BackButtonPressed();
		return;
	}

	if (crate != nullptr)
	{
		crate->CloseCrate();
	}

	/*if (currentHitActor == nullptr || currentHitActor == this)
	{
		return;
	}*/
}

void ACharacterBase::MoveSlotSelectLeft()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->MoveRecipeSelectLeft();
		return;
	}

	if (crate != nullptr)
	{
		crate->MoveSlotSlectionLeft();
		return;
	}
}

void ACharacterBase::MoveSlotSelectRight()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->MoveRecipeSelectRight();
		return;
	}

	if (crate != nullptr)
	{
		crate->MoveSlotSlectionRight();
		return;
	}
}

void ACharacterBase::MoveSlotSelectUp()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->MoveRecipeSelectUp();
		return;
	}

	if (crate != nullptr)
	{
		crate->MoveSlotSlectionUp();
		return;
	}
}

void ACharacterBase::MoveSlotSelectDown()
{
	if (ticketComp->IsRecipeBookOpen())
	{
		ticketComp->MoveRecipeSelectDown();
		return;
	}

	if (crate != nullptr)
	{
		crate->MoveSlotSlectionDown();
		return;
	}
}

void ACharacterBase::LeavePuzzleRoomAudio()
{
	PostEvent(akEnvironmentSounds);
	PostEvent(akSetFarmMusic);

	// Get all the puzzle objects
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleObject::StaticClass(), FoundPlatforms);
	if (FoundPlatforms.Num())
	{

		// Check the platform in Puzzle 2
		if (puzzleVolume->puzzleNumber == 2)
		{
			for (AActor* actor : FoundPlatforms)
			{
				APuzzleObject* obj = Cast<APuzzleObject>(actor);
				if (obj->GetName().Equals("Platform4_2"))
				{
					if (obj->isLevitating)
						obj->MuteHover();
					break;
				}
			}
		}
		// Check the platform in puzzle 3
		else if (puzzleVolume->puzzleNumber == 3)
		{

			for (AActor* actor : FoundPlatforms)
			{
				APuzzleObject* obj = Cast<APuzzleObject>(actor);
				if (obj->GetName().Equals("Platform3_2"))
				{
					if (obj->isLevitating)
						obj->MuteHover();
					break;
				}
			}
		}
	}
}

void ACharacterBase::EnterPuzzleRoomAudio()
{
	PostEvent(akSetPuzzleMusic);

	// If the puzzle has wind, play wind environment sounds
	TArray<AActor*> FoundPlatforms;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleObject::StaticClass(), FoundPlatforms);
	if (FoundPlatforms.Num())
	{

		// Check the platform in Puzzle 2
		if (puzzleVolume->puzzleNumber == 2)
		{
			for (AActor* actor : FoundPlatforms)
			{
				APuzzleObject* obj = Cast<APuzzleObject>(actor);
				if (obj->GetName().Equals("Platform4_2"))
				{
					if (obj->isLevitating)
						obj->UnmuteHover();
					break;
				}
			}
			PostEvent(akWindEnvironment);
		}
		// Check the platform in puzzle 3
		else if (puzzleVolume->puzzleNumber == 3)
		{
			for (AActor* actor : FoundPlatforms)
			{
				APuzzleObject* obj = Cast<APuzzleObject>(actor);
				if (obj->GetName().Equals("Platform3_2"))
				{
					if (obj->isLevitating)
						obj->UnmuteHover();
					break;
				}
			}
		}

		PostEvent(akWindEnvironment);

	}

}

void ACharacterBase::MoveBlockedBy(FHitResult const& result)
{
	Super::MoveBlockedBy(result);

	if (result.GetActor()->IsA(APuzzleObject::StaticClass()))
	{
		APuzzleObject* puzzleObject = Cast<APuzzleObject>(result.GetActor());
		APuzzleObject *carryingObject = puzzleObject->GetCarryingObject();

		if (puzzleObject->canPush)
		{
			AnimPush(true);
			puzzleObject->SetVelocity(result.ImpactNormal * -100.f);
		}
		else if (carryingObject != nullptr && carryingObject->canPush && !OnFloatingPlatform())
		{
			AnimPush(true);
			carryingObject->SetVelocity(result.ImpactNormal * -100.f);
		}
	}
}

bool ACharacterBase::OnFloatingPlatform()
{
	return onFloatingPlatform;
}

// Update the value of onFloatingPlatform. Called every tick.
void ACharacterBase::UpdateOnFloatingPlatform()
{
	TArray<FHitResult> hitResults;

	FCollisionObjectQueryParams COQP;
	COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2);

	FCollisionShape shape = FCollisionShape::MakeCapsule(capsuleComp->GetScaledCapsuleRadius(), capsuleComp->GetScaledCapsuleHalfHeight());

	GetWorld()->SweepMultiByObjectType(hitResults, GetActorLocation(), GetActorLocation() + FVector(0, 0, -30),
		GetActorRotation().Quaternion(), COQP, shape);

	for (int i = 0; i < hitResults.Num(); ++i)
	{
		if (hitResults[i].Actor != nullptr)
		{
			APuzzleObject *puzzleObject = Cast<APuzzleObject>(hitResults[i].Actor);

			if (puzzleObject != nullptr)
			{
				if (puzzleObject->isLevitating)
				{
					onFloatingPlatform = true;
					return;
				}
			}
		}
	}

	onFloatingPlatform = false;
}

// Initialize the process of loading the item DataAssets
void ACharacterBase::InitItemLoading()
{
	// Load the Asset Registry module
	FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));

	// Get the asset registry's instance
	IAssetRegistry& assetRegistry = assetRegistryModule.Get();

	// If the asset registry is still loading assets, tell it to call OnRegistryLoaded when it's done
	if (assetRegistry.IsLoadingAssets())
	{
		assetRegistry.OnFilesLoaded().AddUObject(this, &ACharacterBase::OnRegistryLoaded);
	}
	// Otherwise, call OnRegistryLoaded now
	else
	{
		OnRegistryLoaded();
	}
}

// Load the item DataAssets
void ACharacterBase::OnRegistryLoaded()
{
	// Load the Asset Registry module
	FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));

	// Get the asset registry's instance
	IAssetRegistry& assetRegistry = assetRegistryModule.Get();

	// Construct an Asset Registry filter that looks for instances of  DA_Potion and DA_Seed
	FARFilter filter;
	filter.ClassNames = { TEXT("DA_Potion"), TEXT("DA_Seed"), TEXT("DA_Crop") };

	// Get a list of all DA_Potion and DA_Seed data assets
	TArray<FAssetData> assetList;
	assetRegistry.GetAssets(filter, assetList);

	for (const FAssetData& asset : assetList)
	{
		OnAssetAdded(asset);
	}

	assetRegistry.OnAssetAdded().AddUObject(this, &ACharacterBase::OnAssetAdded);
	assetRegistry.OnAssetRemoved().AddUObject(this, &ACharacterBase::OnAssetRemoved);
}

// Load an individual item DataAsset
void ACharacterBase::OnAssetAdded(const FAssetData& asset)
{
	UObject* obj = asset.GetAsset();

	if (obj == nullptr)
	{
		return;
	}

	if (obj->GetClass()->IsChildOf(UDA_Potion::StaticClass()))
	{
		UDA_Potion* potion = Cast<UDA_Potion>(obj);
		potionMap.Add(potion->name_, potion);
	}
	else if (obj->GetClass()->IsChildOf(UDA_Seed::StaticClass()))
	{
		UDA_Seed* seed = Cast<UDA_Seed>(obj);
		seedMap.Add(seed->name_, seed);
	}
	else if (obj->GetClass()->IsChildOf(UDA_Crop::StaticClass()))
	{
		UDA_Crop* crop = Cast<UDA_Crop>(obj);
		cropMap.Add(crop->name_, crop);
	}
}

// Remove an individual item DataAsset
void ACharacterBase::OnAssetRemoved(const FAssetData& asset)
{
	UObject* obj = asset.GetAsset();

	if (obj == nullptr)
	{
		return;
	}

	if (obj->GetClass()->IsChildOf(UDA_Potion::StaticClass()))
	{
		UDA_Potion* potion = Cast<UDA_Potion>(obj);
		potionMap.Remove(potion->name_);
	}
	else if (obj->GetClass()->IsChildOf(UDA_Seed::StaticClass()))
	{
		UDA_Seed* seed = Cast<UDA_Seed>(obj);
		seedMap.Remove(seed->name_);
	}
	else if (obj->GetClass()->IsChildOf(UDA_Crop::StaticClass()))
	{
		UDA_Crop* crop = Cast<UDA_Crop>(obj);
		cropMap.Remove(crop->name_);
	}
}

// Get the DataAsset for a potion with a specified name
UDA_Potion const* ACharacterBase::GetPotion(FString const& potionName)
{
	UDA_Potion const** p_potion = potionMap.Find(potionName);

	if (p_potion != NULL)
	{
		return *p_potion;
	}
	else
	{
		return NULL;
	}
}

// Get the DataAsset for a seed with a specified name
UDA_Seed const* ACharacterBase::GetSeed(FString const& seedName)
{
	UDA_Seed const** p_seed = seedMap.Find(seedName);

	if (p_seed != NULL)
	{
		return *p_seed;
	}
	else
	{
		return NULL;
	}
}

// Get the DataAsset for a seed with a specified name
UDA_Crop const* ACharacterBase::GetCrop(FString const& cropName)
{
	UDA_Crop const** p_crop = cropMap.Find(cropName);

	if (p_crop != NULL)
	{
		return *p_crop;
	}
	else
	{
		return NULL;
	}
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForInteractables();
	AnimPush(false);
	//if (heldItem != NULL)
	//{
	//	UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(heldItem->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	//	heldItem->SetActorLocationAndRotation(meshComp->GetComponentLocation() + meshComp->GetUpVector() * 60.f, meshComp->GetComponentRotation(), false, nullptr, ETeleportType::ResetPhysics);
	//	//mesh->SetWorldLocationAndRotation(meshComp->GetComponentLocation() + meshComp->GetUpVector() * 60.f, meshComp->GetComponentRotation());
	//}

	// Ensure Roll is 0
	// Set Player direction
	//const FRotator camera_rotation = springArmComp->GetComponentRotation();
	//const FRotator camera(camera_rotation.Pitch, camera_rotation.Yaw, 0); // pitch, yaw, roll
	//SetActorRotation(camera);

	if (GetActorLocation().Z < -1000)
	{
		meshComp->BodyInstance.SetLinearVelocity(FVector::ZeroVector, false);
		SetActorLocation(startPos);
	}

	if (timeStopTimeLeft > 0)
	{
		timeStopTimeLeft -= DeltaTime;
	}

	UpdateOnFloatingPlatform();
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	inventoryComp = Cast<UInventoryComponent>(GetComponentByClass(UInventoryComponent::StaticClass()));

	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACharacterBase::MoveRight);

	PlayerInputComponent->BindAxis("CameraPitch", this, &ACharacterBase::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &ACharacterBase::YawCamera);

	PlayerInputComponent->BindAction("Pause", EInputEvent::IE_Pressed, this, &ACharacterBase::TogglePause).bExecuteWhenPaused = true;
	PlayerInputComponent->BindAction("InteractWithObject", EInputEvent::IE_Pressed, this, &ACharacterBase::InteractWithObject).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("UseItemOnSelf", EInputEvent::IE_Pressed, inventoryComp, &UInventoryComponent::UseSelectedItemOnPlayer).bExecuteWhenPaused = false;
	//PlayerInputComponent->BindAction("UseItemOnObject", EInputEvent::IE_Pressed, inventoryComp, &UInventoryComponent::UseSelectedItemOnObject).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("MoveHotbarSelectRight", EInputEvent::IE_Pressed, inventoryComp, &UInventoryComponent::MoveSelectionRight).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("MoveHotbarSelectLeft", EInputEvent::IE_Pressed, inventoryComp, &UInventoryComponent::MoveSelectionLeft).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("ShowTicket", EInputEvent::IE_Pressed, this, &ACharacterBase::OpenRecipeBook).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("Back", EInputEvent::IE_Pressed, this, &ACharacterBase::BackKeyTrigger).bExecuteWhenPaused = false;
	//PlayerInputComponent->BindAction("MoveCrateSelection", EInputEvent::IE_Pressed, crate, &ACrate::MoveSelection).bExecuteWhenPaused = false;

	PlayerInputComponent->BindAction("MoveSlotSelectLeft", EInputEvent::IE_Pressed, this, &ACharacterBase::MoveSlotSelectLeft).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("MoveSlotSelectRight", EInputEvent::IE_Pressed, this, &ACharacterBase::MoveSlotSelectRight).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("MoveSlotSelectDown", EInputEvent::IE_Pressed, this, &ACharacterBase::MoveSlotSelectDown).bExecuteWhenPaused = false;
	PlayerInputComponent->BindAction("MoveSlotSelectUp", EInputEvent::IE_Pressed, this, &ACharacterBase::MoveSlotSelectUp).bExecuteWhenPaused = false;
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (RootComponent)
	{
		// Attach contact function to all bounding components. 
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACharacterBase::OnOverlapBegin);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ACharacterBase::OnOverlapEnd);
	}
}

//----------------------------------------------------------------------------------------------------
// Crafting Section
//----------------------------------------------------------------------------------------------------

bool ACharacterBase::AddCropToCauldron()
{
	UDA_Crop const* crop = Cast<UDA_Crop>(inventoryComp->GetItem(inventoryComp->selectedIndex).itemType_);

	if (crop == nullptr)
	{
		return false;
	}

	ACauldron* cauldron_ = Cast<ACauldron>(currentHitActor);

	if (cauldron_ && cauldron_->AddItemToCauldron(crop))
	{
		PostEvent(akSmokePuff);

		return true;
	}

	return false;
}

void ACharacterBase::OpenRecipeBook()
{
	if (canOpenRecipeBook == false)
		return;

	if (isHUDOn)
		return;

	ticketComp->OpenRecipeBook();
	inventoryComp->SetLock(true);

	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	controller->bShowMouseCursor = true;
	controller->bEnableClickEvents = true;
	controller->bEnableMouseOverEvents = true;
	PostEvent(akOpenMenu);
}

void ACharacterBase::ResetCrate()
{
	if (crate == nullptr)
		return;

	crate->ResetCrate();
}

void ACharacterBase::SetIsHUDOn(bool b)
{
	isHUDOn = b;
}

bool ACharacterBase::GetIsHUDOn()
{
	return isHUDOn;
}

void ACharacterBase::MoveItemToCrate()
{
	if (crate == nullptr)
		return;

	FUS_Item item = inventoryComp->items[inventoryComp->selectedIndex];
	if (item.itemType_ != nullptr)
	{
		// only move the item to a crate when it is either crop, seed or potion
		if (item.itemType_->GetClass()->IsChildOf(UDA_Crop::StaticClass()) ||
			item.itemType_->GetClass()->IsChildOf(UDA_Seed::StaticClass()) ||
			item.itemType_->GetClass()->IsChildOf(UDA_Potion::StaticClass()))
		{
			if (crate->AddItem(item))
			{
				inventoryComp->items[inventoryComp->selectedIndex].amount_ = 0;
				inventoryComp->items[inventoryComp->selectedIndex].itemType_ = nullptr;
				PostEvent(akAddItem);
			}
			else
			{
				PostEvent(akAddFailed);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Farming Section
//----------------------------------------------------------------------------------------------------

// plants the given seed in inventory if a plot is nearby
bool ACharacterBase::PlantSeed(UDA_Seed const* seed)
{
	if (seed == nullptr) return false;
	if (animationFocus == nullptr) return false;
	// if there is a plot to interact with
	if (animationFocus->IsA(AFarmingPlot::StaticClass()))
	{
		// plant on avaliable plot
		bool result = Cast<AFarmingPlot>(animationFocus)->SpawnPlant(seed);
		if (result) PostEvent(akPlantSeed);
		return result;
	}
	return false;
}

bool ACharacterBase::CanPlantSeed(UDA_Seed const* seed)
{
	if (seed == nullptr) return false;
	if (currentHitActor == nullptr) return false;
	// if there is a plot to interact with
	if (currentHitActor->IsA(AFarmingPlot::StaticClass()))
	{
		if (Cast<AFarmingPlot>(currentHitActor)->IsAvailable())
		{
			return true;
		}
	}
	return false;
}

void ACharacterBase::WaterPlot()
{
	inventoryComp->SetLock(false);
	// if there is a plot to interact with
	if (animationFocus == nullptr)
	{
		if (DEBUG_LOG)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No plot nearby to water.");
		return;
	}

	if (animationFocus->IsA(AFarmingPlot::StaticClass()))
	{
		if (Cast<AFarmingPlot>(animationFocus)->IsWaterable())
		{
			// water the plot
			Cast<AFarmingPlot>(animationFocus)->SetWatered(true);
			if (DEBUG_LOG)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Watered the plot.");
		}
		else if (Cast<AFarmingPlot>(animationFocus)->GetWatered())
		{
			if (DEBUG_LOG)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Already watered.");

			akAudioComponent->PostAkEventByName("ItemPuzzleObjectInvalid");
		}
		else
		{
			if (DEBUG_LOG)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No plant in plot to water.");
		}
	}
	else
	{
		if (DEBUG_LOG)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "No plot nearby to water.");
	}

	animationFocus = NULL;
}

//----------------------------------------------------------------------------------------------------
// Potion Section
//----------------------------------------------------------------------------------------------------
void ACharacterBase::BindPotionFunctions()
{
	// Shrink potion
	PotionTL_Lerp[EFX_SHRINK].BindUFunction(this, FName("Potion_Shrink"));
	PotionTL_Finished[EFX_SHRINK].BindUFunction(this, FName("ShrinkTL_OnEnd"));

	// Grow potion
	PotionTL_Lerp[EFX_GROW].BindUFunction(this, FName("Potion_Grow"));
	PotionTL_Finished[EFX_GROW].BindUFunction(this, FName("GrowTL_OnEnd"));
}

// Use potion on self code
void ACharacterBase::UsePotionOnSelf()
{
	bool success = true; // Whether the potion was successfully used
	UDA_Potion const* potion = Cast<UDA_Potion const>(inventoryComp->GetCurrentItem().itemType_);

	switch (potion->potion_effect_)
	{
		// Shrink effect
	case EFX_SHRINK:
		success = Shrink(potion); // Attempt to shrink player

		if (success)
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Shrinking player...");
			}
		}
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot shrink.");
			}
		}
		break;
		// Grow effect
	case EFX_GROW:
		success = Grow(potion); // Attempt to grow player

		if (success)
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Growing player...");
			}
		}
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot grow.");
			}
		}
		break;
		// Levitation effect
	case EFX_LEVITATE:
		success = true; // Potion was used

		/// Drinking a levitation potion does nothing

		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Nothing seems to happen...");
		}
		break;
		// Gravity effect
	case EFX_GRAVITY:
		success = true; // Potion was used

		/// Drinking a gravity potion does nothing

		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Nothing seems to happen...");
		}
		break;
		// Time stop effect
	case EFX_TIMESTOP:
		success = TimeStop(5.0f);

		if (success)
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Freezing player...");
			}
		}
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot freeze player.");
			}
		}
		break;
		// Default effect (does nothing)
	case EFX_NOTHING:
		success = false; // Potion cannot be used

		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "You can't drink that.");
		}
		break;
		// Invalid effect
	default:
		success = false; // Potion cannot be used

		if (DEBUG_LOG)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Unknown potion effect with index " + FString::FromInt(potion->potion_effect_));
		}
		break;
	}

	// If potion was successfully used, remove it from the inventory
	if (success)
	{
		inventoryComp->DecreaseStack(inventoryComp->selectedIndex); // Consume the potion
	}

	animationFocus = NULL;
	inventoryComp->SetLock(false);
}

// Use potion on other
void ACharacterBase::UsePotionOnObject()
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(inventoryComp->GetCurrentItem().itemType_); // Attempt to cast item to potion

	// If the item is a potion, trigger its effect on the object the player is looking at
	if (potion != NULL)
	{
		APuzzleObject* puzzleObject = Cast<APuzzleObject>(animationFocus); // Attempt to cast object to puzzle object

		// If the player is looking at a puzzle object, attempt to use the potion on it
		if (puzzleObject != NULL)
		{
			bool success = true; // Whether the potion was successfully used

			// Attempt to execute the potion's effect
			switch (potion->potion_effect_)
			{
				// Shrink effect
			case EFX_SHRINK:
				success = puzzleObject->Shrink(potion); // Attempt to shrink object

				if (success)
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Shrinking object...");
					}
				}
				else
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot shrink object.");
					}
				}
				break;
				// Grow effect
			case EFX_GROW:
				success = puzzleObject->Grow(potion); // Attempt to grow object

				if (success)
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Growing object...");
					}
				}
				else
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot grow object.");
					}
				}
				break;
				// Levitation effect
			case EFX_LEVITATE:
				success = puzzleObject->Levitate(); // Attempt to levitate object

				if (success)
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Levitating object...");
					}
				}
				else
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot levitate object.");
					}
				}
				break;
				// Gravity effect
			case EFX_GRAVITY:
				success = puzzleObject->Gravitate(); // Attempt to gravitate object

				if (success)
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Gravitating object...");
					}
				}
				else
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot gravitate object.");
					}
				}
				break;
				// Time stop effect
			case EFX_TIMESTOP:
				success = puzzleObject->TimeStop(); // Attempt to gravitate object

				if (success)
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Freezing object...");
					}
				}
				else
				{
					if (DEBUG_LOG)
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Cannot freeze object.");
					}
				}
				break;
				// Default effect (does nothing)
			case EFX_NOTHING:
				success = false; // Potion cannot be used

				if (DEBUG_LOG)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "You can't use that potion.");
				}
				break;
				// Invalid effect
			default:
				success = false; // Potion cannot be used

				if (DEBUG_LOG)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: Unknown potion effect with index " + FString::FromInt(potion->potion_effect_));
				}
				break;
			}

			// If potion was successfully used, remove it from the inventory
			if (success)
			{
				inventoryComp->DecreaseStack(inventoryComp->selectedIndex); // Consume the potion
			}
		}
		else if (animationFocus->GetClass()->GetName().Contains("MovingPlatform") && potion->potion_effect_ == EFX_TIMESTOP)
		{
			FreezeFloatingPlatform(animationFocus);
		}
		else
		{
			if (DEBUG_LOG)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Error: No puzzle object found.");
			}
		}
		animationFocus = NULL;
		inventoryComp->SetLock(false);
		return;
	}
	animationFocus = NULL;
	inventoryComp->SetLock(false);
}

bool ACharacterBase::Shrink(UDA_Potion const* potion)
{
	if (!timelineComp->IsPlaying() && !is_shrinked && potion->potion_curve_ != nullptr && potion->niagara_system_template_ != nullptr)
	{
		// Add curve and connect it to the interpolate function
		timelineComp->AddInterpFloat(potion->potion_curve_, PotionTL_Lerp[EFX_SHRINK], FName("ShrinkScale"));
		// Add on end function
		timelineComp->SetTimelineFinishedFunc(PotionTL_Finished[EFX_SHRINK]);
		timelineComp->SetLooping(false);
		timelineComp->PlayFromStart();
		niagaraComp->SetAsset(potion->niagara_system_template_);
		niagaraComp->Activate(true);
		PostEvent(akShrink);

		if (heldItem != nullptr && heldItem->IsA(APuzzleObject::StaticClass()))
		{
			APuzzleObject* puzzleObject = Cast<APuzzleObject>(heldItem);
			puzzleObject->SetHeld(false);
			puzzleObject->SetActorLocation(meshComp->GetComponentLocation() + meshComp->GetRightVector() * 65.f);
			capsuleComp->IgnoreActorWhenMoving(heldItem, false);
			heldItem = nullptr;
		}

		return true;
	}

	return false;
}

bool ACharacterBase::Grow(UDA_Potion const* potion)
{
	if (!timelineComp->IsPlaying() && is_shrinked && potion->potion_curve_ != nullptr && potion->niagara_system_template_ != nullptr)
	{
		// Add curve and connect it to the interpolate function
		timelineComp->AddInterpFloat(potion->potion_curve_, PotionTL_Lerp[EFX_GROW], FName("GrowthScale"));
		// Add on end function
		timelineComp->SetTimelineFinishedFunc(PotionTL_Finished[EFX_GROW]);
		timelineComp->SetLooping(false);
		timelineComp->PlayFromStart();
		niagaraComp->SetAsset(potion->niagara_system_template_);
		niagaraComp->Activate(true);
		PostEvent(akGrow);
		return true;
	}

	return false;
}

void ACharacterBase::Potion_Grow(float value)
{
	SetActorScale3D(original_scale_ * value);
}

void ACharacterBase::GrowTL_OnEnd()
{
	is_shrinked = false;
}

void ACharacterBase::Potion_Shrink(float value)
{
	SetActorScale3D(original_scale_ * value);
}


void ACharacterBase::ShrinkTL_OnEnd()
{
	is_shrinked = true;
}

bool ACharacterBase::TimeStop(float duration)
{
	if (timeStopTimeLeft <= 0)
	{
		timeStopTimeLeft = duration;
		return true;
	}

	return false;
}

bool ACharacterBase::IsSmall()
{
	return is_shrinked || (timelineComp != nullptr && timelineComp->IsPlaying());
}

// Returns whether or not the player is under the effects of a Time Stop potion
bool ACharacterBase::IsFrozen()
{
	return timeStopTimeLeft > 0;
}

// Returns whether or not the player is currently inside a puzzle area
bool ACharacterBase::IsInPuzzle()
{
	return inPuzzle;
}

void ACharacterBase::ResetPuzzle()
{
	if (heldItem != nullptr && heldItem->IsA(APuzzleObject::StaticClass()))
	{
		APuzzleObject *puzzleObject = Cast<APuzzleObject>(heldItem);
		puzzleObject->SetHeld(false);

		puzzleObject->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		puzzleObject->SetActorLocation(puzzleObject->startPos);
		puzzleObject->SetActorRotation(FRotator(0, 0, 0));
		puzzleObject->SetActorEnableCollision(true);

		capsuleComp->IgnoreActorWhenMoving(heldItem, false);
		heldItem = nullptr;
		AnimDrop();
	}

	puzzleVolume->ResetPuzzle();
}

FVector ACharacterBase::GetLastDoorPos()
{
	return lastDoorPos;
}

void ACharacterBase::SetLastDoorPos(FVector pos)
{
	lastDoorPos = pos;
}

//----------------------------------------------------------------------------------------------------
// Shop Section
//----------------------------------------------------------------------------------------------------

void  ACharacterBase::SetOverlappingShop(bool value)
{
	overlapping_shop_ = value;
}

bool  ACharacterBase::GetOverlappingShop() const
{
	return overlapping_shop_;
}

void ACharacterBase::OpenShopWidget()
{
	if (DEBUG_LOG)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Open Shop"));
	shopkeeper_->StartShop();
	// Todo: Play shop music
	PostEvent(akSetShopMusic);
}

void ACharacterBase::SetShopActor(AShopkeeper* actor)
{
	if (shopkeeper_ == nullptr)
	{
		shopkeeper_ = actor;
	}
}

void ACharacterBase::ActivateUIControls()
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UCharacterMovementComponent* c_movement = GetCharacterMovement();
	c_movement->StopMovementImmediately();
	c_movement->DisableMovement();
	controller->StopMovement();
	controller->SetInputMode(FInputModeUIOnly());
	controller->bShowMouseCursor = true;
	controller->bEnableClickEvents = true;
	controller->bEnableMouseOverEvents = true;
}
void ACharacterBase::DeactivateUIControls()
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UCharacterMovementComponent* c_movement = GetCharacterMovement();
	c_movement->SetMovementMode(MOVE_Walking);
	controller->SetInputMode(FInputModeGameOnly());
	controller->bShowMouseCursor = false;
	controller->bEnableClickEvents = false;
	controller->bEnableMouseOverEvents = false;
}

bool ACharacterBase::IsGamePadConnected()
{
	auto genericApplication = FSlateApplication::Get().GetPlatformApplication();
	return (genericApplication.Get() != nullptr && genericApplication->IsGamepadAttached());
}

// Returns true if currently holding watering can
bool ACharacterBase::OnWateringCan()
{
	if (inventoryComp == nullptr) return false;

	return inventoryComp->OnWateringCan();
}

// Returns true if currently holding a seed
bool ACharacterBase::OnSeed()
{
	if (inventoryComp == nullptr) return false;

	return inventoryComp->OnSeed();
}

// Returns true if currently holding a crop
bool ACharacterBase::OnCrop()
{
	if (inventoryComp == nullptr) return false;

	return inventoryComp->OnCrop();
}


void ACharacterBase::PostEvent(UAkAudioEvent* akEvent)
{
	if (akEvent == nullptr)
	{
		return;
	}

	// Audio-related
	FOnAkPostEventCallback nullCallback;
	akAudioComponent->PostAkEvent(akEvent, int32(0), nullCallback, akEvent->GetName());
}

// Animation events
void ACharacterBase::PlantSeedBP()
{
	const UDA_Seed* seed = Cast<UDA_Seed>(inventoryComp->GetCurrentItem().itemType_);
	
	if (PlantSeed(seed))
	{
		inventoryComp->DecreaseStack(inventoryComp->selectedIndex);
	}
	inventoryComp->SetLock(false);
	animationFocus = NULL;
}

// Get current hit actor
AActor* ACharacterBase::GetCurrentHitActor()
{
	return currentHitActor;
}

// Lift/pickup puzzle object
APuzzleObject* ACharacterBase::LiftBP(APuzzleObject* puzzleObject)
{
	heldItem = puzzleObject;
	puzzleObject->SetHeld(true);
	capsuleComp->IgnoreActorWhenMoving(heldItem, true);
	PostEvent(akPickup);
	heldItem->SetActorEnableCollision(false);
	return puzzleObject;
}

// Throw function for animation to call
void ACharacterBase::ThrowBP()
{
	APuzzleObject* puzzleObject = Cast<APuzzleObject>(heldItem);

	if (puzzleObject != nullptr)
	{
		puzzleObject->SetVelocity((GetVelocity().GetSafeNormal() * 1.f + FVector::UpVector * 0.5f).GetSafeNormal() * 1200.f);
		puzzleObject->SetHeld(false);

		capsuleComp->IgnoreActorWhenMoving(heldItem, false);
		heldItem = nullptr;
		PostEvent(akPickup);
	}
}

void ACharacterBase::DropBP()
{
	APuzzleObject* puzzleObject = Cast<APuzzleObject>(heldItem);

	if (puzzleObject != nullptr)
	{
		FHitResult hitResult;

		FVector old_pos = puzzleObject->GetActorLocation();

		puzzleObject->SetActorLocation(meshComp->GetComponentLocation() + meshComp->GetForwardVector() * 75.f, false, &hitResult);

		if (hitResult.bBlockingHit)
		{
			puzzleObject->SetActorLocation(old_pos);
			return;
		}
		puzzleObject->SetHeld(false);

		capsuleComp->IgnoreActorWhenMoving(heldItem, false);
		heldItem = nullptr;
		PostEvent(akPickup);
	}
}

// Enable/disable movement
void ACharacterBase::DisableMovementInput(bool disable)
{
	mov_inputs_disabled = disable;
}

bool ACharacterBase::IsMovementInputDisabled()
{
	return mov_inputs_disabled;
}

// Fade in door event
void ACharacterBase::DoorFadeInFinished()
{
	ADoor* door = Cast<ADoor>(doorFocus);
	SetActorLocation(door->destinationComp->GetComponentLocation(), false, nullptr, ETeleportType::TeleportPhysics);

	if (door->destinationDoor != nullptr)
	{
		FRotator newRotation = door->destinationDoor->GetActorRotation();
		newRotation.Yaw += 90.0f;
		SetActorRotation(newRotation, ETeleportType::TeleportPhysics);
		newRotation.Pitch = springArmComp->GetComponentRotation().Pitch;
		springArmComp->SetRelativeRotation(newRotation);
	}

	lastDoorPos = door->destinationComp->GetComponentLocation();
	isHUDOn = false;
	return;
}

// Fade out door event
void ACharacterBase::DoorFadeOutFinished()
{
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());
}

// Fade in reset event
void ACharacterBase::ResetFadeInFinished()
{
	if (heldItem != nullptr && heldItem->IsA(APuzzleObject::StaticClass()))
	{
		Cast<APuzzleObject>(heldItem)->SetHeld(false);
		capsuleComp->IgnoreActorWhenMoving(heldItem, false);
		heldItem = nullptr;
	}

	puzzleVolume->ResetPuzzle();
	SetActorLocation(lastDoorPos);
	return;
}

// Fade out reset event
void ACharacterBase::ResetFadeOutFinished()
{
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());
}