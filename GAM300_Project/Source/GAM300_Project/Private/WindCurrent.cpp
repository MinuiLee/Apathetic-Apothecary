/*
 * File: WindCurrent.cpp
 * Course: GAM300
 * Author(s): Alejandro Ramirez, Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "WindCurrent.h"

#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "PuzzleObject.h"
#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWindCurrent::AWindCurrent() : wind_strength_(25.0f), wind_speed_delta_(0.1f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// Potion
	niagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Particles"));
	niagaraComp->SetupAttachment(RootComponent);

	boxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	boxCollision->SetupAttachment(RootComponent);
	boxCollision->SetBoxExtent(FVector(300, 100, 100), true);
	boxCollision->SetGenerateOverlapEvents(true);
	boxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// Set overlap with interactable
	boxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void AWindCurrent::BeginPlay()
{
	if (niagaraComp)
	{
		FVector forward = FVector(1, 0, 0);
		niagaraComp->SetVectorParameter("WindVelocityMax", forward * (wind_strength_ / 10.f * (1.0f + wind_speed_delta_)));
		niagaraComp->SetVectorParameter("WindVelocityMin", forward * (wind_strength_ / 10.f * (1.0f - wind_speed_delta_)));
		FVector size(0, boxCollision->GetCollisionShape().GetBox().Y, boxCollision->GetCollisionShape().GetBox().Z);
		FVector l(boxCollision->GetCollisionShape().Box.HalfExtentX, boxCollision->GetCollisionShape().GetBox().Y, 0);
		
		//niagaraComp->SetVectorParameter("Offset", niagaraComp->GetComponentLocation() - niagaraComp->GetRelativeLocation() + -GetActorForwardVector() * boxCollision->GetCollisionShape().GetBox().X / 2);
		//niagaraComp->SetRelativeRotation(this->GetActorRotation());
		niagaraComp->SetVectorParameter("BoxSize", size);
	}


	Super::BeginPlay();
}

// Called every frame
void AWindCurrent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<AActor *> overlappingActors;

	GetOverlappingActors(overlappingActors);

	ACharacterBase *player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	for (auto it = overlappingActors.begin(); it != overlappingActors.end(); ++it)
	{
		if ((*it)->IsA(APuzzleObject::StaticClass()))
		{
			APuzzleObject *puzzleObject = Cast<APuzzleObject>(*it);
			
			FHitResult hit;
			FVector middleSide = GetActorLocation() - boxCollision->GetScaledBoxExtent().X * GetActorScale3D().X / 2 * GetActorForwardVector(); // Middle of the side of the box the wind comes from
			float effectivePlayerHalfWidth = objectOcclusionHalfWidth; // The half-width of the player for purposes of determining whether they're hidden from the wind
			FVector windToPlayerProjected = FVector::VectorPlaneProject(puzzleObject->GetActorLocation() - middleSide, GetActorForwardVector());
			middleSide += windToPlayerProjected;
			FCollisionObjectQueryParams COQP;
			COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2);

			// Right
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide + GetActorRightVector() * effectivePlayerHalfWidth,
				puzzleObject->GetActorLocation() + GetActorRightVector() * effectivePlayerHalfWidth, COQP);

			bool occluded = hit.Actor != nullptr || hit.Actor == puzzleObject;

			// Left
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide - GetActorRightVector() * effectivePlayerHalfWidth,
				puzzleObject->GetActorLocation() - GetActorRightVector() * effectivePlayerHalfWidth, COQP);

			occluded = occluded && (hit.Actor != nullptr || hit.Actor == puzzleObject);

			// Up
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide + GetActorUpVector() * effectivePlayerHalfWidth,
				puzzleObject->GetActorLocation() + GetActorUpVector() * effectivePlayerHalfWidth, COQP);

			occluded = occluded && (hit.Actor != nullptr || hit.Actor == puzzleObject);

			// Down
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide - GetActorUpVector() * effectivePlayerHalfWidth,
				puzzleObject->GetActorLocation() - GetActorUpVector() * effectivePlayerHalfWidth, COQP);

			occluded = occluded && (hit.Actor != nullptr || hit.Actor == puzzleObject);

			if (!occluded && puzzleObject->isLevitating && !player->GetIsHUDOn())
			{
				if (puzzleObject->GetObjectVelocity().ProjectOnToNormal(GetActorForwardVector()).Size() < 100.f)
				{
					puzzleObject->SetVelocity(puzzleObject->GetObjectVelocity() + GetActorForwardVector() * wind_strength_ * DeltaTime);

					if (puzzleObject->GetObjectVelocity().ProjectOnToNormal(GetActorForwardVector()).Size() > 100.f)
					{
						puzzleObject->SetVelocity(puzzleObject->GetObjectVelocity() - puzzleObject->GetObjectVelocity().ProjectOnToNormal(GetActorForwardVector()) + GetActorForwardVector() * 100);
					}
				}
			}
		}
		else if ((*it)->IsA(ACharacterBase::StaticClass()))
		{
			FHitResult hit;
			FVector middleSide = GetActorLocation() - boxCollision->GetScaledBoxExtent().X * GetActorScale3D().X / 2 * GetActorForwardVector(); // Middle of the side of the box the wind comes from
			float effectivePlayerHalfWidth = 5; // The half-width of the player for purposes of determining whether they're hidden from the wind
			FVector windToPlayerProjected = FVector::VectorPlaneProject(player->GetActorLocation() - middleSide, GetActorForwardVector());
			middleSide += windToPlayerProjected;
			FCollisionObjectQueryParams COQP;
			COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2);

			// Right
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide + GetActorRightVector() * effectivePlayerHalfWidth,
			                                        player->GetActorLocation() + GetActorRightVector() * effectivePlayerHalfWidth, COQP);

			bool occluded = hit.Actor != nullptr;

			// Left
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide - GetActorRightVector() * effectivePlayerHalfWidth,
				player->GetActorLocation() - GetActorRightVector() * effectivePlayerHalfWidth, COQP);
			
			occluded = occluded && hit.Actor != nullptr;

			// Up
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide + GetActorUpVector() * effectivePlayerHalfWidth,
				player->GetActorLocation() + GetActorUpVector() * effectivePlayerHalfWidth, COQP);
			
			occluded = occluded && hit.Actor != nullptr;

			// Down
			GetWorld()->LineTraceSingleByObjectType(hit, middleSide - GetActorUpVector() * effectivePlayerHalfWidth,
				player->GetActorLocation() - GetActorUpVector() * effectivePlayerHalfWidth, COQP);
			
			occluded = occluded && hit.Actor != nullptr;

			if (!occluded && !player->IsFrozen() && !player->OnFloatingPlatform() && !player->IsMovementInputDisabled() && !player->GetIsHUDOn())
			{
				UCharacterMovementComponent *movement = Cast<UCharacterMovementComponent>(player->GetMovementComponent());
				FVector wind_force = GetActorForwardVector() * wind_strength_ * playerWindForceMultiplier;
				//wind_force.Y += 10.0f;
				player->SetActorLocation(player->GetActorLocation() + wind_force * DeltaTime, true);
				//movement->AddForce(wind_force);
			}
		}
	}
}