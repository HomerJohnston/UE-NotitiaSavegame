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
#include "GameFramework/Character.h"
#include "Notitia/Serialization/NotitiaObjectDeserializer.h"


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
	TWeakObjectPtr<UGameInstance> GameInstance = UNotitiaGISubsystem::GetStaticGameInstance();

	UNotitiaGISubsystem* GISubsystem = GameInstance->GetSubsystem<UNotitiaGISubsystem>();
	
	OnSaveBeginHandle = GISubsystem->OnSaveBegin.AddUObject(this, &UNotitiaWorldSubsystem::OnSaveBegin);
	OnLoadBeginHandle = GISubsystem->OnLoadBegin.AddUObject(this, &UNotitiaWorldSubsystem::OnLoadBegin);

	GISubsystem->SetWorldSubsystem(this);
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

	PersistentLevelJSON->SetStringField("PackageName", PersistentLevelPackage);
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

	TMap<FName, UObject*> LevelObjects;
	
	// TODO implement logic for this 
	bool bLevelReloaded = false;

	if (bLevelReloaded)
	{
		LevelObjects = InitialLevelObjects;
	}
	else
	{
		for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr)
		{
			if (ObjectItr->GetWorld() != GetWorld())
			{
				continue;
			}

			UObject* Object = *ObjectItr;

			FName CandidatePathName(GetWorld()->StripPIEPrefixFromPackageName(Object->GetPathName(), GetWorld()->StreamingLevelsPrefix));
			LevelObjects.Add(CandidatePathName, Object);
		}	
	}
	
	LoadPersistentLevel(PersistentLevelJSON, LevelObjects);
}

void UNotitiaWorldSubsystem::OnLevelChanged(ULevel* Level, UWorld* World) const
{
}

void UNotitiaWorldSubsystem::GetLevelObjectIDs(TMap<int32, UObject*> Map) const
{
}

