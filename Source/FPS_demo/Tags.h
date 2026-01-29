#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace Tags
{
	namespace Abilities
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Primary);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ActivateOnGiven);
	}

	namespace Events
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReact);
	}
}