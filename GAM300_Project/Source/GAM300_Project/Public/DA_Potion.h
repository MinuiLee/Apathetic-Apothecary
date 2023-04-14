/*
 * File: DA_Potion.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "S_Item.h"
#include "Curves/CurveFloat.h"
#include "DA_Potion.generated.h"
/**
 *
 */

class UNiagaraSystem;

UENUM(BlueprintType)
enum E_POTION_EFFECT
{
  EFX_SHRINK              UMETA(DisplayName = "Shrink effect"),
  EFX_GROW                UMETA(DisplayName = "Grow effect"),
  EFX_LEVITATE            UMETA(DisplayName = "Levitate effect"),
  EFX_GRAVITY             UMETA(DisplayName = "Gravity effect"),
  EFX_TIMESTOP            UMETA(DisplayName = "Time Stop effect"),
  EFX_NOTHING             UMETA(DisplayName = "No effect"),
  EFX_MAX                 UMETA(DisplayName = "Max enum effect")
};


UCLASS(BlueprintType)
class GAM300_PROJECT_API UDA_Potion : public UDataAsset
{
  GENERATED_BODY()

  public:
    // Name
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      FString name_;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      FString description_;

	// Attribute Name
    UPROPERTY(EditAnywhere)
        FString attribute_;

    // Texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      UTexture2D* texture_;

    // Texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      int sell_value_ = -1;

    // ID
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      TEnumAsByte<E_POTION_EFFECT> potion_effect_ = EFX_SHRINK;

    UPROPERTY(EditAnywhere)
      UCurveFloat* potion_curve_ = nullptr;

    UPROPERTY(EditAnywhere)
      UNiagaraSystem* niagara_system_template_ = nullptr;
};
