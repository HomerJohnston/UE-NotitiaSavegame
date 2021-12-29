#include "Notitia/Core/NotitiaWorldSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "JsonObjectConverter.h"
#include "Engine/LevelScriptActor.h"
#include "Notitia/Core/NotitiaModule.h"
#include "Notitia/Serialization/NotitiaObjectSerializer.h"
#include "Notitia/Core/NotitiaGISubsystem.h"
#include "Notitia/Debug/NotitiaLogging.h"
#include "Engine/LevelStreaming.h"


UNotitiaWorldSubsystem::UNotitiaWorldSubsystem()
{
	
}

void UNotitiaWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bool bGISubsystemAvailable = false;
	if (UNotitiaGISubsystem::bIsInitialized)
	{
		RegisterToGISubystemEvents();
	}
	else
	{
		OnGISubsystemInitializeCompleteHandle = UNotitiaGISubsystem::OnInitialize.AddUObject(this, &UNotitiaWorldSubsystem::OnGISubsystemInitializeComplete);
	}

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInitialization);
}

void UNotitiaWorldSubsystem::Deinitialize()
{
	if (UNotitiaGISubsystem::bIsInitialized)
	{
		UNotitiaGISubsystem* GISubsystem =  GetWorld()->GetGameInstance()->GetSubsystem<UNotitiaGISubsystem>();

		GISubsystem->OnSaveBegin.Remove(OnSaveBeginHandle);
	}

	if (OnGISubsystemInitializeCompleteHandle.IsValid())
	{
		UNotitiaGISubsystem::OnInitialize.Remove(OnGISubsystemInitializeCompleteHandle);
	}
	
	Super::Deinitialize();
}

void UNotitiaWorldSubsystem::OnGISubsystemInitializeComplete()
{
	RegisterToGISubystemEvents();
}

void UNotitiaWorldSubsystem::RegisterToGISubystemEvents()
{
	UNotitiaGISubsystem* GISubsystem =  GetWorld()->GetGameInstance()->GetSubsystem<UNotitiaGISubsystem>();
	OnSaveBeginHandle = GISubsystem->OnSaveBegin.AddUObject(this, &UNotitiaWorldSubsystem::OnSaveBegin);
}


bool UNotitiaWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UNotitiaWorldSubsystem::OnSaveBegin(TSharedPtr<FJsonObject> SaveJSON) const
{
	NOTITIA_LOG_VERBOSE("World subsystem reacting to save event");

	UWorld* World = GetWorld();

	SavePersistentLevel(SaveJSON, World);
	SaveStreamingLevels(SaveJSON, World);
	
	/*
	UNotitiaGISubsystem* GISubsystem =  GetWorld()->GetGameInstance()->GetSubsystem<UNotitiaGISubsystem>();

	UWorld* World = GetWorld();
	const FString StreamingLevelsPrefix = World->StreamingLevelsPrefix;
	
	const ULevel* PersistentLevel = World->PersistentLevel;
	const FString PersistentLevelString = PersistentLevel->GetPackage()->GetFName().ToString();
	const FString PersistentLevelPackage = World->StripPIEPrefixFromPackageName(PersistentLevelString, StreamingLevelsPrefix);
	NOTITIA_LOG_VERBOSE("Persistent Level: %s", *PersistentLevelPackage);
	GISubsystem->SavePersistentLevel(PersistentLevelPackage);

	
	TSharedPtr<FJsonObject> StreamingLevelsJSON = MakeShareable(new FJsonObject());
	
	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
	for (const ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		const FString StreamingLevelString = StreamingLevel->GetWorldAssetPackageFName().ToString();
		const FString StreamingLevelPackage = World->StripPIEPrefixFromPackageName(StreamingLevelString, StreamingLevelsPrefix);
		NOTITIA_LOG_VERBOSE("Streaming Level: %s, %s", *StreamingLevelPackage, StreamingLevel->HasLoadedLevel() ? TEXT("Loaded") : TEXT("Unloaded"));
		// TODO save streaming level packages
		
		TSharedPtr<FJsonObject> StreamingLevelJSON = MakeShareable(new FJsonObject());
		StreamingLevelsJSON->SetObjectField(StreamingLevelPackage, StreamingLevelJSON);
	};
	*/

	/*
	for(AActor* Actor : TActorRange<AActor>(GetWorld()))
	{
		FString LevelS = Actor->GetLevel()->GetPackage()->GetFName().ToString();
		NOTITIA_LOG_VERBOSE("Processing Actor: %s from %s", *Actor->GetName(), *LevelS);
		
		GISubsystem->SaveObject(Actor);
	}
	*/
}

