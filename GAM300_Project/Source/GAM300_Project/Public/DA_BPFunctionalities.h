/*
 * File: DA_BPFunctionalites.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez, Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "S_Item.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DA_BPFunctionalities.generated.h"

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API UDA_BPFunctionalities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
		static FString GetItemName(FUS_Item const& item);

	UFUNCTION(BlueprintCallable)
		static int GetItemValue(FUS_Item const& item);

	/*UFUNCTION(BlueprintCallable)
		static int GetSellValue(FUS_Item const& item);*/
};
