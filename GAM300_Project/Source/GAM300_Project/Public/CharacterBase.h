/*
 * File: CharacterBase.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DA_Potion.h"
#include "DA_Seed.h"
#include "Cauldron.h"
#include "Crate.h"
#include "TicketCounter.h"
#include "Components/TimelineComponent.h"
#include "InventoryComponent.h"
#include "TicketComponent.h"
#include "Dialogue_Interface.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"
#include "PuzzleVolume.h"

#include "CharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;
class UStaticMeshComponent;
class AShopkeeper;
class UAkComponent;
class AGAM300_ProjectGameModeBase;

UENUM(BlueprintType)
enum E_FACING_DIRECTION
{
	LEFT    UMETA(DisplayName = "LEFT"),
	RIGHT   UMETA(DisplayName = "RIGHT"),
	FRONT   UMETA(DisplayName = "FRONT"),
	BACK	UMETA(DisplayName = "BACK")
};

UCLASS()
class GAM300_PROJECT_API ACharacterBase : public ACharacter, public IDialogue_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* springArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* cameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Timeline")
		UTimelineComponent* timelineComp;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventorymea")
		UInventoryComponent *inventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potions")
		UNiagaraComponent* niagaraComp;

		UCapsuleComponent* capsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ticket")
		UTicketComponent* ticketComp;

	/* Audio Properties */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wwise")
		UAkComponent* akAudioComponent;
	
	#pragma region Wwise Events

	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akEnvironmentSounds;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akSetFarmMusic;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akSetShopMusic;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akSetPuzzleMusic;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPickup;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akSmokePuff;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akCollect;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akHarvest;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akWateringCan;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPlantSeed;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akShrink;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akGrow;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akLevitate;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akCauldron;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akOpenMenu;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPauseGame;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akUnpauseGame;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akMenuBack;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPuzzleInvalid;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akCauldronFull;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPortal;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akPuzzleComplete;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akWindEnvironment;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akAddItem;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akAddFailed;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akTicketCollect;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akTicketComplete;

	#pragma endregion


	/* End of Audio Properties */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potions")
		int currency_;

	UPROPERTY(EditAnywhere)
		UClass *levelUnlockHUDClass;

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
		void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
		bool isPaused();

	UFUNCTION(BlueprintCallable)
		AActor* GetCurrentHitActor();

	UFUNCTION()
		void PostEvent(UAkAudioEvent *akEvent);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void MoveBlockedBy(FHitResult const &result) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	// Returns true if currently holding watering can
	bool OnWateringCan();

	// Returns true if currently holding a seed
	bool OnSeed();

	// Returns true if currently holding a crop
	bool OnCrop();

	bool OnFloatingPlatform();
	UFUNCTION(BlueprintImplementableEvent)
	void SetFloatingPlatformHighlight(AActor *platform, bool highlighted);
	UFUNCTION(BlueprintImplementableEvent)
	void FreezeFloatingPlatform(AActor *platform);

	// For playing Ticket-related sounds
	void PlayTicketCollectSound();
	void PlayTicketCompleteSound();

	AActor *GetHitActor();
	AActor *GetAnimationFocusActor();
	AActor *GetDoorFocusActor();

	UDA_Potion const* GetPotion(FString const& potionName);
	UDA_Seed const* GetSeed(FString const& seedName);
	UDA_Crop const* GetCrop(FString const& cropName);

	UFUNCTION(BlueprintImplementableEvent)
	void EventUsePotionSelf();
	UFUNCTION(BlueprintCallable)
	void UsePotionOnSelf();
	UFUNCTION(BlueprintImplementableEvent)
	void AnimUsePotionObject(AActor* actor);
	UFUNCTION(BlueprintCallable)
	void UsePotionOnObject();
	bool Shrink(UDA_Potion const* potion);
	bool Grow(UDA_Potion const* potion);
	bool TimeStop(float duration);

	// plants the given seed in inventory if a plot is nearby
	// returns true if successful
	bool PlantSeed(UDA_Seed const* seed);
	bool CanPlantSeed(UDA_Seed const* seed);

	// use watering can to water nearby plots
	UFUNCTION(BlueprintImplementableEvent)
	void AnimWaterPlot(AActor* plot);
	UFUNCTION(BlueprintCallable)
	void WaterPlot();

	// add crop(which player is holding now) to cauldron
	bool AddCropToCauldron();
	void OpenRecipeBook();
	UFUNCTION(BlueprintCallable)
		void ResetCrate();
	UFUNCTION(BlueprintCallable)
		void SetIsHUDOn(bool b);
	UFUNCTION(BlueprintCallable)
		bool GetIsHUDOn();
	// button binding function which moves selected slot item on the inventory to a crate
	UFUNCTION()
		void MoveItemToCrate();
	bool canOpenRecipeBook = false;
	ACrate* crate;

	bool IsSmall();
	bool IsFrozen();
	UFUNCTION(BlueprintCallable)
	bool IsInPuzzle();
	UFUNCTION(BlueprintCallable)
	void ResetPuzzle();
	UFUNCTION(BlueprintCallable)
	FVector GetLastDoorPos();
	UFUNCTION(BlueprintCallable)
	void SetLastDoorPos(FVector pos);
	
	// Shop
	void SetOverlappingShop(bool value);
	void SetShopActor(AShopkeeper* actor);
	bool GetOverlappingShop() const;
	void OpenShopWidget();

	// General
	UFUNCTION(BlueprintCallable, Category = "UI Controls")
		void ActivateUIControls();
	UFUNCTION(BlueprintCallable, Category = "UI Controls")
		void DeactivateUIControls();

	bool IsGamePadConnected();

	// Animation stuff ---
	UFUNCTION(BlueprintImplementableEvent)
		void PlantEvent(AActor* plot);
	UFUNCTION(BlueprintCallable)
		void PlantSeedBP();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* animationFocus;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* doorFocus;
	UFUNCTION(BlueprintImplementableEvent)
		void AnimLift(AActor* Item);
	UFUNCTION(BlueprintCallable)
		APuzzleObject* LiftBP(APuzzleObject* puzzleObject);
	UFUNCTION(BlueprintImplementableEvent)
		void AnimThrow();
	UFUNCTION(BlueprintImplementableEvent)
		void AnimPush(bool pushing);
	UFUNCTION(BlueprintCallable)
		void ThrowBP();
	UFUNCTION(BlueprintImplementableEvent)
		void AnimDrop();
	UFUNCTION(BlueprintCallable)
		void DropBP();
	UFUNCTION(BlueprintCallable)
		void DisableMovementInput(bool disable = true);
	UFUNCTION(BlueprintCallable)
		bool IsMovementInputDisabled();

	// FadeInOut Door and reset
	UFUNCTION(BlueprintImplementableEvent)
	void DoorFade();
	UFUNCTION(BlueprintCallable)
	void DoorFadeInFinished();
	UFUNCTION(BlueprintCallable)
	void DoorFadeOutFinished();
	UFUNCTION(BlueprintCallable)
	void ResetFadeInFinished();
	UFUNCTION(BlueprintCallable)
	void ResetFadeOutFinished();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* heldItem; // The object (ingredient or puzzle object) the player is holding

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ACauldron* cauldron;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool b_puzzleSolved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool b_canInteract;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool b_gamepad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float puzzleCameraPitchRange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float puzzleCameraYawRange = 0;

