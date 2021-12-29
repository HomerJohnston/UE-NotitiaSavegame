#include "Notitia/Core/NotitiaGISubsystem.h"

#include "Notitia/Core/NotitiaDeveloperSettings.h"
#include "Notitia/Debug/NotitiaLogging.h"
#include "Algo/ForEach.h"
#include "Compression/lz4.h"
#include "UObject/PropertyIterator.h"

namespace Notitia_Private
{
	TAtomic<uint64> GNextID(1);
}

#define NOTITIA_NULL_IDENTIFIER 0

bool UNotitiaGISubsystem::bIsInitialized = false;
FOnInitializeEvent UNotitiaGISubsystem::OnInitialize;
FOnInitializeEvent UNotitiaGISubsystem::OnDeinitialize;

/*
bool UNotitiaGISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// RF_Transactional object flag indicates that we're in the editor (and not in PIE either)
	if (Outer->GetFlags() == RF_Transactional)
		return false;

	const UWorld* World = Outer->GetWorld();
	return World && (World->WorldType == EWorldType::Type::PIE || World->WorldType == EWorldType::Type::Game);
}*/

void UNotitiaGISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BuildPropertiesSet();

	OnInitialize.Broadcast();
	bIsInitialized = true;
}

void UNotitiaGISubsystem::Deinitialize()
{
	Super::Deinitialize();

	OnDeinitialize.Broadcast();
	bIsInitialized = false;
}

void UNotitiaGISubsystem::BuildPropertiesSet()
{
	const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();

	for (const FNotitiaClassSerializationData& ClassData : DeveloperSettings->GetClassesData())
	{
		// What I'm going to do now is iterate through all of the properties of this class (ignoring the ones set to actually replicate)
		// I'll check if the developersettings is returning them as "checked" - i.e. for a struct, if all the subproperties are checked, the struct itself is checked
		// even though the struct itself has not actually been checked
		for (const FNotitiaProperty& Property : ClassData.Properties)
		{
			// TODO: this obviously crashes if I rename or reparent a class
			UClass* Class = ClassData.Class.Get();

			if (!Class)
			{
				continue;
			}
			
			FName PropName = GenerateFName(Class, Property.Path);
			
			SerializedPropertyPaths.Add(PropName);
		}
	}
}

void UNotitiaGISubsystem::Save(FString Filename)
{
	NOTITIA_LOG_VERBOSE("Running save...");

	// Notitia_Private::GNextID = 1;
	
	TSharedPtr<FJsonObject> SaveJSON = MakeShareable(new FJsonObject());

	OnSaveBegin.Broadcast(SaveJSON);

	FString OutputString;
	TSharedRef<TJsonWriter<>> StringWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(SaveJSON.ToSharedRef(), StringWriter);

	
	
	NOTITIA_LOG_WARNING("Generated SaveJSON:");
	//NOTITIA_LOG_VERBOSE("%s", *OutputString);

	if (Filename.IsEmpty())
	{
		Filename = "Savegame";
	}

	Filename = FPaths::Combine(FPaths::ProjectSavedDir(), Filename);
	
	TArray<uint8> Buffer;
	FMemoryWriter Writer(Buffer, true);
	Writer << OutputString;
	Writer.Close();

	//FFileHelper::SaveArrayToFile(Buffer, *Filename);
	FFileHelper::SaveStringToFile(OutputString, *Filename);
	
	/*
	int32 CompressBound = LZ4_compressBound(Buffer.Num());
	TArray<uint8> CompressedData;
	CompressedData.SetNumUninitialized(CompressBound);
	
	int32 NumBytes = LZ4_compress_default((const char*)&Buffer.GetData()[0], (char*)&CompressedData.GetData()[0], Buffer.Num() * sizeof(uint8), CompressBound);

	CompressedData.SetNum(NumBytes);
	
	FFileHelper::SaveArrayToFile(CompressedData, *Filename);*/
	
}

bool UNotitiaGISubsystem::SavePersistentLevel(const FString& PersistentLevelPackage)
{
	return true;
}

bool UNotitiaGISubsystem::IsPropertyMarkedForSerialization(FName PropertyPath)
{
	return SerializedPropertyPaths.Contains(PropertyPath);
}


bool UNotitiaGISubsystem::AssignIdForObject(UObject* Object)
{
	uint64* ID = ObjectGUIDMap.Find(Object);

	if (ID)
	{
		return false;
	}

	uint64 newID = GenerateUniqueID();
	ObjectGUIDMap.Add(Object, newID);
	ObjectGUIDReverseMap.Add(newID, Object);

	return true;
}

uint64 UNotitiaGISubsystem::GetIdForObject(UObject* Object)
{
	uint64* ID = ObjectGUIDMap.Find(Object);

	if (ID)
	{
		return *ID;
	}

	return NOTITIA_NULL_IDENTIFIER;
}

UObject* UNotitiaGISubsystem::GetObjectForID(uint64 Id)
{
	UObject** Result = ObjectGUIDReverseMap.Find(Id);

	if (Result)
	{
		return *Result;
	}
	
	return nullptr;
}

void UNotitiaGISubsystem::Load(FString Filename)
{
	NOTITIA_LOG_VERBOSE("Running save...");

	Filename = FPaths::Combine(FPaths::ProjectSavedDir(), Filename);

	FString InputString;
	FFileHelper::LoadFileToString(InputString, *Filename);
	
	TSharedPtr<FJsonObject> RootJsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InputString);

	if (FJsonSerializer::Deserialize(Reader, RootJsonObject))
	{
		OnLoadBegin.Broadcast(RootJsonObject);
	}
}

uint64 UNotitiaGISubsystem::GenerateUniqueID()
{
	// Check for the next-to-impossible event that we wrap round to 0, because we reserve 0.
	uint64 NewID = ++Notitia_Private::GNextID;

	if (NewID == NOTITIA_NULL_IDENTIFIER)
	{
		// Increment it again - it might not be zero, so don't just assign it to 1.
		NewID = ++Notitia_Private::GNextID;
	}
	
	return ++NewID;
}

FName UNotitiaGISubsystem::GenerateFName(const UClass* Class, const TArray<FName>& PropertyPath) const
{
	if (!Class)
	{
		return FName("NULL");
	}
	
	//TODO crash on null class / rename class
	const UObject* Blueprint = Class->ClassGeneratedBy;
	
	FString PropertyPathString = (Blueprint) ? Blueprint->GetFName().ToString() + "_C" : Class->GetFName().ToString();

	for (const FName& Name : PropertyPath)
	{
		PropertyPathString += "." + Name.ToString();
	}

	return FName(PropertyPathString);
}
