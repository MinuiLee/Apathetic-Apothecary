/*
 * File: BaseItem.cpp
 * Course: GAM300
 * Author(s): Alejandro Ramirez, Ryan Garvan
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "BaseItem.h"

#define DEBUG_LOG 0

// Sets default values
ABaseItem::ABaseItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
    meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    meshComp->SetupAttachment(RootComponent);
}

void ABaseItem::BeginPlay()
{

}

void ABaseItem::OnInteract_Implementation(AActor* Caller)
{
    if (DEBUG_LOG)
  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("INTERACT"));
}


void ABaseItem::Use_Implementation(AActor* Caller)
{
    if (DEBUG_LOG)
  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("USE"));
}


/*************************************************************************/
  /*!
    \brief
      Get the item description

    \return
      The description of the item.
  */
/*************************************************************************/
FString ABaseItem::GetItemDescription() const
{
	return description_;
}

/*************************************************************************/
  /*!
    \brief
      Flag to see if the baseitem is stackable

    \return
      True if it is stackable, false otherwise.
  */
/*************************************************************************/
bool ABaseItem::IsStackable() const
{
	return is_stackable_;
}

/*************************************************************************/
  /*!
    \brief
      Gets the max stack size

    \return
      The max size of the stack.
  */
/*************************************************************************/
int ABaseItem::GetStackSize() const
{
	return stack_size_;
}
