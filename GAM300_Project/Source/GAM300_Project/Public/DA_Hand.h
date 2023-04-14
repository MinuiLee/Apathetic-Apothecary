/*
 * File: DA_Hand.h
 * Course: GAM300
 * Author(s): Carlos Esteban Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_Hand.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAM300_PROJECT_API UDA_Hand : public UDataAsset
{
	GENERATED_BODY()

  public:

    // Name
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      FString name_;
    
    // Description
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      FString description_;

    // Texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      UTexture2D* texture_ = nullptr;

    // Sell Value
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
        int sell_value_ = -1;
};
