/*
 * File: Door.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "Door.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	promptComp = CreateDefaultSubobject<USceneComponent>(TEXT("Prompt"));
	promptComp->SetupAttachment(RootComponent);

	promptIconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	promptIconComp->SetupAttachment(promptComp);
	
	xIconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("XIcon"));
	xIconComp->SetupAttachment(promptComp);

	vineMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	vineMeshComp->SetupAttachment(RootComponent);

	destinationComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("DestinationIcon"));
	destinationComp->bHiddenInGame = true;
	destinationComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hide prompt when not being looked at
	promptIconComp->SetVisibility(player->GetHitActor() == this);
	xIconComp->SetVisibility(player->GetHitActor() == this && locked);
	vineMeshComp->SetVisibility(locked);

	// Turn prompt to face camera
	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	promptComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
}

void ADoor::ChangeInputSprite(bool is_controller)
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
