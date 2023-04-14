/*
 * File: S_Item.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "S_Item.generated.h"

USTRUCT(BlueprintType)
struct GAM300_PROJECT_API FUS_Item
{
  GENERATED_BODY()

public:
  FUS_Item();
  FUS_Item(UDataAsset const *itemType, int amount);

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UDataAsset const *itemType_;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int amount_;
};
