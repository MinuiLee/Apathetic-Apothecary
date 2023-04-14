/*
 * File: Shopkeeper.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Shopkeeper.generated.h"

class UBoxComponent;
class UUserWidget;
class UDA_Potion;
class UDA_Seed;
class UDA_Crop;
class UTextRenderComponent;
class ACharacterBase;
class UScrollBox;

USTRUCT(BlueprintType)
struct FShopItemBuy : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
		UDA_Seed* seed_;
	UPROPERTY(EditDefaultsOnly)
		UDA_Crop* crop_;
	UPROPERTY(EditDefaultsOnly)
		UDA_Potion* potion_;
	UPROPERTY(EditDefaultsOnly)
		uint64 buy_value_;
};

//USTRUCT(BlueprintType)
//struct FShopItemBuy : public FTableRowBase
//{
//	GENERATED_BODY()
//public:
//	UPROPERTY(EditDefaultsOnly)
//		UDA_Seed* seed_;
//	UPROPERTY(EditDefaultsOnly)
//		UDA_Potion* potion_;
//	UPROPERTY(EditDefaultsOnly)
//		unsigned sell_value_;
//	UPROPERTY(EditDefaultsOnly)
//		unsigned buy_value_;
//};

UCLASS()
class GAM300_PROJECT_API AShopkeeper : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShopkeeper();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact Box")
		UBoxComponent* boxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UBillboardComponent* iconComp; // Interaction icon

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool is_open = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// declare overlap begin function
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void StartShop();

	UFUNCTION(BlueprintCallable)
		void ActivateInteractionSignifier(bool activate);

	UFUNCTION()
		bool IsOpen();

	UFUNCTION(BlueprintImplementableEvent)
		void UpdateShopCrates();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateShopLevel(int level);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UUserWidget* shop_widget_;
private:

	UFUNCTION(BlueprintCallable)
		void CloseShop();
	
	void UpdateHUD();

	//UScrollBox* crates_scroll_box;
	ACharacterBase* player;

	//TArray<ACrate*> crates;
};
