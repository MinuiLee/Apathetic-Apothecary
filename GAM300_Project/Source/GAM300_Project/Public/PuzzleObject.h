/*
 * File: PuzzleObject.h
 * Course: GAM300
 * Author(s): Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "DA_Potion.h"
#include "Engine/StaticMeshActor.h"
#include "Components/TextRenderComponent.h"
#include "Components/BillboardComponent.h"
#include "AkComponent.h"
#include "AkAudioEvent.h"

#include "PuzzleObject.generated.h"

class UNiagaraComponent;
class ACharacterBase;
class UAkComponent;

UCLASS()
class GAM300_PROJECT_API APuzzleObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuzzleObject();

	UPROPERTY(EditAnywhere)
	bool canPickup; // Whether the object can be picked up

	UPROPERTY(EditAnywhere)
	bool canPush; // Whether the object can be pushed in its current state

	UPROPERTY(EditAnywhere)
	bool canPushWhenShrunk; // Whether the object can be pushed in its shrunk state

	UPROPERTY(EditAnywhere)
	bool canPushWhenGrown; // Whether the object can be pushed in its grown state

	UPROPERTY(EditAnywhere)
	bool canShrink; // Whether the object can be shrunk with a Shrink Potion

	UPROPERTY(EditAnywhere)
	bool canGrow; // Whether the object can be grown with a Growth Potion

	UPROPERTY(EditAnywhere)
	bool canLevitate; // Whether the object can be levitated with a Levitation Potion

	UPROPERTY(EditAnywhere)
	bool canTimeStop; // Whether the object can be frozen with a Time Stop Potion

	UPROPERTY(EditAnywhere)
	float timeStopDuration = 5.0f; // How long a Time Stop potion freezes the object for (if applicable)
	
	UPROPERTY(EditAnywhere)
	bool isLevitating; // Whether the object is currently levitating
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timeline")
		UTimelineComponent *timelineComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potions")
		UNiagaraComponent *niagaraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		USceneComponent *promptComp; // Interaction prompt

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Mesh")
		UStaticMeshComponent *meshComp; // Mesh

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UBillboardComponent *textIconComp; // Interaction text

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UBillboardComponent *xComp; // Interaction x

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Interaction")
		UBillboardComponent *iconComp; // Interaction icon
	
	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D *throwIcon;
	
	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D *dropIcon;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* controllerIcon;

	UPROPERTY(EditAnywhere, Category = "Interaction")
		UTexture2D* keyboardIcon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wwise")
		UAkComponent* akAudioComponent;

#pragma region Wwise sfx
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akLevitate;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akGravity;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akStartHover;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akStopHover;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akShrink;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akGrow;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akMutePlatform;
	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent* akUnmutePlatform;
#pragma endregion




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool Shrink(UDA_Potion const *potion);
	bool Grow(UDA_Potion const *potion);
	bool Levitate();
	bool Gravitate();
	bool TimeStop();
	void SetHeld(bool held);
	FVector GetObjectVelocity();
	void SetVelocity(FVector vel);
	void SetPromptVisibility(bool visible);
	bool IsValidPotionEffect(E_POTION_EFFECT effect);
	APuzzleObject *GetCarryingObject();
	// Audio calls (called from CharacterBase when entering/leaving puzzle volumes
	void MuteHover();
	void UnmuteHover();

	FVector startPos;
	bool startedSmall;
	bool initCanPush;
	bool initCanPickup;
	FVector original_scale_;
	FRotator original_angle_;
	bool is_shrinked;

private:
	ACharacterBase *player;
	UTexture2D *pickupIcon;
	
	FVector velocity;
	FVector wind_velocity_;
	float grav = 50.0f;
	bool onGround = false;
	float drag = 10.f;
	bool held = false;
	float levitationTime = 0.f;
	float levitationZStart;
	bool levitationDoneRising = false;
	float timeStopTimeLeft = 0.f;
	void ApplyDrag(float drag_amt, float &val, float dt);
	void SetIcon(UTexture2D *tex);
	APuzzleObject *carriedObject = nullptr;  // The puzzle object on top of this object, if any
	APuzzleObject *carryingObject = nullptr; // The puzzle object this object is on top of, if any

	// Wwise Event ID
	int32 levitation_id;

	// Materials
	UMaterialInstanceDynamic* dynamic_m_inst_;

	// Helper for playing audio
	void PostEvent(UAkAudioEvent* akEvent);

	UFUNCTION(BlueprintCallable)
	void ChangeInputSprite(bool is_controller);

#pragma region	Potion Functions
	void BindPotionFunctions();

	UFUNCTION()
		void Potion_Grow(float value);
	UFUNCTION()
		void GrowTL_OnEnd();

	UFUNCTION()
		void Potion_Shrink(float value);
	UFUNCTION()
		void ShrinkTL_OnEnd();

	FOnTimelineFloat PotionTL_Lerp[EFX_MAX];
	FOnTimelineEvent PotionTL_Finished[EFX_MAX];

	
	
#pragma endregion
};
