/*
 * File: ItemInterface.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAM300_PROJECT_API IItemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Interaction")
	void OnInteract(AActor* Caller);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Interaction")
	void Use(AActor* Caller);
};
