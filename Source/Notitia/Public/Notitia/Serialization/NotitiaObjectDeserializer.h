#pragma once

#include "CoreMinimal.h"

class FNotitiaObjectDeserializer
{
public:
	static bool DeserializeObject_Internal(UObject* Object, TSharedPtr<FJsonObject> ObjectJSON);
	
};