void UNotitiaWorldSubsystem::SavePersistentLevel(TSharedPtr<FJsonObject> SaveJSON, UWorld* World) const
{
	TSharedPtr<FJsonObject> PersistentLevelJSON = MakeShareable(new FJsonObject());
	SaveJSON->SetObjectField("PersistentLevel", PersistentLevelJSON);
	
	FString StreamingLevelsPrefix = World->StreamingLevelsPrefix;
	
	ULevel* PersistentLevel = World->PersistentLevel;
	FString PersistentLevelString = PersistentLevel->GetPackage()->GetFName().ToString();
	FString PersistentLevelPackage = World->StripPIEPrefixFromPackageName(PersistentLevelString, StreamingLevelsPrefix);

	// TODO: Figure out what other properties of persistent level should be saved

	TArray<AActor*>& Actors = PersistentLevel->Actors;

	TSharedPtr<FJsonObject> ObjectsJSON = MakeShareable(new FJsonObject());
	PersistentLevelJSON->SetObjectField("Objects", ObjectsJSON);
	
	for (AActor* Actor : Actors)
	{
		if (!IsValid(Actor))
		{
			NOTITIA_LOG_VERBOSE("Skipping null actor!");
			continue;
		}

		TSharedPtr<FJsonObject> ActorJSON = MakeShareable(new FJsonObject());
		FString ActorName;
		
		if (FNotitiaObjectSerializer::SerializeObject(Actor, ObjectsJSON))
		{
			if (ActorJSON->Values.Num() > 0)
			{
				ObjectsJSON->SetObjectField(ActorName, ActorJSON);
			}
		}
	}

	// TODO: Process the persistent level and determine if any loaded actors or objects were destroyed and save this fact somehow
}

void UNotitiaWorldSubsystem::SaveStreamingLevels(TSharedPtr<FJsonObject> SaveJSON, UWorld* World) const
{
	TSharedPtr<FJsonObject> StreamingLevelsJSON = MakeShareable(new FJsonObject());
	SaveJSON->SetObjectField("StreamingLevels", StreamingLevelsJSON);

	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();

	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		SaveStreamingLevel(StreamingLevelsJSON, World, StreamingLevel);
	}
}

void UNotitiaWorldSubsystem::SaveStreamingLevel(TSharedPtr<FJsonObject> StreamingLevelsJSON, UWorld* World, ULevelStreaming* StreamingLevel) const
{
	const FString StreamingLevelString = StreamingLevel->GetWorldAssetPackageFName().ToString();
	const FString StreamingLevelPackage = World->StripPIEPrefixFromPackageName(StreamingLevelString, World->StreamingLevelsPrefix);
	
	TSharedPtr<FJsonObject> StreamingLevelJSON = MakeShareable(new FJsonObject());
	StreamingLevelsJSON->SetObjectField(StreamingLevelPackage, StreamingLevelJSON);

	// TODO save level properties including whether it was loaded
	// TODO process actors
}

void UNotitiaWorldSubsystem::OnLoadBegin(TSharedPtr<FJsonObject> LoadJSON) const
{
	TSharedPtr<FJsonObject> PersistentLevelJSON = LoadJSON->GetObjectField(TEXT("PersistentLevel"));

	TSharedPtr<FJsonObject> StreamingLevelsJSON = LoadJSON->GetObjectField(TEXT("StreamingLevels"));

	LoadPersistentLevel(PersistentLevelJSON);
}

void UNotitiaWorldSubsystem::LoadPersistentLevel(TSharedPtr<FJsonObject> LoadJSON) const
{
	
}

bool UNotitiaWorldSubsystem::ActorIsDynamic(const AActor* Actor) const
{
	return !LevelActors.Contains(Actor);
}

void UNotitiaWorldSubsystem::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	TArray<UClass*> ClassBlackList { AWorldSettings::StaticClass(), ALevelScriptActor::StaticClass() };
	TArray<AActor*> ActorBlacklist { World->GetDefaultBrush() };

	for (AActor* Actor : TActorRange<AActor>(World))
	{
		if (ActorBlacklist.Contains(Actor))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped blacklisted actor %s"), *Actor->GetName(), *Actor->GetClass()->GetName());
			continue;
		}
		
		if (ClassBlackList.ContainsByPredicate([=] (UClass* BlacklistClass) { return Actor->GetClass()->IsChildOf(BlacklistClass); }))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped class-blacklisted actor %s of class %s"), *Actor->GetName(), *Actor->GetClass()->GetName());
			continue;
		}
		
		LevelActors.Add(Actor);

		UE_LOG(LogTemp, Warning, TEXT("Added %s with class %s"), *Actor->GetName(), *Actor->GetClass()->GetName());
	}
}
