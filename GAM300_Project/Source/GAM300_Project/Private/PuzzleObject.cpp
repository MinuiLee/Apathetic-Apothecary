/*
 * File: PuzzleObject.cpp
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "PuzzleObject.h"
#include "NiagaraComponent.h"
#include "CharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APuzzleObject::APuzzleObject() : canPickup(false), canPush(false), canPushWhenShrunk(false), canPushWhenGrown(false), canShrink(false), canGrow(false), canLevitate(false), isLevitating(false), startedSmall(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Timeline --
	timelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//meshComp->SetupAttachment(RootComponent);
	SetRootComponent(meshComp);

	// Potion
	niagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Potion"));
	niagaraComp->SetupAttachment(meshComp);

	promptComp = CreateDefaultSubobject<USceneComponent>(TEXT("InteractionPrompt"));
	promptComp->SetupAttachment(meshComp);

	xComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionX"));
	xComp->bHiddenInGame = false;
	xComp->SetupAttachment(promptComp);

	iconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionIcon"));
	iconComp->bHiddenInGame = false;
	iconComp->SetupAttachment(promptComp);

	textIconComp = CreateDefaultSubobject<UBillboardComponent>(TEXT("InteractionText"));
	textIconComp->SetupAttachment(promptComp);

	// Audio
	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;

	is_shrinked = false;
	BindPotionFunctions();
}

// Called when the game starts or when spawned
void APuzzleObject::BeginPlay()
{
	Super::BeginPlay();

	if (canGrow)
	{
		startedSmall = true;
		is_shrinked = true;
	}

	original_scale_ = GetActorScale();
	original_angle_ = GetActorRotation();
	startPos = GetActorLocation();
	initCanPush = canPush;
	initCanPickup = canPickup;

	// Set dynamic material instance
	//dynamic_m_inst_ = meshComp->CreateDynamicMaterialInstance(0, meshComp->GetMaterial(0));
	promptComp->SetVisibility(false, true);

	player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	pickupIcon = iconComp->Sprite;

	xComp = Cast<UBillboardComponent>(GetDefaultSubobjectByName(TEXT("InteractionX")));
	iconComp = Cast<UBillboardComponent>(GetDefaultSubobjectByName(TEXT("InteractionIcon")));
	textIconComp = Cast<UBillboardComponent>(GetDefaultSubobjectByName(TEXT("InteractionText")));
}

void APuzzleObject::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void APuzzleObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation().Z < -500)
	{
		velocity = FVector::ZeroVector;
		SetActorLocation(startPos);
	}

	promptComp->SetVisibility(false, true);

	if (player->GetHitActor() == this || held)
	{
		FUS_Item const &item = player->inventoryComp->items[player->inventoryComp->selectedIndex];
		promptComp->SetVisibility(true, true);
		FRotator cameraRot = player->springArmComp->GetComponentRotation() - player->GetActorRotation();
		promptComp->SetWorldRotation(FRotator(-cameraRot.Pitch, cameraRot.Yaw + 180.f, cameraRot.Roll));

		/*
		if (held)
		{
			if (player->GetVelocity().IsNearlyZero())
			{
				SetIcon(dropIcon);
			}
			else
			{
				SetIcon(throwIcon);
			}
		}
		else if (player->inventoryComp->OnHand())
		{
			SetIcon(pickupIcon);
		}
		else if (item.itemType_->IsA(UDA_Potion::StaticClass()))
		{
			UTexture2D *tex = Cast<UDA_Potion>(item.itemType_)->texture_;
			SetIcon(tex);
		}
		*/

		if (held)
		{
			if (player->GetVelocity().IsNearlyZero())
			{
				SetIcon(dropIcon);
			}
			else
			{
				SetIcon(throwIcon);
			}
		}
		else if (!player->inventoryComp->OnHand() && item.itemType_->IsA(UDA_Potion::StaticClass()) && IsValidPotionEffect(Cast<UDA_Potion>(item.itemType_)->potion_effect_))
		{
			UTexture2D* tex = Cast<UDA_Potion>(item.itemType_)->texture_;
			SetIcon(tex);
		}
		else
		{
			SetIcon(pickupIcon);
		}

		//if ((player->inventoryComp->OnHand() && canPickup && !player->IsSmall()) || held || (!player->inventoryComp->OnHand() && item.itemType_->IsA(UDA_Potion::StaticClass()) && IsValidPotionEffect(Cast<UDA_Potion>(item.itemType_)->potion_effect_)))
		if ((canPickup && !player->IsSmall()) || held || (!player->inventoryComp->OnHand() && item.itemType_->IsA(UDA_Potion::StaticClass()) && IsValidPotionEffect(Cast<UDA_Potion>(item.itemType_)->potion_effect_)))
		{
			xComp->SetVisibility(false);
		}
	}


	FVector oldPos = GetActorLocation();

	TArray<FHitResult> hits;
	
	FCollisionObjectQueryParams COQP;
	COQP.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2);

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(30, 30, 30));

	GetWorld()->SweepMultiByObjectType(hits, GetActorLocation(), GetActorLocation() + FVector(0, 0, 1), GetActorRotation().Quaternion(), COQP, shape);

	if (!held)
	{
		if (timeStopTimeLeft > 0.f)
		{
			timeStopTimeLeft -= DeltaTime;

			if (timeStopTimeLeft <= 0.f)
			{
				canPickup = initCanPickup;
			}
		}
		else
		{
			if (!isLevitating)
			{
				velocity += FVector::DownVector * grav;
			}
			else
			{
				velocity.Z = 0.f;
				levitationTime += DeltaTime;
				FVector pos = GetActorLocation();

				if (!levitationDoneRising)
				{
					float old_z = pos.Z;
					pos.Z = levitationZStart + (sin(levitationTime * 2.f - PI / 2.f) + 1.f) / 2.f * 50.f;
					if (old_z > pos.Z)
					{
						levitationDoneRising = true;
						levitationTime = 0.f;
					}
				}
				else
				{
					pos.Z = levitationZStart + 35.f + (sin(levitationTime * 3.f + PI / 2.f) + 1.f) / 2.f * 15.f;
				}

				SetActorLocation(pos);
			}

			FHitResult hitResult;

			meshComp->IgnoreActorWhenMoving(player, true);

			if (carriedObject != nullptr)
			{
				meshComp->IgnoreActorWhenMoving(carriedObject, true);
				carriedObject->meshComp->IgnoreActorWhenMoving(this, true);
			}

			SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, 5.f));

			SetActorLocation(GetActorLocation() + FVector(velocity.X, 0.f, 0.f) * DeltaTime, true, &hitResult);

			if (hitResult.bBlockingHit)
			{
				velocity.X = 0.f;
			}

			SetActorLocation(GetActorLocation() + FVector(0.f, velocity.Y, 0.f) * DeltaTime, true, &hitResult);

			if (hitResult.bBlockingHit)
			{
				velocity.Y = 0.f;
			}

			SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, -5.f));

			SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, velocity.Z) * DeltaTime, true, &hitResult);


			if (meshComp->IsRegistered())
			{
				meshComp->IgnoreActorWhenMoving(player, false);
			}
		
		
			if (isLevitating)
			{
				ApplyDrag(drag * 9.f, velocity.X, DeltaTime);
				ApplyDrag(drag * 9.f, velocity.Y, DeltaTime);
			}
			else if (hitResult.bBlockingHit)
			{
				ApplyDrag(drag * 0.175f * abs(velocity.Z), velocity.X, DeltaTime);
				ApplyDrag(drag * 0.175f * abs(velocity.Z), velocity.Y, DeltaTime);
			}
			else
			{
				//ApplyDrag(drag, velocity.X, DeltaTime);
				//ApplyDrag(drag, velocity.Y, DeltaTime);
			}

			if (hitResult.bBlockingHit)
			{
				velocity.Z = 0;
				onGround = true;
			}
			else
			{
				onGround = false;
			}
		}

		if (carriedObject != nullptr)
		{
			meshComp->IgnoreActorWhenMoving(carriedObject, false);
			carriedObject->meshComp->IgnoreActorWhenMoving(this, false);
			carriedObject->carryingObject = nullptr;
			carriedObject = nullptr;
		}

		if (isLevitating)
		{
			for (int i = 0; i < hits.Num(); ++i)
			{
				if (hits[i].Actor != this)
				{
					APuzzleObject *puzzleObject = Cast<APuzzleObject>(hits[i].Actor);

					if (puzzleObject != nullptr)
					{
						puzzleObject->SetActorLocation(puzzleObject->GetActorLocation() + (GetActorLocation() - oldPos) / 2.0f);
						puzzleObject->SetVelocity(FVector::ZeroVector);
						carriedObject = puzzleObject;
						carriedObject->carryingObject = this;
					}
				}
			}
		}
	}
}

