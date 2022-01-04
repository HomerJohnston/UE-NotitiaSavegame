#pragma once

#include "CoreMinimal.h"

#include "NotitiaWorldSubsystem.generated.h"

/**  */
UCLASS()
class NOTITIA_API UNotitiaWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

// ------------------------------------------------------------------------------------------------
// Constructors
public:
	UNotitiaWorldSubsystem();

// ------------------------------------------------------------------------------------------------
// Settings

// ------------------------------------------------------------------------------------------------
// State
private:
	
	UPROPERTY()
	int32 NextIdentifier;

	///** Store all saveable actors/components in this map */ 
	//UPROPERTY()
	//TMap<int32, TScriptInterface<INotitiaSaveable>> IdentifierToSaveablesMap;

	/** Store level actors/components in this map */
	UPROPERTY()
	TMap<int32, FString> IdentifierToSaveablesPathsMap;

	/** Reverse lookup for save/load processes */
	UPROPERTY()
	TMap<UObject*, int32> ObjectToIdentifierMap;

	FDelegateHandle OnGISubsystemInitializeCompleteHandle;
	
	FDelegateHandle OnSaveBeginHandle;
	
	FDelegateHandle OnLoadBeginHandle;

	TMap<FName, UObject*> InitialLevelObjects;
// ------------------------------------------------------------------------------------------------
// Overridden API
	/**  */
	//virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
	void OnGISubsystemInitializeComplete();

protected:
	void RegisterToGISubystemEvents();

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
		
// ------------------------------------------------------------------------------------------------
// Public API
public:
	/**  */
	void OnSaveBegin(TSharedPtr<FJsonObject> SaveJSON) const;

	/**  */
	void SavePersistentLevel(TSharedPtr<FJsonObject> SaveJSON, UWorld* World) const;

	/**  */
	void SaveStreamingLevels(TSharedPtr<FJsonObject> SaveJSON, UWorld* World) const;

	/**  */
	void SaveStreamingLevel(TSharedPtr<FJsonObject> StreamingLevelsJSON, UWorld* World, ULevelStreaming* StreamingLevel) const;

	/**  */
	void OnLoadBegin(TSharedPtr<FJsonObject> LoadJSON) const;

	/**  */
	void OnLevelChanged(ULevel* Level, UWorld* World) const;
	
	/**  */
	void GetLevelObjectIDs(TMap<int32, UObject*> Map) const;
	
	/**  */
	void LoadPersistentLevel(TSharedPtr<FJsonObject> LoadJSON, TMap<FName, UObject*>& LevelObjects) const;

// ------------------------------------------------------------------------------------------------
// Private API
private:

	/**  */
	void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);

	/**  */
	void StashLevelObjects(UWorld* World);
};
