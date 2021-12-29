#include "Notitia/Helpers/NotitiaPropertyIterator.h"

bool FNotitiaPropertyIterator::GetProperties(UClass* InClass, TArray<FProperty*>& PropertiesOut)
{
	return false;/*
	TArray<FProperty*> NestedProperties;
	
	GetNestedProperties(Property, NestedProperties);
	
	if (NestedProperties.Num() > 0)
	{
		for (FProperty* NestedProp : NestedProperties)
		{
			ContainerPath.Push(Property->GetFName());
			SaveProp(Object, ContainerPath, NestedProp, JSONObject);
		}
	}
	else
	{
		FName PropPath = GenerateFName(Object->GetClass(), ContainerPath, Property->GetFName());

		//NOTITIA_LOG_VERBOSE("Checking if property %s should be saved", *PropPath.ToString());
		
		if (SerializedProperties.Contains(PropPath))
		{
			if (FArrayProperty* PropertyAsArray = CastField<FArrayProperty>(Property))
			{
				void* ArrayPtr = PropertyAsArray->ContainerPtrToValuePtr<void*>(Object);
				FScriptArrayHelper ArrayHelper(PropertyAsArray, ArrayPtr);
				
				TArray<TSharedPtr<FJsonValue>> PropertyArrayJSON;

				FProperty* Inner = PropertyAsArray->Inner;
				
				for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
				{
					uint8* ValPtr = ArrayHelper.GetRawPtr(Index);

					if (FBoolProperty* InnerAsBool = CastField<FBoolProperty>(Inner))
					{
						bool bVal = InnerAsBool->GetPropertyValue(ValPtr);
						TSharedPtr<FJsonValueBoolean> BoolJSON = MakeShareable(new FJsonValueBoolean(bVal));
						PropertyArrayJSON.Add(BoolJSON);
					}
				}
				
				TSharedPtr<FJsonValueArray> ArrayJSON = MakeShareable(new FJsonValueArray(PropertyArrayJSON));

				//NOTITIA_LOG_VERBOSE("Saving array element %s", *Property->GetFName().ToString());
				JSONObject->SetField(Property->GetFName().ToString(), ArrayJSON);
			}
			else
			{
				SaveLeafProp(Object, Property, JSONObject);
			}
		}
	}
	
	return true;*/
}

void FNotitiaPropertyIterator::GetNestedProperties(FProperty* Property, TArray<FProperty*>& NestedPropertiesOut)
{
	if (CastField<FStructProperty>(Property))
	{
		
	}

	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		FProperty* InnerProperty = ArrayProperty->Inner;
	}

	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		FProperty* ElementProperty = SetProperty->ElementProp;
	}

	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		FProperty* KeyProperty = MapProperty->KeyProp;
		FProperty* ValueProperty = MapProperty->ValueProp;
	}
}