void APuzzleObject::SetHeld(bool is_held)
{
	held = is_held;

	if (held)
	{
		//mesh->SetMobility(EComponentMobility::Stationary);
		//mesh->SetSimulatePhysics(false);
		//mesh->BodyInstance.bLockXTranslation = false;
		//mesh->BodyInstance.bLockYTranslation = false;
	}
	else
	{
		//mesh->SetMobility(EComponentMobility::Movable);
		//mesh->SetSimulatePhysics(true);
		//mesh->BodyInstance.bLockXTranslation = true;
		//mesh->BodyInstance.bLockYTranslation = true;
		velocity = FVector::ZeroVector;
	}
}

FVector APuzzleObject::GetObjectVelocity()
{
	return velocity;
}

void APuzzleObject::SetVelocity(FVector vel)
{
	velocity = vel;
}

void APuzzleObject::ApplyDrag(float drag_amt, float &val, float dt)
{
	if (abs(val) <= drag_amt)
	{
		val = 0.f;
	}
	else if (val > 0)
	{
		val -= drag_amt * dt;
	}
	else
	{
		val += drag_amt * dt;
	}
}

void APuzzleObject::SetIcon(UTexture2D *tex)
{
	UTexture2D *oldTex = iconComp->Sprite;
	FVector oldScale = iconComp->GetComponentScale();
	float new_scale = oldScale.X * (float)oldTex->GetSizeX() / (float)tex->GetSizeX();
	iconComp->SetRelativeScale3D(FVector(new_scale, new_scale, new_scale));
	iconComp->SetSprite(tex);
}

