/*
 * File: DA_BPFunctionalities.cpp
 * Course: GAM300
 * Author(s): Alejandro Ramirez, Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */
#include "DA_BPFunctionalities.h"
#include "DA_Crop.h"
#include "DA_Potion.h"
#include "DA_Seed.h"
#include "DA_WateringCan.h"

FString UDA_BPFunctionalities::GetItemName(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_);

	if (potion != NULL)
	{
		return potion->name_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_);

	if (seed != NULL)
	{
		return seed->name_;
	}

	UDA_WateringCan const* can = Cast<UDA_WateringCan const>(item.itemType_);

	if (can != NULL)
	{
		return can->name_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_);

	if (crop != NULL)
	{
		return crop->name_;
	}

	return "";
}

int UDA_BPFunctionalities::GetItemValue(FUS_Item const& item)
{
	UDA_Potion const* potion = Cast<UDA_Potion const>(item.itemType_);

	if (potion != NULL)
	{
		return potion->sell_value_;
	}

	UDA_Seed const* seed = Cast<UDA_Seed const>(item.itemType_);

	if (seed != NULL)
	{
		return seed->sell_value_;
	}

	UDA_WateringCan const* can = Cast<UDA_WateringCan const>(item.itemType_);

	if (can != NULL)
	{
		return can->sell_value_;
	}

	UDA_Crop const* crop = Cast<UDA_Crop const>(item.itemType_);

	if (crop != NULL)
	{
		return crop->sell_value_;
	}

	return -2;
}