void UNotitiaWorldSubsystem::LoadPersistentLevel(TSharedPtr<FJsonObject> LoadJSON, TMap<FName, UObject*>& LevelObjects) const
{
	TMap<FName, int32> LevelObjectIDs;

	// Iterate through all saved objects. Check if they exist in the level. If they do, make appropriate assignments. If they don't, spawn them.
	TSharedPtr<FJsonObject> ObjectsJSON = LoadJSON->GetObjectField("Objects");

	if (ObjectsJSON == nullptr)
	{
		NOTITIA_LOG_WARNING("Persistent level load failed, no objects!");
		return;
	}

	TArray<AActor*> ActorsToUpdate;

	FVector Test;
	
	for (TPair<FString, TSharedPtr<FJsonValue>>& Pair : ObjectsJSON->Values)
	{
		FString ObjectName = Pair.Key;
		TSharedPtr<FJsonValue> ObjectData = Pair.Value;
		const TSharedPtr<FJsonObject>* JSONObjectDataPtr;

		NOTITIA_LOG_WARNING("Loading object: %s", *ObjectName);
		
		if (ObjectData->TryGetObject(JSONObjectDataPtr))
		{
			TSharedPtr<FJsonObject> JSONObjectData = *JSONObjectDataPtr;
			FString ClassName = JSONObjectData->GetStringField("Class");
			int32 NotitiaId = JSONObjectData->GetNumberField("NotitiaId");

			const TSharedPtr<FJsonObject>* JSONPropertiesDataPtr = nullptr;
			
			if (JSONObjectData->TryGetObjectField("Properties", JSONPropertiesDataPtr))
			{
				/*for (TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*JSONPropertiesDataPtr)->Values)
				{
					FString PropertyName = Pair.Key;
					TSharedPtr<FJsonValue> PropertyData = Pair.Value;
				}*/
			}
			
			FName ObjectPath(ObjectName);
			
			UObject* ActualObject = nullptr;
			UObject** ActualObjectPtr = LevelObjects.Find(ObjectPath);

			if (LevelObjects.Contains(ObjectPath))
			{
				ActualObject = *ActualObjectPtr;
				LevelObjects.Remove(ObjectPath);

				NOTITIA_LOG_VERBOSE("Associating existing UObject: [%s] with Notitia ID [%i]", *ActualObject->GetName(), NotitiaId);
			}
			else
			{
				// TODO spawn a new object for it
				//NOTITIA_LOG_VERBOSE("No match found for: %s [%i]", *ObjectName, NotitiaId);
			}
			
			if (ActualObject && JSONPropertiesDataPtr)
			{
				TSharedPtr<FJsonObject> JSONPropertiesData = *JSONPropertiesDataPtr;

				FString ClassNameClean = ClassName;
				ClassNameClean.RemoveFromEnd(TEXT("_C"));
				// TODO do I need this???
				//ClassNameClean.RemoveFromStart("SKEL_");

				if (USceneComponent* ObjectAsSceneComp = Cast<USceneComponent>(ActualObject))
				{
				//	NOTITIA_LOG_WARNING("%s Location Before: %s", *ActualObject->GetName(), *ObjectAsSceneComp->GetRelativeLocation().ToString());
					//ObjectAsSceneComp->SetWorldLocation(ObjectAsSceneComp->GetComponentLocation());
					//ObjectAsSceneComp->MarkRenderTransformDirty();
				}
				
				// TODO can I cache class find results, do I need to?
				if (UClass* Class = FindObjectSafe<UClass>(ANY_PACKAGE, *ClassNameClean, true))
				{
					FNotitiaObjectDeserializer::JsonObjectToUStruct(JSONPropertiesData.ToSharedRef(), Class, ActualObject);
				}
				else if (UBlueprint* Blueprint = FindObjectSafe<UBlueprint>(ANY_PACKAGE, *ClassNameClean, true))
				{
					FNotitiaObjectDeserializer::JsonObjectToUStruct(JSONPropertiesData.ToSharedRef(), Blueprint->GetBlueprintClass(), ActualObject);					
				}
				else if (UScriptStruct* Struct = FindObjectSafe<UScriptStruct>(ANY_PACKAGE, *ClassNameClean, true))
				{
					FNotitiaObjectDeserializer::JsonObjectToUStruct(JSONPropertiesData.ToSharedRef(), Struct, ActualObject);
				}
				else
				{
					NOTITIA_LOG_WARNING("FAILED TO DESERIALIZE OBJECT %s", *ActualObject->GetName());
				}

				
				if (APawn* ObjectAsActor = Cast<APawn>(ActualObject))
				{
					ActorsToUpdate.Add(ObjectAsActor);
				}
				else if (USceneComponent* ObjectAsSceneComp = Cast<USceneComponent>(ActualObject))
				{
					//ObjectAsSceneComp->SetWorldLocation(ObjectAsSceneComp->GetComponentLocation());
					//ObjectAsSceneComp->MarkRenderTransformDirty();
				}
				
				if (USceneComponent* ObjectAsSceneComp = Cast<USceneComponent>(ActualObject))
				{
					//ObjectAsSceneComp->SetWorldLocation(ObjectAsSceneComp->GetComponentLocation());
					//ObjectAsSceneComp->MarkRenderTransformDirty();
					AActor* Owner = ObjectAsSceneComp->GetOwner();
					if (Owner && Owner->IsA(APawn::StaticClass()))
					{
						if (Owner->GetRootComponent() == ObjectAsSceneComp)
						{
							//Test = ObjectAsSceneComp->GetRelativeLocation();
							//NOTITIA_LOG_WARNING("%s GetRelativeLocation: %s", *ActualObject->GetName(), *ObjectAsSceneComp->GetRelativeLocation().ToString());
							//Owner->SetActorLocation(ObjectAsSceneComp->GetRelativeLocation(), false, nullptr, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
	}

	for (AActor* Actor : ActorsToUpdate)
	{

		//NOTITIA_LOG_WARNING("%s Setting location to: %s", *Actor->GetName(), *Actor->GetRootComponent()->GetRelativeLocation().ToString());
		//NOTITIA_LOG_WARNING("%s Setting transform to: %s", *Actor->GetName(), *T.ToString());

		
		USceneComponent* SC = Actor->GetRootComponent();
		FVector V1 = SC->GetRelativeLocation();
		FTransform T1 = SC->GetComponentTransform();

		SC->SetRelativeLocationAndRotation(SC->GetRelativeLocation(), SC->GetRelativeRotation());
		FVector V2 = SC->GetRelativeLocation();
		FTransform T2 = SC->GetComponentTransform();
		
		SC->SetWorldLocationAndRotationNoPhysics(SC->GetRelativeLocation(), SC->GetRelativeRotation());
		FVector V3 = SC->GetRelativeLocation();
		FTransform T3 = SC->GetComponentTransform();

		NOTITIA_LOG_VERBOSE("Actual Location:\n%s\n%s\n%s\nActual Transform:\n%s\n%s\n%s", *V1.ToString(), *V2.ToString(), *V3.ToString(), *T1.ToString(), *T2.ToString(), *T3.ToString());
		
		//Actor->SetActorTransform(SC->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);		
	}
}

/*
UScriptStruct* UNotitiaWorldSubsystem::FindStructFromName_AnyThread(FName TypeName)
{
	//Use previously loaded one if part of the cache
	if (TypeCache.Contains(TypeName))
	{
		return TypeCache[TypeName];
	}

	const bool bExactClass = true;
	UScriptStruct* Struct = FindObjectSafe<UScriptStruct>(ANY_PACKAGE, *TypeName.ToString(), bExactClass);
	if (Struct)
	{
		TypeCache.FindOrAdd(TypeName) = Struct;
	}

	return Struct;
}*/

void UNotitiaWorldSubsystem::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	// TODO implement option for full level reload
	if (false) // bFullReload
	{
		StashLevelObjects(World);
	}
}

void UNotitiaWorldSubsystem::StashLevelObjects(UWorld* World)
{
	TArray<UClass*> ClassBlackList { AWorldSettings::StaticClass(), ALevelScriptActor::StaticClass() };
	TArray<AActor*> ActorBlacklist { World->GetDefaultBrush() };

	for (TObjectIterator<UObject> ObjectItr; ObjectItr; ++ObjectItr)
	{
		if (ObjectItr->GetWorld() != World)
		{
			continue;
		}

		UObject* Object = *ObjectItr;

		if (AActor* ObjectAsActor = Cast<AActor>(Object))
		{
			if (ActorBlacklist.Contains(ObjectAsActor))
			{
				continue;
			}
		}

		if (ClassBlackList.ContainsByPredicate([=] (UClass* BlacklistClass) { return Object->GetClass()->IsChildOf(BlacklistClass); }))
		{
			continue;
		}

		FString Path = World->StripPIEPrefixFromPackageName(Object->GetPathName(), World->StreamingLevelsPrefix);
		InitialLevelObjects.Add(FName(Path), Object);
	}
	
	/*
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
	}*/
}
