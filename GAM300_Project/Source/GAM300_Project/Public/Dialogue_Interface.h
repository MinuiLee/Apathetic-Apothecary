/*
 * File: Dialogue_Interface.h
 * Course: GAM300
 * Author(s): Macie White
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Dialogue_Interface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDialogue_Interface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAM300_PROJECT_API IDialogue_Interface
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Change these functions from AActor to ACharacter
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue Interaction")
	void OnInteract(ACharacter* Caller);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue Interaction")
		void OnTrace(ACharacter* Caller);


};
