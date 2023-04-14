/*
 * File: DA_Seed.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez, Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnumElement.h"
#include "DA_Seed.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAM300_PROJECT_API UDA_Seed : public UDataAsset
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

    // Texture
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
      int sell_value_ = -1;

    // Plant Element
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ELEMENT)
        TEnumAsByte<EnumElementType::State> element_;

    /*
    // Time to grow
    UPROPERTY(EditAnywhere)
      int32 time_to_grow_ = 0;

    // Plant to place when used
    UPROPERTY(EditAnywhere)
        AActor* plant_actor_ = 0;
    */
};
