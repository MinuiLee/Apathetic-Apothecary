/*
 * File: PuzzleResetButton.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "PuzzleResetButton.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APuzzleResetButton::APuzzleResetButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APuzzleResetButton::BeginPlay()
{
	Super::BeginPlay();
	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	textComp = Cast<UWidgetComponent>(GetComponentByClass(UWidgetComponent::StaticClass()));
}

// Called every frame
void APuzzleResetButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	textComp->SetVisibility(player->GetHitActor() == this);
	FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
	textComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));
}

