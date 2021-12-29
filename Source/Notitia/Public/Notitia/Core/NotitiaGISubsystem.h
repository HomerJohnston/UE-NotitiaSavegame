#pragma once

#include "CoreMinimal.h"
#include "NotitiaGISubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInitializeEvent)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSaveBegin, TSharedPtr<FJsonObject>);

/**  */
UCLASS()
class NOTITIA_API UNotitiaGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

// Internal state
protected:
	/**  */ // TODO can I store actual FProperty* objects instead? Can I store "gateway" properties to allow me to check if I'm going down a dead-end when iterating a class?
	TSet<FName> SerializedPropertyPaths;

	/**  */
	TMap<UObject*, uint64> ObjectGUIDMap;

	/**  */
	TMap<uint64, UObject*> ObjectGUIDReverseMap;

// Public state
public:
	/**  */
	static FOnInitializeEvent OnInitialize;

	/**  */
	static FOnInitializeEvent OnDeinitialize;

	/**  */
	static bool bIsInitialized;

	/**  */
	FOnSaveBegin OnSaveBegin;

	/**  */
	FOnSaveBegin OnLoadBegin;

// Creation
public:
	/**  */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	virtual void Deinitialize() override;

// Creation helpers
protected:
	/**  */
	void BuildPropertiesSet();

// Public API
public:
	/**  */
	UFUNCTION(BlueprintCallable)
	void Save(FString Filename = "");

	/**  */
	bool SavePersistentLevel(const FString& PersistentLevelPackage);

	/**  */
	bool IsPropertyMarkedForSerialization(FName PropertyPath);

	/**  */
	bool AssignIdForObject(UObject* Object);
	
	/**  */
	uint64 GetIdForObject(UObject* Object);

	/**  */
	UObject* GetObjectForID(uint64 Id);

	/**  */
	void Load(FString Filename = "");
	
// Internal helpers
protected:
	/**  */
	uint64 GenerateUniqueID();

public:
	FName GenerateFName(const UClass* Class, const TArray<FName>& PropertyPath) const;
};
