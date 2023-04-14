/*
 * File: WwiseAnimNotify.h
 * Course: GAM300
 * Author(s): Alfaroh Corney III
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AkComponent.h"
#include "WwiseAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class GAM300_PROJECT_API UWwiseAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;


public:

	UPROPERTY(EditAnywhere, Category = "Wwise")
		UAkAudioEvent * audioEvent;

};
