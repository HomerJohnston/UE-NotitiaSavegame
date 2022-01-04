#pragma once

#include "CoreMinimal.h"
#include "NotitiaGISubsystem.generated.h"

class UNotitiaWorldSubsystem;
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
	TMap<UObject*, int32> ObjectGUIDMap;

	/**  */
	TMap<int32, UObject*> ObjectGUIDReverseMap;

	TWeakObjectPtr<UNotitiaWorldSubsystem> WorldSubsystem;
	
	static TWeakObjectPtr<UGameInstance> GameInstance;
	
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
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	void SetWorldSubsystem(UNotitiaWorldSubsystem* InWorldSubsystem);

	/**  */
	void Deinitialize() override;
	
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
	int32 GetIdForObject(UObject* Object);

	/**  */
	UObject* GetObjectForID(int32 Id);

	/**  */
	UFUNCTION(BlueprintCallable)
	void Load(FString Filename = "");

	static TWeakObjectPtr<UGameInstance> GetStaticGameInstance();
	
// Internal helpers
protected:
	/**  */
	int32 GenerateUniqueID();

public:
	FName GenerateFName(const UClass* Class, const TArray<FName>& PropertyPath) const;
};
