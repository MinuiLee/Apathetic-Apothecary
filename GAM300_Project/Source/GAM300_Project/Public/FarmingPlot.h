/*
 * File: FarmingPlot.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DA_Seed.h"
#include "Crate.h"
#include "AkComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"
#include "CharacterBase.h"
#include "GameStats.h"
#include "FarmingPlot.generated.h"

// Plant to Spawn
class ATestPlant;

// Audio Component Forward Declaration
class AkComponent;

// Text Block Forward Declaration
class UTextBlock;

// spring arm forward declaration
class USpringArmComponent;

// niagara comp forward declaration
class UNiagaraComponent;
// niagara sys forward declaration
class UNiagaraSystem;

UCLASS()
class GAM300_PROJECT_API AFarmingPlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFarmingPlot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// watering icon
	UUserWidget* wateringIcon;

	// the key to interact with
	UImage* keyText;

	bool b_watered_;

	bool b_occupied_;

	bool b_in_range_of_player_;

	ATestPlant* plant_;

	ACharacterBase* player_;

	UImage* slot_image_;

	bool b_is_earth_;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Watered plot will allow the plant to grow
	// Plot becomes dry after a day passed
	// Mesh changes when switched
	void SetWatered(bool watered);

	// returns whether the plot is watered or not
	bool GetWatered();

	// returns whether the plot can be watered
	bool IsWaterable();

	// spawn the plant that will be in this farm plot
	bool SpawnPlant(UDA_Seed const* seed);

	// water the plot if dry, harvest if plant is ready
	bool OnInteract();

	// remove harvestable plant from plot
	bool HarvestPlant();

	// Checks if plot is available
	bool IsAvailable();

	// called by player when player is in range to interact with plot
	void InRangeOfPlayer();

	// called by plant when harvested, plot is no longer occupied
	void SetPlotEmpty();

	// returns plant crop type
	UDA_Crop* GetPlantCropType();
	
	UPROPERTY()
		USceneComponent* Root;

	// Location to place plant
	//UPROPERTY(EditAnywhere)
		//FSHVector3 spawn_location_;

	// Mesh for wooden plot
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* mesh_plot_;

	// Mesh for dry dirt
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* mesh_dry_dirt_;

	// Mesh for wet dirt
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* mesh_wet_dirt_;

	// Mesh for showing the plot is in interact range
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* mesh_interact_signifier_;
	
	// Particle component for plots
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
		UNiagaraComponent* niagaraComp_;
	// particle system to use
	UPROPERTY(EditAnywhere)
		UNiagaraSystem* niagara_system_template_ = nullptr;

	// Blueprint Class to Spawn
	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ATestPlant> bp_TerraPlant_;

	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ATestPlant> bp_AquaPlant_;

	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ATestPlant> bp_VentusPlant_;

	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ATestPlant> bp_IgnisPlant_;

	UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
		TSubclassOf<ATestPlant> bp_SpiritusPlant_;

	// Bools accessable in Blueprints to create particle effects
	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_watered_this_frame_;

	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_harvested_this_frame_;

	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_harvestable_;

	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_first_plant_;

	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_first_water_;

	UPROPERTY(BlueprintReadWrite, Category = ParticleBools)
		bool b_first_harvest_;

	UPROPERTY(BlueprintReadWrite, Category = "GameStatsVar")
		AGameStats* game_stats_;

	UPROPERTY(BlueprintReadOnly, Category = "Wwise")
		UAkComponent* akAudioComponent;

	// widget arm, vector to rotate UI
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		USpringArmComponent* widget_arm;

	// widget component for showing UI elements
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UWidgetComponent* widgetComp;
	
	// texture for watering can
	UPROPERTY(EditDefaultsOnly, Category = "Texture")
		UTexture2D* watercan_tex;

	// texture for planting icon
	UPROPERTY(EditDefaultsOnly, Category = "Texture")
		UTexture2D* planting_tex;
};
