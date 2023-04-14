/*
 * File: TestPlant.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "TestPlant.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Image.h"
#include "CharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATestPlant::ATestPlant()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	widget_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("WidgetArm"));
	widget_arm->SetupAttachment(RootComponent);
	widget_arm->TargetArmLength = 45.0f;
	widget_arm->bDoCollisionTest = false;

	bar_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("BarArm"));
	bar_arm->SetupAttachment(RootComponent);
	bar_arm->TargetArmLength = 45.0f;
	bar_arm->bDoCollisionTest = false;

	barComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("GrowthBars"));
	barComp->SetupAttachment(bar_arm, USpringArmComponent::SocketName);
	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	widgetComp->SetupAttachment(widget_arm, USpringArmComponent::SocketName);

	particleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	particleComp->SetupAttachment(RootComponent);
	
	// create and name meshes
	Mesh_Stage_01 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Stage 01"));
	Mesh_Stage_02 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Stage 02"));
	Mesh_Stage_03 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Stage 03"));
	Mesh_Stage_04 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Stage 04"));

	// attach meshes to root
	Mesh_Stage_01->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	Mesh_Stage_02->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	Mesh_Stage_03->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	Mesh_Stage_04->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	currStage = 0;

	// all other meshes are invisible
	Mesh_Stage_02->SetVisibility(false);
	Mesh_Stage_03->SetVisibility(false);
	Mesh_Stage_04->SetVisibility(false);

	// set timer
	water_timer_ = 0.0f;
	growth_timer_ = secs_to_grow_;

	b_grew_this_frame_ = false;

	// Setup niagara comp
	niagaraComp_ = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFX"));
	niagaraComp_->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATestPlant::BeginPlay()
{
	Super::BeginPlay();
	
	// find the player
	player_ = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	widget_arm->SetWorldLocation(GetActorLocation() + FVector(0.0f, 0.0f, 148.0f));
	bar_arm->SetWorldLocation(GetActorLocation() + FVector(0.0f, 0.0f, 148.0f));

	// set up widget component and UI elements
	widgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	barComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	interactwidget = widgetComp->GetUserWidgetObject();
	widgetComp->AddLocalRotation(FRotator(0, 180, 0));
	widgetComp->SetAllPhysicsPosition(GetActorLocation() + FVector(-40.f, 210.f, -140.f));//FVector(0.0f, 210.0f, -50.0f));

	// growth and water bars
	barswidget = barComp->GetUserWidgetObject();
	barComp->AddLocalRotation(FRotator(0, 180, 0));
	barComp->SetAllPhysicsPosition(GetActorLocation() + FVector(-40.0f, 0.f, 60.0f));//FVector(0.0f, 210.0f, -50.0f));

	growthbar = Cast<UProgressBar>(barswidget->GetWidgetFromName("GrowthBar"));
	waterbar = Cast<UProgressBar>(barswidget->GetWidgetFromName("WaterBar"));

	UImage* slotImage = Cast<UImage>(interactwidget->GetWidgetFromName("ElementImage"));
	slotImage->SetBrushFromTexture(cropData->attributeTexture_);
	keyText = Cast<UImage>(interactwidget->GetWidgetFromName("KeyText"));
	interactwidget->SetVisibility(ESlateVisibility::Hidden);
	keyText->SetVisibility(ESlateVisibility::Hidden);
	//keyText->Text = FText::FromString(FString(TEXT("")));

	// set timer
	water_timer_ = 0.0f;
	growth_timer_ = secs_to_grow_;

	// Set niagara asset
	niagaraComp_->SetAsset(niagara_system_template_);
}

// Called every frame
void ATestPlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	b_grew_this_frame_ = false;

	// rotate UI prompt to face camera
	if (player_ != nullptr && player_->springArmComp != nullptr)
	{
		FRotator cameraRot = player_->springArmComp->GetComponentRotation() - player_->GetActorRotation();
		widget_arm->SetWorldRotation(FRotator(0.0f, cameraRot.Yaw + 0.0f, 0.0f));
		bar_arm->SetWorldRotation(FRotator(0.0f, cameraRot.Yaw + 0.0f, 0.0f));
	}

	if (IsHarvestable())// && player_->GetHitActor() == this)
	{
		interactwidget->SetVisibility(ESlateVisibility::Visible);
		keyText->SetVisibility(ESlateVisibility::Visible);
		barComp->SetVisibility(false, true);
	}
	float gper = 1.0f - ((float)growth_timer_ / secs_to_grow_);
	float wper = water_timer_ / secs_water_is_active_;
	growthbar->SetPercent((gper < 0.0f) ? 0.0f : gper);
	waterbar->SetPercent((wper < 0.0f)? 0.0f : wper);

	//TODO: needs to be frame rate independent
	if (water_timer_ > 0.0f)
	{
		water_timer_ -= DeltaTime;
		if (growth_timer_ > 0.0f)
			growth_timer_ -= (DeltaTime * water_modifier_);
	}
	else if (growth_timer_ > 0.0f)
	{
		growth_timer_ -= DeltaTime;
	}

	if (growth_timer_ <= (secs_to_grow_ * 0.66) && currStage == 0)
	{
		++currStage;
		GrowToNextStage();
	}
	else if (growth_timer_ <= (secs_to_grow_ * 0.33) && currStage == 1)
	{
		++currStage;
		GrowToNextStage();
	}
	else if (growth_timer_ <= 0.0f && currStage == 2)
	{
		++currStage;
		GrowToNextStage();
	}
}

// set water timer
void ATestPlant::Water()
{
	water_timer_ = secs_water_is_active_;
}

void ATestPlant::GrowToNextStage()
{
	Mesh_Stage_01->SetVisibility(false);
	Mesh_Stage_02->SetVisibility(false);
	Mesh_Stage_03->SetVisibility(false);
	Mesh_Stage_04->SetVisibility(false);

	switch (currStage)
	{
		case 0:
			Mesh_Stage_01->SetVisibility(true);
			break;
		case 1:
			Mesh_Stage_02->SetVisibility(true);
			break;
		case 2:
			Mesh_Stage_03->SetVisibility(true);
			break;
		case 3:
			Mesh_Stage_04->SetVisibility(true);
			niagaraComp_->Activate(true);
			break;
		default:
			break;
	}
}

// true - if plant can now be removed from plot
bool ATestPlant::IsHarvestable()
{
	return (currStage == (MAX_STAGES-1));
}

// set visibility of key prompt
void ATestPlant::SetKeyVisible(bool visible)
{
	if (IsHarvestable()) return;

	if (visible)
	{
		keyText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		keyText->SetVisibility(ESlateVisibility::Hidden);
	}
}

// set visibility of growth bars
void ATestPlant::SetBarVisible(bool visible)
{
	barComp->SetVisibility(visible);
}

bool ATestPlant::HarvestPlant(ACrate* const crate_)
{

	Mesh_Stage_04->SetVisibility(false);
	Mesh_Stage_04->SetCollisionProfileName("NoCollision");
	widgetComp->SetVisibility(false);
	barComp->SetVisibility(false, true);
	niagaraComp_->Deactivate();

	return true;
}

UDA_Crop* ATestPlant::GetCropData()
{
	return cropData;
}
