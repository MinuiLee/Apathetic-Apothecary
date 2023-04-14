/*
 * File: WwiseAnimNotify.cpp
 * Course: GAM300
 * Author(s): Alfaroh Corney III
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */


#include "WwiseAnimNotify.h"
#include "CharacterBase.h"
#include "AkAudioEvent.h"

void UWwiseAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
  if (MeshComp && MeshComp->GetAttachmentRootActor())
  {
    // Get root component
    ACharacterBase* character = Cast<ACharacterBase>(MeshComp->GetAttachmentRootActor());
    if (character)
    {
      // Get Audio Component
      UAkComponent* akComponent = character->akAudioComponent;

      // Play the event
      FOnAkPostEventCallback nullCallback;
      akComponent->PostAkEvent(audioEvent, 
                               int32(0), 
                               nullCallback, 
                               audioEvent->GetName());
    }
  }
}