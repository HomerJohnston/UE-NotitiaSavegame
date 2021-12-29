#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NotitiaDeveloperSettings.generated.h"

// ================================================================================================
USTRUCT()
struct NOTITIA_API FNotitiaProperty
{
	GENERATED_BODY()

	FNotitiaProperty();
	
	FNotitiaProperty(const TArray<FName>& InPath);
	
	/** Path to the property, the final element is the actual property name, any prior elements represent structs/containers */
	UPROPERTY(EditAnywhere, Config)
	TArray<FName> Path;

	bool operator<(const FNotitiaProperty& Other) const;
	
	bool operator==(const FNotitiaProperty& Other) const;
	
	bool Equals(const FNotitiaProperty& Other) const;
};

uint32 GetTypeHash(const FNotitiaProperty& Property);

// ================================================================================================
/** Contains all of the serialization data for a particular class */
USTRUCT()
struct NOTITIA_API FNotitiaClassSerializationData
{
	GENERATED_BODY()

	FNotitiaClassSerializationData();
	
	FNotitiaClassSerializationData(const UClass* InClass);
	
	/** The actual class targeted by this data */
	UPROPERTY(EditAnywhere, Config)
	TSoftClassPtr<UObject> Class;

	/** Each individual property serialized for this class, starting from root because no recursive properties in developersettings */
	UPROPERTY(EditAnywhere, Config)
	TArray<FNotitiaProperty> Properties;
	
	void AddProperty(const FNotitiaProperty& PropertyPath);

	void RemoveProperty(const FNotitiaProperty& PropertyPath);

	ECheckBoxState GetPropertyState(const FNotitiaProperty& PropertyPath) const;
	
	int32 GetSavedPropertyCount() const;
};


// ================================================================================================
/** Project Settings */
UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Notitia Save System"))
class NOTITIA_API UNotitiaDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

//
protected:
	/** Contains all of the automatic serialization / properties settings for the project */
	UPROPERTY(EditAnywhere, Config)
	TArray<FNotitiaClassSerializationData> Classes;

//
public:
	void AddProperty(const UClass* Class, const TArray<FName>& PropertyPath);
	
	void RemoveProperty(const UClass* Class, const TArray<FName>& PropertyPath);
	
	ECheckBoxState GetPropertyState(const UClass* Class, const TArray<FName>& PropertyPath) const;

	bool ContainsClass(const UClass* Class, bool& ExactMatch) const;

	int32 GetNumSelectedProperties(UClass* Class) const;

	const TArray<FNotitiaClassSerializationData>& GetClassesData() const;
};
