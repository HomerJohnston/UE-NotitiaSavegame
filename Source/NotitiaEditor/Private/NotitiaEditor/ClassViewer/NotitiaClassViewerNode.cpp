#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNode.h"

#include "Notitia/Core/NotitiaDeveloperSettings.h"
#include "ClassViewer/Private/UnloadedBlueprintData.h"

#include "NotitiaEditor/ClassViewer/NotitiaClassViewerNameTypeToDisplay.h"
#include "NotitiaEditor/ClassViewer/NotitiaUnloadedBlueprintDataInterface.h"

FNotitiaClassViewerNode::FNotitiaClassViewerNode(UClass* InClass)
{
	// TODO error handling null input class
	SetClass(InClass);

	ClassName = MakeShareable(new FString(Class->GetName()));
	ClassDisplayName = MakeShareable(new FString(Class->GetDisplayNameText().ToString()));
	ClassPath = FName(*Class->GetPathName());

	if (Class->GetSuperClass())
	{
		ParentClassPath = FName(*Class->GetSuperClass()->GetPathName());
	}

	if (Class->ClassGeneratedBy && Class->ClassGeneratedBy->IsA(UBlueprint::StaticClass()))
	{
		Blueprint = Cast<UBlueprint>(Class->ClassGeneratedBy);
	}
	else
	{
		Blueprint = nullptr;
	}
	
	bPassesFilter = false;
	bPassesFilterRegardlessTextFilter = false;
}

FNotitiaClassViewerNode::FNotitiaClassViewerNode(const FString& InClassName, const FString& InClassDisplayName)
{
	ClassName = MakeShareable(new FString(InClassName));
	ClassDisplayName = MakeShareable(new FString(InClassDisplayName));

	bPassesFilter = false;
	bPassesFilterRegardlessTextFilter = false;

	// TODO ugly ugly ugly ugly. Basically this constructor is only *normally* called when creating this node for an unloaded BP.
	ClassFName = FName(*(InClassName + "_C"));
	
	SetClass(nullptr);
	Blueprint = nullptr;
}

FNotitiaClassViewerNode::FNotitiaClassViewerNode(const FNotitiaClassViewerNode& InCopyObject)
{
	ClassFName = InCopyObject.ClassFName;
	ClassName = InCopyObject.ClassName;
	ClassDisplayName = InCopyObject.ClassDisplayName;
	bPassesFilter = InCopyObject.bPassesFilter;
	bPassesFilterRegardlessTextFilter = InCopyObject.bPassesFilterRegardlessTextFilter;

	SetClass(InCopyObject.GetClassWeakPtr().Get());
	Blueprint = InCopyObject.Blueprint;

	UnloadedBlueprintData = InCopyObject.UnloadedBlueprintData;

	ClassPath = InCopyObject.ClassPath;
	ParentClassPath = InCopyObject.ParentClassPath;
	BlueprintAssetPath = InCopyObject.BlueprintAssetPath;

	NumProperties = InCopyObject.NumProperties;
}

TWeakObjectPtr<UClass> FNotitiaClassViewerNode::GetClassWeakPtr() const
{
	return Class;
}

void FNotitiaClassViewerNode::SetClass(UClass* NewClass)
{
	Class = NewClass;

	NumProperties = 0;

	if (NewClass)
	{
		ClassFName = Class->GetFName();
		
		for (TFieldIterator<FProperty> PropIt(Class.Get()); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			NumProperties += GetPropertyCount(Property);
		}
	}

	MarkDirty();
}

void FNotitiaClassViewerNode::AddChild(TSharedPtr<FNotitiaClassViewerNode> Child)
{
	check(Child.IsValid());

	Child->ParentNode = AsShared();
	ChildrenList.Add(Child);
}

void FNotitiaClassViewerNode::AddUniqueChild(TSharedPtr<FNotitiaClassViewerNode> NewChild)
{
	check(NewChild.IsValid());

	const UClass* NewChildClass = NewChild->Class.Get();

	if (NewChildClass)
	{
		for (int32 ChildIndex = 0; ChildIndex < ChildrenList.Num(); ++ChildIndex)
		{
			TSharedPtr<FNotitiaClassViewerNode> OldChild = ChildrenList[ChildIndex];

			if (OldChild.IsValid() && OldChild->Class == NewChildClass)
			{
				const bool bNewChildHasMoreInfo = NewChild->UnloadedBlueprintData.IsValid();
				const bool bOldChildHasMoreInfo = OldChild->UnloadedBlueprintData.IsValid();

				if (bNewChildHasMoreInfo && !bOldChildHasMoreInfo)
				{
					for (int OldChildIndex = 0; OldChildIndex < OldChild->ChildrenList.Num(); ++OldChildIndex)
					{
						NewChild->AddUniqueChild(OldChild->ChildrenList[OldChildIndex]);
					}

					ChildrenList[ChildIndex] = NewChild;
				}
				
				return;
			}
		}
	}

	AddChild(NewChild);
}

TArray<TSharedPtr<FNotitiaClassViewerNode>>& FNotitiaClassViewerNode::GetChildrenList()
{
	return ChildrenList;
}