void APuzzleObject::SetPromptVisibility(bool visible)
{
	textIconComp->SetVisibility(visible);
}

bool APuzzleObject::IsValidPotionEffect(E_POTION_EFFECT effect)
{
	switch (effect)
	{
		case EFX_SHRINK:
			return canShrink;
		case EFX_GROW:
			return canGrow;
		case EFX_LEVITATE:
			return canLevitate && !isLevitating;
		case EFX_GRAVITY:
			return isLevitating;
	}

	return false;
}

APuzzleObject *APuzzleObject::GetCarryingObject()
{
	return carryingObject;
}

void APuzzleObject::MuteHover()
{
	PostEvent(akMutePlatform);
}

void APuzzleObject::UnmuteHover()
{
	PostEvent(akUnmutePlatform);
}

//----------------------------------------------------------------------------------------------------
// Potion Section
//----------------------------------------------------------------------------------------------------
void APuzzleObject::BindPotionFunctions()
{
	// Shrink potion
	PotionTL_Lerp[EFX_SHRINK].BindUFunction(this, FName("Potion_Shrink"));
	PotionTL_Finished[EFX_SHRINK].BindUFunction(this, FName("ShrinkTL_OnEnd"));

	// Grow potion
	PotionTL_Lerp[EFX_GROW].BindUFunction(this, FName("Potion_Grow"));
	PotionTL_Finished[EFX_GROW].BindUFunction(this, FName("GrowTL_OnEnd"));
}

