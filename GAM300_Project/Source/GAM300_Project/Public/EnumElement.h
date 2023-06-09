/*
 * File: EnumElement.h
 * Course: GAM350
 * Author(s): Carlos Garcia-Perez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
namespace EnumElementType
{
    enum State
    {
        EE_NONE     UMETA(DisplayName = "No Element"),
        EE_IGNIS    UMETA(DisplayName = "Ignis"),
        EE_AQUA     UMETA(DisplayName = "Aqua"),
        EE_TERRA UMETA(DisplayName = "Terra"),
        EE_VENTUS UMETA(DisplayName = "Ventus"),
        EE_SPIRITUS UMETA(DisplayName = "Spiritus"),
    };
}

