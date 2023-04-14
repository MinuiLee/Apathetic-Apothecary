/*
 * File: Crop.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "DA_Crop.h"
#include "Crate.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Crop.generated.h"

class ACharacterBase;
UCLASS(BlueprintType)
class GAM300_PROJECT_API ACrop : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACrop();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UWidgetComponent* widgetComp;

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	bool SendDataToCrate(ACrate* const crate);
	void SetCollisiioProfileName(std::string preset, bool isHeld = true);
	UDA_Crop* GetCropData();

private:
	bool isHeld;
	
	UPROPERTY(EditAnywhere, Category = "DataAsset")
		UDA_Crop* cropData;
	ACharacterBase* player;

	UUserWidget* cropUI;
	UTextBlock* keyText;
};
