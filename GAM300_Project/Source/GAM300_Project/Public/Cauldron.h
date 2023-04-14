/*
 * File: Cauldron.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DA_Crop.h"
#include "DA_Potion.h"
#include "S_Item.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanel.h"
#include "Components/WidgetComponent.h"
#include "Components/TimelineComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Actor.h"
#include "AkComponent.h"
#include "Cauldron.generated.h"

class UNiagaraComponent;
class ACharacterBase;

UENUM(BlueprintType)
enum E_CAULDRON_UI_KIND
{
	CROP    UMETA(DisplayName = "CROP"),
	POTION   UMETA(DisplayName = "POTION"),
	NONE   UMETA(DisplayName = "NONE")
};

USTRUCT(BlueprintType)
struct FCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
		UDA_Potion* potion;
	UPROPERTY(EditDefaultsOnly)
		unsigned ignisElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned aquaElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned terraElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned spiritusElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned ventusElement;
};

USTRUCT(BlueprintType)
struct FAttribute : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
		unsigned ignisElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned aquaElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned terraElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned spiritusElement;
	UPROPERTY(EditDefaultsOnly)
		unsigned ventusElement;
	UPROPERTY(EditDefaultsOnly)
		UTexture2D* texture;
};

UCLASS()
class GAM300_PROJECT_API ACauldron : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACauldron();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UWidgetComponent* widgetComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UParticleSystemComponent* particleComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timeline")
		UTimelineComponent* timelineComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Niagara")
		UNiagaraComponent* niagaraComp;

	UPROPERTY(EditAnywhere, Category = "Niagara")
		UNiagaraSystem* niagaraSystem;

	UPROPERTY(EditAnywhere, Category = "Niagara")
		UNiagaraSystem* niagaraSystemCraft;

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* emptySlotTexture;

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* highlightedSlotTexture;

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* emptyPanelTexture;

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* highlightedPanelTexture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wwise")
		UAkComponent* akAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Potion")
		TSubclassOf<AActor> potionActorBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Potion")
		UCurveFloat* potionCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Crafting")
		float bubblingTime;

	UPROPERTY(EditAnywhere, Category = "Crafting")
		int defaultPotionNumber;

	UPROPERTY(EditAnywhere, Category = "Crafting")
		int maxCraftNumber;

	UPROPERTY(EditAnywhere, Category = "Crafting")
		UDA_Potion* failedPotion;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString craftedPotionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool isPotionCrafted = false;
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	bool AddItemToCauldron(UDA_Crop const* crop);
	bool CanInteract();
	bool CanAddCrop(UDA_Crop const* crop);
	int Interact(); // 0 = invalid, 1 = start potion, 2 = collect

private:
	void EmptyCauldron();
	void UpdateUI();
	void SetCurrentUIKind(E_CAULDRON_UI_KIND kind);
	void TurnOffBubbling();
	int MakePotion();

	UTexture2D* GetItemTexture(UDA_Crop const* crop);
	FString GetItemName(UDA_Crop const* crop);

	UFUNCTION()
		void PotionPopup(float value);
	UFUNCTION()
		void PotionPopupEnd();
	
//Variables
	static const int sizeLimit = 2;
	ACharacterBase* player;
	E_CAULDRON_UI_KIND UIKind;
	FTimerHandle timer;
	
	UDataTable* craftingTable;
	UDataTable* attributesTable;

	TArray<UDA_Crop const*> crops;
	TArray<int> cropNumbers;
	UDA_Potion* potion;
	int potionNumber;

	//Potion
	AActor* potionActor;
	FOnTimelineFloat potionLerp;
	FOnTimelineEvent potionFinished;
	
	//UIs
	UUserWidget* cauldronUI;
	UTextBlock* cropNames[sizeLimit];
	UTextBlock* cropCounts[sizeLimit];
	UTextBlock* potionName;
	UTextBlock* potionCount;
	UImage* backgroundImages[sizeLimit];
	UImage* cropImages[sizeLimit];
	UImage* cropHighlightImages[sizeLimit];
	UImage* potionImage;
	UHorizontalBox* horizontalBoxes[sizeLimit];
	UCanvasPanel* key;
	UTextBlock* newPotion;
};

