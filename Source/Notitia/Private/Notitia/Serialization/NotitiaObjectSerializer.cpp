#include "Notitia/Serialization/NotitiaObjectSerializer.h"
#include "Notitia/Debug/NotitiaLogging.h"
#include "Notitia/Temp/NotitiaPropertyContainer.h"
#include "JsonObjectConverter.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Notitia/Core/NotitiaDeveloperSettings.h"
#include "Notitia/Core/NotitiaGISubsystem.h"
#include "Notitia/Serialization/NotitiaObjectSerializer.h"

class FNetPropertyHook;

bool FNotitiaObjectSerializer::SerializeObject(UObject* Object, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	if (!IsValid(Object))
	{
		return false;
	}

	UWorld* World = Object->GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	UGameInstance* GameInstance = World->GetGameInstance();

	if (!IsValid(GameInstance))
	{
		return false;
	}

	UNotitiaGISubsystem* GISubsystem = GameInstance->GetSubsystem<UNotitiaGISubsystem>();

	if (!IsValid(GISubsystem))
	{
		return false;
	}

	if (!GISubsystem->AssignIdForObject(Object))
	{
		return false;
	}
	
	return SerializeObject_Internal(Object, ObjectsContainerJSON);
}

bool FNotitiaObjectSerializer::SerializeObject_Internal(UObject* Object, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	const UClass* Class = nullptr;
	const UNotitiaDeveloperSettings* DeveloperSettings = nullptr;

	if (!SetupPrerequisites(Object, Class, DeveloperSettings))
	{
		NOTITIA_LOG_VERBOSE("DynamicObjectSerializer - NOT serializing: %s", *Object->GetName());
		return false;
	}
	
	NOTITIA_LOG_VERBOSE("DynamicObjectSerializer - serializing: %s", *Object->GetName());

	TSharedPtr<FJsonObject> ObjectJSON = MakeShareable(new FJsonObject());
	
	ObjectJSON->SetStringField("Class", Class->GetFName().ToString());
	FString ObjectName = Object->GetFName().ToString();

	TSoftObjectPtr<UObject> SoftObjectPtr(Object);
	FString SoftObjectPtrPath = SoftObjectPtr.ToString();
	SoftObjectPtrPath = Object->GetWorld()->RemovePIEPrefix(SoftObjectPtrPath);
	ObjectJSON->SetStringField("Path", SoftObjectPtrPath);
	
	if (UWorld* World = Object->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UNotitiaGISubsystem* GISubsystem = GameInstance->GetSubsystem<UNotitiaGISubsystem>())
			{
				uint64 ObjectId = GISubsystem->GetIdForObject(Object);
				FString ObjectIdString = FString::Printf(TEXT("%llu"), ObjectId);
				TSharedPtr<FJsonValueNumberString> ObjectIdJSON = MakeShared<FJsonValueNumberString>(ObjectIdString);

				ObjectJSON->SetField("NotitiaId", ObjectIdJSON);
			}
		}
	}
	
	TSharedPtr<FJsonObject> PropertiesJSON = MakeShareable(new FJsonObject());
	
	TArray<FName> PropertyPath;
	
	for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt )
	{
		FProperty* Property = *PropIt;

		PropertyPath.Add(Property->GetFName());
		TrySaveProperty(Object, PropertyPath, Property, PropertiesJSON, ObjectsContainerJSON);
		PropertyPath.Pop();
	}

	if (PropertiesJSON->Values.Num() > 0)
	{
		ObjectJSON->SetObjectField("Properties", PropertiesJSON);
	}

	if (ObjectJSON->Values.Num() > 0)
	{
		ObjectsContainerJSON->SetObjectField(ObjectName, ObjectJSON);
	}
	
	return true;
}

bool FNotitiaObjectSerializer::SetupPrerequisites(const UObject* Object, const UClass*& OutClass, const UNotitiaDeveloperSettings*& OutDeveloperSettings)
{
	OutDeveloperSettings =  GetDefault<UNotitiaDeveloperSettings>();
	OutClass = Object->GetClass();

	bool bExactMatch;
	if (!OutDeveloperSettings->ContainsClass(OutClass, bExactMatch))
	{
		//NOTITIA_LOG_VERBOSE("Skipping %s", *Object->GetName());
		return false;
	}

	return true;
}

