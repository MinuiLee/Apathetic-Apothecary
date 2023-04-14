/*
 * File: TicketComponent.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"

#include "DA_Potion.h"
#include "DA_Crop.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "GameStats.h"
#include "TicketComponent.generated.h"

class ACharacterBase;

UENUM(BlueprintType)
enum E_NPC_KIND
{
	Fiz,
	Aoife,
	Bran
};

UENUM(BlueprintType)
enum E_NPC_LOCATION_KIND
{
	MainHub,
	LevelHub,
	Puzzle1,
	Puzzle2,
	Puzzle3
};

USTRUCT(BlueprintType)
struct FTicket : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
		FText name;
	UPROPERTY(EditDefaultsOnly)
		FText description;
	UPROPERTY(EditDefaultsOnly)
		UDA_Potion* requirement;
	UPROPERTY(EditDefaultsOnly)
		TArray<FName> attributesNames;
	UPROPERTY(EditDefaultsOnly)
		int currency;
	UPROPERTY(EditDefaultsOnly)
		int puzzleNumber;
	UPROPERTY(EditDefaultsOnly)
		TEnumAsByte<E_NPC_KIND> npc;
	UPROPERTY(EditDefaultsOnly)
		TEnumAsByte<E_NPC_LOCATION_KIND> npcLocation;
	UPROPERTY(EditDefaultsOnly)
		UTexture2D* npcImage;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAM300_PROJECT_API UTicketComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Sets default values for this component's properties
	UTicketComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override; 
	
	void SetRecipeIngredients(int recipeNumber, TArray<const UDA_Crop*> ingredients);
	void SetRecipeVisible(int recipeNumber);
	void SetTicket(FTicket* ticket, int totalNumber);
	void OpenRecipeBook();
	void ToggleCongratulationsHUD(FText text = FText::FromString(""));
	void MoveRecipeSelectLeft();
	void MoveRecipeSelectRight();
	void MoveRecipeSelectUp();
	void MoveRecipeSelectDown();
	void BackButtonPressed();
	void SelectButtonPressed();
	bool IsRecipeBookOpen();

	FTicket* GetTicket();

	UPROPERTY(EditAnywhere, Category = "Textures")
		UTexture2D* unknownRecipeTexture;
	UPROPERTY(BlueprintReadWrite, Category = "GameStatsVar")
		AGameStats* game_stats_;

	int currentPuzzleNumber = 0;
public:
	UFUNCTION(BlueprintCallable)
		bool CanReturnTicket(E_NPC_KIND npc, E_NPC_LOCATION_KIND npcLocation);
	UFUNCTION(BlueprintCallable)
		void ReturnTicket();
	UFUNCTION(BlueprintCallable)
		void SetCurrentPuzzleNumber(int puzzleNum);
	UFUNCTION(BlueprintCallable)
		bool HasTicket();
	UFUNCTION(BlueprintCallable)
		void SetKeyIndicatorVisibility(bool b); 
	UFUNCTION(BlueprintCallable)
		void ResetRecipeBook();
	UFUNCTION()
		void MoveToRecipeDetail();
	UFUNCTION()
		void CloseRecipeBook();
	UFUNCTION()
		void RecipeButtonOnClicked();
	
private:
	void UpdateRecipies();
	void UpadteHoveredEventLeft();
	void UpadteHoveredEventRight(); //update ingredient detail keyboard&mouse
	void UpdateIngredientDetail(bool reset); //update ingredient detail controller

	void StartFadeOut();

	void UpdateOutlines();
	void UpdateRecipeDetail();

private:
	static const int requirementSize = 2;
	static const int rewardsSize = 1;
	static const int recipeSize = 6;
	static const int ingredientSize = 2;
	int onClickedRecipe = -1;
	int onHoveredRecipe = 0;
	int onClickedIngredient = 0;
	int onHoveredIngredient = 0;
	bool recipeVisibility[recipeSize];
	bool isRecipeBookOpen = false;
	bool isFadeOut = false; 
	bool isOnLeft = true;
	bool isCoinParticleOn = false;

	TArray<const UDA_Crop*> recipeIngredients[recipeSize];

	ACharacterBase* player;
	UDataTable* attributesTable;
	UDataTable* craftingTable;
	FTicket* ticketData;
	FTimerHandle timer;

	//Widgets
	UUserWidget* ingredientUI[2];
	UUserWidget* recipeBookUI;
	UUserWidget* ticketHUD;

	//Recipe Book components
	UPanelWidget* recipePanel;
	UTextBlock* potionName;
	UTextBlock* potionDescription; 
	UButton* recipeButtons[recipeSize];
	UImage* recipeImages[recipeSize];
	UImage* recipeOutlines[recipeSize];
	UImage* recipeSelectedOutlines[recipeSize];
	UImage* potionImage;
	UImage* potionImageOutline;
	UImage* potionIngredients[2];
	UImage* potionIngredientOutlines[2];

	//Ticket components
	UPanelWidget* ticketPanel;
	UTextBlock* ticketName;
	UTextBlock* ticketDescription;
	UTextBlock* rewardName;
	UTextBlock* npcName;
	UImage* ticketPotionImage;
	UImage* npcImage;

	//HUDs
	UPanelWidget* congratPanel;
	UPanelWidget* keyIndicator;
};
