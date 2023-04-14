/*
 * File: FarmingPlot.cpp
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "FarmingPlot.h"
#include "TestPlant.h"
#include "Components/Image.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/SpringArmComponent.h"

#define SCALE 25.0f
#define DEBUG_LOG 0

// Sets default values
AFarmingPlot::AFarmingPlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// create and name meshes
	mesh_plot_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Wooden Plot"));
	mesh_dry_dirt_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Dry Dirt"));
	mesh_wet_dirt_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Watered Dirt"));
	mesh_interact_signifier_ = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Interact Signifier"));

	// attach meshes to root
	mesh_plot_->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	mesh_dry_dirt_->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	mesh_wet_dirt_->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	mesh_interact_signifier_->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	// scale meshes
	mesh_plot_->SetRelativeScale3D(FVector(SCALE, SCALE, SCALE));
	mesh_dry_dirt_->SetRelativeScale3D(FVector(SCALE, SCALE, SCALE));
	mesh_wet_dirt_->SetRelativeScale3D(FVector(SCALE, SCALE, SCALE));
	mesh_interact_signifier_->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
	mesh_interact_signifier_->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 160.0f), FQuat::MakeFromEuler(FVector(0.0f, 180.0f, 0.0f)));

	widget_arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("WidgetArm"));
	widget_arm->SetupAttachment(RootComponent);
	widget_arm->TargetArmLength = 45.0f;
	widget_arm->bDoCollisionTest = false;

	// Widget Component
	widgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	widgetComp->SetupAttachment(widget_arm, USpringArmComponent::SocketName);
	widgetComp->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));

	// all other meshes are invisible
	mesh_wet_dirt_->SetVisibility(false);
	mesh_interact_signifier_->SetVisibility(false);

	b_watered_ = false;
	b_occupied_ = false;
	b_in_range_of_player_ = false;
	b_is_earth_ = false;
	plant_ = nullptr;

	// Spawn Parameters
	FActorSpawnParameters SpawnParams;

	// Set up Audio component
	akAudioComponent = CreateDefaultSubobject<UAkComponent>(TEXT("CPPCreatedAKComponent"));
	akAudioComponent->SetupAttachment(RootComponent);
	akAudioComponent->OcclusionRefreshInterval = 0.0f;

	// Setup niagara comp
	niagaraComp_ = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFX"));
	niagaraComp_->SetupAttachment(RootComponent);
}


// Called when the game starts or when spawned
void AFarmingPlot::BeginPlay()
{
	Super::BeginPlay();
	
	// find the player
	player_ = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	widgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	wateringIcon = widgetComp->GetUserWidgetObject();
	widget_arm->SetWorldLocation(GetActorLocation() + FVector(0.0f, 0.0f, 148.0f));

	slot_image_ = Cast<UImage>(wateringIcon->GetWidgetFromName("ElementImage"));
	slot_image_->SetBrushFromTexture(watercan_tex);
	keyText = Cast<UImage>(wateringIcon->GetWidgetFromName("KeyText"));

	wateringIcon->SetVisibility(ESlateVisibility::Hidden);
	keyText->SetVisibility(ESlateVisibility::Hidden);
	
	niagaraComp_->SetAsset(niagara_system_template_);

	b_first_plant_ = false;
	b_first_water_ = false;
	b_first_harvest_ = false;
}

void AFarmingPlot::BeginDestroy()
{
	Super::BeginDestroy();
}

void AFarmingPlot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (wateringIcon)
	{
		wateringIcon->SetVisibility(ESlateVisibility::Hidden);
		b_occupied_ = false;
	}

	if (plant_)
	{
		plant_->Destroy();
		plant_ = nullptr;
	}
}

// Called every frame
void AFarmingPlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	b_watered_this_frame_ = false;
	b_harvested_this_frame_ = false;

	// rotate UI prompt to face camera
	FRotator cameraRot = player_->springArmComp->GetComponentRotation() - player_->GetActorRotation();
	widget_arm->SetWorldRotation(FRotator(0.0f, cameraRot.Yaw + 0.0f, 0.0f));

	// show planting icon?
	if (player_ && player_->OnSeed() && plant_ == nullptr && b_in_range_of_player_)
	{
		slot_image_->SetBrushFromTexture(planting_tex);
		wateringIcon->SetVisibility(ESlateVisibility::Visible);
		keyText->SetVisibility(ESlateVisibility::Visible);
	}
	// show watering icon?
	else if (player_ && player_->OnWateringCan() && !b_watered_ && plant_ != nullptr && b_in_range_of_player_)
	{
		slot_image_->SetBrushFromTexture(watercan_tex);
		wateringIcon->SetVisibility(ESlateVisibility::Visible);
		keyText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		wateringIcon->SetVisibility(ESlateVisibility::Hidden);
		keyText->SetVisibility(ESlateVisibility::Hidden);
	}

	// show bars?
	if (plant_ != nullptr)
	{
		if (b_in_range_of_player_)
		{
			plant_->SetBarVisible(true);
		}
		else
		{
			plant_->SetBarVisible(false);
		}
	}

	// if plant is harvestable, don't show anyway
	if (plant_ && plant_->IsHarvestable())
	{
		wateringIcon->SetVisibility(ESlateVisibility::Hidden);
		keyText->SetVisibility(ESlateVisibility::Hidden);
		plant_->SetBarVisible(false);
		b_harvestable_ = true;
	}
	else
	{
		b_harvestable_ = false;
	}

	// key prompt on plant
	if (plant_ != nullptr)
	{
		plant_->SetKeyVisible(true);//b_in_range_of_player_);
	}

	// red cone on plot
	if (b_in_range_of_player_)
	{
		// show that this is the plot the player can interact with
		mesh_interact_signifier_->SetVisibility(true);
		b_in_range_of_player_ = false;
	}
	else
	{
		mesh_interact_signifier_->SetVisibility(false);
	}

	if (plant_ && plant_->water_timer_ < 0.0f)
	{
		SetWatered(false);
	}
}

// Set whether the plot is watered
void AFarmingPlot::SetWatered(bool watered)
{
	b_watered_ = watered;

	if (b_watered_)
	{
		mesh_dry_dirt_->SetVisibility(false);
		mesh_wet_dirt_->SetVisibility(true);
		b_watered_this_frame_ = true;
		// TODO: watering sound here
		if (plant_)
		{
			plant_->Water();
		}
		b_first_water_ = true;
	}
	else
	{
		mesh_dry_dirt_->SetVisibility(true);
		mesh_wet_dirt_->SetVisibility(false);
	}
}


// spawn the plant that will be in this farm plot
bool AFarmingPlot::SpawnPlant(UDA_Seed const* seed)
{
	// if the plot is empty
	if (!b_occupied_)
	{
		// spawn the plant
		FActorSpawnParameters SpawnParams;
		SpawnParams.OverrideLevel = this->GetLevel();
		FTransform trans_plant;
		trans_plant.SetTranslation(GetTransform().GetTranslation() + FVector(0.0f, 0.0f, 18.0f));

		switch (seed->element_)
		{
		case EnumElementType::State::EE_IGNIS:
			plant_ = GetWorld()->SpawnActor<ATestPlant>(bp_IgnisPlant_, trans_plant, SpawnParams);
			break;
		case EnumElementType::State::EE_AQUA:
			plant_ = GetWorld()->SpawnActor<ATestPlant>(bp_AquaPlant_, trans_plant, SpawnParams);
			break;
		case EnumElementType::State::EE_VENTUS:
			plant_ = GetWorld()->SpawnActor<ATestPlant>(bp_VentusPlant_, trans_plant, SpawnParams);
			break;
		case EnumElementType::State::EE_TERRA:
			plant_ = GetWorld()->SpawnActor<ATestPlant>(bp_TerraPlant_, trans_plant, SpawnParams);
			break;
		case EnumElementType::State::EE_SPIRITUS:
			plant_ = GetWorld()->SpawnActor<ATestPlant>(bp_SpiritusPlant_, trans_plant, SpawnParams);
			break;
		default:
			if(DEBUG_LOG)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "Planted " + seed->name_ + " has no element type!");
			break;
		}

		// TODO: This needs changing
		if (seed->name_.Equals("AquaSeed"))
		{
			b_is_earth_ = false;
		}
		else if (seed->name_.Equals("TerraSeed"))
		{
			b_is_earth_ = true;
		}

		GLog->Log("Planted the seed!");
		if(DEBUG_LOG)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Planted a " + seed->name_ + "!");
		b_occupied_ = true;
		akAudioComponent->PostAkEventByName("PlayerPlantSeed");
		b_first_plant_ = true;
		return true;
	}

	return false;
}

// return true if plant on plot is harvestable
bool AFarmingPlot::OnInteract()
{
	// return true if plant on plot is harvestable
	return b_occupied_ && plant_->IsHarvestable();
}

// remove harvestable plant from plot
bool AFarmingPlot::HarvestPlant()
{
	// currently add to crate not implemented 
	plant_->HarvestPlant(nullptr);

	b_occupied_ = false;
	//plant_->Destroy();
	plant_ = nullptr;
	b_harvested_this_frame_ = true;
	// increase harvest crops count
	game_stats_->IncrementCropCount(b_is_earth_);
	SetWatered(false);

	// harvest sound
	akAudioComponent->PostAkEventByName("PlayerHarvest");

	// play vfx
	niagaraComp_->Activate(true);
	b_first_harvest_ = true;
	return true;
}

// called by player when player is in range to interact with plot
void AFarmingPlot::InRangeOfPlayer()
{
	b_in_range_of_player_ = true;
}

// called by plant when harvested, plot is no longer occupied
void AFarmingPlot::SetPlotEmpty()
{
	b_occupied_ = false;
}

UDA_Crop* AFarmingPlot::GetPlantCropType()
{
	return plant_->GetCropData();
}

// returns whether the plot is watered or not
bool AFarmingPlot::GetWatered()
{
	return b_watered_;
}

// returns whether the plot can be watered
bool AFarmingPlot::IsWaterable()
{
	if (!b_watered_ && plant_ != nullptr) return true;
	return false;
}

// Checks if plot is available
bool AFarmingPlot::IsAvailable()
{
	return !b_occupied_;
}