bool FNotitiaObjectSerializer::TrySaveProperty(const UObject* Object, TArray<FName>& PropertyPath, FProperty* Property, TSharedPtr<FJsonObject> PropertiesJSON, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();

	UNotitiaGISubsystem* NotitiaGISubsystem = Object->GetWorld()->GetGameInstance()->GetSubsystem<UNotitiaGISubsystem>();

	UClass* PropertyClass = Property->GetOwnerClass();

	if (!PropertyClass)
	{
		PropertyClass = Object->GetClass();
	}
	
	FName PropertyPathFName = NotitiaGISubsystem->GenerateFName(PropertyClass, PropertyPath);

	if (NotitiaGISubsystem->IsPropertyMarkedForSerialization(PropertyPathFName))
	{
		TSharedPtr<FJsonValue> PropertyJSON = UPropertyToJsonValue(Property, Property->ContainerPtrToValuePtr<void*>(Object), nullptr, ObjectsContainerJSON);
		PropertiesJSON->SetField(Property->GetNameCPP(), PropertyJSON);
		return true;
	}
	else
	{
		if (Property->ArrayDim > 1)
		{
			if (FStructProperty* PropertyAsStruct = CastField<FStructProperty>(Property))
			{
				for (TFieldIterator<FProperty> PropIt(PropertyAsStruct->Struct); PropIt; ++PropIt)
				{
					FProperty* ChildProperty = *PropIt;
					PropertyPath.Add(ChildProperty->GetFName());

					TSharedPtr<FJsonObject> ChildPropertiesJSON = MakeShareable(new FJsonObject());
					TrySaveProperty(Object, PropertyPath, ChildProperty, ChildPropertiesJSON, ObjectsContainerJSON);
				}
			}
		}
	}
	
	return false;
}

