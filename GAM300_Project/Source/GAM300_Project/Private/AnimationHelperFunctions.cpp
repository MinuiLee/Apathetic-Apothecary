// All content © 2020 DigiPen (USA) Corporation, all rights reserved.


#include "AnimationHelperFunctions.h"
#include "CharacterBase.h"

void UAnimationHelperFunctions::RotateToFaceActor(ACharacterBase* character, UCurveFloat* curve,
																									FOnTimelineFloat InterpFunc, FName PropertyName,
																									FOnTimelineEvent NewTimelineFinishedFunc)
{
	if (!character->timelineComp->IsPlaying())
	{
		//character->timelineComp->
		// Add curve and connect it to the interpolate function
		character->timelineComp->AddInterpFloat(curve, InterpFunc, PropertyName);
		// Add on end function
		character->timelineComp->SetTimelineFinishedFunc(NewTimelineFinishedFunc);
		character->timelineComp->SetLooping(false);
		character->timelineComp->PlayFromStart();
	}
}
