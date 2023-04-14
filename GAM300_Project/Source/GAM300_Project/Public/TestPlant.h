/*
 * File: TestPlant.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FarmingPlot.h"
#include "DA_Crop.h"
#include "Crate.h"
#include "CharacterBase.h"
#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "TestPlant.generated.h"

class UTextBlock;
class UNiagaraComponent;
class UNiagaraSystem;
// spring arm forward declaration
class USpringArmComponent;

UCLASS()
class GAM300_PROJECT_API ATestPlant : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestPlant();

	float water_timer_;
	float growth_timer_;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GrowToNextStage();

	int currStage;

	// growthbarUI
	UProgressBar* growthbar;

	// waterbarUI
	UProgressBar* waterbar;

	// widget containing two bars
	UUserWidget* barswidget;

	// widget containing interact icon
	UUserWidget* interactwidget;

	// the key to interact with
	UImage* keyText;

	// data about what crop this is
	UPROPERTY(EditAnywhere, Category = "DataAsset")
		UDA_Crop* cropData;

	// pointer to the player character
	ACharacterBase* player_;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// true - if plant can now be removed from plot
	bool IsHarvestable();

	// set reference to the plot the plant is on
	//void SetPlotPointer(AFarmingPlot* pplot);

	// increment the number of days in this stage
	void IncrementDayCount();

	// set visibility of key prompt
	void SetKeyVisible(bool visible);

	// set visibility of growth bars
	void SetBarVisible(bool visible);
	
	bool HarvestPlant(ACrate* const crate);

	// set water timer
	void Water();

	// returns crop data
	UDA_Crop* GetCropData();
	
	UPROPERTY()
		USceneComponent* Root;

	// The number of stages for this plant
	UPROPERTY(EditAnywhere)
		int MAX_STAGES = 3;


	// Timers between stages
	/*
	UPROPERTY(EditAnywhere)
		int days_to_stage_02_ = 1;
		float days_to_stage_02 = 5.0f;

	UPROPERTY(EditAnywhere)
		int days_to_stage_03_ = 2;
		float days_to_stage_03 = 10.0f;

	UPROPERTY(EditAnywhere)
		float days_to_stage_04 = 10.0f;
	*/
	UPROPERTY(BlueprintReadWrite, Category = Growth)
		bool b_grew_this_frame_;

	UPROPERTY(EditAnywhere)
		float secs_to_grow_;

	UPROPERTY(EditAnywhere)
		float water_modifier_;

	UPROPERTY(EditAnywhere)
		float secs_water_is_active_;

	// Mesh for each stage
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh_Stage_01;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh_Stage_02;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh_Stage_03;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh_Stage_04;

	// widget arm, vector to rotate UI
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		USpringArmComponent* bar_arm;

	// widget arm, vector to rotate UI
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		USpringArmComponent* widget_arm;

	// widget component for showing UI elements
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UWidgetComponent* widgetComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UWidgetComponent* barComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
		UParticleSystemComponent* particleComp;

	// Particle component for plots
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
		UNiagaraComponent* niagaraComp_;
	// particle system to use
	UPROPERTY(EditAnywhere)
		UNiagaraSystem* niagara_system_template_ = nullptr;
};