TSharedPtr<FJsonValue> FNotitiaObjectSerializer::ConvertScalarFPropertyToJsonValue(FProperty* Property, const void* Value, FProperty* OuterProperty, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	const FString ObjectClassNameKey = "_ClassName";

	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		// export enums as strings
		UEnum* EnumDef = EnumProperty->GetEnum();
		FString StringValue = EnumDef->GetNameStringByValue(EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Value));
		return MakeShared<FJsonValueString>(StringValue);
	}
	else if (FNumericProperty *NumericProperty = CastField<FNumericProperty>(Property))
	{
		// see if it's an enum
		UEnum* EnumDef = NumericProperty->GetIntPropertyEnum();
		if (EnumDef != NULL)
		{
			// export enums as strings
			FString StringValue = EnumDef->GetNameStringByValue(NumericProperty->GetSignedIntPropertyValue(Value));
			return MakeShared<FJsonValueString>(StringValue);
		}

		// We want to export numbers as numbers
		if (NumericProperty->IsFloatingPoint())
		{
			return MakeShared<FJsonValueNumber>(NumericProperty->GetFloatingPointPropertyValue(Value));
		}
		else if (NumericProperty->IsInteger())
		{
			return MakeShared<FJsonValueNumber>(NumericProperty->GetSignedIntPropertyValue(Value));
		}

		// fall through to default
	}
	else if (FBoolProperty *BoolProperty = CastField<FBoolProperty>(Property))
	{
		// Export bools as bools
		return MakeShared<FJsonValueBoolean>(BoolProperty->GetPropertyValue(Value));
	}
	else if (FStrProperty *StringProperty = CastField<FStrProperty>(Property))
	{
		return MakeShared<FJsonValueString>(StringProperty->GetPropertyValue(Value));
	}
	else if (FTextProperty *TextProperty = CastField<FTextProperty>(Property))
	{
		return MakeShared<FJsonValueString>(TextProperty->GetPropertyValue(Value).ToString());
	}
	else if (FArrayProperty *ArrayProperty = CastField<FArrayProperty>(Property))
	{
		TArray< TSharedPtr<FJsonValue> > Out;
		FScriptArrayHelper Helper(ArrayProperty, Value);
		for (int32 i=0, n=Helper.Num(); i<n; ++i)
		{
			TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(ArrayProperty->Inner, Helper.GetRawPtr(i), ArrayProperty, ObjectsContainerJSON);
			if ( Elem.IsValid() )
			{
				// add to the array
				Out.Push(Elem);
			}
		}
		return MakeShared<FJsonValueArray>(Out);
	}
	else if ( FSetProperty* SetProperty = CastField<FSetProperty>(Property) )
	{
		TArray< TSharedPtr<FJsonValue> > Out;
		FScriptSetHelper Helper(SetProperty, Value);
		for ( int32 i=0, n=Helper.Num(); n; ++i )
		{
			if ( Helper.IsValidIndex(i) )
			{
				TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(SetProperty->ElementProp, Helper.GetElementPtr(i), SetProperty, ObjectsContainerJSON);
				if ( Elem.IsValid() )
				{
					// add to the array
					Out.Push(Elem);
				}

				--n;
			}
		}
		return MakeShared<FJsonValueArray>(Out);
	}
	else if ( FMapProperty* MapProperty = CastField<FMapProperty>(Property) )
	{
		TSharedRef<FJsonObject> Out = MakeShared<FJsonObject>();

		FScriptMapHelper Helper(MapProperty, Value);
		for ( int32 i=0, n = Helper.Num(); n; ++i )
		{
			if ( Helper.IsValidIndex(i) )
			{
				TSharedPtr<FJsonValue> KeyElement = UPropertyToJsonValue(MapProperty->KeyProp, Helper.GetKeyPtr(i), MapProperty, ObjectsContainerJSON);
				TSharedPtr<FJsonValue> ValueElement = UPropertyToJsonValue(MapProperty->ValueProp, Helper.GetValuePtr(i), MapProperty, ObjectsContainerJSON);
				if ( KeyElement.IsValid() && ValueElement.IsValid() )
				{
					FString KeyString;
					if (!KeyElement->TryGetString(KeyString))
					{
						MapProperty->KeyProp->ExportTextItem(KeyString, Helper.GetKeyPtr(i), nullptr, nullptr, 0);
						if (KeyString.IsEmpty())
						{
							UE_LOG(LogJson, Error, TEXT("Unable to convert key to string for property %s."), *MapProperty->GetName())
							KeyString = FString::Printf(TEXT("Unparsed Key %d"), i);
						}
					}

					Out->SetField(KeyString, ValueElement);
				}

				--n;
			}
		}

		return MakeShared<FJsonValueObject>(Out);
	}
	else if (FStructProperty *StructProperty = CastField<FStructProperty>(Property))
	{
		UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();
		// Intentionally exclude the JSON Object wrapper, which specifically needs to export JSON in an object representation instead of a string
		if (StructProperty->Struct != FJsonObjectWrapper::StaticStruct() && TheCppStructOps && TheCppStructOps->HasExportTextItem())
		{
			FString OutValueStr;
			TheCppStructOps->ExportTextItem(OutValueStr, Value, nullptr, nullptr, PPF_None, nullptr);
			return MakeShared<FJsonValueString>(OutValueStr);
		}

		TSharedRef<FJsonObject> Out = MakeShared<FJsonObject>();
		if (UStructToJsonObject(StructProperty->Struct, Value, Out, ObjectsContainerJSON))
		{
			return MakeShared<FJsonValueObject>(Out);
		}
	}
	else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
	{
		// Reference to another object - store as my own custom ID solution
		UObject* Object = ObjectProperty->GetObjectPropertyValue(Value);

		if (Object)
		{
			if (ObjectProperty->HasAnyPropertyFlags(CPF_PersistentInstance) || (OuterProperty && OuterProperty->HasAnyPropertyFlags(CPF_PersistentInstance)))
			{
				// TODO Instanced property, what do?
			}
			else
			{
				// Actor or asset reference
				// TODO how can I check for an asset reference?
				if (UWorld* World = Object->GetWorld())
				{
					if (UGameInstance* GameInstance = World->GetGameInstance())
					{
						if (UNotitiaGISubsystem* GISubsystem = GameInstance->GetSubsystem<UNotitiaGISubsystem>())
						{
							SerializeObject(Object, ObjectsContainerJSON);
							
							uint64 ObjectId = GISubsystem->GetIdForObject(Object);
							FString ObjectIdString = FString::Printf(TEXT("%llu"), ObjectId);
							
							TSharedPtr<FJsonObject> ObjectJSON = MakeShareable(new FJsonObject());
							FString ObjectName;
							
							return MakeShared<FJsonValueNumberString>(ObjectIdString);
						}
					}
				}
				else
				{
					check(false);
				}
			}
		}
		else
		{
			FString StringValue;
			Property->ExportTextItem(StringValue, Value, nullptr, nullptr, PPF_None);
			return MakeShared<FJsonValueString>(StringValue);
		}
		/*
		// Instanced properties should be copied by value, while normal UObject* properties should output as asset references
		UObject* Object = ObjectProperty->GetObjectPropertyValue(Value);
		if (Object && (ObjectProperty->HasAnyPropertyFlags(CPF_PersistentInstance) || (OuterProperty && OuterProperty->HasAnyPropertyFlags(CPF_PersistentInstance))))
		{
			TSharedRef<FJsonObject> Out = MakeShared<FJsonObject>();

			Out->SetStringField(ObjectClassNameKey, Object->GetClass()->GetFName().ToString());
			if (UStructToJsonObject(ObjectProperty->GetObjectPropertyValue(Value)->GetClass(), Object, Out))
			{
				TSharedRef<FJsonValueObject> JsonObject = MakeShared<FJsonValueObject>(Out);
				JsonObject->Type = EJson::Object;
				return JsonObject;
			}
		}
		else
		{
			FString StringValue;
			Property->ExportTextItem(StringValue, Value, nullptr, nullptr, PPF_None);
			return MakeShared<FJsonValueString>(StringValue);
		}
		*/
	}
	else
	{
		// Default to export as string for everything else
		FString StringValue;
		Property->ExportTextItem(StringValue, Value, NULL, NULL, PPF_None);
		return MakeShared<FJsonValueString>(StringValue);
	}

	// invalid
	return TSharedPtr<FJsonValue>();
}