TSharedPtr<FString> FNotitiaClassViewerNode::GetClassName(bool bUseDisplayName) const
{
	return bUseDisplayName ? ClassDisplayName : ClassName;
}

TSharedPtr<FString> FNotitiaClassViewerNode::GetClassName(ENotitiaClassViewerNameTypeToDisplay NameType) const
{
	switch (NameType)
	{
		case ENotitiaClassViewerNameTypeToDisplay::ClassName:
		{
			return ClassName;
		}
		case ENotitiaClassViewerNameTypeToDisplay::DisplayName:
		{
			return ClassDisplayName;
		}
		case ENotitiaClassViewerNameTypeToDisplay::Dynamic:
		{
			FString CombinedName;
			FString SanitizedName = FName::NameToDisplayString(*ClassName.Get(), false);

			if (ClassDisplayName.IsValid() && !ClassDisplayName->IsEmpty() && !ClassDisplayName->Equals(SanitizedName) && !ClassDisplayName->Equals(*ClassName.Get()))
			{
				TArray<FStringFormatArg> Args;

				Args.Add(*ClassName.Get());
				Args.Add(*ClassDisplayName.Get());

				CombinedName = FString::Format(TEXT("{0} ({1})"), Args);
			}
			else
			{
				CombinedName = *ClassName.Get();
			}

			return MakeShareable(new FString(CombinedName));
		}
	}

	ensureMsgf(false, TEXT("FNotitiaClassSelectNode::GetClassName called with invalid name type."));
	return ClassName;
}

FName FNotitiaClassViewerNode::GetClassFName() const
{
	return ClassFName;
}

int32 FNotitiaClassViewerNode::GetNumProperties() const
{
	return NumProperties;
}

int32 FNotitiaClassViewerNode::GetNumSelectedProperties() const
{
	const UNotitiaDeveloperSettings* DeveloperSettings = GetDefault<UNotitiaDeveloperSettings>();

	return DeveloperSettings->GetNumSelectedProperties(Class.Get());
}

bool FNotitiaClassViewerNode::IsClassPlaceable() const
{
	const UClass* LoadedClass = Class.Get();

	if (LoadedClass)
	{
		const bool bPlaceableFlags = !LoadedClass->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable);
		const bool bBasedOnActor = LoadedClass->IsChildOf(AActor::StaticClass());
		const bool bNotABrush = !LoadedClass->IsChildOf(ABrush::StaticClass());

		return bPlaceableFlags && bBasedOnActor && bNotABrush;
	}

	if (UnloadedBlueprintData.IsValid())
	{
		const bool bPlaceableFlags = !UnloadedBlueprintData->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable);
		const bool bBasedOnActor = UnloadedBlueprintData->IsChildOf(AActor::StaticClass());
		const bool bNotABrush = !UnloadedBlueprintData->IsChildOf(ABrush::StaticClass());

		return bPlaceableFlags && bBasedOnActor && bNotABrush;
	}

	return false;
}

bool FNotitiaClassViewerNode::IsBlueprintClass() const
{
	return BlueprintAssetPath != NAME_None;
}

bool FNotitiaClassViewerNode::IsEditorOnlyClass() const
{
	return Class.IsValid() && IsEditorOnlyObject(Class.Get());
}

bool FNotitiaClassViewerNode::IsRestricted() const
{
	return PropertyHandle.IsValid() && PropertyHandle->IsRestricted(*ClassName);
}

void FNotitiaClassViewerNode::MarkDirty()
{
	bIsDirty = true;

	for (TSharedPtr<FNotitiaClassViewerNode> Node : ChildrenList)
	{
		Node->MarkDirty();
	}
}

bool FNotitiaClassViewerNode::CleanDirty()
{
	bool bWasDirty = bIsDirty;

	bIsDirty = false;

	return bWasDirty;
}

int32 FNotitiaClassViewerNode::GetPropertyCount(FProperty* Property)
{
	/*
	FStructProperty* StructProperty = CastField<FStructProperty>(Property);
	
	if (StructProperty)
	{
		int32 Count = 0;

		for (TFieldIterator<FProperty> PropIt(StructProperty->Struct); PropIt; ++PropIt)
		{
			FProperty* ChildProperty = *PropIt;
			Count += GetPropertyCount(ChildProperty);
		}

		return Count;
	}

	return 1;
	*/
	int i = 0;
	if (Class->GetName().Contains("OmgActor"))
	{
		i = 1;
	}
	
	if (!Property)
	{
		return 0;
	}
	
	UClass* OwnerClass = Property->GetOwnerClass();

	if (OwnerClass && OwnerClass != Class)
	{
		return 0;
	}
	else
	{
		FProperty* ParentProp = Property->GetOwnerProperty();
		OwnerClass = ParentProp->GetOwnerClass();

		if (OwnerClass && OwnerClass != Class)
		{
			return 0;
		}
	}
	
	FStructProperty* StructProperty = CastField<FStructProperty>(Property);

	if (StructProperty)
	{
		int32 Count = 0;

		for (TFieldIterator<FProperty> PropIt(StructProperty->Struct); PropIt; ++PropIt)
		{
			FProperty* ChildProperty = *PropIt;
			Count += GetPropertyCount(ChildProperty);
		}

		return Count;
	}
	
	return 1;
}
