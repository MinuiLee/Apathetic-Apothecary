/*
 * File: Shopkeeper.cpp
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "Shopkeeper.h"

#include "Components/BoxComponent.h"
#include "CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BillboardComponent.h"
#include "CharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ScrollBox.h"
#include "GAM300_Project/GAM300_ProjectGameModeBase.h"

// Sets default values
AShopkeeper::AShopkeeper() : shop_widget_(nullptr)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ShopTrigger"));
	boxCollision->SetupAttachment(RootComponent);
	boxCollision->SetBoxExtent(FVector(300, 100, 100), true);
	boxCollision->SetGenerateOverlapEvents(true);
	boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// Set overlap with interactible
	boxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	boxCollision->OnComponentBeginOverlap.AddDynamic(this, &AShopkeeper::OnOverlapBegin);
	boxCollision->OnComponentEndOverlap.AddDynamic(this, &AShopkeeper::OnOverlapEnd);

	iconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	iconComp->bHiddenInGame = false;
	iconComp->SetVisibility(false);
	iconComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AShopkeeper::BeginPlay()
{
	Super::BeginPlay();

	FStringClassReference shopHUDClassRef(TEXT("/Game/UI/ShopUI_BluePrints/ShopUI.ShopUI_C"));
	UClass* shopClass = shopHUDClassRef.TryLoadClass<UUserWidget>();
	shop_widget_ = CreateWidget<UUserWidget>(GetWorld(), shopClass);
	shop_widget_->AddToViewport();
	shop_widget_->SetVisibility(ESlateVisibility::Collapsed);

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	//TArray<AActor*> actors;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrate::StaticClass(), actors);
	//for (size_t i = 0; i < actors.Num(); i++)
	//{
	//	crates.Add(Cast<ACrate>(actors[i]));
	//}
	
	//crates_scroll_box = Cast<UScrollBox>(Cast<UUserWidget>(shop_widget_->GetWidgetFromName("ShopCreateUI"))->GetWidgetFromName("Crates"));
}

// Called every frame
void AShopkeeper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	iconComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
	//UpdateHUD();
}

// Called to bind functionality to input
void AShopkeeper::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// declare overlap begin function
UFUNCTION()
void AShopkeeper::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, 
																 class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
																 const FHitResult& SweepResult)
{
	// Check if its the player we are colliding with
	if (OtherActor->IsA(ACharacterBase::StaticClass()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("SHOP IN!"));
		
		ACharacterBase* character = Cast<ACharacterBase>(OtherActor);

		character->SetOverlappingShop(true);
		character->SetShopActor(this);
	}
}

UFUNCTION()
void AShopkeeper::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if its the player we are colliding with
	if (OtherActor->IsA(ACharacterBase::StaticClass()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("SHOP OUT!"));

		// Remove interact
		ACharacterBase* character = Cast<ACharacterBase>(OtherActor);

		character->SetOverlappingShop(false);
		iconComp->SetVisibility(false);
	}
}


void AShopkeeper::StartShop()
{
	ACharacterBase* character = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	AGAM300_ProjectGameModeBase* game_mode = Cast<AGAM300_ProjectGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller)
	{
		controller->SetInputMode(FInputModeGameAndUI());
		controller->bShowMouseCursor = true;
	}
	if (character)
	{
		//character->ActivateUIControls();
		character->inventoryComp->RemoveInventoryHUD();
		character->ticketComp->SetKeyIndicatorVisibility(false);
		character->DisableMovementInput(true);
		character->DisableInput(controller);
	}
	if (game_mode)
	{
		game_mode->HideUI();
	}
	if (shop_widget_)
	{
		is_open = true;
		UpdateShopCrates();
		shop_widget_->SetVisibility(ESlateVisibility::Visible);
		UScrollBox* scroll = Cast<UScrollBox>(shop_widget_->GetWidgetFromName("Item_ScrollBox"));
		UUserWidget* child = Cast<UUserWidget>(scroll->GetChildAt(0));

		if (child)
		{
			Cast<UUserWidget>(child->GetWidgetFromName("FocusableButton_WithText"))->GetWidgetFromName("ButtonBase")->SetFocus();
		}
		else
		{
			Cast<UUserWidget>(shop_widget_->GetWidgetFromName("Buy_Button"))->GetWidgetFromName("ButtonBase")->SetFocus();
		}
	}
}

void AShopkeeper::UpdateHUD()
{
	//
}

void AShopkeeper::ActivateInteractionSignifier(bool activate)
{
	iconComp->SetVisibility(activate);
}

bool AShopkeeper::IsOpen()
{
	return is_open;
}

// Not called currently
void AShopkeeper::CloseShop()
{
	ACharacterBase* character = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (character)
	{
		character->DeactivateUIControls();
		character->inventoryComp->AddInventoryHUD();
		character->ticketComp->SetKeyIndicatorVisibility(true);
	}

	if (shop_widget_)
	{
		is_open = false;
		shop_widget_->SetVisibility(ESlateVisibility::Hidden);
	}
}