TSharedPtr<FJsonValue> FNotitiaObjectSerializer::UPropertyToJsonValue(FProperty* Property, const void* Value, FProperty* OuterProperty, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	if (Property->ArrayDim == 1)
	{
		return ConvertScalarFPropertyToJsonValue(Property, Value, OuterProperty, ObjectsContainerJSON);
	}

	TArray< TSharedPtr<FJsonValue> > Array;
	for (int Index = 0; Index != Property->ArrayDim; ++Index)
	{
		Array.Add(ConvertScalarFPropertyToJsonValue(Property, (char*)Value + Index * Property->ElementSize, OuterProperty, ObjectsContainerJSON));
	}
	return MakeShared<FJsonValueArray>(Array);
}

bool FNotitiaObjectSerializer::UStructToJsonObject(const UStruct* StructDefinition, const void* Struct, TSharedRef<FJsonObject> OutJsonObject, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	return UStructToJsonAttributes(StructDefinition, Struct, OutJsonObject->Values, ObjectsContainerJSON);
}

bool FNotitiaObjectSerializer::UStructToJsonAttributes(const UStruct* StructDefinition, const void* Struct, TMap<FString, TSharedPtr<FJsonValue>>& OutJsonAttributes, TSharedPtr<FJsonObject> ObjectsContainerJSON)
{
	if (StructDefinition == FJsonObjectWrapper::StaticStruct())
	{
		// Just copy it into the object
		const FJsonObjectWrapper* ProxyObject = (const FJsonObjectWrapper *)Struct;

		if (ProxyObject->JsonObject.IsValid())
		{
			OutJsonAttributes = ProxyObject->JsonObject->Values;
		}
		return true;
	}

	for (TFieldIterator<FProperty> It(StructDefinition); It; ++It)
	{
		FProperty* Property = *It;
		
		FString VariableName = Property->GetName();
		const void* Value = Property->ContainerPtrToValuePtr<uint8>(Struct);

		// convert the property to a FJsonValue
		TSharedPtr<FJsonValue> JsonValue = UPropertyToJsonValue(Property, Value, nullptr, ObjectsContainerJSON);
		if (!JsonValue.IsValid())
		{
			FFieldClass* PropClass = Property->GetClass();
			UE_LOG(LogJson, Error, TEXT("UStructToJsonObject - Unhandled property type '%s': %s"), *PropClass->GetName(), *Property->GetPathName());
			return false;
		}

		// set the value on the output object
		OutJsonAttributes.Add(VariableName, JsonValue);
	}

	return true;
}
