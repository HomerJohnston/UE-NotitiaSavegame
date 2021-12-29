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
	uint64 NextIdentifier;

	///** Store all saveable actors/components in this map */ 
	//UPROPERTY()
	//TMap<uint64, TScriptInterface<INotitiaSaveable>> IdentifierToSaveablesMap;

	/** Store level actors/components in this map */
	UPROPERTY()
	TMap<uint64, FString> IdentifierToSaveablesPathsMap;

	/** Reverse lookup for save/load processes */
	UPROPERTY()
	TMap<UObject*, uint64> ObjectToIdentifierMap;

	FDelegateHandle OnGISubsystemInitializeCompleteHandle;
	FDelegateHandle OnSaveBeginHandle;

	UPROPERTY()
	TSet<AActor*> LevelActors;
	
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
	void LoadPersistentLevel(TSharedPtr<FJsonObject> LoadJSON) const;

// ------------------------------------------------------------------------------------------------
// Private API
private:

	/**  */
	bool ActorIsDynamic(const AActor* Actor) const;

	void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
};
