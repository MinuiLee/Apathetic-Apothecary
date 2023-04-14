/*
 * File: Crop.cpp
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "Crop.h"
#include "Cauldron.h"
#include "Crate.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ACrop::ACrop()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	meshComp->SetupAttachment(RootComponent);
	meshComp->SetCollisionProfileName(TEXT("Interactable"));

	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	widgetComp->SetupAttachment(RootComponent);
}

void ACrop::BeginPlay()
{
	Super::BeginPlay();

	widgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	cropUI = widgetComp->GetUserWidgetObject();
	widgetComp->AddLocalRotation(FRotator(0, 180, 0));
	widgetComp->SetAllPhysicsPosition(GetActorLocation() + FVector(- 40.f, 210.f, - 150.f));

	UImage* slotImage = Cast<UImage>(cropUI->GetWidgetFromName("ElementImage"));
	slotImage->SetBrushFromTexture(cropData->attributeTexture_);
	keyText = Cast<UTextBlock>(cropUI->GetWidgetFromName("KeyText"));

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	isHeld = false;
}

void ACrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(isHeld)
		widgetComp->SetAllPhysicsPosition(GetActorLocation() + FVector(- 40.f, 210.f, - 150.f));

	if (player->GetHitActor() == this)
	{
		cropUI->SetVisibility(ESlateVisibility::Visible);
		keyText->SetVisibility(ESlateVisibility::Visible);
	}
	else if(isHeld == false)
	{
		cropUI->SetVisibility(ESlateVisibility::Hidden);
		keyText->SetVisibility(ESlateVisibility::Hidden);
	}
}

bool ACrop::SendDataToCrate(ACrate* const crate_)
{
	/*if(crate_->AddCrop(cropData))
	{
		meshComp->SetVisibility(false);
		meshComp->SetCollisionProfileName("NoCollision");
		widgetComp->SetVisibility(false);
		
		return true;
	}
	else*/
	{
		return false;
	}
}

void ACrop::SetCollisiioProfileName(std::string preset, bool isHeld_)
{
	isHeld = isHeld_;

	keyText->SetVisibility(ESlateVisibility::Hidden);

	if (isHeld_)
	{
		cropUI->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		cropUI->SetVisibility(ESlateVisibility::Hidden);
	}

	meshComp->SetCollisionProfileName(FName(preset.c_str()));
}

UDA_Crop* ACrop::GetCropData()
{
	return cropData;
}
