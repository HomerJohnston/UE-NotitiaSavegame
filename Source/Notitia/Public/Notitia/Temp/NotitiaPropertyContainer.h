#pragma once

#include "CoreMinimal.h"
#include "NotitiaPropertyContainer.generated.h"

USTRUCT()
struct FNotitiaPropertyContainer
{
	GENERATED_BODY()

	UObject* Object;

	int32 NestLevel;

	FProperty* Property;
};
