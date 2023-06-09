// All content © 2020 DigiPen (USA) Corporation, all rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/TimelineComponent.h"
#include "AnimationHelperFunctions.generated.h"

class ACharacterBase;

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API UAnimationHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
		static void RotateToFaceActor(ACharacterBase* character, UCurveFloat* curve, 
																	FOnTimelineFloat InterpFunc, FName PropertyName, 
																	FOnTimelineEvent NewTimelineFinishedFunc);
};
