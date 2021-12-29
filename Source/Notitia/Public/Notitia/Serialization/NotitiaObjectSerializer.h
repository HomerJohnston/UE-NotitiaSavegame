#pragma once

#include "CoreMinimal.h"

class UNotitiaDeveloperSettings;
class FProperty;
struct FNotitiaPropertyContainer;

class FNotitiaObjectSerializer
{
public:
	static bool SerializeObject(UObject* Object, TSharedPtr<FJsonObject> ObjectsContainerJSON);

protected:
	static bool SerializeObject_Internal(UObject* Object, TSharedPtr<FJsonObject> ObjectsContainerJSON); 
	
	static bool SetupPrerequisites(const UObject* Object, const UClass*& OutClass, const UNotitiaDeveloperSettings*& OutDeveloperSettings);

	static bool TrySaveProperty(const UObject* Object, TArray<FName>& PropertyPath, FProperty* Property, TSharedPtr<FJsonObject> PropertiesJSON, TSharedPtr<FJsonObject> ObjectsContainerJSON);

	static TSharedPtr<FJsonValue> ConvertScalarFPropertyToJsonValue(FProperty* Property, const void* Value, FProperty* OuterProperty, TSharedPtr<FJsonObject> ObjectsContainerJSON);
	
	static TSharedPtr<FJsonValue> UPropertyToJsonValue(FProperty* Property, const void* Value, FProperty* OuterProperty, TSharedPtr<FJsonObject> ObjectsContainerJSON);
	
	static bool UStructToJsonObject(const UStruct* StructDefinition, const void* Struct, TSharedRef<FJsonObject> OutJsonObject, TSharedPtr<FJsonObject> ObjectsContainerJSON);

	static bool UStructToJsonAttributes(const UStruct* StructDefinition, const void* Struct, TMap< FString, TSharedPtr<FJsonValue> >& OutJsonAttributes, TSharedPtr<FJsonObject> ObjectsContainerJSON);

	/*
	static int32 PropertyCount;
	
	static TSet<FProperty*> ProcessedProperties;

	static void ProcessActor(AActor* Actor);

	static void ProcessUObject(TArray<FNotitiaPropertyContainer>& PropertyStack, UObject* Object, int32 NestLevel);
	
	static void ProcessPropertyContainer(TArray<FNotitiaPropertyContainer>& PropertyStack, TArray<FNotitiaPropertyContainer>& PropertyList, FNotitiaPropertyContainer PropertyContainer);

	static void LogProperty(FNotitiaPropertyContainer PropertyContainer);

	static FString GetPropertyValueAsString(FNotitiaPropertyContainer PropertyContainer);
	
	UFUNCTION(BlueprintCallable)
	static void Test();

	UFUNCTION(BlueprintCallable)
	static void ReadLevelActor(AActor* Actor);
	*/
};



