#pragma once

#include "CoreMinimal.h"
#include "NotitiaClassSettings.generated.h"

USTRUCT()
struct FNotitiaClassSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TMap<FString, bool> SerializeMap;
};
