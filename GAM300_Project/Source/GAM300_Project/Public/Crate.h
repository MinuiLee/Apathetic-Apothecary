/*
 * File: Crate.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "DA_Crop.h"
#include "S_Item.h"
#include "Components/WidgetComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/Actor.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"

#include "Crate.generated.h"

class ACharacterBase;
UCLASS(BlueprintType)
class GAM300_PROJECT_API ACrate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrate();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
		USceneComponent* promptComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prompt")
		UBillboardComponent* promptIconComp;

	static const int storageSize = 9;
	
	UPROPERTY(EditAnywhere, Category = "Items")
		FUS_Item items[storageSize];

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* emptySlotTexture;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* controllerIcon;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* keyboardIcon;

	#pragma region Wwise sfx
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akOpenCrate;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akCloseCrate;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akAddItem;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akAddFailed;
	#pragma endregion

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void OpenCrate();
	void ResetCrate();
	void MoveSlotSlectionLeft();
	void MoveSlotSlectionRight();
	void MoveSlotSlectionUp();
	void MoveSlotSlectionDown();
	void SelectButtonPressed();
	bool AddItem(FUS_Item const & item);

	UFUNCTION()
		void CloseCrate();

	UFUNCTION()
		void AddToInventory();

	UFUNCTION(BlueprintCallable)
		void ChangeInputSprite(bool is_controller);

	UFUNCTION(BlueprintCallable)
		FUS_Item& GetItem();

	UFUNCTION(BlueprintCallable)
		void SellCrops();

	UFUNCTION(BlueprintCallable)
		int SellValue();

	int controllerPosition = 0; //0:storage 1:inventory 
private:
	void UpdateUI();
	void UpdateOutlines();

	UTexture2D* GetItemTexture(FUS_Item const& item);
	FString GetItemName(FUS_Item const& item);

private:
	int selectedSlot = 0;
	bool isCrateOpen = false;

	ACharacterBase* player;
	UUserWidget* storageUI;
	UImage* slotImages[storageSize];
	UImage* slotOutlines[storageSize];
	UTextBlock* slotNames[storageSize];
	UTextBlock* slotAmounts[storageSize];
	UButton* slotButtons[storageSize];

	// Wwise calls audio-playing functions with this
  UAkComponent* akAudioComponent;
	void PostEvent(UAkAudioEvent* akEvent);

	
};
