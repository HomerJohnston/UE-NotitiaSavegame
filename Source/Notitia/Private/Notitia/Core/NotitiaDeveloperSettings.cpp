#include "Notitia/Core/NotitiaDeveloperSettings.h"




/*
bool FNotitiaContainerPath::operator==(const FNotitiaContainerPath& Other) const
{
	if (Container.Num() != Other.Container.Num())
	{
		return false;
	}

	for (int i = 0; i < Container.Num(); ++i)
	{
		if (Container[i] != Other.Container[i])
		{
			return false;
		}
	}

	return true;
}

bool FNotitiaContainerPath::Equals(const FNotitiaContainerPath& Other) const
{
	if (Container.Num() != Other.Container.Num())
	{
		return false;
	}

	for (int i = 0; i < Container.Num(); ++i)
	{
		if (Container[i] != Other.Container[i])
		{
			return false;
		}
	}

	return true;
}

FString FNotitiaContainerPath::GetAsString() const
{
	FString S = "";

	for (FName Name : Container)
	{
		S += Name.ToString();
	}

	return S;
}

ECheckBoxState FNotitiaStructSerializationData::GetPropertyState(const FName PropertyName) const
{
	if (Properties.Contains(PropertyName))
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Unchecked;
}

void FNotitiaStructSerializationData::AddProperty(const FName PropertyName)
{
	Properties.AddUnique(PropertyName);
}

void FNotitiaStructSerializationData::RemoveProperty(const FName PropertyName)
{
	Properties.Remove(PropertyName);
}

FNotitiaContainerPath::FNotitiaContainerPath()
{
}

FNotitiaContainerPath::FNotitiaContainerPath(TArray<FName> InContainerPath)
{
	Container = InContainerPath;
}

uint32 GetTypeHash(const FNotitiaContainerPath& Path)
{
	uint32 Hash = 0;

	for (int i = 0; i < Path.Container.Num(); ++i)
	{
		Hash = HashCombine(Hash, GetTypeHash(Path.Container[i]));
	}

	return Hash;
}
*/

// ================================================================================================

FNotitiaProperty::FNotitiaProperty()
{
}

FNotitiaProperty::FNotitiaProperty(const TArray<FName>& InPath)
{
	Path = InPath;
}

bool FNotitiaProperty::operator<(const FNotitiaProperty& Other) const
{
	for (int i = 0; i < Path.Num(); ++i)
	{
		if (!Other.Path.IsValidIndex(i))
		{
			return false;
		}

		if (Path[i] == Other.Path[i])
		{
			continue;
		}
		
		return Path[i].ToString() < Other.Path[i].ToString();
	}

	return true;
}

bool FNotitiaProperty::operator==(const FNotitiaProperty& Other) const
{
	// TODO can I just reuse the other function
	if (Path.Num() != Other.Path.Num())
	{
		return false;
	}

	for (int i = 0; i < Path.Num(); ++i)
	{
		if (Path[i] != Other.Path[i])
		{
			return false;
		}
	}

	return true;
}

bool FNotitiaProperty::Equals(const FNotitiaProperty& Other) const
{
	if (Path.Num() != Other.Path.Num())
	{
		return false;
	}

	for (int i = 0; i < Path.Num(); ++i)
	{
		if (Path[i] != Other.Path[i])
		{
			return false;
		}
	}

	return true;
}

uint32 GetTypeHash(const FNotitiaProperty& Property)
{
	uint32 Hash = 0;

	for (int i = 0; i < Property.Path.Num(); ++i)
	{
		Hash = HashCombine(Hash, GetTypeHash(Property.Path[i]));
	}

	return Hash;
}

// ================================================================================================

