/*
 * File: NPC_Base.cpp
 * Course: GAM300
 * Author(s): Macie White
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#include "NPC_Base.h"

// Sets default values
ANPC_Base::ANPC_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANPC_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANPC_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void ANPC_Base::OnTrace_Implementation(AActor* Caller)
//{
//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message! but cooler"));
//}