bool APuzzleObject::Shrink(UDA_Potion const *potion)
{
	if (canShrink && !timelineComp->IsPlaying() && !is_shrinked)
	{
		// Add curve and connect it to the interpolate function
		timelineComp->AddInterpFloat(potion->potion_curve_, PotionTL_Lerp[EFX_SHRINK], FName("ShrinkScale"));
		// Add on end function
		timelineComp->SetTimelineFinishedFunc(PotionTL_Finished[EFX_SHRINK]);
		timelineComp->SetLooping(false);
		timelineComp->PlayFromStart();
		niagaraComp->SetAsset(potion->niagara_system_template_);
		niagaraComp->Activate(true);

		canShrink = false;
		canGrow = true;
		canPickup = true;
		if (!isLevitating) canPush = canPushWhenShrunk;
		
		PostEvent(akShrink);

		return true;
	}

	return false;
}

bool APuzzleObject::Grow(UDA_Potion const *potion)
{
	if (canGrow && !timelineComp->IsPlaying() && is_shrinked)
	{
		// Add curve and connect it to the interpolate function
		timelineComp->AddInterpFloat(potion->potion_curve_, PotionTL_Lerp[EFX_GROW], FName("GrowthScale"));
		// Add on end function
		timelineComp->SetTimelineFinishedFunc(PotionTL_Finished[EFX_GROW]);
		timelineComp->SetLooping(false);
		timelineComp->PlayFromStart();
		niagaraComp->SetAsset(potion->niagara_system_template_);
		niagaraComp->Activate(true);

		canGrow = false;
		canShrink = true;
		canPickup = false;
		if (!isLevitating) canPush = canPushWhenGrown;

		meshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		PostEvent(akGrow);

		return true;
	}

	return false;
}

void APuzzleObject::Potion_Grow(float value)
{
	FVector newScale;
	
	if (startedSmall)
	{
		newScale = original_scale_ * (value + 1.0f);
	}
	else
	{
		newScale = original_scale_ * value;
	}
	
	SetActorScale3D(newScale);
}

void APuzzleObject::GrowTL_OnEnd()
{
	is_shrinked = false;
	canPickup = false;
}

void APuzzleObject::Potion_Shrink(float value)
{
	FVector newScale;

	if (startedSmall)
	{
		newScale = original_scale_ * (value + 1.0f);
	}
	else
	{
		newScale = original_scale_ * value;
	}

	SetActorScale3D(newScale);
}


void APuzzleObject::ShrinkTL_OnEnd()
{
	is_shrinked = true;
}

bool APuzzleObject::Levitate()
{
	if (canLevitate && !isLevitating && onGround)
	{
		isLevitating = true;
		levitationDoneRising = false;
		velocity.Z = 100.f;
		levitationTime = 0.f;
		levitationZStart = GetActorLocation().Z;
		canPush = true;
		canPickup = false;

		//dynamic_m_inst_->SetScalarParameterValue("Emissive_Multiplier", 1.0f);
		
		akAudioComponent->SetAttenuationScalingFactor(100.0f);
		PostEvent(akLevitate);
		PostEvent(akStartHover);
		

		return true;
	}

	return false;
}

bool APuzzleObject::Gravitate()
{
	if (isLevitating)
	{
		isLevitating = false;
		canPush = initCanPush;
		canPickup = initCanPickup;
		//dynamic_m_inst_->SetScalarParameterValue("Emissive_Multiplier", 0.0f);

		PostEvent(akGravity);
		PostEvent(akStopHover);

		return true;
	}

	return false;
}

bool APuzzleObject::TimeStop()
{
	if (canTimeStop)
	{
		timeStopTimeLeft = timeStopDuration;
		canPickup = false;
		return true;
	}

	return false;
}


void APuzzleObject::PostEvent(UAkAudioEvent* akEvent)
{
	// Audio-related
	if (akEvent != nullptr)
	{
		FOnAkPostEventCallback nullCallback;
		akAudioComponent->PostAkEvent(akEvent, int32(0), nullCallback, akEvent->GetName());
	}
}

void APuzzleObject::ChangeInputSprite(bool is_controller)
{
	if (!is_controller)
	{
		textIconComp->SetSprite(keyboardIcon);
	}
	else
	{
		textIconComp->SetSprite(controllerIcon);
	}
}