FNotitiaClassSerializationData::FNotitiaClassSerializationData()
{
	Class = nullptr;
}
// ------------------------------------------------------------------------------------------------
FNotitiaClassSerializationData::FNotitiaClassSerializationData(const UClass* InClass)
{
	Class = InClass;
}
// ------------------------------------------------------------------------------------------------
void FNotitiaClassSerializationData::AddProperty(const FNotitiaProperty& PropertyPath)
{
	Properties.AddUnique(PropertyPath);
	Properties.Sort();
}
// ------------------------------------------------------------------------------------------------
void FNotitiaClassSerializationData::RemoveProperty(const FNotitiaProperty& PropertyPath)
{
	Properties.Remove(PropertyPath);
}
// ------------------------------------------------------------------------------------------------
ECheckBoxState FNotitiaClassSerializationData::GetPropertyState(const FNotitiaProperty& PropertyPath) const
{
	if (Properties.Contains(PropertyPath))
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Unchecked;
}
// ------------------------------------------------------------------------------------------------
int32 FNotitiaClassSerializationData::GetSavedPropertyCount() const
{
	return Properties.Num();
}

// ================================================================================================

void UNotitiaDeveloperSettings::AddProperty(const UClass* Class, const TArray<FName>& PropertyPath)
{
	FNotitiaProperty Property(PropertyPath);
	
	FNotitiaClassSerializationData* ClassSerializationData = Classes.FindByPredicate
	(
		[&] (const FNotitiaClassSerializationData& Data)
		{
			return Data.Class == Class;
		}
	);

	if (ClassSerializationData == nullptr)
	{
		int NewIndex = Classes.Add(FNotitiaClassSerializationData(Class));
		ClassSerializationData = &Classes[NewIndex];
	}

	ClassSerializationData->AddProperty(Property);
}

void UNotitiaDeveloperSettings::RemoveProperty(const UClass* Class, const TArray<FName>& PropertyPath)
{
	FNotitiaProperty Property(PropertyPath);
	
	int32 IndexOfClassData = Classes.IndexOfByPredicate
	(
		[&] (const FNotitiaClassSerializationData& Data)
		{
			return Data.Class == Class;
		}
	);

	if (IndexOfClassData == INDEX_NONE)
	{
		return;
	}

	Classes[IndexOfClassData].RemoveProperty(Property);

	if (Classes[IndexOfClassData].Properties.Num() == 0)
	{
		Classes.RemoveAt(IndexOfClassData);
	}
}

ECheckBoxState UNotitiaDeveloperSettings::GetPropertyState(const UClass* Class, const TArray<FName>& PropertyPath) const
{
	FNotitiaProperty Property(PropertyPath);
	
	const FNotitiaClassSerializationData* ClassSerializationData = Classes.FindByPredicate
	(
		[&] (const FNotitiaClassSerializationData& Data)
		{
			return Data.Class == Class;
		}
	);
	
	if (!ClassSerializationData)
	{
		return ECheckBoxState::Unchecked;
	}

	return ClassSerializationData->GetPropertyState(Property);
}

bool UNotitiaDeveloperSettings::ContainsClass(const UClass* Class, bool& ExactMatch) const
{
	if (!Class)
	{
		return false;
	}

	bool bAnyMatch = false;
	ExactMatch = false;
	
	for (int i = 0; i < Classes.Num(); ++i)
	{
		UClass* AvailableClass = Classes[i].Class.Get();

		if (Class->IsChildOf(AvailableClass))
		{
			if (Class == AvailableClass)
			{
				ExactMatch = true;
				return true;
			}
			bAnyMatch = true;
		}
	}

	return bAnyMatch;
}

int32 UNotitiaDeveloperSettings::GetNumSelectedProperties(UClass* Class) const
{
	if (!Class)
	{
		return 0;
	}
	
	for (int i = 0; i < Classes.Num(); ++i)
	{
		if (Classes[i].Class == Class)
		{
			return Classes[i].GetSavedPropertyCount();
		}
	}

	return 0;
}

const TArray<FNotitiaClassSerializationData>& UNotitiaDeveloperSettings::GetClassesData() const
{
	return Classes;
}
