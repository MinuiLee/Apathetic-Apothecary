/*
 * File: DA_Crop.h
 * Course: GAM300
 * Author(s): Minui Lee
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_Crop.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class GAM300_PROJECT_API UDA_Crop : public UDataAsset
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
	UTexture2D* texture_;

	// Texture
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int sell_value_ = -1;

	// Texture
	UPROPERTY(EditAnywhere)
	UTexture2D* attributeTexture_;

	// Attibute texture with outline
	UPROPERTY(EditAnywhere)
		UTexture2D* attributeTextureOutline_;

	//Attributes
	UPROPERTY(EditAnywhere)
	FString attribute_;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> cropActor_;
};