private:
	void MoveForward(float value);
	void MoveRight(float value);
	void CheckForInteractables();
	void UpdateOnFloatingPlatform();

	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);

	// Item asset management functions
	void InitItemLoading();
	void OnRegistryLoaded();
	void OnAssetAdded(const FAssetData& asset);
	void OnAssetRemoved(const FAssetData& asset);

	void InteractWithObject();
	void BackKeyTrigger();
	void MoveSlotSelectLeft();
	void MoveSlotSelectRight();
	void MoveSlotSelectUp();
	void MoveSlotSelectDown();

	// Helper for muting/unmuting platforms as needed
	void LeavePuzzleRoomAudio();
	void EnterPuzzleRoomAudio();

#pragma region	Potion Functions
	void BindPotionFunctions();
	
	UFUNCTION()
	void Potion_Grow(float value);
	UFUNCTION()
	void GrowTL_OnEnd();

	UFUNCTION()
	void Potion_Shrink(float value);
	UFUNCTION()
	void ShrinkTL_OnEnd();

	FOnTimelineFloat PotionTL_Lerp[EFX_MAX];
	FOnTimelineEvent PotionTL_Finished[EFX_MAX];

	// TEMP
	UPROPERTY(EditAnywhere, Category = "Potion")
		UDA_Potion* test_;

	FVector original_scale_;
	bool is_shrinked;
	float timeStopTimeLeft = 0;

#pragma endregion

	bool overlapping_shop_;
	AShopkeeper* shopkeeper_;

	AGAM300_ProjectGameModeBase* game_mode_;

	float movementSpeed;
	float sphereTraceRadius;
	TEnumAsByte<E_FACING_DIRECTION> currFacingDir;
	FVector2D CameraInput;
	AActor* currentHitActor;
	ACharacter* currentActor; // MACIE
	UUserWidget* pauseMenu;								// The ESC menu widget
	TMap<FString, UDA_Potion const*> potionMap;			// A map of all loaded potion type DataAssets
	TMap<FString, UDA_Seed const*> seedMap;				// A map of all loaded seed type DataAssets
	TMap<FString, UDA_Crop const*> cropMap;				// A map of all loaded crop type DataAssets
	APlayerController *playerController;				// The player controller
	
	FVector startPos;
	bool inPuzzle = false; // Whether the player is in a puzzle area
	FVector lastDoorPos; // The last position the player teleported to using a door
	FRotator puzzleCameraAngle; // The rotation of the camera in the current puzzle, if any
	APuzzleVolume *puzzleVolume; // The last puzzle volume the player touched
	bool mov_inputs_disabled = false;
	bool isHUDOn = false;
	bool onFloatingPlatform = false; // Whether the player is currently standing on a levitating puzzle object

	//TEMP!
	UPROPERTY(EditAnywhere, Category = "Particle")
	UNiagaraSystem* testNS;
};
