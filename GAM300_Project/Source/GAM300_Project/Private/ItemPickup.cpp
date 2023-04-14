/*
 * File: ItemPickup.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "ItemPickup.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AItemPickup::AItemPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create item icon
	iconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	iconComp->bHiddenInGame = false;
	iconComp->SetupAttachment(RootComponent);

	// Create pickup prompt
	promptIconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	promptIconComp->SetupAttachment(iconComp);
}

// Called when the game starts or when spawned
void AItemPickup::BeginPlay()
{
	Super::BeginPlay();
	startPos = GetActorLocation();
	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

// Called every frame
void AItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hide prompt when not being looked at
	promptIconComp->SetVisibility(player->GetHitActor() == this);

	// Rotate prompt to face player
	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	promptIconComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));

	// Gently bob up and down
	SetActorLocation(startPos + FVector(0, 0, sinf(GetGameTimeSinceCreation()) * 20.0f));
}

void AItemPickup::ChangeInputSprite(bool is_controller)
{
	if (is_controller)
	{
		promptIconComp->SetSprite(controllerIcon);
	}
	else
	{
		promptIconComp->SetSprite(keyboardIcon);
	}
}

