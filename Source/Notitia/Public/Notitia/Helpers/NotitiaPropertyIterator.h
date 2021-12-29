#pragma once

#include "CoreMinimal.h"

class FNotitiaPropertyIterator
{
	static bool GetProperties(UClass* InClass, TArray<FProperty*>& PropertiesOut);
	
	void GetNestedProperties(FProperty* Property, TArray<FProperty*>& NestedPropertiesOut);
};