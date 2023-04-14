/*
 * File: S_Item.cpp
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#include "S_Item.h"

FUS_Item::FUS_Item() : itemType_(NULL), amount_(0)
{
}

FUS_Item::FUS_Item(UDataAsset const *itemType, int amount) : itemType_(itemType), amount_(amount)
{
}